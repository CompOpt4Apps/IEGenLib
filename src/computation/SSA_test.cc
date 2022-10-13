/*!
 * \file SSA_test.cc
 *
 * \brief Tests for the dominanceTree class.
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
#include "Computation.h"
#include <gtest/gtest.h>
#include "set_relation/set_relation.h"
#include <utility>
#include <vector>
#include <map>
#include "code_gen/parser/parser.h"
#include "omega/Relation.h"
using namespace SSA;
using namespace std;


TEST(SSATest123, DominanceTreeTEST111){


    Computation * comp = new Computation();
    comp->addDataSpace("x", "int");

    //s1
    //    iegenlib::Set* s1 = new iegenlib::Set("{[0]}");
    comp->addStmt(new Stmt (
            "x=1;",
            "{[0]}",
            "{[0]->[0]}",
            {},
            {{"x", "{[0]->[0]}"}}
    ));

    //s2
    // iegenlib::Set* s2 = new iegenlib::Set("{[1]}");

    comp->addStmt(new Stmt (
            "x=2;",
            "{[0]}",
            "{[0]->[1]}",
            {},
            {{"x", "{[0]->[0]}"}}
    ));

//    //s3
//    //iegenlib::Set* s3 = new iegenlib::Set("{[2,t,0]: 0<=t<M}");
//
//    comp->addStmt(new Stmt (
//            "x=2;",
//            "{[t]:0<=t<M }",
//            "{[t]->[2,t,0]}",
//            {},
//            {{"x", "{[t]->[t]}"}}
//    ));

    //s4
    //iegenlib::Set* s4 = new iegenlib::Set("{[2,t,1,p,0]: 0<=t<M && p> 10 }");

    comp->addStmt(new Stmt (
            "x=2;",
            "{[t,p]:0<=t<M && p> 10}",
            "{[t,p]->[2,t,0,p,0]}",
            {},
            {{"x", "{[t,p]->[t,p]}"}}
    ));

    //s5
    //iegenlib::Set* s5 = new iegenlib::Set("{[2,t,1,p,1,q,0]:0<=t<M && p>10 && q>10}");

    comp->addStmt(new Stmt (
            "x=2;",
            "{[t,p,q]:0<=t<M && p>10 && q>10}",
            "{[t,p,q]->[2,t,0,p,1,q,0]}",
            {},
            {{"x", "{[t,p,q]->[t,p,q]}"}}
    ));

    //s6
    //iegenlib::Set* s6 = new iegenlib::Set("{[2,t,1,p,1,n,0]:0<=t<M && p>10 && n<=10 }");

    comp->addStmt(new Stmt (
            "x=2;",
            "{[t,p,n]:0<=t<M && p>10 && n<=10}",
            "{[t,p,v]->[2,t,0,p,1,v,0]}",
            {},
            {{"x", "{[t,p,v]->[t,p,v]}"}}
    ));

    //s7
    //iegenlib::Set* s7 = new iegenlib::Set("{[2,t,1,p,2]: 0<=t<M && p>10}");

    comp->addStmt(new Stmt (
            "x=2;",
            "{[t,p]:0<=t<M && p>10}",
            "{[t,p]->[2,t,0,p,2]}",
            {},
            {{"x", "{[t,p]->[t,p]}"}}
    ));

    //s8
    //iegenlib::Set* s8 = new iegenlib::Set("{[2,t,2,m,0]: 0<=t<M && m<=10}");


    comp->addStmt(new Stmt (
            "x=2;",
            "{[t,m]:0<=t<M && m<=10}",
            "{[t,u]->[2,t,0,u,0]}",
            {},
            {{"x", "{[t,u]->[t,u]}"}}
    ));

    //s9
    //iegenlib::Set* s9 = new iegenlib::Set("{[2,t,3]: 0<=t<M}");

    comp->addStmt(new Stmt (
            "x=2;",
            "{[t]:0<=t<M}",
            "{[t]->[2,t,1]}",
            {},
            {{"x", "{[t]->[t]}"}}
    ));

    //s10
    //iegenlib::Set* s10 = new iegenlib::Set("{[2,t,4,s,0,r,0]: 0<=t<M && 0<=s<S && r>10}");

    comp->addStmt(new Stmt (
            "x=2;",
            "{[t,s,r]:0<=t<M}",
            "{[t,s,r]->[2,t,3,s,0,r,0]}",
            {},
            {{"x", "{[t,s,r]->[t,s,r]}"}}
    ));
    //s11
    //iegenlib::Set* s11 = new iegenlib::Set("{[2,t,5]: 0<=t<M}");

    comp->addStmt(new Stmt (
            "x=2;",
            "{[t]:0<=t<M}",
            "{[t]->[2,t,4]}",
            {},
            {{"x", "{[t]->[t]}"}}
    ));
    //s12
    //iegenlib::Set* s12 = new iegenlib::Set("{[3]}");

    comp->addStmt(new Stmt (
            "x=2;",
            "{[0]}",
            "{[0]->[3]}",
            {},
            {{"x", "{[0]->[0]}"}}
    ));
    Node * node;
    node = createScheduleTree(comp);

  // node->printBreadthFirst();

   node->calc_all_pred();

//   std::cout << "the size of pred " << SSA::Member::predecessor.size() << std::endl;
//
//    for(auto m: SSA::Member::predecessor){
//        std::cout<< "the pred dom list for node  " << m.first->getExecutionSchedule()->prettyPrintString() <<std::endl;
//        for (int i = 0; i < m.second.size(); i++) {
//            std::cout << "  is " << m.second[i]->getExecutionSchedule()->prettyPrintString() << std::endl;
//        }
//        std::cout << "-------===------------"<<std::endl;
//    }


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


    for(auto m: SSA::Node::DF){
        std::cout<< "DF for node " << m.first->getExecutionSchedule()->prettyPrintString() <<std::endl;
        for (int i = 0; i < m.second.size(); i++) {
            std::cout << "  is " << m.second[i]->getExecutionSchedule()->prettyPrintString() << std::endl;
        }
        std::cout << "-------===------------"<<std::endl;
    }

    std:: cout << std::endl;
  // comp->finalize();
   //std:: cout << comp->toDotString();


    EXPECT_EQ(1,1);

}

TEST(SSATest, SSARenaming1) {

    Computation* computation = new Computation();

    computation->addParameter("foo", "int");
    computation->addDataSpace("bar", "int");

    Stmt* s1 = new Stmt("bar = foo;",
                        "{[0]}",
                        "{[0]->[0]}",
                        {{"foo", "{[0]->[0]}"}},
                        {{"bar", "{[0]->[0]}"}}
    );
    computation->addStmt(s1);

    Stmt* s2 =new Stmt("foo = bar + 1",
                       "{[0]}", "{[0]->[1]}",
                       {{"bar", "{[0]->[0]}"}},
                       {{"foo", "{[0]->[0]}"}}
    );
    computation->addStmt(s2);

    computation->finalize();
    //std:: cout << codeGen;
    EXPECT_EQ("a","a");
    delete computation;
}



