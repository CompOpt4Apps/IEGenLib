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
public:
    void setMembers(const std::vector<Member *> &members);

public:
    Node();
    ~Node();
    //void create_member(Set*S, Stmt*st);
    Node* insert(Member* m);

    bool isOrdered() const;

    void setOrdered(bool ordered);

    int getCommonArity() const;

    void setCommonArity(int commonArity);

    const std::vector<Member*> &getMembers() const;

    void printBreadthFirst();

};
class Member{
private:
    Set* schedule;
public:
    Set *getSchedule() const;

    void setSchedule(Set *schedule);

    Stmt *getStmt() const;

    void setStmt(Stmt *stmt);

    Node *getChild() const;

    void setChild(Node *child);

     std::pair<Node *, Member*> &getParent() ;

    void setParent(Node *, Member*);

private:
    Stmt* stmt;
    Node * child;
    std::pair<Node*, Member*> parent;
public:
    Member();
    Member(Set* s, Stmt * s1);
    void printBreadthFirst();
    ~Member();


};
    Node* generateSSA(Computation * comp);
    Node* createScheduleTree( Computation* Comp);
    std::vector<Set*> getPrefixes(Set*s);
    std::vector<Stmt> pred_and_dom(Node* n, int idx);
};
#endif
