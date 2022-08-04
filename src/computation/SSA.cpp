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
using namespace SSA;
DominanceTree::DominanceTree() {}

DominanceTree::~DominanceTree() {
}

int DominanceTree::push_back(std::pair<int, iegenlib::Set*> data) {
    Node node;
    node.parent= -1;
    node.children = {};
    node.data = data;
    nodes.push_back(node);
    return(nodes.size()-1);
}

void DominanceTree::add_edge(int parent, int child) {
    nodes[parent].children.push_back(child);
    nodes[child].parent = parent;
}

bool DominanceTree::equivalent(DominanceTree) {
    return true;
}

void DominanceTree::createDominanceTree( std::vector<std::pair<int, iegenlib::Set*>> executionS){
    std::sort( executionS.begin(), executionS.end(), []
    (const std::pair<int, iegenlib::Set*> &a, const std::pair<int, iegenlib::Set*> &b)
    {return a.second->getTupleDecl() < b.second->getTupleDecl();});


    for( std::vector<std::pair<int, iegenlib::Set*>>::reverse_iterator it = executionS.rbegin(); it != executionS.rend(); ++it ){


        auto tupl = it->second->getTupleDecl();
        for( std::vector<std::pair<int, iegenlib::Set*>>::reverse_iterator it1 = executionS.rbegin(); it1 != executionS.rend(); ++it1 ){
            if(tupl.getSize()==it1->second->getTupleDecl().getSize()){
                std::cout<< it1->first<<'\n';
                std::cout<< it1->second-> prettyPrintString()<<'\n';
                std::string a =  it1->second->getTupleDecl().toString();
                std::cout << "this is test  " << a<<'\n';
                int p1 = this->DominanceTree::push_back(*it1);
                int p2 = this-> DominanceTree::push_back(*it);
//                this->DominanceTree::add_edge(p1, p2);
                break;
            }
        }
        break;
    }
}