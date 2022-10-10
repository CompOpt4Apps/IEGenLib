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

std::vector<Set*> SSA::getPrefixes(Set*s) {

    std::vector<Set*>v;
    Set* res = new Set(*s);

    TupleDecl tl = s->getTupleDecl();
    v.push_back(res);

    if( tl.size()==1 ){
        return(v);
    }

    for(int i= tl.size()-1; i>0 ;i--) {
        res = res->projectOut(i);
        v.push_back(res);
    }
    return v;
}

 SSA::Node* SSA::createScheduleTree(iegenlib:: Computation* comp){

    std::vector<Stmt*> stmts  ;

    Node * rootNode = new Node();
    rootNode->setOrdered( true);
    rootNode->setCommonArity(1);

    // remove this for loop
    for( int a=0;a<comp->getNumStmts();a++){
        stmts.push_back(comp->getStmt(a));
    }
    std::vector<Set> processedList;

    for(int i=0; i<stmts.size(); i++){
        iegenlib::Set* s1 = stmts[i]->getExecutionSchedule()->Apply( stmts[i]->getIterationSpace());
        //std::cout << s1->prettyPrintString()<<'\n';
        std::vector<Set*>v;
        v = getPrefixes(s1);
        SSA::Node * current = rootNode;

        for(int j= v.size()-1;j>=0;j--){
            //std:: cout << "prefixes " << (*v[j]).prettyPrintString()<<'\n';
//            if ( std::find(processedList.begin(), processedList.end(), (*v[j])) != processedList.end() ){
//                continue;
//            }
//            processedList.push_back((*v[j]));

            SSA::Member * m;
            if ( j ==0){
                m = new SSA::Member(v[j], stmts[i]);
            }
            else{
                m = new SSA::Member(v[j], NULL);
            }
            current =  current->insert(m);
        }
    }
    return rootNode;
}

SSA::Node::Node(){
    members = {};
}

SSA::Member::Member(Set * s, Stmt * st) {
    schedule = s;
    child = new SSA::Node();
    stmt = st;
}

Set *Member::getSchedule() const {
    return schedule;
}

void Member::setSchedule(Set *schedule) {
    Member::schedule = schedule;
}

Stmt *Member::getStmt() const {
    return stmt;
}

void Member::setStmt(Stmt *stmt) {
    Member::stmt = stmt;
}

Node *Member::getChild() const {
    return child;
}

void SSA::Member::setChild(SSA::Node *child) {
    child = child;
}

 std::pair<SSA::Node *, SSA::Member*> &SSA::Member::getParent() {
    return parent;
}

void Member::setParent(Node * n, Member* s) {
    parent = std::make_pair(n,s);
}


Computation* SSA::generateSSA(iegenlib::Computation *comp) {
    Node * node = createScheduleTree(comp);

    /// perform further operation
    //
    
    return comp;
}

SSA::Node* SSA::Node::insert(SSA::Member * m){
//
    if(m->getSchedule()->getArity() != common_arity){
        return NULL;
    }
    if(ordered){
        for(auto current=members.begin(); current!=members.end(); ++current ){
            //current = members[i];
            if((*(*current)->getSchedule())== (*m->getSchedule())){
                return (*current)->getChild();
            }
        }
        for(auto current=members.begin(); current!=members.end(); ++current ){
            if(!((*current)->getSchedule())->LexiLess(m->getSchedule())){
                members.emplace(current, m);
                m->setParent(this, m);
                m->getChild()->setCommonArity(getCommonArity()+1);
                return m->getChild();
            }
            members.push_back(m);
            m->getChild()->setCommonArity(getCommonArity()+1);
            m->setParent(this,m );
            return m->getChild();
        }
    }
    for(auto current=members.begin(); current!=members.end();current++ ){
        if((*(*current)->getSchedule())== (*m->getSchedule())){
            return (*current)->getChild();
        }
    }
    members.push_back(m);
    m->getChild()->setCommonArity(getCommonArity()+1);
    m->setParent(this,m ); // parent == null
    return m->getChild();
}

bool SSA::Node::isOrdered() const {
    return ordered;
}

void SSA::Node::setOrdered(bool ordered) {
    Node::ordered = ordered;
}

int SSA::Node::getCommonArity() const {
    return common_arity;
}

void SSA::Node::setCommonArity(int commonArity) {
    common_arity = commonArity;
}

void SSA::Node::printBreadthFirst() {
    for(auto it=members.begin(); it!=members.end();it++){
        (*it)->printBreadthFirst();
    }
        std::cout << "------------------"<<'\n';
}

void Node::setMembers(const std::vector<Member *> &members) {
    Node::members = members;
}

void SSA::Member::printBreadthFirst() {
    std::cout << schedule->prettyPrintString()<<'\n';
    child->printBreadthFirst();
}

std::vector<Stmt> SSA::pred_and_dom(Node* n, int idx) {

    std::vector<Stmt> listOfStatements{};
    return listOfStatements;
//    int i;
//    for(i = idx; i>=0 ;i--) {
//        if (n[i] == statement) {
//            listOfStatements.push_back(n[i]->);
//            return listOfStatements;
//        }
//
//        for (auto c: n[i].getMembers()){
//            listOfStatements.push_back(pred_and_dom(c,c->.size()-1));
//        }
//
//        if(i==-1){
//            listOfStatements.push_back(pred_and_dom(n.parent,n.parent.index);
//        }
//
//
//    }

}

string SSA::rename( std::map<string, int> &counter, std::map<string, int> &stack,string n ){
    int i = counter[n];
    counter[n] = counter[n]+1;
    stack[n] = i;
    n.erase(n.begin());
    n.erase(n.end()-1);
    //std:: cout << "    "<<n <<'\n';
    return n + std::to_string(i);
    //return n.insert( n.length()-1, "1" ) ;//+ std::to_string(i);

}