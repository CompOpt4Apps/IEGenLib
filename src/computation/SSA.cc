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
#include "Computation.h"
#include <utility>
#include <string>

namespace iegenlib{
    class Stmt;
    class Computation;
}
using namespace SSA;
using namespace iegenlib;
ScheduleTree::ScheduleTree() {}

ScheduleTree::~ScheduleTree() {
}

std::vector<Set*> ScheduleTree::getPrefixes(Set*s) {

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

void ScheduleTree::createScheduleTree(iegenlib:: Computation* comp){

    std::vector<Stmt*> stmts  ;
    for( int a=0;a<comp->getNumStmts();a++){
        stmts.push_back(comp->getStmt(a));
    }
    std::vector<iegenlib::Set*> executionS;
    std::map<string, std::vector<int>> globals;

    for(int i=0; i<stmts.size(); i++){
        iegenlib::Set* s1 = stmts[i]->getExecutionSchedule()->Apply( stmts[i]->getIterationSpace());
        std::cout << s1->prettyPrintString()<<'\n';
    }


}
void SSA::generateSSA(iegenlib::Computation *comp) {
    ScheduleTree * s = new ScheduleTree();
    s->createScheduleTree(comp);
}
