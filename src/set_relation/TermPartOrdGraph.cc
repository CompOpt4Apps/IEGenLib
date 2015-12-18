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

TermPartOrdGraph::TermPartOrdGraph() : mNumTerms(0), mGraphPtr(NULL) {}

//! Delete all of the terms and expressions we are storing.
TermPartOrdGraph::~TermPartOrdGraph() {
    std::set<Term*>::const_iterator iter;
    for (iter=mNonNegativeTerms.begin(); iter!=mNonNegativeTerms.end(); iter++){
        delete (*iter);
    }
}

//! This should be only routine that uses RTTI to determine the term type.
//! Don't want to expose IDs in TermPartOrdGraph interface.
int TermPartOrdGraph::findOrInsertTermId(const Term* t) {
    int termId = mNumTerms; // Initial assumption is insertion.
    int initialNumTerms = mNumTerms;
    
    Term* temp = t->clone();
    temp->setCoefficient(1);

    // Insert the term into the correct map.
    if (temp->type()=="UFCallTerm") {
        UFCallTerm* ufterm = dynamic_cast<UFCallTerm*>(temp);
        if (mUFCallTerm2IntMap.find(*ufterm)==mUFCallTerm2IntMap.end()) {   
            mUFCallTerm2IntMap[*ufterm] = mNumTerms++;
        } else {
            termId = mUFCallTerm2IntMap[*ufterm];
        }
    } else if (temp->type()=="TupleVarTerm") {
        TupleVarTerm* tvterm = dynamic_cast<TupleVarTerm*>(temp);
        if (mTupleVarTerm2IntMap.find(*tvterm)==mTupleVarTerm2IntMap.end()) {
            mTupleVarTerm2IntMap[*tvterm] = mNumTerms++;
        } else {
            termId = mTupleVarTerm2IntMap[*tvterm];
        }
    } else if (temp->type()=="VarTerm") {
        VarTerm* vterm = dynamic_cast<VarTerm*>(temp);
        if (mVarTerm2IntMap.find(*vterm)==mVarTerm2IntMap.end()) {
            mVarTerm2IntMap[*vterm] = mNumTerms++;
        } else {
            termId = mVarTerm2IntMap[*vterm];
        }
    } else {
        std::cerr << "TermPartOrdGraph::findOrInsertTermId: "
                     "ERROR unhandled term type" << std::endl;
        assert(0); // FIXME: should be using assert_exceptions instead!!
    }
    
    delete temp;
    
    // Check that we didn't just do an insertion after doneInsertingItems call.
    assert( (isDoneInsertingTerms() and (initialNumTerms==mNumTerms))
            || !isDoneInsertingTerms());
    
    return termId;    
}

//! Use this to insert a term.
//! Changes coeff to 1 and then makes sure the term is unique.
//! Does NOT take ownership of term.
void TermPartOrdGraph::insertTerm( const Term* t ) {
    findOrInsertTermId(t);
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

//===============================================================
// Methods for creating partial orderings.

//! Once user is done inserting items we can create an instance
//! of the PartOrdGraph data structure.
void TermPartOrdGraph::doneInsertingTerms() {
    mGraphPtr = new PartOrdGraph(mNumTerms);
}

//! Once user is done inserting items we can create an instance
//! of the PartOrdGraph data structure.
bool TermPartOrdGraph::isDoneInsertingTerms() const {
    return mGraphPtr != NULL;
}

//! Term1 <= Term2
void TermPartOrdGraph::insertLTE( Term* term1, Term* term2 ) {
    assert(isDoneInsertingTerms());
    int term1Id = findOrInsertTermId(term1);
    int term2Id = findOrInsertTermId(term2);
    mGraphPtr->nonStrict(term1Id,term2Id);
}

//! Term1 < Term2
void TermPartOrdGraph::insertLT( Term* term1, Term* term2 ) {
    assert(isDoneInsertingTerms());
    int term1Id = findOrInsertTermId(term1);
    int term2Id = findOrInsertTermId(term2);
    mGraphPtr->strict(term1Id,term2Id);
}

//! Term1 == Term2
void TermPartOrdGraph::insertEqual( Term* term1, Term* term2 ) {
    assert(isDoneInsertingTerms());
    int term1Id = findOrInsertTermId(term1);
    int term2Id = findOrInsertTermId(term2);
    mGraphPtr->equal(term1Id,term2Id);
}


//===============================================================
// Query Methods

//! returns true if Term1 <= Term2
bool TermPartOrdGraph::isLTE( Term* term1, Term* term2 ) {
    assert(isDoneInsertingTerms());
    int term1Id = findOrInsertTermId(term1);
    int term2Id = findOrInsertTermId(term2);
    return mGraphPtr->isNonStrict(term1Id,term2Id);
}
//! returns true if Term1 < Term2
bool TermPartOrdGraph::isLT( Term* term1, Term* term2 ) {
    assert(isDoneInsertingTerms());
    int term1Id = findOrInsertTermId(term1);
    int term2Id = findOrInsertTermId(term2);
    return mGraphPtr->isStrict(term1Id,term2Id);
}
//! returns true if Term1 == Term2
bool TermPartOrdGraph::isEqual( Term* term1, Term* term2 ) {
    assert(isDoneInsertingTerms());
    int term1Id = findOrInsertTermId(term1);
    int term2Id = findOrInsertTermId(term2);
    return mGraphPtr->isEqual(term1Id,term2Id);
}


//! Templated helper routine to avoid repetitive code that
//! loops over the type specific term maps to grab terms.
//! FIXME: would still like to generalize this by passing 
//! in a lambda to operate on each map in the pair.  Possible?
//! Want to generalize with termMapToString used below.
template <typename SetElemType, typename TermType>
void termMapCopyTerms( const std::map<TermType,int>& termMap, 
                       std::set<SetElemType>& targetSet ) {
    typename std::map<TermType,int>::const_iterator iter;
    for (iter=termMap.begin(); iter!=termMap.end(); iter++) {
        targetSet.insert(new TermType(iter->first));
    }
}

//! Returns a set of all unique UFCallTerms that have been inserted.
//! Caller owns all of the return Terms.
std::set<UFCallTerm*> TermPartOrdGraph::getUniqueUFCallTerms() const {
    std::set<UFCallTerm*> uniqueUFCallTerms;
    termMapCopyTerms<UFCallTerm*,UFCallTerm>(mUFCallTerm2IntMap,
                                             uniqueUFCallTerms);
    return uniqueUFCallTerms;
}

//! Returns a set of all unique terms that have been inserted.
std::set<Term*> TermPartOrdGraph::getAllUniqueTerms() const {
    std::set<Term*> uniqueTerms;
    termMapCopyTerms<Term*,UFCallTerm>(mUFCallTerm2IntMap, uniqueTerms);
    termMapCopyTerms<Term*,TupleVarTerm>(mTupleVarTerm2IntMap, uniqueTerms);
    termMapCopyTerms<Term*,VarTerm>(mVarTerm2IntMap, uniqueTerms);
    return uniqueTerms;
}

//! Templated helper routine to avoid repetitive code that
//! loops over the type specific term maps to generate a
//! string.
//! FIXME: would still like to generalize this by templating
//! the return value and passing in a lambda to operate on
//! each map in the pair.  Possible?
template <typename TermType>
std::string termMapToString( const std::map<TermType,int>& termMap ) {
    std::stringstream ss;
    typename std::map<TermType,int>::const_iterator iter;
    for (iter=termMap.begin(); iter!=termMap.end(); iter++) {
        ss << "\t\tterm = " << iter->first.toString() 
           << ", id = "     << iter->second << std::endl;
    }
    return ss.str();
}

//! Returns a string representation of the class instance for debugging.
std::string TermPartOrdGraph::toString() const {
    std::stringstream ss;
    ss << "TermPartOrdGraph:" << std::endl;
    
    ss << "\tDoneInsertingTerms = " << isDoneInsertingTerms() << std::endl;
    
    ss << "\tNumTerms = " << mNumTerms << std::endl;
    
    ss << "\tNonNegativeTerms = " << std::endl;
    std::set<Term*>::const_iterator iter;
    for (iter=mNonNegativeTerms.begin(); iter!=mNonNegativeTerms.end(); iter++){
        ss << "\t\t" << (*iter)->toString() << std::endl;
    }
    
    // Output mapping of terms to integer ids.
    ss << "\tUFCallTerm2IntMap = " << std::endl;
    ss << termMapToString<UFCallTerm>(mUFCallTerm2IntMap);
    ss << "\tTupleVarTerm2IntMap = " << std::endl;
    ss << termMapToString<TupleVarTerm>(mTupleVarTerm2IntMap);
    ss << "\tVarTerm2IntMap = " << std::endl;
    ss << termMapToString<VarTerm>(mVarTerm2IntMap);
    
    // Underlying partial ordering on integer ids.
    if (mGraphPtr!=NULL) { ss << mGraphPtr->toString(); }
    
    return ss.str();
}
    

}
