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
#include <stack>
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
// j is predecessor of i
void DominanceTree::add_predecessors(int i, int j){
    nodes[i].predecessors.push_back(j);
}
int DominanceTree::getVectorSize() {
    return nodes.size();
}
Set* DominanceTree::getElem(int i){
    return(nodes[i].data.second);

}
bool DominanceTree::isParent(int parent, int child) {
   for(auto v:  nodes[parent].children){
       if(v == child){
           return true;
       }
   }
    return  false;
}
std::vector<int> DominanceTree::getPredecessor(int i){
    return(nodes[i].predecessors);
}
bool DominanceTree::equivalent(DominanceTree dt) {

//    std::cout <<"expected "<< dt.nodes[3].data.second->prettyPrintString()<<'\n';
//    std::cout <<"actual "<< this->nodes[3].data.second->prettyPrintString()<<'\n';

 for(int i=0;i<dt.nodes.size();i++) {
     if (dt.nodes[i].data.second->prettyPrintString() != this->nodes[i].data.second->prettyPrintString()) return false;

     std::vector<int> v1 = dt.nodes[i].children;
     std::vector<int> v2 = this->nodes[i].children;
     std::sort(v1.begin(), v1.end());
     std::sort(v2.begin(), v2.end());
     if(v1.size()) continue;

     if (v1 != v2) {
         return false;
     }
 }
    return true;

}
bool DominanceTree::predecessorEquivalent(DominanceTree dt){
for(int i=0;i<dt.getVectorSize();i++) {
    for(int j = 0; j<dt.nodes[i].predecessors.size();j++){
        if(this->nodes[i].predecessors.size()>0 && this->nodes[i].predecessors.size()== dt.nodes[i].predecessors.size()) {
            if (dt.nodes[i].predecessors[j] != this->nodes[i].predecessors[j]) return false;
            if (dt.nodes[i].predecessors[j] != this->nodes[i].predecessors[j]) return false;
        }
        else{ return  false;}
}
}
    return true;
}


bool SSA::isReverseDominator(iegenlib::Set * s1, iegenlib::Set * s2){
    if(s2->getArity()< s1->getArity()){
        return true;
    }
    while( s2->getArity() > s1->getArity()) {

        TupleDecl tl = s2->getTupleDecl();
        // std::cout << "before" <<childP->prettyPrintString()<<'\n';
        // std::cout << parentP->prettyPrintString() <<'\n';
        s2 = s2->projectOut(tl.getSize()-1);
    }
    if (s2->isSubset(s1)) return false;
    return true;

}

// function returns list of prefixes
std::vector<Set*> DominanceTree::getPrefixes(Set*s) {

    std::vector<Set*>v;
    Set* res = new Set(*s);
    TupleDecl tl = s->getTupleDecl();
    if( tl.size()==1 ||  tl.size()==2){
        return v;
    }
    for(int i= tl.size()-1; i>0 ;i--){
        if( tl.elemIsConst(i)) {
            //std::cout<< "the const elem are " << i <<'\n';
            res = res->projectOut(i);
            v.push_back(res);
        }
        else{
            res = res->projectOut(i);
        }
    }
    //std::cout << "print "<< s->prettyPrintString() <<'\n';
    return v;
}


DominanceTree* SSA::findPredecessors(DominanceTree* dt) {

    std::stack<std::pair<Set*, int>> stack;
    for (int i = dt->getVectorSize() -1; i >= 0; i--) {
        for (int j = i-1; j >= 0; j--) {
            bool flag = true;
            for(int k: dt->getPredecessor(i)){
                flag = SSA::isReverseDominator( dt->getElem(k),dt->getElem(j) );
            }
            if(flag){
                dt->add_predecessors(i, j);
            }
            // //
            std::vector<Set*>  v = dt->getPrefixes(dt->getElem(j) );
            for(int i=v.size()-1;i>=0;i--) {
                std::cout << "print2 "<< j<< " " << v[i]->prettyPrintString() << '\n';

                stack.push({v[i], i});
            }
            if(dt->isParent(j,i)) break;
        }
    }
    return dt;
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

        TupleDecl tl = childP->getTupleDecl();
       // std::cout << "before" <<childP->prettyPrintString()<<'\n';
       // std::cout << parentP->prettyPrintString() <<'\n';
        childP = childP->projectOut(tl.getSize()-1);
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
    //  std::cout << "after sort" << v.second->prettyPrintString() <<'\n';
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
                break;
            }
        }
    }
    return rval;
}