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
    if( tl.size()==1 ){
        return v;
    }
    for(int i= tl.size()-1; i>0 ;i--) {
        res = res->projectOut(i);
        v.push_back(res);
    }
    //std::cout << "print "<< s->prettyPrintString() <<'\n';
    return v;
}
ScheduleTree::member* ScheduleTree::create_member(Set* S, Stmt* st) {
    member* m = new member();
    m->schedule = S;
    m->child = new Node();
    m->stmt = st;
    return m;
}
void ScheduleTree::createScheduleTree(iegenlib:: Computation* comp){

    std::vector<Stmt*> stmts  ;

    Node * schedTree = new Node();
    schedTree->ordered = true;

    for( int a=0;a<comp->getNumStmts();a++){
        stmts.push_back(comp->getStmt(a));
    }
    std::vector<iegenlib::Set*> executionS;
    std::map<string, std::vector<int>> globals;

    for(int i=0; i<stmts.size(); i++){
        iegenlib::Set* s1 = stmts[i]->getExecutionSchedule()->Apply( stmts[i]->getIterationSpace());
        //std::cout << s1->prettyPrintString()<<'\n';
        std::vector<Set*>v;
        v = getPrefixes(s1);

//        for(auto s: v){
//            std::cout << s->prettyPrintString()<<'\n';
//        }
//        std::cout << "-------------------------------"<<'\n';

        for(int j= 0;j<v.size();j++){
             create_member(v[j],  stmts[i]);
        }


    }


}
void SSA::generateSSA(iegenlib::Computation *comp) {
    ScheduleTree * s = new ScheduleTree();
    s->createScheduleTree(comp);
}

void insert(struct member* m){
//
//    if(m.set.arity != common_arity)
//            ERROR
//            If (ordered){
//                Foreach em in members
//                If em == m
//                Return em.child
//                If em lexicogreater m
//                members.insertBefore em,m
//                M.parent = ?????
//                Return m.child
//            }
//        Members.push_back m;
//        Return m.child;
//    }
//    // this is not an ordered list
//    Foreach em in members
//    If em == m
//            Return m.child
//    }
//    Members.push_back m
//    Return m.child

}