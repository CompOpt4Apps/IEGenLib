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
    rootNode->setParent(NULL,NULL);

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

Set *Member::getSchedule()  {
    return schedule;
}

void Member::setSchedule(Set *schedule) {
    Member::schedule = schedule;
}

Stmt *Member::getStmt()  {
    return stmt;
}

void Member::setStmt(Stmt *stmt) {
    Member::stmt = stmt;
}

Node *Member::getChild()  {
    return child;
}

void SSA::Member::setChild(SSA::Node *child) {
    child = child;
}

 std::pair<SSA::Node *, SSA::Member*> &SSA::Node::getParent() {
    return parent;
}

void Node::setParent(Node * n, Member* s) {
    parent = std::make_pair(n,s);
}


Computation* SSA::generateSSA(iegenlib::Computation *comp) {
    Node * node = createScheduleTree(comp);

    node->calc_all_pred();

    //pred_and_dom();

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

            if(!((*current)->getSchedule())->LexiLess(m->getSchedule())){
                members.emplace(current, m);
                m->getChild()->setParent(this, m);
                m->getChild()->setCommonArity(getCommonArity()+1);
                return m->getChild();
            }

        }
        members.push_back(m);
        m->getChild()->setCommonArity(getCommonArity()+1);
        m->getChild()->setParent(this,m );
        return m->getChild();
    }
    for(auto current=members.begin(); current!=members.end();current++ ){
        if((*(*current)->getSchedule())== (*m->getSchedule())){
            return (*current)->getChild();
        }
    }
    members.push_back(m);
    m->getChild()->setCommonArity(getCommonArity()+1);
    m->getChild()->setParent(this,m ); // parent == null
    return m->getChild();
}

bool SSA::Node::isOrdered()  {
    return ordered;
}

void SSA::Node::setOrdered(bool ordered) {
    Node::ordered = ordered;
}

int SSA::Node::getCommonArity()  {
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


void SSA::Node::calc_all_pred() {

    for(auto it=members.begin(); it!=members.end();it++){
        (*it)->calc_all_pred(this);
    }
    std::cout << "------------------"<<'\n';
}

void Node::setMembers( std::vector<Member *> &members) {
    Node::members = members;
}


void SSA::Member::printBreadthFirst() {
    std::cout << schedule->prettyPrintString()<<'\n';
    child->printBreadthFirst();
}
void SSA::Member::calc_all_pred(Node * n){

    if(stmt!=NULL){
        int j;
        for(j=0;j<n->getMembers().size();j++ ){
            if(this==n->getMembers()[j] ){
                break;
            }
        }
        std::vector<Stmt*> stmtList;

        stmtList = pred_and_dom(n, j-1);

        std::cout << "pred for " << schedule->prettyPrintString()<<std::endl;

        for(int i =0;i<stmtList.size();i++){
            std:: cout << " The schedule is " << stmtList[i]->getExecutionSchedule()->prettyPrintString()<<'\n';
        }

    }

    child->calc_all_pred();
}

std::vector<Stmt*> SSA::Member::pred_and_dom(Node* n, int idx) {

    std::vector<Stmt*> listOfStatements{};

    int i;

    for(i = idx; i>=0 ;i--) {
        //this case is for when we hit a dominator

        if (n->getMembers()[i]->getStmt()!=NULL) {
            listOfStatements.push_back(n->getMembers()[i]->getStmt());
            return listOfStatements;
        }
        //this case is for when we are adding predecessors that aren't dominators
        for (auto c:   n->getMembers()[i]->getChild()->getMembers() ){
            std::vector<Stmt*> s;
            s = pred_and_dom(  c->getChild()  , c->getChild()->getMembers().size()-1 ) ;
            listOfStatements.insert(listOfStatements.end(), s.begin(), s.end());
        }
    }
    if(i==-1){
        // this is for the root node
        if(n->getParent().first==NULL){
            return  listOfStatements;
        }
        // stepping up to find the location of the dominator in the member vector
        int j;
        for(j=0;j<n->getParent().first->getMembers().size();j++ ){
            if(n->getParent().second==n->getParent().first->getMembers()[j] ){
                break;
            }
        }

        std::vector<Stmt*> s;
        s = pred_and_dom(n->getParent().first,j-1);
        listOfStatements.insert(listOfStatements.end(), s.begin(), s.end());
        return  listOfStatements;
    }
}

std::vector<Member*> SSA::Node::getMembers(){
    return members;
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