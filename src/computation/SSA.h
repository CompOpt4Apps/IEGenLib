/*!
 * \file SSA.h
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

#ifndef SSA_H_
#define SSA_H_

#include <vector>
#include <tuple>
#include <iostream>
#include "set_relation/set_relation.h"
class SSA {
public:
    SSA();
    virtual ~SSA();
    struct dominanceTree{
        dominanceTree * parent; // parent to the node
        std::vector<dominanceTree*>child;  // list of the Children's
        std::pair<int, std::vector<std::string>> data; // iteration domain with stmtIdx as key
    };
    dominanceTree** getDominanceTree(std::vector<std::pair<int,iegenlib::Set*>>&executionS);
};

#endif
