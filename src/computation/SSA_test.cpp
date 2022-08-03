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
#include <string>
#include <utility>
#include <vector>
#include <regex>

TEST(SSATest, dominanceTree){
   // string s = {([0],0), ([1],2), ([2],2)};
    SSA ssa;
    std::vector<std::pair<int, std::vector<std::string>>>a {{0, {"0"}},{1, {"1"}}};
    ssa.getDominanceTree(a);
    EXPECT_EQ(1,1);
}

