/*!
 * \file setRelationship.cc
 *
 * \brief Implementation of functionality that tries to determine set type relationship
 * (SuperSet, SubSetEqual, etc) between IEGenLib Sets and Relations that can have
 * uninterpreted function symbols in their constraints.
 *
 * \date Started: 9/07/18
 *
 * \authors Mahdi Soltan Mohammadi
 *
 * Copyright (c) 2015-2018, University of Arizona <br>
 * All rights reserved. <br>
 * See ../../COPYING for details. <br>
 */

#include "set_relation.h"
#include "expression.h"
#include "UFCallMap.h"
#include "Visitor.h"

#include <util/util.h>
#include <gtest/gtest.h>

#include <utility>
#include <fstream>
#include <iostream>


namespace iegenlib{


/**
 ** This function uses islSetProjectOut function to project out all
 ** iterators in a given set except for the specified ones in eigenTvs
 ** Note, an important assumption is that the given set only has affine constraints.
 ** The function owns the inSet object, and it deletes it at the end
 **/
Set* islSetProjectOutAll(Set *inSet, std::set<int> eigenTvs, bool removeFromTuple = true){
  bool at_least_one_projected = false;
  Set *retSet;
  for(int tV = (inSet->arity()-1); tV >= 0; tV--){
    if( eigenTvs.find(tV) != eigenTvs.end() ){
      continue;
    } else {
       retSet = islSetProjectOut(inSet, tV, removeFromTuple);
       delete inSet;
       inSet = retSet;
       at_least_one_projected = true;
    }
  }
  if(at_least_one_projected)  return retSet;
  else                        return inSet;
}


/**
 ** This function determines the relationship between 2 iegenlib:sets
 ** using isl's function.
 ** Currently, it only supports sets with one conjunction.
   A ?? B

 ** Inputs:

    set1:  Left-hand side dependence  (A)
    set1EigenTvs:
       Tuple variables in the first set that are not going to be projected out.
    set2:  Right-hand side dependence (B)
    set2EigenTvs:
       Tuple variables in the second set that are not going to be projected out.

 ** Output is one of following:

     SetEqual, SubSetEqual, SubSet, SuperSet, SuperSetEqual, UnKnown
 **
 **/
SetRelationshipType iegenSetRelationship(Set *set1, std::set<int> set1EigenTvs,
                                Set *set2, std::set<int> set2EigenTvs){
  SetRelationshipType ret = UnKnown;
  UFCallMap *ufcmap1 = new UFCallMap(set1->getTupleDecl());

  // Getting the affine verions of the sets by appling Ackerman reduction
  Set *affineSet1 = set1->superAffineSet(ufcmap1,false);

  UFCallMap *ufcmap2 = new UFCallMap(set2->getTupleDecl());
  Set *affineSet2 = set2->superAffineSet(ufcmap2,false);

  Set* islSet1 = islSetProjectOutAll(affineSet1, set1EigenTvs);

  Set* islSet2 = islSetProjectOutAll(affineSet2, set2EigenTvs);

  ret = strISLSetRelationship(islSet1->toISLString(), islSet2->toISLString());

  return ret;
}

/**
 ** This function determines the relationship between 2 data dependence
 ** constraint sets, presented as iegenlib:Relations:

   A ?? B

 ** Inputs:

    *this:     Left-hand side dependence  (A)
    rightSide: Right-hand side dependence (B)
    parallelLoopLevel:
       Starting from 0, loop level that is going to be parallelized.
       Therefore, in a relation like following:
       dep = {[itv0,itv1,...]->[otv0,otv1,...]: ...}
       all but the itv($parallelLoopLevel$) and otv($parallelLoopLevel$), are projected out.

 ** Output is one of following:

     SetEqual, SubSetEqual, SubSet, SuperSet, SuperSetEqual, UnKnown

 **
 **/
SetRelationshipType Relation::dataDependenceRelationship(Relation* rightSide, int parallelLoopLevel){

  SetRelationshipType ret = UnKnown;
  // Getting a string representation of the relations that their tuple variables
  // have general names: tv0, tv1 ...
  string genericStringDep1 = this->getString(true);
  string genericStringDep2 = rightSide->getString(true);

  // Getting a set representation of the relations
  Set *eqSet1 = new Set( relationStr2SetStr(genericStringDep1 , inArity(), outArity()) );
  Set *eqSet2 = new Set( relationStr2SetStr(genericStringDep2 ,
                         rightSide->inArity(), rightSide->outArity()) );

  // What tuple variables are not going to be projected out?
  // The tuple variable that represent the parallel loop level
  std::set<int> eigenLTvs;
  std::set<int> eigenRTvs;
  eigenLTvs.insert( parallelLoopLevel );
  eigenLTvs.insert( inArity()+parallelLoopLevel );
  eigenRTvs.insert( parallelLoopLevel );
  eigenRTvs.insert( rightSide->inArity()+parallelLoopLevel );

  return iegenSetRelationship(eqSet1, eigenLTvs, eqSet2, eigenRTvs);
}



}//end namespace iegenlib
