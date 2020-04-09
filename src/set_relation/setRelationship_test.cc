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
  Relation *r1 = new Relation("{ [i,m,k,l] -> [ip] : k = colPtr(ip) && i < ip && 0 <= i < n-1 && "
             "0 <= ip < n-1 && colPtr(i)+1 <= m < colPtr(i+1) && "
             "colPtr(rowIdx(m)) <= k < colPtr(rowIdx(m)+1) && "
             "m <= l < colPtr(i+1) && rowIdx(l+1) <= rowIdx(k) "
             "&& rowIdx(l) = rowIdx(k) }");

  // r2: [In Incomplete Cholesky 0 ]  a write in S1 (val[colPtr[i]]);  a read in S3 (val[k]); (Anti dependence)
  Relation *r2 = new Relation("{ [ip,mp,kp,lp] -> [i] : kp = colPtr(i) && ip < i && 0 <= i < n-1 && "
             "0 <= ip < n-1 && colPtr(ip)+1 <= mp < colPtr(ip+1) && "
             "colPtr(rowIdx(mp)) <= kp < colPtr(rowIdx(mp)+1) && "
             "mp <= lp < colPtr(ip+1) && rowIdx(lp+1) <= rowIdx(kp) && "
             "rowIdx(lp) = rowIdx(kp) }");

  // The reason we can expect these two dependences to be SetEqual is following:
  // First let us change the name of tuple variables into general names 
  // as only their ordering is important for forming the polyhedral space 
  // (and for our purposes determining relationsship between two polehedral):

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

  delete r1;
  delete r2;


  // Exact same reason for following two sets being equal as the earlier case applies here.
  // r1: [In Incomplete Cholesky 0 ]  a read in S3 (val[k]);  a write in S3 (val[k]); (Flow dependence)
  r1 = new Relation("{ [i,m,k,l] -> [ip,mp,kp,lp] : k = colPtr(ip) && i < ip && 0 <= i < n-1 && "
             "0 <= ip < n-1 && colPtr(i)+1 <= m < colPtr(i+1) && "
             "colPtr(rowIdx(m)) <= k < colPtr(rowIdx(m)+1) && "
             "m <= l < colPtr(i+1) && rowIdx(l+1) <= rowIdx(k) "
             "&& rowIdx(l) = rowIdx(k) "
             "&& colPtr(ip)+1 <= mp < colPtr(ip+1) && "
             "colPtr(rowIdx(mp)) <= kp < colPtr(rowIdx(mp)+1) && "
             "mp <= lp < colPtr(ip+1) && rowIdx(lp+1) <= rowIdx(kp) "
             "&& rowIdx(lp) = rowIdx(kp)}");

  // r2: [In Incomplete Cholesky 0 ]  a read in S3 (val[k]);  a write in S3 (val[k]); (Anti dependence)
  r2 = new Relation("{ [ip,mp,kp,lp] -> [i,m,k,l] : colPtr(i) = kp && ip < i && 0 <= i < n-1 && "
             "0 <= ip < n-1 && colPtr(i)+1 <= m < colPtr(i+1) && "
             "colPtr(rowIdx(m)) <= k < colPtr(rowIdx(m)+1) && "
             "m <= l < colPtr(i+1) && rowIdx(l+1) <= rowIdx(k) "
             "&& rowIdx(l) = rowIdx(k) "
             "&& colPtr(ip)+1 <= mp < colPtr(ip+1) && "
             "colPtr(rowIdx(mp)) <= kp < colPtr(rowIdx(mp)+1) && "
             "mp <= lp < colPtr(ip+1) && rowIdx(lp+1) <= rowIdx(kp) "
             "&& rowIdx(lp) = rowIdx(kp)}");

  // Testing r1->dataDependenceRelationship(r2) 
  EXPECT_EQ( iegenlib::SetEqual, r1->dataDependenceRelationship(r2,0) );

  delete r1;
  delete r2;
}


#pragma mark setTrivialSuperSetTEST
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

  // r1:  [In Incomplete Cholesky 0 ] a read in S1 (val[colPtr[i]]);  a write in S3 (val[k]); (Flow dependence)
  Relation *r1 = new Relation("{ [i,m,k,l] -> [ip] : k = colPtr(ip) && i < ip && 0 <= i < n-1 && "
             "0 <= ip < n-1 && colPtr(i)+1 <= m < colPtr(i+1) && "
             "colPtr(rowIdx(m)) <= k < colPtr(rowIdx(m)+1) && "
             "m <= l < colPtr(i+1) && rowIdx(l+1) <= rowIdx(k) "
             "&& rowIdx(l) = rowIdx(k) }");

  // r2:  [In Incomplete Cholesky 0 ] a read in S2 (val[colPtr[i]]);  a write in S3 (val[k]); (Flow dependence)
  Relation *r2 = new Relation("{ [ip,mp,kp,lp] -> [i,m] : kp = colPtr(i) && ip < i && 0 <= i < n-1 && "
             "0 <= ip < n-1 && colPtr(ip)+1 <= mp < colPtr(ip+1) && "
             "colPtr(rowIdx(mp)) <= kp < colPtr(rowIdx(mp)+1) && "
             "mp <= lp < colPtr(ip+1) && rowIdx(lp+1) <= rowIdx(kp) && "
             "rowIdx(lp) = rowIdx(kp) && colPtr(i)+1 <= m < colPtr(i+1)}");

  // Since r2 has all the constraints of the r1, plus some other constraints, in this case 
  // we say that r1 is SuperSet of r2. To make this more clear,
  // first let us change the name of tuple variables into general names 
  // as only their ordering is important for forming the polyhedral space 
  // (and for our purposes determining relationsship between two polehedral):

  // 
  // r1 = { [tv1,tv2,tv3,tv4] -> [tv5] : tv3 = colPtr(tv5) && tv1 < tv5 && 0 <= tv1,tv5 < n-1 &&
  //         colPtr(tv1)+1 <= tv2 < colPtr(tv1+1) &&
  //         colPtr(rowIdx(tv2)) <= tv3 < colPtr(rowIdx(tv2)+1) && 
  //         tv2 <= tv4 < colPtr(tv1+1) && rowIdx(tv4+1) <= rowIdx(tv3) 
  //         && rowIdx(tv4) = rowIdx(tv3) }
  // r2 = { [tv1,tv2,tv3,tv4] -> [tv5,tv6] : tv3 = colPtr(tv5) && tv1 < tv5 && 0 <= tv1,tv5 < n-1 &&
  //         colPtr(tv1)+1 <= tv2 < colPtr(tv1+1) &&
  //         colPtr(rowIdx(tv2)) <= tv3 < colPtr(rowIdx(tv2)+1) && 
  //         tv2 <= tv4 < colPtr(tv1+1) && rowIdx(tv4+1) <= rowIdx(tv3) 
  //         && rowIdx(tv4) = rowIdx(tv3)
  //         && colPtr(tv5)+1 <= tv6 < colPtr(tv5+1)}
  //
  //   r2 has the constraints: "colPtr(tv5)+1 <= tv6 < colPtr(tv5+1)" that r1 does not have.
  EXPECT_EQ( iegenlib::SuperSet, r1->dataDependenceRelationship(r2,0) );

  delete r1;
  delete r2;

}


#pragma mark setTrivialSubSetTEST
TEST(setRelationshipTest, setTrivialSubSetTEST){

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

  // r1: [In Incomplete Cholesky 0 ] a read in S3 (val[m]);  a write in S3 (val[k]); (Flow dependence)
  Relation *r1 = new Relation("{ [i,m,k,l] -> [ip,mp,kp,lp] : k = mp && i < ip && 0 <= i < n-1 && "
             "0 <= ip < n-1 && colPtr(i)+1 <= m < colPtr(i+1) && "
             "colPtr(rowIdx(m)) <= k < colPtr(rowIdx(m)+1) && "
             "m <= l < colPtr(i+1) && rowIdx(l+1) <= rowIdx(k) "
             "&& rowIdx(l) = rowIdx(k) "
             "&& colPtr(ip)+1 <= mp < colPtr(ip+1) && "
             "colPtr(rowIdx(mp)) <= kp < colPtr(rowIdx(mp)+1) && "
             "mp <= lp < colPtr(ip+1) && rowIdx(lp+1) <= rowIdx(kp) "
             "&& rowIdx(lp) = rowIdx(kp)}");

  // r1: [In Incomplete Cholesky 0 ] a write in S1 (val[m]);  a read in S3 (val[k]); (Anti dependence)
  Relation *r2 = new Relation("{ [ip,mp,kp,lp] -> [i,m] : kp = m && ip < i && 0 <= i < n-1 && "
             "0 <= ip < n-1 && colPtr(ip)+1 <= mp < colPtr(ip+1) && "
             "colPtr(rowIdx(mp)) <= kp < colPtr(rowIdx(mp)+1) && "
             "mp <= lp < colPtr(ip+1) && rowIdx(lp+1) <= rowIdx(kp) && "
             "rowIdx(lp) = rowIdx(kp) && colPtr(i)+1 <= m < colPtr(i+1)}");


  // r1 has all the constraints of the r2, plus some other constraints, in this case 
  // we say that r1 is SubSetEqual of r2, which is trivially to see.
  EXPECT_EQ( iegenlib::SubSet, r1->dataDependenceRelationship(r2,0) );

  delete r1;
  delete r2;
}


#pragma mark setComplicatedSetEqualityTEST
TEST(setRelationshipTest, setComplicatedSetEqualityTEST){

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

  // r1: [In Incomplete Cholesky 0 ] a read in S3 (val[m]);  a write in S3 (val[k]); (Flow dependence)
  Relation *r1 = new Relation("{ [i,m,k,l] -> [ip,mp,kp,lp] :        k = mp "
             "&& i < ip && 0 <= i < n-1 && "
             "0 <= ip < n-1 && colPtr(i)+1 <= m < colPtr(i+1) && "
             "colPtr(rowIdx(m)) <= k < colPtr(rowIdx(m)+1) && "
             "m <= l < colPtr(i+1) && rowIdx(l+1) <= rowIdx(k) "
             "&& rowIdx(l) = rowIdx(k) "
             "&& colPtr(ip)+1 <= mp < colPtr(ip+1) && "
             "colPtr(rowIdx(mp)) <= kp < colPtr(rowIdx(mp)+1) && "
             "mp <= lp < colPtr(ip+1) && rowIdx(lp+1) <= rowIdx(kp) "
             "&& rowIdx(lp) = rowIdx(kp)}");

  // r1: [In Incomplete Cholesky 0 ] a read in S3 (val[l]);  a write in S3 (val[k]); (Flow dependence)
  Relation *r2 = new Relation("{ [i,m,k,l] -> [ip,mp,kp,lp] :        k = lp "
             "&& i < ip && 0 <= i < n-1 && "
             "0 <= ip < n-1 && colPtr(i)+1 <= m < colPtr(i+1) && "
             "colPtr(rowIdx(m)) <= k < colPtr(rowIdx(m)+1) && "
             "m <= l < colPtr(i+1) && rowIdx(l+1) <= rowIdx(k) "
             "&& rowIdx(l) = rowIdx(k) "
             "&& colPtr(ip)+1 <= mp < colPtr(ip+1) && "
             "colPtr(rowIdx(mp)) <= kp < colPtr(rowIdx(mp)+1) && "
             "mp <= lp < colPtr(ip+1) && rowIdx(lp+1) <= rowIdx(kp) "
             "&& rowIdx(lp) = rowIdx(kp)}");


  // Intuitive reason as to why r1 and r2 are equal:
  // r2 has all the constraints of the r1, except for one different equality" r1 has "k = mp" and r2 has "k = lp"
  // In this case we look into that different equality, since one side of the equalities is the same variable, namely k,
  // we can consider the different sides, mp Vs. lp, to determine r1 and r2 relationship.
  // And since mp and lp variables have the same domain (colPtr(ip)+1 <= mp <= lp < colPtr(ip+1)),
  // we can see that r1 and r2 are equal.

  EXPECT_EQ( iegenlib::SetEqual, r1->dataDependenceRelationship(r2,0) );

  delete r1;
  delete r2;
}
