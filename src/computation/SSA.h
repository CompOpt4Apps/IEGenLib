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
#include <utility>
#include <unordered_set>
namespace iegenlib{
    class Stmt;
    class Computation;
}
using namespace iegenlib;

namespace SSA{
class ScheduleTree {
private:
    struct Node{
        bool ordered;
        int common_arity;
        std::vector<struct  member>members;
    };
    struct member{
        Set* schedule;
        Stmt* stmt;
        Node * child;
        std::pair<Node*,int> parent;
    };
public:
    ScheduleTree();
    ~ScheduleTree();
    void createScheduleTree( Computation* Comp);
    std::vector<Set*> getPrefixes(Set*s);

};
    void generateSSA(Computation * comp);

};
#endif
