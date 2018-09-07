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

/*****************************************************************************/
#pragma mark -

// 
SetRelationshipType Conjunction::setRelationship(Conjunction* rightSide){

  int lEqC, lInEqC, lConstC, rEqC, rInEqC, rConstC, supSetEqC, supSetInEqC, supSetContC;

  lEqC = this->mEqualities.size();
  lInEqC = this->mInequalities.size();
  lConstC = lEqC+lInEqC;
  rEqC = rightSide->mEqualities.size();
  rInEqC = rightSide->mInequalities.size();
  rConstC = rEqC+rInEqC;

  std::cout<<"\n\nConjunction::setRelationship:\n Left EqC = "<<lEqC<<" InEqC = "<<lInEqC<<"  ConstC = "<<lConstC<<" Right EqC = "<<rEqC <<" InEqC = "<<rInEqC <<"  ConstC = "<<rConstC<<"\n\n";

  Conjunction *supSetCond, *subSetCond;

  if(lConstC <= rConstC){
    supSetCond = this;
    subSetCond = rightSide;
    supSetEqC = lEqC;
    supSetInEqC = lInEqC;
    supSetContC = lConstC;
  } else {
    supSetCond = rightSide;
    subSetCond = this;
    supSetEqC = rEqC;
    supSetInEqC = rInEqC;
    supSetContC = rConstC;
  }

  std::set<Exp> supSetEq, supSetInEq;

  for (std::list<Exp*>::const_iterator i=supSetCond->mEqualities.begin();
              i != supSetCond->mEqualities.end(); i++) {
  //  mEqualities.push_back((*i)->clone());
  }

  for (std::list<Exp*>::const_iterator i=supSetCond->mInequalities.begin();
              i != supSetCond->mInequalities.end(); i++) {
  //  mInequalities.push_back((*i)->clone());
  }

  return UnKnown;
}

bool islUnSat(std::string str){
  srParts parts = getPartsFromStr(str);
  if( trim(parts.constraints) == "FALSE") return true;
  return false;
}
// 
SetRelationshipType Set::setRelationship(Set* rightSide){
  // get clones of the sets
  Set* ls = new Set(*this);
  Set* rs = new Set(*rightSide);
  ls->normalize(false); rs->normalize(false);
  // See if the inormalization has detected the sets as unsat 
  // * normalization can detect trivially unsat sets
  if( islUnSat(ls->getString()) || islUnSat(rs->getString()) ) return UnSatSet;
  // See if the sets are equal * normalization exposes the trivially equal sets
  if(*ls == *rs) return SetEqual;

  Conjunction *lside = *(ls->mConjunctions.begin());
  Conjunction *rside = *(rs->mConjunctions.begin());

  return lside->setRelationship(rside);
}

// 
SetRelationshipType Relation::setRelationship(Relation* rightSide){
  // get clones of the sets
  Relation* ls = new Relation(*this);
  Relation* rs = new Relation(*rightSide);
  ls->normalize(false); rs->normalize(false);
  // See if the inormalization has detected the sets as unsat 
  // * normalization can detect trivially unsat sets
  if( islUnSat(ls->getString()) || islUnSat(rs->getString()) ) return UnSatSet;
  // See if the sets are equal * normalization exposes the trivially equal sets
  if(*ls == *rs) return SetEqual;

  Conjunction *lside = *(ls->mConjunctions.begin());
  Conjunction *rside = *(rs->mConjunctions.begin());

  return lside->setRelationship(rside);
}


}//end namespace iegenlib
