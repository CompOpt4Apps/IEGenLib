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
    class Member;
    class Node;
class Node {
private:
    bool ordered;
    int common_arity;
    std::vector<Member*>members;
    std::pair<Node*, Member*> parent;

public:
    static std::map<Stmt*, std::vector<Stmt*>> predecessor;
    void setMembers( std::vector<Member *> &members);

public:
    Node();
    ~Node();
    //void create_member(Set*S, Stmt*st);
    Node* insert(Member* m);

    bool isOrdered() ;

    void setOrdered(bool ordered);

    int getCommonArity() ;

    void setCommonArity(int commonArity);

     std::vector<Member*> getMembers() ;

    void calc_all_pred();

    void printBreadthFirst();

    std::pair<Node *, Member*> &getParent() ;

    void setParent(Node *, Member*);

};
class Member{
private:
    Set* schedule;
public:
    Set *getSchedule() ;

    void setSchedule(Set *schedule);

    Stmt *getStmt() ;

    void setStmt(Stmt *stmt);

    Node *getChild() ;

    void setChild(Node *child);


private:
    Stmt* stmt;
    Node * child;

public:
    Member();
    Member(Set* s, Stmt * s1);
    void printBreadthFirst();
    void calc_all_pred(Node* n);
    std::vector<Stmt*> pred_and_dom(Node* n, int idx);
    ~Member();


};

    Computation* generateSSA(Computation * comp);
    Node* createScheduleTree( Computation* Comp);
    std::vector<Set*> getPrefixes(Set*s);
    string rename( std::map<string, int> &counter, std::map<string, int> &stack,string n );
};
#endif
