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

/*

// **** NOTE: Dependece relations in this test file are extracted from the following benchmarks

// Incomplete Cholesky 0 CSC from SparseLib++

  for (i = 0; i < n - 1; i++){
    val[colPtr[i]] = sqrt(val[colPtr[i]]);                           //S1

    for (m = colPtr[i] + 1; m < colPtr[i+1]; m++){
      val[m] = val[m] / val[colPtr[i]];                              //S2
    }

    for (m = colPtr[i] + 1; m < colPtr[i+1]; m++) {
      for (k = colPtr[rowIdx[m]] ; k < colPtr[rowIdx[m]+1]; k++){
        for ( l = m; l < colPtr[i+1] ; l++){
          if (rowIdx[l] == rowIdx[k] && rowIdx[l+1] <= rowIdx[k]){
            val[k] -= val[m]* val[l];                                //S3
          }
        }
      }
    }
  } 



// Static Left Cholesky a modifie version of Left Cholesky from CSparse Library

  for (colNo = 0; colNo < n; colNo++) {
    //Uncompressing a col into a 1D array
    for (nzNo = c[colNo]; nzNo < c[colNo + 1]; nzNo++) {
      f[r[nzNo]] = values[nzNo];//Copying nonzero of the col
    }
    for (i = prunePtr[colNo]; i < prunePtr[colNo + 1]; i++) {
      sw=0;
      for (l = lC[pruneSet[i]]; l < lC[pruneSet[i] + 1]; l++) {
        if (lR[l] == colNo && sw == 0) {
          tmp = lValues[l];
          sw=1;
        }
        if(sw == 1){
          f[lR[l]] -= lValues[l] * tmp;                            // S1
        }
      }
    }
    temp = f[colNo];
    tmpSqrt = sqrt(temp);
    f[colNo] = 0;
    lValues[lC[colNo]] = tmpSqrt;                                 // S2
    for (j = lC[colNo] + 1; j < lC[colNo + 1]; j++) {
      lValues[j] = f[lR[j]] / tmpSqrt;                            // S3
      f[lR[j]] = 0;
    }
  }

*/
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

  // r1: [In Incomplete Cholesky 0 ] a read in S1 (val[colPtr[i]]);  a write in S3 (val[k]); (Flow dependence)
  // r1 = { [i,m,k,l] -> [ip] : k = colPtr(ip) && i < ip && 0 <= i,ip < n-1 &&
  //         colPtr(i)+1 <= m < colPtr(i+1) &&
  //         colPtr(rowIdx(m)) <= k < colPtr(rowIdx(m)+1) && 
  //         m <= l < colPtr(i+1) && rowIdx(l+1) <= rowIdx(k) 
  //         && rowIdx(l) = rowIdx(k) }
  // Note: Following is the automatically extracted dependence
  Relation *r1 = new Relation("{ [0, In_2, 2, In_4, 0, In_6, 0, In_8, 0] -> [0, Out_2, 0, 0, 0, 0, 0, 0, 0] : colPtr(Out_2) = In_6 && rowIdx(In_8) = rowIdx(In_6) && 0 <= In_2 && 0 <= Out_2 && In_4 <= In_8 && colPtr(rowIdx(In_4)) <= In_6 && rowIdx(In_8 + 1) <= rowIdx(In_8) && In_2 < Out_2 && In_2 + 1 < n && In_4 < colPtr(In_2 + 1) && colPtr(In_2) < In_4 && In_6 < colPtr(rowIdx(In_4) + 1) && In_8 < colPtr(In_2 + 1) && Out_2 + 1 < n }");

  // r2: [In Incomplete Cholesky 0 ]  a write in S1 (val[colPtr[i]]);  a read in S3 (val[k]); (Anti dependence)
  // r2 = { [ip,mp,kp,lp] -> [i] : kp = colPtr(i) && ip < i && 0 <= i,ip < n-1 && 
  //         colPtr(ip)+1 <= mp < colPtr(ip+1) && 
  //         colPtr(rowIdx(mp)) <= kp < colPtr(rowIdx(mp)+1) && 
  //         mp <= lp < colPtr(ip+1) && rowIdx(lp+1) <= rowIdx(kp) && 
  //         rowIdx(lp) = rowIdx(kp) }
  // Note: Following is the automatically extracted dependence
  Relation *r2 = new Relation("{ [0, Out_2, 2, Out_4, 0, Out_6, 0, Out_8, 0] -> [0, In_2, 0, 0, 0, 0, 0, 0, 0] : colPtr(In_2) = Out_6 && rowIdx(Out_8) = rowIdx(Out_6) && 0 <= Out_2 && 0 <= In_2 && Out_4 <= Out_8 && colPtr(rowIdx(Out_4)) <= Out_6 && rowIdx(Out_8 + 1) <= rowIdx(Out_8) && Out_2 < In_2 && Out_2 + 1 < n && Out_4 < colPtr(Out_2 + 1) && colPtr(Out_2) < Out_4 && Out_6 < colPtr(rowIdx(Out_4) + 1) && Out_8 < colPtr(Out_2 + 1) && In_2 + 1 < n }");

  // The reason we can expect these two dependences to be SetEqual is following:
  // First let us change the name of tuple variables into general names 
  // as only their ordering is important for determining relationsship between two polehedral:

  // 
  // r1 = { [tv1,tv2,tv3,tv4] -> [tv5] : tv3 = colPtr(tv5) && tv1 < tv5 && 0 <= tv1,tv5 < n-1 &&
  //         colPtr(tv1)+1 <= tv2 < colPtr(tv1+1) &&
  //         colPtr(rowIdx(tv2)) <= tv3 < colPtr(rowIdx(tv2)+1) && 
  //         tv2 <= tv4 < colPtr(tv1+1) && rowIdx(tv4+1) <= rowIdx(tv3) 
  //         && rowIdx(tv4) = rowIdx(tv3) }
  // r2 = { [tv1,tv2,tv3,tv4] -> [tv5] : tv3 = colPtr(tv5) && tv1 < tv5 && 0 <= tv1,tv5 < n-1 &&
  //         colPtr(tv1)+1 <= tv2 < colPtr(tv1+1) &&
  //         colPtr(rowIdx(tv2)) <= tv3 < colPtr(rowIdx(tv2)+1) && 
  //         tv2 <= tv4 < colPtr(tv1+1) && rowIdx(tv4+1) <= rowIdx(tv3) 
  //         && rowIdx(tv4) = rowIdx(tv3) }
  //
  //  As you can see the dependences with generic tuple variables names are actually identical.

  EXPECT_EQ( iegenlib::SetEqual, r1->dataDependenceRelationship(r2,1) );


  // r1: [In Incomplete Cholesky 0 ]  a read in S3 (val[k]);  a write in S3 (val[k]); (Flow dependence)
  r1 = new Relation("{ [0, In_2, 2, In_4, 0, In_6, 0, In_8, 0] -> [0, Out_2, 2, Out_4, 0, Out_6, 0, Out_8, 0] : Out_6 = In_6 && rowIdx(Out_8) = rowIdx(Out_6) && rowIdx(In_8) = rowIdx(In_6) && 0 <= In_2 && 0 <= Out_2 && In_4 <= In_8 && colPtr(rowIdx(In_4)) <= In_6 && Out_4 <= Out_8 && colPtr(rowIdx(Out_4)) <= Out_6 && rowIdx(Out_8 + 1) <= rowIdx(Out_6) && rowIdx(In_8 + 1) <= rowIdx(In_8) && In_2 < Out_2 && In_2 + 1 < n && In_4 < colPtr(In_2 + 1) && colPtr(In_2) < In_4 && In_6 < colPtr(rowIdx(In_4) + 1) && In_8 < colPtr(In_2 + 1) && Out_2 + 1 < n && Out_4 < colPtr(Out_2 + 1) && colPtr(Out_2) < Out_4 && Out_6 < colPtr(rowIdx(Out_4) + 1) && Out_8 < colPtr(Out_2 + 1) }");

  // r2: [In Incomplete Cholesky 0 ]  a read in S3 (val[k]);  a write in S3 (val[k]); (Anti dependence)
  r2 = new Relation("{ [0, Out_2, 2, Out_4, 0, Out_6, 0, Out_8, 0] -> [0, In_2, 2, In_4, 0, In_6, 0, In_8, 0] : In_6 = Out_6 && rowIdx(In_8) = rowIdx(In_6) && rowIdx(Out_8) = rowIdx(Out_6) && 0 <= Out_2 && 0 <= In_2 && Out_4 <= Out_8 && colPtr(rowIdx(Out_4)) <= Out_6 && In_4 <= In_8 && colPtr(rowIdx(In_4)) <= In_6 && rowIdx(In_8 + 1) <= rowIdx(In_8) && rowIdx(Out_8 + 1) <= rowIdx(Out_6) && Out_2 < In_2 && Out_2 + 1 < n && Out_4 < colPtr(Out_2 + 1) && colPtr(Out_2) < Out_4 && Out_6 < colPtr(rowIdx(Out_4) + 1) && Out_8 < colPtr(Out_2 + 1) && In_2 + 1 < n && In_4 < colPtr(In_2 + 1) && colPtr(In_2) < In_4 && In_6 < colPtr(rowIdx(In_4) + 1) && In_8 < colPtr(In_2 + 1) }");

  // Testing r1->setRelationship(r2) 
  EXPECT_EQ( iegenlib::SetEqual, r1->dataDependenceRelationship(r2,1) );

  delete r1;
  delete r2;


  // r1: [In Incomplete Cholesky 0 ]  a write in S3 (val[k]);  a read in S3 (val[m]); (Anti dependence)
  r1 = new Relation("{ [0, Out_2, 2, Out_4, 0, Out_6, 0, Out_8, 0] -> [0, In_2, 1, In_4, 0, 0, 0, 0, 0] : In_4 = Out_6 && rowIdx(Out_8) = rowIdx(Out_6) && 0 <= Out_2 && 0 <= In_2 && Out_4 <= Out_8 && colPtr(rowIdx(Out_4)) <= Out_6 && rowIdx(Out_8 + 1) <= rowIdx(Out_6) && Out_2 < In_2 && Out_2 + 1 < n && Out_4 < colPtr(Out_2 + 1) && colPtr(Out_2) < Out_4 && Out_6 < colPtr(rowIdx(Out_4) + 1) && Out_8 < colPtr(Out_2 + 1) && In_2 + 1 < n && In_4 < colPtr(In_2 + 1) && colPtr(In_2) < In_4 }");

  // r2: [In Incomplete Cholesky 0 ]  a read in S2 (val[m]);  a write in S3 (val[k]); (Flow dependence)
  r2 = new Relation("{ [0, In_2, 2, In_4, 0, In_6, 0, In_8, 0] -> [0, Out_2, 1, Out_4, 0, 0, 0, 0, 0] : Out_4 = In_6 && rowIdx(In_8) = rowIdx(In_6) && 0 <= In_2 && 0 <= Out_2 && In_4 <= In_8 && colPtr(rowIdx(In_4)) <= In_6 && rowIdx(In_8 + 1) <= rowIdx(In_8) && In_2 < Out_2 && In_2 + 1 < n && In_4 < colPtr(In_2 + 1) && colPtr(In_2) < In_4 && In_6 < colPtr(rowIdx(In_4) + 1) && In_8 < colPtr(In_2 + 1) && Out_2 + 1 < n && Out_4 < colPtr(Out_2 + 1) && colPtr(Out_2) < Out_4 }");

  // Testing r1->setRelationship(r2) 
  EXPECT_EQ( iegenlib::SetEqual, r1->dataDependenceRelationship(r2,1) );

  delete r1;
  delete r2;
}


#pragma mark setEqualityTEST
TEST(setRelationshipTest, setTrivialSuperSetTEST){

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

  iegenlib::appendCurrEnv("lR",
            new Set("{[i]:0<=i &&i<nnz}"),      // Domain 
            new Set("{[j]:0<=j &&j<m}"),        // Range
            false,                              // Bijective?!
            iegenlib::Monotonic_NONE            // monotonicity
            );
  iegenlib::appendCurrEnv("lC",
   new Set("{[i]:0<=i &&i<m}"), 
   new Set("{[j]:0<=j &&j<nnz}"), false, iegenlib::Monotonic_Nondecreasing);


  iegenlib::appendCurrEnv("pruneSet",
            new Set("{[i]:0<=i &&i<nnz}"),      // Domain 
            new Set("{[j]:0<=j &&j<m}"),        // Range
            false,                              // Bijective?!
            iegenlib::Monotonic_NONE            // monotonicity
            );
  iegenlib::appendCurrEnv("prunePtr",
   new Set("{[i]:0<=i &&i<m}"), 
   new Set("{[j]:0<=j &&j<nnz}"), false, iegenlib::Monotonic_Nondecreasing);



  // r2 has all the constraints of the r1, plus some other constraints, in this case 
  // we say that r1 is SuperSetEqual of r2..

  // r1:  [In Incomplete Cholesky 0 ] a read in S1 (val[colPtr[i]]);  a write in S3 (val[k]); (Flow dependence)
  Relation *r1 = new Relation("{ [0, In_2, 2, In_4, 0, In_6, 0, In_8, 0] -> [0, Out_2, 0, 0, 0, 0, 0, 0, 0] : colPtr(Out_2) = In_6 && rowIdx(In_8) = rowIdx(In_6) && 0 <= In_2 && 0 <= Out_2 && In_4 <= In_8 && colPtr(rowIdx(In_4)) <= In_6 && rowIdx(In_8 + 1) <= rowIdx(In_8) && In_2 < Out_2 && In_2 + 1 < n && In_4 < colPtr(In_2 + 1) && colPtr(In_2) < In_4 && In_6 < colPtr(rowIdx(In_4) + 1) && In_8 < colPtr(In_2 + 1) && Out_2 + 1 < n }");

  // r1:  [In Incomplete Cholesky 0 ] a read in S2 (val[colPtr[i]]);  a write in S3 (val[k]); (Flow dependence)
  Relation *r2 = new Relation("{ [0, In_2, 2, In_4, 0, In_6, 0, In_8, 0] -> [0, Out_2, 1, Out_4, 0, 0, 0, 0, 0] : colPtr(Out_2) = In_6 && rowIdx(In_8) = rowIdx(In_6) && 0 <= In_2 && 0 <= Out_2 && In_4 <= In_8 && colPtr(rowIdx(In_4)) <= In_6 && rowIdx(In_8 + 1) <= rowIdx(In_8) && In_2 < Out_2 && In_2 + 1 < n && In_4 < colPtr(In_2 + 1) && colPtr(In_2) < In_4 && In_6 < colPtr(rowIdx(In_4) + 1) && In_8 < colPtr(In_2 + 1) && Out_2 + 1 < n && Out_4 < colPtr(Out_2 + 1) && colPtr(Out_2) < Out_4 }");

  // Testing r1->setRelationship(r2) 
  EXPECT_EQ( iegenlib::SuperSet, r1->dataDependenceRelationship(r2,1) );

  delete r1;
  delete r2;


  // r2 has all the constraints of the r1, plus some other constraints, in this case 
  // we say that r1 is SuperSetEqual of r2.
  // r1: [In Static Left Cholesky] a write  lValues[lC[colNo]] in S2; A read lValues[l] in S1
  r1 = new Relation("{ [0, In_2, 2, 0, 0, 0, 0] -> [0, Out_2, 1, Out_4, 1, Out_6, 1] : lC(In_2) = Out_6 && 0 <= In_2 && 0 <= Out_2 && prunePtr(Out_2) <= Out_4 && lC(pruneSet(Out_4)) <= Out_6 && In_2 < Out_2 && In_2 < n && Out_2 < n && Out_4 < prunePtr(Out_2 + 1) && Out_6 < lC(pruneSet(Out_4) + 1) }");  // && 1 = sw

  r2 = new Relation("{ [0, In_2, 2, 0, 0, 0, 0] -> [0, Out_2, 1, Out_4, 1, Out_6, 0] : lR(Out_6) = Out_2 && lC(In_2) = Out_6 && 0 <= In_2 && 0 <= Out_2 && prunePtr(Out_2) <= Out_4 && lC(pruneSet(Out_4)) <= Out_6 && In_2 < Out_2 && In_2 < n && Out_2 < n && Out_4 < prunePtr(Out_2 + 1) && Out_6 < lC(pruneSet(Out_4) + 1) }");  // 0 = sw && 

  // Testing r1->setRelationship(r2) 
  EXPECT_EQ( iegenlib::SuperSet, r1->dataDependenceRelationship(r2,1) );

  delete r1;
  delete r2;



  // lChol R6 and R2
  // r2 has all the constraints of the r1, plus some other constraints, in this case 
  // we say that r1 is SuperSetEqual of r2, which is trivially to see.
  r1 = new Relation("{ [0, In_2, 3, In_4, 0, 0, 0] -> [0, Out_2, 1, Out_4, 1, Out_6, 1] : Out_6 = In_4 && 0 <= In_2 && 0 <= Out_2 && prunePtr(Out_2) <= Out_4 && lC(pruneSet(Out_4)) <= Out_6 && In_2 < Out_2 && In_2 < n && In_4 < lC(In_2 + 1) && lC(In_2) < In_4 && Out_2 < n && Out_4 < prunePtr(Out_2 + 1) && Out_6 < lC(pruneSet(Out_4) + 1) }");  // && 1 = sw

  r2 = new Relation("{ [0, In_2, 3, In_4, 0, 0, 0] -> [0, Out_2, 1, Out_4, 1, Out_6, 0] : Out_6 = In_4 && lR(Out_6) = Out_2 && 0 <= In_2 && 0 <= Out_2 && prunePtr(Out_2) <= Out_4 && lC(pruneSet(Out_4)) <= Out_6 && In_2 < Out_2 && In_2 < n && In_4 < lC(In_2 + 1) && lC(In_2) < In_4 && Out_2 < n && Out_4 < prunePtr(Out_2 + 1) && Out_6 < lC(pruneSet(Out_4) + 1) }");  // 0 = sw && 

  // Testing r1->setRelationship(r2) 
  EXPECT_EQ( iegenlib::SuperSet, r1->dataDependenceRelationship(r2,1) );

  delete r1;
  delete r2;
}


#pragma mark setEqualityTEST
TEST(setRelationshipTest, setTrivialSubSetEqualityTEST){

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

  // Mahdi: Self note: R28 and R17: needs isl normalization
  // r1 has all the constraints of the r2, plus some other constraints, in this case 
  // we say that r1 is SubSetEqual of r2, which is trivially to see.
  Relation *r1 = new Relation("{ [0, In_2, 2, In_4, 0, In_6, 0, In_8, 0] -> [0, Out_2, 2, Out_4, 0, Out_6, 0, Out_8, 0] : Out_4 = In_6 && rowIdx(Out_8) = rowIdx(Out_6) && rowIdx(In_8) = rowIdx(In_6) && 0 <= In_2 && 0 <= Out_2 && In_4 <= In_8 && colPtr(rowIdx(In_4)) <= In_6 && Out_4 <= Out_8 && colPtr(rowIdx(Out_4)) <= Out_6 && rowIdx(Out_8 + 1) <= rowIdx(Out_6) && rowIdx(In_8 + 1) <= rowIdx(In_8) && In_2 < Out_2 && In_2 + 1 < n && In_4 < colPtr(In_2 + 1) && colPtr(In_2) < In_4 && In_6 < colPtr(rowIdx(In_4) + 1) && In_8 < colPtr(In_2 + 1) && Out_2 + 1 < n && Out_4 < colPtr(Out_2 + 1) && colPtr(Out_2) < Out_4 && Out_6 < colPtr(rowIdx(Out_4) + 1) && Out_8 < colPtr(Out_2 + 1) }");

  Relation *r2 = new Relation("{ [0, Out_2, 2, Out_4, 0, Out_6, 0, Out_8, 0] -> [0, In_2, 1, In_4, 0, 0, 0, 0, 0] : In_4 = Out_6 && rowIdx(Out_8) = rowIdx(Out_6) && 0 <= Out_2 && 0 <= In_2 && Out_4 <= Out_8 && colPtr(rowIdx(Out_4)) <= Out_6 && rowIdx(Out_8 + 1) <= rowIdx(Out_6) && Out_2 < In_2 && Out_2 + 1 < n && Out_4 < colPtr(Out_2 + 1) && colPtr(Out_2) < Out_4 && Out_6 < colPtr(rowIdx(Out_4) + 1) && Out_8 < colPtr(Out_2 + 1) && In_2 + 1 < n && In_4 < colPtr(In_2 + 1) && colPtr(In_2) < In_4 }");

  // Testing r1->setRelationship(r2) 
  EXPECT_EQ( iegenlib::SubSet, r1->dataDependenceRelationship(r2,1) );

  delete r1;
  delete r2;


  // R28 and R22
  // r1 has all the constraints of the r2, plus some other constraints, in this case 
  // we say that r1 is SubSetEqual of r2, which is trivially to see.
  r1 = new Relation("{ [0, In_2, 2, In_4, 0, In_6, 0, In_8, 0] -> [0, Out_2, 2, Out_4, 0, Out_6, 0, Out_8, 0] : Out_4 = In_6 && rowIdx(Out_8) = rowIdx(Out_6) && rowIdx(In_8) = rowIdx(In_6) && 0 <= In_2 && 0 <= Out_2 && In_4 <= In_8 && colPtr(rowIdx(In_4)) <= In_6 && Out_4 <= Out_8 && colPtr(rowIdx(Out_4)) <= Out_6 && rowIdx(Out_8 + 1) <= rowIdx(Out_6) && rowIdx(In_8 + 1) <= rowIdx(In_8) && In_2 < Out_2 && In_2 + 1 < n && In_4 < colPtr(In_2 + 1) && colPtr(In_2) < In_4 && In_6 < colPtr(rowIdx(In_4) + 1) && In_8 < colPtr(In_2 + 1) && Out_2 + 1 < n && Out_4 < colPtr(Out_2 + 1) && colPtr(Out_2) < Out_4 && Out_6 < colPtr(rowIdx(Out_4) + 1) && Out_8 < colPtr(Out_2 + 1) }");

  r2 = new Relation("{ [0, In_2, 2, In_4, 0, In_6, 0, In_8, 0] -> [0, Out_2, 1, Out_4, 0, 0, 0, 0, 0] : Out_4 = In_6 && rowIdx(In_8) = rowIdx(In_6) && 0 <= In_2 && 0 <= Out_2 && In_4 <= In_8 && colPtr(rowIdx(In_4)) <= In_6 && rowIdx(In_8 + 1) <= rowIdx(In_8) && In_2 < Out_2 && In_2 + 1 < n && In_4 < colPtr(In_2 + 1) && colPtr(In_2) < In_4 && In_6 < colPtr(rowIdx(In_4) + 1) && In_8 < colPtr(In_2 + 1) && Out_2 + 1 < n && Out_4 < colPtr(Out_2 + 1) && colPtr(Out_2) < Out_4 }");

  // Testing r1->setRelationship(r2) 
  EXPECT_EQ( iegenlib::SubSet, r1->dataDependenceRelationship(r2,1) );

  delete r1;
  delete r2;
}


#pragma mark setEqualityTEST
TEST(setRelationshipTest, setComplicatedSuperSetEqualityTEST){

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


  // R22 and R30
  // r2 has all the constraints of the r1, except for one different equality
  // (alos some extra other constraints), in this case we look into that 
  // different equality, if the equality of r1 subsumes the equality in r2,
  // we say that r1 is SuperSetEqual of r2, which is trivially to see.
  Relation *r1 = new Relation("{ [0, In_2, 2, In_4, 0, In_6, 0, In_8, 0] -> [0, Out_2, 2, Out_4, 0, Out_6, 0, Out_8, 0] : Out_4 = In_6 && rowIdx(Out_8) = rowIdx(Out_6) && rowIdx(In_8) = rowIdx(In_6) && 0 <= In_2 && 0 <= Out_2 && In_4 <= In_8 && colPtr(rowIdx(In_4)) <= In_6 && Out_4 <= Out_8 && colPtr(rowIdx(Out_4)) <= Out_6 && rowIdx(Out_8 + 1) <= rowIdx(Out_6) && rowIdx(In_8 + 1) <= rowIdx(In_8) && In_2 < Out_2 && In_2 + 1 < n && In_4 < colPtr(In_2 + 1) && colPtr(In_2) < In_4 && In_6 < colPtr(rowIdx(In_4) + 1) && In_8 < colPtr(In_2 + 1) && Out_2 + 1 < n && Out_4 < colPtr(Out_2 + 1) && colPtr(Out_2) < Out_4 && Out_6 < colPtr(rowIdx(Out_4) + 1) && Out_8 < colPtr(Out_2 + 1) }");

  Relation *r2 = new Relation("{ [0, In_2, 2, In_4, 0, In_6, 0, In_8, 0] -> [0, Out_2, 2, Out_4, 0, Out_6, 0, Out_8, 0] : Out_8 = In_6 && rowIdx(Out_8) = rowIdx(Out_6) && rowIdx(In_8) = rowIdx(In_6) && 0 <= In_2 && 0 <= Out_2 && In_4 <= In_8 && colPtr(rowIdx(In_4)) <= In_6 && Out_4 <= Out_8 && colPtr(rowIdx(Out_4)) <= Out_6 && rowIdx(Out_8 + 1) <= rowIdx(Out_6) && rowIdx(In_8 + 1) <= rowIdx(In_8) && In_2 < Out_2 && In_2 + 1 < n && In_4 < colPtr(In_2 + 1) && colPtr(In_2) < In_4 && In_6 < colPtr(rowIdx(In_4) + 1) && In_8 < colPtr(In_2 + 1) && Out_2 + 1 < n && Out_4 < colPtr(Out_2 + 1) && colPtr(Out_2) < Out_4 && Out_6 < colPtr(rowIdx(Out_4) + 1) && Out_8 < colPtr(Out_2 + 1) }");

  // Testing r1->setRelationship(r2) 
  EXPECT_EQ( iegenlib::SetEqual, r1->dataDependenceRelationship(r2,1) );

  delete r1;
  delete r2;
}
