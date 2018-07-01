/*!
 * \file complexityForPartialParallel_test.cc
 *
 * \brief complexity For Partial Parallel tests.
 *
 * \date Started: 2/5/18
 *
 * \authors Michelle Strout, Mahdi Soltan Mohammadi
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

// set and relation classes
using iegenlib::Conjunction;
using iegenlib::SparseConstraints;
using iegenlib::Set;
using iegenlib::Relation;

#pragma mark complexityForPartialParallelTEST
TEST(complexityTest, complexityForPartialParallelTEST){

  string complexity = "";  

  // Introducing UFSymbols
  iegenlib::setCurrEnv();
  iegenlib::appendCurrEnv("colidx",
          new Set("{[i]:0<=i &&i<nnz}"),         // Domain 
          new Set("{[j]:0<=j &&j<m}"),           // Range
          false,                                 // Not bijective.
          iegenlib::Monotonic_NONE               // no monotonicity
          );
  iegenlib::appendCurrEnv("rowptr",
      new Set("{[i]:0<=i &&i<m}"), 
      new Set("{[j]:0<=j &&j<nnz}"), false, iegenlib::Monotonic_Increasing);
  iegenlib::appendCurrEnv("diagptr",
      new Set("{[i]:0<=i &&i<m}"), 
      new Set("{[j]:0<=j &&j<nnz}"), false, iegenlib::Monotonic_Increasing);

  
  // We need to define which iterators are meant for parallelization
  // therefore those iterators must not considered projectable. In other
  // words the assumption is that those iterators must be in final inspector
  // and hence we must count their complexity regardless of whatever else.
  std::set<int> parallelTvs;


  // DIFFERENT examples:


  // 1: No iterator can be projected, and there is no useful equalities
  Set *s = new Set("[n] -> {[i,ip,k,kp]: i < ip"
                                 " && 0 <= i && i < n"
                                " && 0 <= ip && ip < n"
                         " && rowptr(i) <= k && k < diagptr(i)"
                       " && rowptr(ip) <= kp && kp < diagptr(ip)"
                               " && colidx(k) = colidx(kp) }");
  parallelTvs.insert( 0 ); // i
  parallelTvs.insert( 1 ); // ip

  complexity = s->complexityForPartialParallel(parallelTvs);

  // {i, ip, k, kp} === (n)*(n)*(nnz/n)*(nnz/n) = nnz^2
  EXPECT_EQ( std::string("O(nnz^2)") , complexity );

  delete s;


  // 2: No iterator can be projected, but we have an useful equality
  s = new Set("[n] -> {[i,ip,k,kp]: i < ip"
                               " && 0 <= i && i < n"
                              " && 0 <= ip && ip < n"
                       " && rowptr(i) <= k && k < diagptr(i)"
                     " && rowptr(ip) <= kp && kp < diagptr(ip)"
                             " && colidx(k) = colidx(kp)"
                                      " && k = kp}");
  parallelTvs.clear();
  parallelTvs.insert( 0 ); // i
  parallelTvs.insert( 1 ); // ip

  complexity = s->complexityForPartialParallel(parallelTvs);

  // {i, ip, k} === (n)*(n)*(nnz/n) = n*nnz;   kp can be produced using kp = k
  EXPECT_EQ( std::string("O(n^1*nnz^1)") , complexity );

  delete s;


  // 3: k and kp can be projected, equality becomes irrelevant
  Relation *r = new Relation("[n] -> {[i,k] -> [ip,kp]: i < ip"
                               " && 0 <= i && i < n"
                              " && 0 <= ip && ip < n"
                       " && rowptr(i) <= k && k < diagptr(i)"
                     " && rowptr(ip) <= kp && kp < diagptr(ip)"
                                      " && k = kp}");
  parallelTvs.clear();
  parallelTvs.insert( 0 ); // i
  parallelTvs.insert( 2 ); // ip

  complexity = r->complexityForPartialParallel(parallelTvs);

  // {i, ip} === (n)*(n) = n^2;
  EXPECT_EQ( std::string("O(n^2)") , complexity );

  delete r;

  // 4: k can be projected, and i = colidx(kp) can be used to get i from kp
  r = new Relation("[n] -> {[i,k] -> [ip,kp]: i < ip"
                               " && 0 <= i && i < n"
                              " && 0 <= ip && ip < n"
                       " && rowptr(i) <= k && k < diagptr(i)"
                     " && rowptr(ip) <= kp && kp < diagptr(ip)"
                                    " && i = colidx(kp) }");
  parallelTvs.clear();
  parallelTvs.insert( 0 ); // i
  parallelTvs.insert( 2 ); // ip

  complexity = r->complexityForPartialParallel(parallelTvs);

  // {ip, kp} === (n)*(nnz/n) = nnz;
  EXPECT_EQ( std::string("O(nnz^1)") , complexity );

  delete r;



  // 4: k can be projected, and i = colidx(kp) can be used to get i from kp
  iegenlib::appendCurrEnv("rowIdx",
          new Set("{[i]:0<=i &&i<nnz}"),         // Domain 
          new Set("{[j]:0<=j &&j<m}"),           // Range
          false,                                 // Not bijective.
          iegenlib::Monotonic_NONE               // no monotonicity
          );
  iegenlib::appendCurrEnv("colPtr",
      new Set("{[i]:0<=i &&i<m}"), 
      new Set("{[j]:0<=j &&j<nnz}"), false, iegenlib::Monotonic_Increasing);
  r = new Relation("{ [In_2, In_4, In_6, In_8] -> [Out_2, Out_4, Out_6, Out_8] : "
"Out_6 = In_6 && rowIdx(In_8) = rowIdx(In_6) && rowIdx(Out_8) = rowIdx(Out_6) && "
"0 <= In_2 && 0 <= In_4 && 0 <= In_6 && 0 <= In_8 && 0 <= Out_2 && 0 <= Out_4 && "
"0 <= Out_8 && 0 <= colPtr(In_2) && 0 <= colPtr(In_2 + 1) && 0 <= colPtr(Out_2) "
"&& 0 <= colPtr(Out_2 + 1) && 0 <= colPtr(rowIdx(In_4)) && 0 <= colPtr(rowIdx(In_4) + 1) "
"&& 0 <= colPtr(rowIdx(Out_4)) && 0 <= colPtr(rowIdx(Out_4) + 1) && 0 <= rowIdx(In_4) "
"&& 0 <= rowIdx(In_6) && 0 <= rowIdx(In_8 + 1) && 0 <= rowIdx(Out_4) && 0 <= rowIdx(Out_6) "
"&& 0 <= rowIdx(Out_8 + 1) && In_4 <= In_8 && colPtr(rowIdx(In_4)) <= In_6 && "
"colPtr(rowIdx(Out_4)) <= In_6 && Out_4 <= Out_8 && rowIdx(In_8 + 1) <= rowIdx(In_6) "
"&& rowIdx(Out_8 + 1) <= rowIdx(Out_6) && In_2 < Out_2 && In_2 + 1 < m && In_2 + 1 < n "
"&& In_4 < nnz && In_4 < colPtr(In_2 + 1) && colPtr(In_2) < In_4 && In_6 < nnz "
"&& In_6 < colPtr(rowIdx(In_4) + 1) && In_6 < colPtr(rowIdx(Out_4) + 1) && In_8 + 1 < nnz "
"&& In_8 < colPtr(In_2 + 1) && Out_2 + 1 < m && Out_2 + 1 < n && Out_4 < nnz "
"&& Out_4 < colPtr(Out_2 + 1) && colPtr(Out_2) < Out_4 && Out_8 + 1 < nnz "
"&& Out_8 < colPtr(Out_2 + 1) && rowIdx(In_4) + 1 < m && rowIdx(In_6) < m "
"&& rowIdx(In_8 + 1) < m && rowIdx(Out_4) + 1 < m && rowIdx(Out_6) < m "
"&& rowIdx(Out_8 + 1) < m && colPtr(In_2) < nnz && colPtr(In_2 + 1) < nnz "
"&& colPtr(Out_2) < nnz && colPtr(Out_2 + 1) < nnz && colPtr(rowIdx(In_4)) < nnz "
"&& colPtr(rowIdx(In_4) + 1) < nnz && colPtr(rowIdx(Out_4)) < nnz "
"&& colPtr(rowIdx(Out_4) + 1) < nnz }");
  parallelTvs.clear();
  parallelTvs.insert( 0 ); // i
  parallelTvs.insert( 4 ); // ip

  complexity = r->complexityForPartialParallel(parallelTvs);

//std::cout<<"\n\n Comp = "<<complexity<<"\n\n"

  // {ip, kp} === (n)*(nnz/n) = nnz;
  EXPECT_EQ( std::string("O(nnz^5/n^3)") , complexity );

  delete r;

}

