/*!
 * \file UFCallMapAndBounds.h
 *
 * \brief Interface of UFCallMapAndBounds class
 *
 * \date Started: 2013-07-30
 *
 * \authors Michelle Strout
 *
 * Copyright (c) 2013, Colorado State University 
 * All rights reserved. 
 * See ../../COPYING for details. <br>
 */
 
#ifndef UFCALLMAPANDBOUNDS_H_
#define UFCALLMAPANDBOUNDS_H_

#include "expression.h"
#include "TupleDecl.h"
#include <map>
#include <string>

namespace iegenlib{

class Set;

/*!
 * \class UFCallMapAndBounds
 *
 * This class should ONLY be used internally by SparseConstraints, 
 * Exp, and UFCallTerm.  Needed for normalization of Sets and Relations.
 *
 * The collected constraints will have the same initial tuple variables
 * as the original set, or relation, for which constraints are being
 * collected and then extra tuple vars will be added to represent
 * UFCalls.  Parameter expressions will be bound in the constraints
 * by passing in their UFCallTerms.
 * Temporary variables will be created for each UF call result
 * by calling boundByRange.
 *
 * The user can get determine what tuple variables are temporary by
 *      // getting the set of collected constraints
 *      Set* collectedConstraints = ufcallmap.cloneConstraints();
 *      // checking to see the arity
 *      int arity = collectedConstraints->arity();
 *      // and checking to see how many temporary variables were created
 *      unsigned int num_temp = ufcallmap.numTempVars();
 * The temporary variables are then indexed by what tuple variables they are.
 * Their tuple locations come after the tuple declaration for the original 
 * set/relation.  Therefore they start at
 *          arity - num_temp
 * and go through
 *          arity - 1
 *
 * \brief Assignment of UF call results to temp vars and collection of bounds
 *        due to UF domains and ranges.
 */
class UFCallMapAndBounds {
public:
    //! \param tdecl TupleDecl of the Set or Relation for which 
    //!        we are collecting bounds.
    UFCallMapAndBounds(TupleDecl tdecl);
    ~UFCallMapAndBounds();
    
    //! Returns the number of temporary variables.
    unsigned int numTempVars() const;

    //! Returns UFCallTerm associated with given temporary var.
    //! Temporary variable is indexed by its tuple location.
    //! See the description of the UFCallMapAndBounds class for
    //! how that works.
    UFCallTerm* cloneUFCall( unsigned int varnum ) const;

    //! Introduces temporary variables as needed for each integer
    //! returned by a function (could be more than one for a function
    //! that returns a tuple).  Bounds those temporaries in
    //! collected constraints.
    TupleExpTerm* boundByRange( const UFCallTerm* uf_call );

    //! Collects bounds parameters to the given UF call.
    //! Will bound the parameters by the domain of the UF.
    //! Takes ownership of passed in uf_call.
    void boundByDomain( const UFCallTerm* uf_call );

    //! returns a clone of the Set of maintained constraints.
    Set* cloneConstraints() const;
    
    //! Returns a string representation of the class instance for debugging.
    std::string toString() const;        
        
private:
    std::vector<UFCallTerm*>            mUFCallVector;
    Set*                                mCurrentConstraints;
    
    //! arity of the original set we are collecting constraints for
    int                                 mInitialArity;    
};

}

#endif
