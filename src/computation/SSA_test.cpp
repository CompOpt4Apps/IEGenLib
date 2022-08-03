/*!
 * \file SSA_test.cc
 *
 * \brief Tests for the SSA class.
 *
 * \date Started: 08/02/22
 *
 * \authors Kalyan Bhetwal
 *
 * Copyright (c) 2020, Boise State University <br>
 * All rights reserved. <br>
 * See ../../COPYING for details. <br>
 */
#include "SSA.h"
#include <gtest/gtest.h>
#include "set_relation/set_relation.h"
#include <string>
#include <utility>
#include <vector>
#include <regex>
using namespace SSA;

TEST(SSATest, dominanceTree){
   // string s = {([0],0), ([1],2), ([2],2)};
    iegenlib::Set* s1 = new iegenlib::Set("{[0,i,1] : 0 <=i< N}");
    iegenlib::Set* s2 = new iegenlib::Set("{[0,j,1] : 0 <=j< M}");
    std::vector<std::pair<int, iegenlib::Set*>> executionS {{0, s1}, {1, s2}};
    DominanceTree* dt;
    dt->DominanceTree::createDominanceTree(executionS);
    DominanceTree edt;
    int p1 = edt.push_back({0, s1});
    int p2 =  edt.push_back({1, s2});
    edt.add_edge(p1,p2);
    EXPECT_TRUE(edt.equivalent(*dt));

}

