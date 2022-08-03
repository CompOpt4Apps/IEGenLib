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

    std::sort( executionS.begin(), executionS.end(), [] (const std::pair<int, iegenlib::Set*> &a, const std::pair<int, iegenlib::Set*> &b){return a.second->getTupleDecl() < b.second->getTupleDecl();});
    for( auto v: executionS){
        std::cout<< v.first<<'\n';
        std::cout<< v.second-> prettyPrintString()<<'\n';


    }
//    std::cout  << executionS[1].first;
    return {};
}
