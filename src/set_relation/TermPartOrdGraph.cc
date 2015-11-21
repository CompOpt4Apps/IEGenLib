/*!
 * \file TermPartOrdGraph.cc
 *
 * \brief Implementation of FactorToIntMap class
 *
 * \date Started: 2015-11-20
 *
 * \authors Michelle Strout
 *
 * Copyright (c) 2015, University of Arizona
 * All rights reserved. 
 * See ../../COPYING for details. <br>
 */

#include "TermPartOrdGraph.h"
#include <util/util.h>
#include <iostream>
#include <assert.h>

namespace iegenlib{

TermPartOrdGraph::TermPartOrdGraph() : mDoneInsertingTerms(false),
        mNumTerms(0), mGraphPtr(NULL) {}

//! Delete all of the terms and expressions we are storing.
TermPartOrdGraph::~TermPartOrdGraph() {
    std::set<Term*>::const_iterator iter;
    for (iter=mNonNegativeTerms.begin(); iter!=mNonNegativeTerms.end(); iter++){
        delete (*iter);
    }
}

//! Use this to insert a term.
//! TermPartOrdGraph takes ownership of expression.
//! Changes coeff to 1 and then makes sure the term is unique.
//! Does NOT take ownership of term.
void TermPartOrdGraph::insertTerm( const Term* t ) {
    Term* temp = t->clone();
    temp->setCoefficient(1);

    // Insert the term into the correct map.
    if (temp->type()=="UFCallTerm") {
        UFCallTerm* ufterm = dynamic_cast<UFCallTerm*>(temp);
        if (mUFCallTerm2IntMap.find(*ufterm)==mUFCallTerm2IntMap.end()) {   
            mUFCallTerm2IntMap[*ufterm] = mNumTerms++;
        }
    } else if (temp->type()=="TupleVarTerm") {
        TupleVarTerm* tvterm = dynamic_cast<TupleVarTerm*>(temp);
        if (mTupleVarTerm2IntMap.find(*tvterm)==mTupleVarTerm2IntMap.end()) {
            mTupleVarTerm2IntMap[*tvterm] = mNumTerms++;
        }
    } else if (temp->type()=="VarTerm") {
        VarTerm* vterm = dynamic_cast<VarTerm*>(temp);
        if (mVarTerm2IntMap.find(*vterm)==mVarTerm2IntMap.end()) {
            mVarTerm2IntMap[*vterm] = mNumTerms++;
        }
    } else {
        std::cerr << "TermPartOrdGraph::insertTerm: ERROR unhandled term type" 
                  << std::endl;
        assert(0); // FIXME: should be using assert_exceptions instead!!
    }
    
    delete temp;
}

//! Indicate the given term is non-negative.  Will pretend coeff 1.
//! Assumes that given term has already been inserted.
void TermPartOrdGraph::termNonNegative( const Term* t ) {
    Term* temp = t->clone();
    temp->setCoefficient(1);
    
    // Make sure the term has already been inserted into graph.
    
    // If so then insert into non-negative set if it is not
    // already in there.
    if (!isNonNegative(t)) {
        mNonNegativeTerms.insert(temp); // Will own temp so don't delete.
    } 
}

bool TermPartOrdGraph::isNonNegative( const Term* term ) const {
    Term* temp = term->clone();
    temp->setCoefficient(1);
    bool retval = false;

    // Search through the Set of non-negative terms and see
    // if it is in there.
    std::set<Term*>::const_iterator iter;
    for (iter=mNonNegativeTerms.begin(); iter!=mNonNegativeTerms.end(); iter++){
        Term* nonNegTerm = *iter;
        if (*nonNegTerm == *temp) {
            retval = true;
        }
    }
    
    delete temp;
    return retval;
}


//! Returns a set of all unique UFCallTerms that have been inserted.
//! Caller owns all of the return Terms.
std::set<UFCallTerm*> TermPartOrdGraph::getUniqueUFCallTerms() const {
    std::set<UFCallTerm*> retval;
    
    // Gather up all the UFCallTerms.
    std::map<UFCallTerm,int>::const_iterator iter;
    for (iter=mUFCallTerm2IntMap.begin(); 
            iter!=mUFCallTerm2IntMap.end(); iter++) {
        retval.insert(new UFCallTerm(iter->first));
    }
    return retval;
}

//! Returns a set of all unique terms that have been inserted.
std::set<Term*> TermPartOrdGraph::getAllUniqueTerms() const {
    std::set<Term*> retval;
        
    // Gather up all the UFCallTerms.
    {
        std::map<UFCallTerm,int>::const_iterator iter;
        for (iter=mUFCallTerm2IntMap.begin(); 
                iter!=mUFCallTerm2IntMap.end(); iter++) {
            retval.insert(new UFCallTerm(iter->first));
        }
    }
    
    // Gather up all the TupleVarTerms.
    {
        std::map<TupleVarTerm,int>::const_iterator iter;
        for (iter=mTupleVarTerm2IntMap.begin(); 
                iter!=mTupleVarTerm2IntMap.end(); iter++) {
            retval.insert(new TupleVarTerm(iter->first));
        }
    }
    
    // Gather up all the VarTerms.
    {
        std::map<VarTerm,int>::const_iterator iter;
        for (iter=mVarTerm2IntMap.begin(); 
                iter!=mVarTerm2IntMap.end(); iter++) {
            retval.insert(new VarTerm(iter->first));
        }
    }
    
    return retval;
}

// Want this to be templated.  How do I do that?
//std::string termMapToString( 
//        std::map<UFCallTerm,Int>::const_iterator iter;
//        for (iter=mUFCallTerm2IntMap.begin(); iter!=mTerm2ExpMap.end(); iter++) {
//            ss << "\tterm = " << iter->first->toString() 
//               << ", exp = " << iter->second->toString() << std::endl;
//    }

//! Returns a string representation of the class instance for debugging.
std::string TermPartOrdGraph::toString() const {
    std::stringstream ss;
    ss << "TermPartOrdGraph:" << std::endl;
    ss << "\tmDoneInsertingTerms = " << mDoneInsertingTerms << std::endl;
    ss << "\tmNumTerms = " << mNumTerms << std::endl;
    ss << "\tmNonNegativeTerms = " << std::endl;
    std::set<Term*>::const_iterator iter;
    for (iter=mNonNegativeTerms.begin(); iter!=mNonNegativeTerms.end(); iter++){
        ss << "\t\t" << (*iter)->toString() << std::endl;
    }
    if (mGraphPtr!=NULL) { ss << mGraphPtr->toString(); }
    return ss.str();
}
    

}
