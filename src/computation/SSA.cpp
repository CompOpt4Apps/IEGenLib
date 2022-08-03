/*!
 * \file SSA.cpp
 *
 * \brief Implementation of the SSA class
 *
 * The SSA class is the class that gives us the dominance graph.
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
#include <vector>
#include <tuple>
#include "set_relation/set_relation.h"
#include <iostream>

SSA::SSA() {}

SSA::~SSA() {
}

std::vector<SSA::dominanceTree> SSA::getDominanceTree(std::vector<std::pair<int, iegenlib::Set*>>& executionS ) {

    std::cout  << executionS[1].first;
    return {};
}
