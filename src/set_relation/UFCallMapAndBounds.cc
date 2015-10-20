/*!
 * \file UFCallMapAndBounds.cc
 *
 * \brief Implementation of UFCallMapAndBounds class
 *
 * \date Started: 2013-07-30
 * # $Revision:: 789                $: last committed revision
 * # $Date:: 2013-08-20 12:01:52 -0#$: date of last committed revision
 * # $Author:: kreaseck             $: author of last committed revision
 *
 * \authors Michelle Strout
 *
 * Copyright (c) 2013, Colorado State University 
 * All rights reserved. 
 * See ../../COPYING for details. <br>
 */

#include "UFCallMapAndBounds.h"
#include "set_relation.h"
#include <util/util.h>
#include <iostream>

namespace iegenlib{

//! \param tdecl TupleDecl of the Set or Relation for which 
//!        we are collecting bounds.
UFCallMapAndBounds::UFCallMapAndBounds(TupleDecl tdecl) {
    mInitialArity = tdecl.size();
    
    // Our initial constraints are no constraints.
    // But need tuple variables for the initial tuple declaration.
    mCurrentConstraints = new Set(tdecl.size());
    mCurrentConstraints->setTupleDecl(tdecl);
    
    // Put NULL UFCallTerms for these initial tuple var locations.
    // These initial ones are NOT considered temporary variables.
    for (unsigned int i=0; i<tdecl.size(); i++) {
        mUFCallVector.push_back(NULL);
    }
}

UFCallMapAndBounds::~UFCallMapAndBounds() {
    delete mCurrentConstraints;
    for (unsigned int i=0; i<mUFCallVector.size(); i++) {
        if (mUFCallVector[i]) { delete mUFCallVector[i]; }
    }
    mUFCallVector.clear();
}

//! Returns the number of temporary variables.
unsigned int UFCallMapAndBounds::numTempVars() const {
    return mUFCallVector.size() - mInitialArity;
}


//! Returns UFCallTerm associated with given temporary var.
//! Temporary variable is indexed by its tuple location.
//! See the description of the UFCallMapAndBounds class for
//! how that works.
UFCallTerm* UFCallMapAndBounds::cloneUFCall( unsigned int varnum ) const {
    if (varnum >= mUFCallVector.size()) {
        throw assert_exception("variable number is out of bounds");
    }
    return new UFCallTerm(*(mUFCallVector[varnum]));
}

//! Introduces temporary variables as needed for each integer
//! returned by a function (could be more than one for a function
//! that returns a tuple).  Bounds those temporaries in
//! collected constraints.
TupleExpTerm* UFCallMapAndBounds::boundByRange( const UFCallTerm* uf_call ) {

    // look up range for uninterpreted function
    Set* range = iegenlib::queryRangeCurrEnv(uf_call->name());
    
    // Assuming that uf call and its range align.
    if (! uf_call->isIndexed() 
            && ((unsigned)range->arity() != uf_call->size()) ) {
        throw assert_exception("UFCallMapAndBounds::boundByRange: "
            "ufcall returning fewer dimensions than declared range");
    }

    // For each dimension of the return value create a temporary variable,
    // associate it with UFCall, create equalities between temporary
    // variable and return expression, and build up the tuple expression 
    // of temporary variables that will be passed to range->boundTupleExp(...)
    // and will be returned.
    TupleExpTerm tuple_exp(uf_call->size());
    
    // Determine what the output arity of this particular UF call is
    // taking into consideration that it could be indexed.
    unsigned int out_arity = range->arity();
    if (uf_call->isIndexed()) {
        out_arity = 1; // only one element in the output tuple is being accessed
    } 
    for (unsigned int i=0; i<out_arity; i++) {
    
        // Create a temporary variable and maintain correspondence 
        // with UFCallTerm.  Add one more tuple var to constraints.
        int tuple_var_loc_for_temp = mCurrentConstraints->arity();
        UFCallTerm* indexed_uf_call;
        if (out_arity==1) {
            indexed_uf_call = new UFCallTerm(*uf_call);
        } else {
            indexed_uf_call = new UFCallTerm(*uf_call);
            indexed_uf_call->setTupleIndex( i );
        }
        mUFCallVector.push_back(indexed_uf_call);
        TupleDecl cc_tdecl = mCurrentConstraints->getTupleDecl();
        mCurrentConstraints->setTupleDecl(cc_tdecl.concat(TupleDecl(1)));
        
        // Create a TupleExpTerm from the new temporary var.
        Exp* tuple_var_exp = new Exp();
        tuple_var_exp->addTerm(new TupleVarTerm(tuple_var_loc_for_temp));
        tuple_exp.setExpElem(i,tuple_var_exp);
                
    }
    
    // have the range create the constraints and store those constraints
    Set* constraintSet = range->boundTupleExp(tuple_exp);

    // The constraintSet returned by boundTupleExp
    // will not have any tuple variables.
    // Have constraintSet take on the same tuple declaration
    // as the mCurrentConstraints.  This way if any of the parameter
    // expressions had tuple variable terms from the original set
    // we are maintaining bounds for, the tuple variable declaration
    // will line up.
    constraintSet->setTupleDecl( mCurrentConstraints->getTupleDecl() );
    
    // Intersect the new set of constraints with the existing constraints.
    Set* mCC = mCurrentConstraints;     // pointer to current set
    mCurrentConstraints = mCC->Intersect(constraintSet);

    delete mCC;
    delete constraintSet;
    delete range;
    
    // return the set of tuple variables that were assigned to
    // the UF call results
    return new TupleExpTerm( tuple_exp );
}

//! Collects bounds parameters to the given UF call.
//! Will bound the parameters by the domain of the UF.
//! Takes ownership of passed in uf_call.
void UFCallMapAndBounds::boundByDomain(const UFCallTerm* uf_call) {

    // Create a TupleExpTerm from the parameter expressions.
    TupleExpTerm tuple_exp(uf_call->numArgs());
    for (unsigned int count=0; count<uf_call->numArgs(); count++) {
        tuple_exp.setExpElem(count, uf_call->getParamExp(count)->clone());
    }

    // look up bound for uninterpreted function
    Set* domain = iegenlib::queryDomainCurrEnv(uf_call->name());

    // have the domain create the constraints and store those constraints
    Set* constraintSet = domain->boundTupleExp(tuple_exp);

    // The constraintSet returned by boundTupleExp
    // will not have any tuple variables.
    // Have constraintSet take on the same tuple declaration
    // as the constraintSet.  This way if any of the parameter
    // expressions had tuple variable terms from the original set
    // we are maintaining bounds for, the tuple variable declaration
    // will line up.
    constraintSet->setTupleDecl( mCurrentConstraints->getTupleDecl() );
    
    // Intersect the new set of constraints with the existing constraints.
    Set* mCC = mCurrentConstraints;     // pointer to current set
    mCurrentConstraints = mCC->Intersect(constraintSet);
    
    delete mCC; // cleanup old guy
    delete constraintSet;
    delete domain;
}

//! returns a clone of the Set of maintained constraints.
Set* UFCallMapAndBounds::cloneConstraints() const {
    if (mCurrentConstraints!=NULL) {
        return new Set(*mCurrentConstraints);
    } else {
        throw assert_exception("UFCallMapAndBounds does not have a current "
                               "set of constraints");
    }
}

//! Returns a string representation of the class instance for debugging.
std::string UFCallMapAndBounds::toString() const {
    std::stringstream ss;
    for (unsigned int i=mInitialArity; i<mUFCallVector.size(); i++) {
            ss << "_tmp" << i << " = " << mUFCallVector[i]->toString() << std::endl;
    }
    
    return ss.str();
}

}  // end of namespace iegenlib
