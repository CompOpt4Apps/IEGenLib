/*!
 * \file SSA_test.cc
 *
 * \brief Tests for the dominanceTree class.
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
#include <utility>
#include <vector>
using namespace SSA;

TEST(SSATest, DominanceTreeTEST){
   // string s = {([0],0), ([1],2), ([2],2)};
    iegenlib::Set* s1 = new iegenlib::Set("{[0,i,0] : 0 <=i< N}");
    iegenlib::Set* s2 = new iegenlib::Set("{[0,i,1] : 0 <=i< N && x >10}");
    iegenlib::Set* s3 = new iegenlib::Set("{[0,i,2] : 0 <=i< N && x <=10}");

    std::vector<std::pair<int, iegenlib::Set*>> executionS {{0, s1}, {1, s2}, {2, s3}};

    DominanceTree* dt = createDominanceTree(executionS);
    DominanceTree edt;
    int p1 = edt.push_Back({0, s1});
    int p2 =  edt.push_Back({1, s2});
    int p3 =  edt.push_Back({2, s3});

    edt.add_edge(p1, p2);
    edt.add_edge(p1, p3);

    //EXPECT_TRUE(edt.equivalent(*dt));

}

TEST(SSATest, IsDominator){
    EXPECT_EQ("", "");
}


