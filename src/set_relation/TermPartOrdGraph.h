/*!
 * \file TermPartOrdGraph.h
 *
 * \brief Interface for partial ordering graph specifically for Terms.
 *
 * See class header for usage.
 *
 * \date Started: 2013-08-20
 *
 * \authors Michelle Strout
 *
 * Copyright (c) 2013, Colorado State University 
 * All rights reserved. 
 * See ../../COPYING for details. <br>
 */
 
#ifndef TERMPARTORDGRAPH_H_
#define TERMPARTORDGRAPH_H_

#include <set>
#include <map>
#include <string>

#include "expression.h"
#include <util/PartOrdGraph.h>

namespace iegenlib{

/*!
 * \class TermPartOrdGraph
 * FIXME Mahdi
 * This class will keep track of unique terms found in the constraints,
 * then once all initial terms have been added 
 * (TermPartOrdGraph::doneInsertingInitialTerms)
 * it is possible to start adding partial ordering constraints between
 * any of the Terms.
 *
 * Usage example:
 *      TermPartOrdGraph g;
 *      UFCallTerm uf = new UFCallTerm( ... );
 *      TupleVarTerm tv = new TupleVarTerm( ... );
 *      g.insertTerm( uf );
 *      g.insertTerm( tv );
 *      g.insertTerm( uf );     // Will do object comparison and only keep one.
 *      
 *      g.doneInsertingInitialTerms();
 *
 *      // Insertions of orderings can only come after doneInsertingInitialTerms.
 *      g.insertLTE( uf, tv);   // (*uf) <= (*tv)
 *      g.insertLT( uf, tv);    // (*uf) < (*tv)
 *      g.insertEqual( uf,tv ); // Will cause an error in this case.
 *
 *      // Indicating a term is non-negative can happen at anytime
 *      // after the term has been inserted.
 *      g.termNonNegative( tv );
 *
 *      //====== Queries ======
 *
 *      if (g.isNonNegative(uf)) { ... }
 *
 *      std::set<UFCallTerm*> ufTerms = g.getUniqueUFCallTerms();
 *      for (std::set<Term*>::const_iterator iter1 = ufTerms.begin();
 *              iter1!=ufTerms.end(); iter1++) {
 *          for (std::set<Term*>::const_iterator iter2 = ufTerms.begin();
 *                  iter2!=ufTerms.end(); iter2++) {
 *              if (g.isLT(*iter1, *iter2)) {
 *                  ...
 *              }
 *          }
 *      }
 */
class TermPartOrdGraph {
public:
    TermPartOrdGraph(int maxNumTerms);
    
    //! Copy constructor.  Performs a deep copy of PartOrdGraph, but does not
    //! own any Terms so just copying their pointers.
    TermPartOrdGraph(const TermPartOrdGraph& other);
    //! Copy assignment.
    TermPartOrdGraph& operator=(const TermPartOrdGraph& other);

    ~TermPartOrdGraph();
    
    //! helper function for implementing copy-and-swap
    void swap(TermPartOrdGraph& second) throw();

    //! Indicate the given term is non-negative.  Will pretend coeff 1.
    //! Assumes that given term has already been inserted.
    //! Does not take ownership of term.
    void termNonNegative( const Term* term );
 
    //! Queries whether a term is non-negative.  Will pretend coeff 1.
    //! Does not take ownership of term.
    bool isNonNegative( const Term* term );

    //! ==== NONE of the below take ownership of term.
    //! ==== They all will pretend the coeffs are 1.
    //! Term1 <= Term2
    void insertLTE( Term* term1, Term* term2 );
    //! Term1 < Term2
    void insertLT( Term* term1, Term* term2 );
    //! Term1 == Term2
    void insertEqual( Term* term1, Term* term2 );

    //! ==== Query methods 
    //! ==== NONE of the below take ownership of term.
    //! returns true if Term1 <= Term2
    bool isLTE( Term* term1, Term* term2 );
    //! returns true if Term1 < Term2
    bool isLT( Term* term1, Term* term2 );
    //! returns true if Term1 == Term2
    bool isEqual( Term* term1, Term* term2 );

    //! Returns a set of all unique UFCallTerms that have been inserted.
    //! Caller does NOT own returned terms.
    std::set<UFCallTerm*> getUniqueUFCallTerms();

    //! Returns a set of all unique terms that have been inserted.
    //! Caller will own returned terms.  FIXME: can we return owned ptrs or something?
//    std::set<Term*> getAllUniqueTerms() const;
// Don't think we need this and having it causes a cleanup pain.  Although
// just realized I could keep a set of the copies in here and then clean them up
// upon destruction.
    
    //! Returns a string representation of the class instance for debugging.
    std::string toString() const;
    // Query if underlying partial ordering has become inconsistence
    bool isUnsat(){ return mGraphPtr->isUnsat();}


private:
    int                         mNumTerms;
    int                         mMaxNumTerms;

    std::map<UFCallTerm,int>    mUFCallTerm2IntMap;
    std::map<TupleVarTerm,int>  mTupleVarTerm2IntMap;
    std::map<VarTerm,int>       mVarTerm2IntMap;
    std::map<Term,int>          mTerm2IntMap;
    
    std::set<Term*>             mNonNegativeTerms;
    
    // kept so can own and clean up
    std::set<UFCallTerm*>       mUniqueUFCallTerms; 
    
    PartOrdGraph*               mGraphPtr;
    
    // Helper routines.
    
    int findOrInsertTermId(const Term* t);

};

}

#endif
