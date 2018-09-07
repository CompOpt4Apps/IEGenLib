/*!
 * \file setRelationship_test.cc
 *
 * \brief test cases for detecting set relationship between Sets and Relations.
 *
 * \date Started: 9/06/18
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

// Exp and Term classes
using iegenlib::Exp;
using iegenlib::Term;
using iegenlib::UFCallTerm;
using iegenlib::TupleVarTerm;
using iegenlib::VarTerm;
using iegenlib::TupleDecl;

// set and relation classes
using iegenlib::Conjunction;
using iegenlib::SparseConstraints;
using iegenlib::Set;
using iegenlib::Relation;
using iegenlib::Environment;
using iegenlib::UninterpFunc;
using iegenlib::SubMap;
using iegenlib::UniQuantRule;
using iegenlib::currentEnv;
using iegenlib::UFCallMap;
using iegenlib::UniQuantRuleType;
using iegenlib::ruleInstantiation;
using iegenlib::instantiate;


#pragma mark setEqualityTEST
TEST(setRelationshipTest, setEqualityTEST){

  // Introduce the UFCalls to enviroment, and indicate their domain, range
  // whether they are bijective, or monotonic.
  iegenlib::setCurrEnv();
  iegenlib::appendCurrEnv("rowIdx",
            new Set("{[i]:0<=i &&i<nnz}"),      // Domain 
            new Set("{[j]:0<=j &&j<m}"),        // Range
            false,                              // Bijective?!
            iegenlib::Monotonic_NONE            // monotonicity
            );
  iegenlib::appendCurrEnv("colPtr",
   new Set("{[i]:0<=i &&i<m}"), 
   new Set("{[j]:0<=j &&j<nnz}"), false, iegenlib::Monotonic_Nondecreasing);

  Relation *r1 = new Relation("{ [0, In_2, 2, In_4, 0, In_6, 0, In_8, 0] -> [0, Out_2, 0, 0, 0, 0, 0, 0, 0] : colPtr(Out_2) = In_6 && rowIdx(In_4) = Out_2 && rowIdx(In_8) = rowIdx(In_6) && 0 <= In_2 && 0 <= Out_2 && In_4 <= In_8 && colPtr(rowIdx(In_4)) <= In_6 && rowIdx(In_8 + 1) <= rowIdx(In_8) && In_2 < Out_2 && In_2 + 1 < n && In_4 < colPtr(In_2 + 1) && colPtr(In_2) < In_4 && In_6 < colPtr(rowIdx(In_4) + 1) && In_8 < colPtr(In_2 + 1) && Out_2 + 1 < n }");

  Relation *r2 = new Relation("{ [0, Out_2, 2, Out_4, 0, Out_6, 0, Out_8, 0] -> [0, In_2, 0, 0, 0, 0, 0, 0, 0] : colPtr(In_2) = Out_6 && rowIdx(Out_4) = In_2 && rowIdx(Out_8) = rowIdx(Out_6) && 0 <= Out_2 && 0 <= In_2 && Out_4 <= Out_8 && colPtr(rowIdx(Out_4)) <= Out_6 && rowIdx(Out_8 + 1) <= rowIdx(Out_6) && Out_2 < In_2 && Out_2 + 1 < n && Out_4 < colPtr(Out_2 + 1) && colPtr(Out_2) < Out_4 && Out_6 < colPtr(rowIdx(Out_4) + 1) && Out_8 < colPtr(Out_2 + 1) && In_2 + 1 < n }");

  EXPECT_EQ( iegenlib::SetEqual, r1->setRelationship(r2) );

  
}


#pragma mark setEqualityTEST
TEST(setRelationshipTest, setTrivialSuperSetEqualityTEST){

  // Introduce the UFCalls to enviroment, and indicate their domain, range
  // whether they are bijective, or monotonic.
  iegenlib::setCurrEnv();
  iegenlib::appendCurrEnv("rowIdx",
            new Set("{[i]:0<=i &&i<nnz}"),      // Domain 
            new Set("{[j]:0<=j &&j<m}"),        // Range
            false,                              // Bijective?!
            iegenlib::Monotonic_NONE            // monotonicity
            );
  iegenlib::appendCurrEnv("colPtr",
   new Set("{[i]:0<=i &&i<m}"), 
   new Set("{[j]:0<=j &&j<nnz}"), false, iegenlib::Monotonic_Nondecreasing);

  Relation *r1 = new Relation("{ [0, In_2, 2, In_4, 0, In_6, 0, In_8, 0] -> [0, Out_2, 0, 0, 0, 0, 0, 0, 0] : colPtr(Out_2) = In_6 && rowIdx(In_4) = Out_2 && rowIdx(In_8) = rowIdx(In_6) && 0 <= In_2 && 0 <= Out_2 && In_4 <= In_8 && colPtr(rowIdx(In_4)) <= In_6 && rowIdx(In_8 + 1) <= rowIdx(In_8) && In_2 < Out_2 && In_2 + 1 < n && In_4 < colPtr(In_2 + 1) && colPtr(In_2) < In_4 && In_6 < colPtr(rowIdx(In_4) + 1) && In_8 < colPtr(In_2 + 1) && Out_2 + 1 < n }");

  Relation *r2 = new Relation("{ [0, Out_2, 2, Out_4, 0, Out_6, 0, Out_8, 0] -> [0, In_2, 1, In_4, 0, 0, 0, 0, 0] : In_4 = Out_6 && rowIdx(Out_4) = In_2 && rowIdx(Out_8) = rowIdx(Out_6) && 0 <= Out_2 && 0 <= In_2 && Out_4 <= Out_8 && colPtr(rowIdx(Out_4)) <= Out_6 && rowIdx(Out_8 + 1) <= rowIdx(Out_6) && Out_2 < In_2 && Out_2 + 1 < n && Out_4 < colPtr(Out_2 + 1) && colPtr(Out_2) < Out_4 && Out_6 < colPtr(rowIdx(Out_4) + 1) && Out_8 < colPtr(Out_2 + 1) && In_2 + 1 < n && In_4 < colPtr(In_2 + 1) && colPtr(In_2) < In_4 }");

  EXPECT_EQ( iegenlib::SetEqual, r1->setRelationship(r2) );


  
}
