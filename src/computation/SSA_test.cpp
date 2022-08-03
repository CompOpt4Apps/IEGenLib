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

TEST(SSATest, dominanceTree){
   // string s = {([0],0), ([1],2), ([2],2)};
    SSA ssa;
    iegenlib::Set* s1 = new iegenlib::Set("{[i]}");
    iegenlib::Set* s2 = new iegenlib::Set("{[j]}");
    std::vector<std::pair<int, iegenlib::Set*>> executionS {{0, s1}, {1, s2}};
    ssa.getDominanceTree(executionS);
    EXPECT_EQ(1,1);
}

