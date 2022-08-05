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
using namespace iegenlib;
DominanceTree::DominanceTree() {}

DominanceTree::~DominanceTree() {
}

int DominanceTree::push_Back(std::pair<int, iegenlib::Set*> data) {
    Node* node = new Node();
    node->parent= -1;
    node->children = {};
    node->data = data;
    nodes.push_back(*node);
    return(nodes.size()-1);
}

void DominanceTree::add_edge(int parent, int child) {
    nodes[parent].children.push_back(child);
    nodes[child].parent = parent;
}

bool DominanceTree::equivalent(DominanceTree) {
    return true;
}

bool SSA::isDominator(iegenlib::Set * parent, iegenlib::Set * child){
    std::cout<< child -> prettyPrintString()<<'\n';
    std::cout << parent -> prettyPrintString() <<'\n';
    if( parent->getTupleDecl().getSize() > child->getTupleDecl().getSize()){
        return  false;
    }
    Set* childN = child-> projectOutConst(child);

    return false;
}

DominanceTree* SSA::createDominanceTree(std::vector<std::pair<int, iegenlib::Set *>>executionS) {
    DominanceTree * rval = new DominanceTree();

    std::sort(executionS.begin(), executionS.end(), []
            (const std::pair<int, iegenlib::Set *> &a, const std::pair<int, iegenlib::Set *> &b) {
        Set* s;
        bool status = s->Set::LexiSort(a.second, b.second);
        return(status);
        delete s;
    });

    for (auto v: executionS) {
        rval->push_Back(v);
    }
    for (int i = executionS.size() - 1; i >= 0; i--) {
        for (int j = i-1; j >= 0; j--) {
            bool isDominator1 = SSA::isDominator(executionS[j].second, executionS[i].second);
            if(isDominator1){
                rval -> add_edge(j,i);
            }
        }
    }
    return rval;
}

//
//    for( std::vector<std::pair<int, iegenlib::Set*>>::reverse_iterator it = executionS.rbegin(); it != executionS.rend(); ++it ){
//
//
    //        auto tupl = it->second->getTupleDecl();
//        for( std::vector<std::pair<int, iegenlib::Set*>>::reverse_iterator it1 = executionS.rbegin(); it1 != executionS.rend(); ++it1 ){
//            if(tupl.getSize()==it1->second->getTupleDecl().getSize()){
//                std::cout<< it1->first<<'\n';
//                std::cout<< it1->second-> prettyPrintString()<<'\n';
//                std::string a =  it1->second->getTupleDecl().toString();
//                std::cout << "this is test  " << a<<'\n';
//               this->DominanceTree::add_edge(p1, p2);
//                break;
//            }
//        }
//        break;
//    }
