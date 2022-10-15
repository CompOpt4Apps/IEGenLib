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


using namespace SSA;
using namespace iegenlib;

std::map<Stmt*, std::vector<Stmt*>> SSA::Member::predecessor{};
std::map<Stmt*, std::vector<Stmt*>> SSA::Node::DF{};
std::map<string, std::vector<Stmt*>> SSA::Node::globals{};

//std::map<Stmt*, std::vector<Stmt*>> SSA::predecessor;


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

        int numWrites = stmts[i]->getNumWrites();
        for (int j = 0; j < numWrites; ++j){
            Node::globals[stmts[i]->getWriteDataSpace(j)].push_back(stmts[i]);
            //std::cout << "variables    " << stmts[i]->getWriteDataSpace(j)<<" ,  "<< i<<'\n';
        }

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
    if(stmt==NULL){
        return NULL;
    }
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
    node-> computeDF();

    std::map<string, std::vector<Stmt*>>::iterator it;
    std::map<string,  std::map<Stmt *, std::vector<Stmt *>>> readLoc;
    std::map<Stmt* , Stmt*> phi_to_stmt;
    for (it = SSA::Node::globals.begin(); it != SSA::Node::globals.end(); it++) {
        string newName = it->first;
        newName.erase(newName.begin());
        newName.erase(newName.end() - 1);
        std::map<Stmt *, std::vector<Stmt *>> phiLoc;
        for (int v = 0; v < it->second.size(); v++) {

            if (Node::DF.find(it->second[v]) != Node::DF.end()) {
                std::cout << "contributing node " << it->second[v]->getExecutionSchedule()->prettyPrintString()
                          << std::endl;
                std::vector<Stmt *> insert_phi_at = Node::DF.at(it->second[v]);
                for (auto stmt: insert_phi_at) {

                    if (std::find(phiLoc[stmt].begin(), phiLoc[stmt].end(), it->second[v]) == phiLoc[stmt].end()) {
                        phiLoc[stmt].push_back(it->second[v]);
                    }

                }
            }
            // std::cout << "----------------------------------------"<<std::endl;
        }
        std::map<Stmt *, std::vector<Stmt *>>::iterator phis;
        for (phis = phiLoc.begin(); phis != phiLoc.end(); phis++) {
            string itrspace = phis->first->getIterationSpace()->prettyPrintString();
            string executionSch = phis->first->getExecutionSchedule()->prettyPrintString();

            Stmt* phi  = new Stmt(
                    "phi",
                    itrspace,
                    executionSch,
                    {{newName,"{[0]->[0]}" }},
                    {{newName, "{[0]->[0]}"}}
            );
            phi->setPhiNode(true);
            comp->addStmt(phi);
            phi_to_stmt[phi] = phis->first;

        }


        readLoc[it->first] = phiLoc;
    }

    int counter = 0;
    for (int a = 0; a < comp->getNumStmts(); a++) {
        Stmt *s;
        s = comp->getStmt(a);

        for (int j = 0; j < s->getNumWrites(); j++) {
           std::string write = s->getWriteDataSpace(j);

            write.erase(write.begin());
            write.erase(write.end()-1);
           string newWrite = write + "_"+ std::to_string(counter);

           s->replaceWrite(write,  newWrite);
            }
        counter++;
      //  std:: cout << "updated stmt " << s->prettyPrintString() << std::endl;
    }

    for (int b = 0; b < comp->getNumStmts(); b++) {
        Stmt *s1;
        s1 = comp->getStmt(b);

        for (int j = 0; j < s1->getNumReads(); j++) {
            std::string read = s1->getReadDataSpace(j);
            std::string test = read;
            test.erase(test.end()-1);
            test = test + '_';
            std:: cout << "the reads are " << read << std::endl;
            if( s1->isPhiNode()){
                if(readLoc.find(read)==readLoc.end()){
                    continue;
                }
                std::map<Stmt *, std::vector<Stmt *>> read_locations = readLoc[read];

                if(phi_to_stmt.find(s1)!=phi_to_stmt.end()){
                    Stmt* st = phi_to_stmt[s1];
                    if(read_locations.find(st)!=read_locations.end()){
                        std::vector<Stmt*> r = read_locations[st];
                        for(auto v: r){

                            int numWrites =v->getNumWrites();
                            for (int j = 0; j < v->getNumWrites(); j++){
                                if(v->getWriteDataSpace(j).find(test)!= std::string::npos){

                                    std::cout << "variables    " << v->getWriteDataSpace(j)<<'\n';

                                    break;
                                }

                            }
                        }

                    }
                }

                std:: cout << " the phi node is " << s1->getExecutionSchedule()->prettyPrintString()<< std::endl;
            }

        }

    }

    return comp;
}

void SSA::Node::computeDF() {
    std::map<Stmt*, std::vector<Stmt*>>::iterator it;
    //for all statements in computation
    for (it = Member::predecessor.begin(); it != Member::predecessor.end(); it++)
    {
        Stmt* runner;
        //for all pred of that statement
        if(it->second.size()> 1) {
            for (int j = 0; j < it->second.size(); j++) {
                runner = it->second[j];
                // while the runner isn't equal to dominator of n
                // DF of runner gets added to the
                while (runner != it->second[it->second.size() - 1]) {
                    if (Node::DF.find(runner) == Node::DF.end()) {
                        Node::DF[runner] = {};
                    }
                    Node::DF[runner].push_back(it->first);
                    runner = Member::predecessor.at(runner).back();
                }
            }

        }
    }
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
    //std::cout << "------------------"<<'\n';


}

void Node::setMembers( std::vector<Member *> &members) {
    Node::members = members;
}


void SSA::Member::printBreadthFirst() {
    std::cout << schedule->prettyPrintString()<<'\n';
    child->printBreadthFirst();
}
void SSA::Member::calc_all_pred(Node * n){

    if(stmt!=NULL) {
        int j;
        for (j = 0; j < n->getMembers().size(); j++) {
            if (this == n->getMembers()[j]) {
                break;
            }
        }
        std::vector<Stmt *> stmtList;

        stmtList = pred_and_dom(n, j - 1);

        std::vector<Stmt*> rduplicates;

        rduplicates  = predecessor[stmt];


        for (int i = 0; i < stmtList.size(); i++) {
            if(stmtList[i]== stmt){
                continue;
            }
            if(std::find(rduplicates.begin(), rduplicates.end(),stmtList[i] ) == rduplicates.end()){
                rduplicates.push_back(stmtList[i]);
            }
        }
        predecessor[stmt] = rduplicates;

    }
    child->calc_all_pred();
}

std::vector<Stmt*> SSA::Member::pred_and_dom(Node* n, int idx) {

   // std:: cout << "The ordered value is "<< n->isOrdered()<<std::endl;
    std::vector < Stmt * > listOfStatements{};
    int i;
    for (i = idx; i >= 0; i--) {
        //this case is for when we hit a dominator
        if (n->getMembers()[i]->getStmt() != NULL) {
            listOfStatements.push_back(n->getMembers()[i]->getStmt());
            return listOfStatements;
        }
        //this case is for when we are adding predecessors that aren't dominators
        for (auto c: n->getMembers()[i]->getChild()->getMembers()) {
            std::vector < Stmt * > s;
            s = pred_and_dom(c->getChild(), c->getChild()->getMembers().size() - 1);
            listOfStatements.insert(listOfStatements.end(), s.begin(), s.end());
        }
    }
    // if we are here we did not find a dominator within the loop.
    // Now we have to do three things.
    // 1. we have to find possible predecessors by looking at parent unordered node
    // 2. we have to find the dominator by looking at grandparent node
    // 3. we have to find possible intraloop predecessors by looking from
    // the end of the loop backward until we reach idx
    // I am going to do step 3 first, then 1, and then 2
    //
    // This is step 3 above.
    if(n->getCommonArity()!=1) {
        for (i = n->getMembers().size() - 1; i != idx; i--) {
            //this case is for when we hit a dominator
            if (n->getMembers()[i]->getStmt() != NULL) {
                listOfStatements.push_back(n->getMembers()[i]->getStmt());
                break;
            }
            //this case is for when we are adding predecessors that aren't dominators
            for (auto c: n->getMembers()[i]->getChild()->getMembers()) {
                std::vector<Stmt *> s;
                s = pred_and_dom(c->getChild(), c->getChild()->getMembers().size() - 1);
                listOfStatements.insert(listOfStatements.end(), s.begin(), s.end());
            }
        }
    }
    // this is for the root node
   // std::cout <<" root node "<< n->getParent().first << std::endl;

    if (n->getParent().first == NULL) {
        return listOfStatements;
    }
    // stepping up to find the location of the dominator in the member vector
    Node* p = n->getParent().first;
    for (auto c:p->getMembers()){
        if(c->getChild()!= n) {
            std::vector<Stmt *> s;
            s = pred_and_dom(c->getChild(), c->getChild()->getMembers().size() - 1);
            listOfStatements.insert(listOfStatements.end(), s.begin(), s.end());
        }
    }
   // std::cout <<" get parent "<< p->getParent().second->getSchedule()->prettyPrintString() << std::endl;

    Node * gp = p->getParent().first;
    Member * gpm = p->getParent().second;
    if(gp != NULL){
        std::vector<Stmt*> s;
        int j;
        for(j=0;j<gp->getMembers().size();j++ ){
            if(gpm==gp->getMembers()[j] ){
                break;
            }
        }
        s = pred_and_dom(gp,j-1);
        listOfStatements.insert(listOfStatements.end(), s.begin(), s.end());
    }
    return listOfStatements;
}

std::vector<Member*> SSA::Node::getMembers(){
    if(members.empty())  return std::vector<Member*>();
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