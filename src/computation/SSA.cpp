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

bool DominanceTree::equivalent(DominanceTree dt) {
    if(dt.nodes[1].data.second->prettyPrintString()!= this->nodes[1].data.second->prettyPrintString()) return false;
//    for(auto v: dt.nodes[0].children){
//        std:: cout << "dt "<< v <<'\n';
//    }
//
//
//    for(auto v: this->nodes[0].children){
//        std:: cout << "edt " <<  v<<'\n';
//    }
        std::vector<int> v1 = dt.nodes[0].children;
        std::vector<int> v2 = this->nodes[0].children;
        std::sort(v1.begin(),v1.end());
        std::sort(v2.begin(),v2.end() );

    if (v1 !=v2 ){
          return false;
      }
    return true;
}

bool SSA::isDominator(iegenlib::Set * parent, iegenlib::Set * child){
    // project out constant both parent and child
    Set* childP, *parentP;

    childP = child->projectOutConst(child);
    parentP = parent->projectOutConst(parent);


    if( parentP->getArity() > childP->getArity()){
        return  false;
    }
    while( parentP->getArity() < childP->getArity()) {
        TupleDecl tl = child->getTupleDecl();
        childP = childP->projectOut(tl.size()-1);
        //std::cout << childP->prettyPrintString();

    }
    // child subset of parent;
    return (childP->isSubset(parentP));
}

DominanceTree* SSA::createDominanceTree(std::vector<std::pair<int, iegenlib::Set *>>executionS) {
    DominanceTree * rval = new DominanceTree();

    //perform the lexicographical sort

    std::sort(executionS.begin(), executionS.end(), []
            (const std::pair<int, iegenlib::Set *> &a, const std::pair<int, iegenlib::Set *> &b) {
        Set* s;
        bool status = s->Set::LexiSort(a.second, b.second);
        return(status);
        delete s;
    });

    // collect nodes into node list
    for (auto v: executionS) {
        rval ->push_Back(v);
    }
    // set up the relations( parent and child)
    //
    //
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