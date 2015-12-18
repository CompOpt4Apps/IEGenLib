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
 *
 * This class will keep track of unique terms found in the constraints,
 * then once all terms have been added (TermPartOrdGraph::doneInsertingTerms)
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
 *      g.doneInsertingTerms();
 *
 *      // Insertions of orderings can only come after doneInsertingTerms.
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
    TermPartOrdGraph();
    ~TermPartOrdGraph();
    
    //! Insert a term into graph vertex set.  Will pretend coeff 1.
    //! Does not take ownership of term.
    void insertTerm( const Term* term);
    
    //! Indicate the given term is non-negative.  Will pretend coeff 1.
    //! Assumes that given term has already been inserted.
    //! Does not take ownership of term.
    void termNonNegative( const Term* term );
 
    //! Queries whether a term is non-negative.  Will pretend coeff 1.
    //! Does not take ownership of term.
    bool isNonNegative( const Term* term ) const;
   
    //! Call when all term insertions are done and ready for partial orders.
    void doneInsertingTerms();

    //! ==== NONE of the below take ownership of term.
    //! ==== They all will pretend the coeffs are 1.
    //! ==== They only work after doneInsertingTerms has been called.
    //! ==== Will throw an assert exception if called before that.
    //! Term1 <= Term2
    void insertLTE( Term* term1, Term* term2 );
    //! Term1 < Term2
    void insertLT( Term* term1, Term* term2 );
    //! Term1 == Term2
    void insertEqual( Term* term1, Term* term2 );

 
    //! ==== Query methods
 
    //! Returns true if doneInsertingTerms() has been called.
    bool isDoneInsertingTerms() const;
 
    //! ==== NONE of the below take ownership of term.
    //! ==== They all will pretend the coeffs are 1.
    //! ==== They only work after doneInsertingTerms has been called.
    //! ==== Will throw an assert exception if called before that.
    //! returns true if Term1 <= Term2
    bool isLTE( Term* term1, Term* term2 );
    //! returns true if Term1 < Term2
    bool isLT( Term* term1, Term* term2 );
    //! returns true if Term1 == Term2
    bool isEqual( Term* term1, Term* term2 );

    //! Returns a set of all unique UFCallTerms that have been inserted.
    //! Caller will own returned terms.
    std::set<UFCallTerm*> getUniqueUFCallTerms() const;

    //! Returns a set of all unique terms that have been inserted.
    //! Caller will own returned terms.
    std::set<Term*> getAllUniqueTerms() const;
   
     //! Returns a string representation of the class instance for debugging.
    std::string toString() const;        
        
private:
    int                         mNumTerms;

    std::map<UFCallTerm,int>    mUFCallTerm2IntMap;
    std::map<TupleVarTerm,int>  mTupleVarTerm2IntMap;
    std::map<VarTerm,int>       mVarTerm2IntMap;
    
    std::set<Term*>             mNonNegativeTerms;
    
    PartOrdGraph*               mGraphPtr;
    
    // Helper routines.
    
    int findOrInsertTermId(const Term* t);

};

}

#endif
