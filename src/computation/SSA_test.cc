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
            "x = 1;",
            "{[0]}",
            "{[0]->[0]}",
            {},
            {{"x", "{[0]->[0]}"}}
    ));

    //s2
    // iegenlib::Set* s2 = new iegenlib::Set("{[1]}");

    comp->addStmt(new Stmt (
            "x = 2;",
            "{[0]}",
            "{[0]->[1]}",
            {{"x", "{[0]->[0]}"}},
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
            "x = 2;",
            "{[t,p]:0<=t<M && p> 10}",
            "{[t,p]->[2,t,0,p,0]}",
            {{"x", "{[t,p]->[t,p]}"}},
            {{"x", "{[t,p]->[t,p]}"}}
    ));

    //s5
    //iegenlib::Set* s5 = new iegenlib::Set("{[2,t,1,p,1,q,0]:0<=t<M && p>10 && q>10}");

    comp->addStmt(new Stmt (
            "x = 2;",
            "{[t,p,q]:0<=t<M && p>10 && q>10}",
            "{[t,p,q]->[2,t,0,p,1,q,0]}",
            {{"x", "{[t,p,q]->[t,p,q]}"}},
            {{"x", "{[t,p,q]->[t,p,q]}"}}
    ));

    //s6
    //iegenlib::Set* s6 = new iegenlib::Set("{[2,t,1,p,1,n,0]:0<=t<M && p>10 && n<=10 }");

    comp->addStmt(new Stmt (
            "x = 2;",
            "{[t,p,v]:0<=t<M && p>10 && v<=10}",
            "{[t,p,v]->[2,t,0,p,1,v,0]}",
            {{"x", "{[t,p,v]->[t,p,v]}"}},
            {{"x", "{[t,p,v]->[t,p,v]}"}}
    ));

    //s7
    //iegenlib::Set* s7 = new iegenlib::Set("{[2,t,1,p,2]: 0<=t<M && p>10}");

    comp->addStmt(new Stmt (
            "x = 2;",
            "{[t,p]:0<=t<M && p>10}",
            "{[t,p]->[2,t,0,p,2]}",
            {{"x", "{[t,p]->[t,p]}"}},
            {{"x", "{[t,p]->[t,p]}"}}
    ));

    //s8
    //iegenlib::Set* s8 = new iegenlib::Set("{[2,t,2,m,0]: 0<=t<M && m<=10}");


    comp->addStmt(new Stmt (
            "x = 2;",
            "{[t,u]:0<=t<M && u<=10}",
            "{[t,u]->[2,t,0,u,0]}",
            {{"x", "{[t,u]->[t,u]}"}},
            {{"x", "{[t,u]->[t,u]}"}}
    ));

    //s9
    //iegenlib::Set* s9 = new iegenlib::Set("{[2,t,3]: 0<=t<M}");

    comp->addStmt(new Stmt (
            "x = 2;",
            "{[t]:0<=t<M}",
            "{[t]->[2,t,1]}",
            {{"x", "{[t]->[t]}"}},
            {{"x", "{[t]->[t]}"}}
    ));

    //s10
    //iegenlib::Set* s10 = new iegenlib::Set("{[2,t,4,s,0,r,0]: 0<=t<M && 0<=s<S && r>10}");

    comp->addStmt(new Stmt (
            "x = 2;",
            "{[t,s,r]:0<=t<M && s>10 && r>20}",
            "{[t,s,r]->[2,t,3,s,0,r,0]}",
            {{"x", "{[t,s,r]->[t,s,r]}"}},
            {{"x", "{[t,s,r]->[t,s,r]}"}}
    ));
    //s11
    //iegenlib::Set* s11 = new iegenlib::Set("{[2,t,5]: 0<=t<M}");

    comp->addStmt(new Stmt (
            "x = 2;",
            "{[t]:0<=t<M}",
            "{[t]->[2,t,4]}",
            {{"x", "{[t]->[t]}"}},
            {{"x", "{[t]->[t]}"}}
    ));
    //s12
    //iegenlib::Set* s12 = new iegenlib::Set("{[3]}");

    comp->addStmt(new Stmt (
            "x = 2;",
            "{[0]}",
            "{[0]->[3]}",
            {{"x", "{[0]->[0]}"}},
            {{"x", "{[0]->[0]}"}}
    ));
//    Node * node;
//    node = createScheduleTree(comp);
//

    SSA::generateSSA(comp);
    comp->finalize();
    std:: cout << comp->toDotString();
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
TEST(SSATest, MTTKRP){

    // this one is dense
    //void mttkrp(int I,int J, int K, int R,double *X,
    //               double *A, double *B, double *C) {
    // for (i = 0; i < I; i++)
    //   for (j = 0; j < J; j++)
    //     for (k = 0; k < K; k++)
    //       for (r = 0; r < R; r++)
    //         A[i,r] += X[i,j,k]*B[j,r]*C[k,r];
    ///
    vector < pair<string, string> > dataReads;
    vector < pair<string, string> > dataWrites;
    Computation mttkrp;
    mttkrp.addDataSpace("X","double*");
    mttkrp.addDataSpace("A","double*");
    mttkrp.addDataSpace("B","double*");
    mttkrp.addDataSpace("C","double*");
    Stmt *s = new Stmt("x=1", "{[i,j,k,r] : 0 <= i < I and 0<=j<J and 0<=k<K and 0<=r<R}",
                       "{[i,j,k,r]->[0,i,0,j,0,k,0,r,0]}",
                       {},
                       { {"A", "{[i,k,l,j]->[i,j]}"},
                         {"B", "{[i,k,l,j]->[i,k,l]}"},
                         {"D", "{[i,k,l,j]->[l,j]}"},
                         {"C", "{[i,k,l,j]->[k,j]}"}});
    mttkrp.addStmt(s);
    Stmt *s0 = new Stmt("A(i,r) += X(i,j,k)*B(j,r)*C(k,r)",
                        "{[i,j,k,r] : 0 <= i < I and 0<=j<J and 0<=k<K and 0<=r<R}",
                        "{[i,j,k,r]->[1,i,0,j,0,k,0,r,0]}",
                        {
                                // data reads
                                {"A", "{[i,k,l,j]->[i,j]}"},
                                {"B", "{[i,k,l,j]->[i,k,l]}"},
                                {"D", "{[i,k,l,j]->[l,j]}"},
                                {"C", "{[i,k,l,j]->[k,j]}"},
                        },
                        {
                                // data writes
                                {"A", "{[i,k,l,j]->[i,j]}"},
                        });

    mttkrp.addStmt(s0);
    Stmt *s2 = new Stmt("x=2", "{[0]}", "{[0]->[2]}",{},{});
    mttkrp.addStmt(s2);

    SSA::generateSSA(&mttkrp);

    mttkrp.finalize();
    std:: cout << mttkrp.toDotString();

    // this one is COO
    Computation mttkrp_sps;
    mttkrp_sps.addDataSpace("X","double*");
    mttkrp_sps.addDataSpace("A","double*");
    mttkrp_sps.addDataSpace("B","double*");
    mttkrp_sps.addDataSpace("C","double*");
    Stmt *s1 = new Stmt("A(x,i,j,k,r) += X(x,i,j,k,r)*B(x,i,j,k,r)*C(x,i,j,k,r)",
                        "{[x,i,j,k,r] :  0<=x< NNZ and i=UFi(x) and j=UFj(x) and k=UFk(x) and 0<=r<R}",
                        "{[x,i,j,k,r]->[0,x,0,i,0,j,0,k,0,r,0]}",
                        dataReads,
                        dataWrites);

    mttkrp_sps.addStmt(s1);


    //Calling
//    std::cout << "Codegen:\n";
//    std::cout << mttkrp_sps.codeGen();

    EXPECT_EQ("1","1");

}





