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
class DominanceTree {
private:
    struct Node{
        int parent; // parent to the node
        std::vector<int>children;  // list of the Children's
        std::vector<int> predecessors{}; // list of the dominators for particular node
        std::pair<int, iegenlib::Set*> data; // iteration domain with stmtIdx as key
        std::vector<int>dominanceFrontier;
        std::pair<std::string, std:: vector<int>> phis{};
        std::vector<int> successors{}; // list of successor for particular node
    };
    std::vector<Node> nodes;
public:
    DominanceTree();
    ~DominanceTree();
    int push_Back(std::pair<int,iegenlib::Set*>);
    void add_edge(int parent , int child);
    bool equivalent(DominanceTree);
    int getVectorSize();
    void add_predecessors(int i, int j);
    Set* getElem(int i);
    std::vector<int> getPredecessor(int i);
    bool predecessorEquivalent(DominanceTree dt);
    bool isParent(int parent, int child);
    std::vector<Set*>getPrefixes(Set*s);
    void DFCal();
    void insertPhiNode(std::map<string, std::vector<int>>, Computation* comp );
    void SSARenaming( std::vector<std::map<string, std::vector<int>> >phi_nodes,  Computation* comp);
    string rename( std::map<string, int> &counter, std::map<string, int> &stack,string n);
};
    bool isDominator(iegenlib::Set* parent, iegenlib::Set* child);
    bool isReverseDominator(iegenlib::Set * s1, iegenlib::Set * s2);
    DominanceTree* createDominanceTree(std::vector<iegenlib::Set*> executionS);
    DominanceTree* findPredecessors(DominanceTree* dt);
    void generateSSA(Computation * comp);

};
#endif
