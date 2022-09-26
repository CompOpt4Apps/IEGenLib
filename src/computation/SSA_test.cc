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
TEST(SSATest, IsDominator){

    iegenlib::Set* s1 = new iegenlib::Set("{[0,i,0] : 0 <=i< N}");
    iegenlib::Set* s2 = new iegenlib::Set("{[0,i,1] : 0 <=i< N && x >10}");
    iegenlib::Set* s3 = new iegenlib::Set("{[0,i,2] : 0 <=i< N && x <=10}");
    bool status = SSA::isDominator(s1,s3);
    EXPECT_EQ(status, true);

    bool status1 = SSA::isDominator(s1,s2);
    EXPECT_EQ(status1, true);


    bool status3 = SSA::isDominator(s2,s1);
    EXPECT_EQ(status3, false);

    iegenlib::Set* s4 = new iegenlib::Set("{ [2, t, 1, p, 0] : t >= 0 && p - 11 >= 0 && -t + M - 1 >= 0 }");
    iegenlib::Set* s5 = new iegenlib::Set("{ [2, t, 1, p, 1, q, 0] : t >= 0 && p - 11 >= 0 && q - 11 >= 0 && -t + M - 1 >= 0 }");

    bool status4 = SSA::isDominator(s4,s5);
    EXPECT_EQ(status4, true);

}

TEST(SSATest, IsReverseDominator){

    iegenlib::Set* s0 = new iegenlib::Set("{[2,t,3,s,0,r,0]: 0<=t<M && 0<=s<S && r>10}");
    iegenlib::Set* s1 = new iegenlib::Set("{[2,t,4]: 0<=t<M}");

    bool status = SSA::isReverseDominator(s1, s0);

    EXPECT_EQ(status, false);

}

TEST(SSATest, DominanceTreeTEST1){
   // string s = {([0],0), ([1],2), ([2],2)};
    /*
     * for(i=0;i<N;i++){
     * s0: x= 5+y;
     *  if(x>10){
     *  s1: x=6;
     * }else{
     * s2: x = 10;
     * }
     *
     */
    iegenlib::Set* s1 = new iegenlib::Set("{[0,i,0] : 0 <=i< N}");
    iegenlib::Set* s2 = new iegenlib::Set("{[0,i,1] : 0 <=i< N && x >10}");
    iegenlib::Set* s3 = new iegenlib::Set("{[0,i,2] : 0 <=i< N && x <=10}");

    std::vector<std::pair<int, iegenlib::Set*>> executionS {{0, s1}, {1, s2}, {2, s3}};

    DominanceTree* dt = createDominanceTree(executionS);

    DominanceTree edt;
    int p1 = edt.push_Back({0, s1});
    int p2 =  edt.push_Back({1, s2});
    int p3 =  edt.push_Back({2, s3});

    edt.add_edge(p1, p2);
    edt.add_edge(p1, p3);

    EXPECT_TRUE(edt.equivalent(*dt));

}

TEST(SSATest, DominanceTreeTEST2){

    iegenlib::Set* s1 = new iegenlib::Set("{[0]}");
    iegenlib::Set* s2 = new iegenlib::Set("{[1]}");

    iegenlib::Set* s3 = new iegenlib::Set("{[2,t,0]: 0<=t<M}");
    iegenlib::Set* s4 = new iegenlib::Set("{[2,t,0,p,0]: 0<=t<M && p> 10 }");

    iegenlib::Set* s5 = new iegenlib::Set("{[2,t,0,p,1,q,0]:0<=t<M && p>10 && q>10}");
    iegenlib::Set* s6 = new iegenlib::Set("{[2,t,0,p,1,n,0]:0<=t<M && p>10 && n<=10 }");

    iegenlib::Set* s7 = new iegenlib::Set("{[2,t,0,p,2]: 0<=t<M && p>10}");
    iegenlib::Set* s8 = new iegenlib::Set("{[2,t,0,m,0]: 0<=t<M && m<=10}");

    iegenlib::Set* s9 = new iegenlib::Set("{[2,t,1]: 0<=t<M}");
    iegenlib::Set* s10 = new iegenlib::Set("{[2,t,3,s,0,r,0]: 0<=t<M && 0<=s<S && r>10}");

    iegenlib::Set* s11 = new iegenlib::Set("{[2,t,4]: 0<=t<M}");
    iegenlib::Set* s12 = new iegenlib::Set("{[3]}");


    std::vector<std::pair<int, iegenlib::Set*>>
    executionS {{0, s1}, {1, s2}, {2, s3}, {3,s4},{4,s5},
                {5,s6},{6,s7},{7,s8},{8,s9},{9,s10}, {10,s11},
                {11,s12}};

    DominanceTree* dt = createDominanceTree(executionS);
    DominanceTree edt;

    int p1 = edt.push_Back({0, s1});
    int p2 =  edt.push_Back({1, s2});
    int p3 =  edt.push_Back({2, s3});
    int p4 =  edt.push_Back({3, s4});
    int p5 =  edt.push_Back({4, s5});
    int p6 =  edt.push_Back({5, s6});
    int p7 =  edt.push_Back({6, s7});
    int p8 =  edt.push_Back({7, s8});
    int p9 =  edt.push_Back({8, s9});
    int p10 =  edt.push_Back({9, s10});
    int p11 =  edt.push_Back({10, s11});
    int p12 =  edt.push_Back({11, s12});

    edt.add_edge(p1, p2);

    edt.add_edge(p2, p3);
    edt.add_edge(p2, p12);

    edt.add_edge(p3, p4);
    edt.add_edge(p3, p8);
    edt.add_edge(p3, p10);


    edt.add_edge(p4, p5);
    edt.add_edge(p4, p6);
    edt.add_edge(p4, p7);

    edt.add_edge(p9, p10);
    edt.add_edge(p9, p11);

    edt.add_predecessors(p12, p11);
    edt.add_predecessors(p12, p1);

    EXPECT_TRUE(edt.equivalent(*dt));

}



TEST(SSATest, DominanceTreeTEST3){

    iegenlib::Set* s1 = new iegenlib::Set("{[0]}");
    iegenlib::Set* s2 = new iegenlib::Set("{[1]}");

    iegenlib::Set* s3 = new iegenlib::Set("{[2,t,0]: 0<=t<M}");
    iegenlib::Set* s4 = new iegenlib::Set("{[2,t,1,p,0]: 0<=t<M && p> 10 }");

    iegenlib::Set* s5 = new iegenlib::Set("{[2,t,1,p,1,q,0]:0<=t<M && p>10 && q>10}");
    iegenlib::Set* s6 = new iegenlib::Set("{[2,t,1,p,1,n,0]:0<=t<M && p>10 && n<=10 }");

    iegenlib::Set* s7 = new iegenlib::Set("{[2,t,1,p,2]: 0<=t<M && p>10}");
    iegenlib::Set* s8 = new iegenlib::Set("{[2,t,2,m,0]: 0<=t<M && m<=10}");

    iegenlib::Set* s9 = new iegenlib::Set("{[2,t,3]: 0<=t<M}");
    iegenlib::Set* s10 = new iegenlib::Set("{[2,t,4,s,0,r,0]: 0<=t<M && 0<=s<S && r>10}");

    iegenlib::Set* s11 = new iegenlib::Set("{[2,t,5]: 0<=t<M}");
    iegenlib::Set* s12 = new iegenlib::Set("{[3]}");


    std::vector<std::pair<int, iegenlib::Set*>>
            executionS {{0, s1}, {1, s2}, {2, s3}, {3,s4},{4,s5},
                        {5,s6},{6,s7},{7,s8},{8,s9},{9,s10}, {10,s11},
                        {11,s12}};

    DominanceTree* dt = createDominanceTree(executionS);
    DominanceTree* dt1 = findPredecessors(dt);

    DominanceTree edt;

    int p1 = edt.push_Back({0, s1});
    int p2 =  edt.push_Back({1, s2});
    int p3 =  edt.push_Back({2, s3});
    int p4 =  edt.push_Back({3, s4});
    int p5 =  edt.push_Back({4, s5});
    int p6 =  edt.push_Back({5, s6});
    int p7 =  edt.push_Back({6, s7});
    int p8 =  edt.push_Back({7, s8});
    int p9 =  edt.push_Back({8, s9});
    int p10 =  edt.push_Back({9, s10});
    int p11 =  edt.push_Back({10, s11});
    int p12 =  edt.push_Back({11, s12});

    edt.add_edge(p1, p2);

    edt.add_edge(p2, p3);
    edt.add_edge(p2, p12);

    edt.add_edge(p3, p4);
    edt.add_edge(p3, p8);
    edt.add_edge(p3, p9);


    edt.add_edge(p4, p5);
    edt.add_edge(p4, p6);
    edt.add_edge(p4, p7);

    edt.add_edge(p9, p10);
    edt.add_edge(p9, p11);

    edt.add_predecessors(p12, p11);
    edt.add_predecessors(p12, p2);

    //EXPECT_TRUE(edt.equivalent(*dt));
   // EXPECT_TRUE(edt.predecessorEquivalent(*dt1));

}

// This test is to check if correct predecessors will be created 
// for situations with back edges.
TEST(SSATest, DominanceTreeTEST4){

    iegenlib::Set* s1 = new iegenlib::Set("{[0,i,0,j,0]: 0 <= i  < N && 0 <= j < M}");
    iegenlib::Set* s2 = new iegenlib::Set("{[0,i,1,j,0]: 0 <= i  < N && 0 <= j < M}");


    std::vector<std::pair<int, iegenlib::Set*>>
            executionS {{0, s1}, {1, s2}};

    DominanceTree* dt = createDominanceTree(executionS);
    DominanceTree* dt1 = findPredecessors(dt);

    DominanceTree edt;

    int p1 = edt.push_Back({0, s1});
    int p2 =  edt.push_Back({1, s2});

    edt.add_edge(p1, p2);

    edt.add_predecessors(p1, p2);
   // edt.add_predecessors(p2, p1);


    EXPECT_TRUE(edt.equivalent(*dt));
    EXPECT_TRUE(edt.predecessorEquivalent(*dt1));

}

TEST(SSATest, DominanceTreeTEST5){

    iegenlib::Set* s1 = new iegenlib::Set("{[0,i,0] : 0 <=i< N}");
    iegenlib::Set* s2 = new iegenlib::Set("{[0,i,1] : 0 <=i< N && x >10}");
    iegenlib::Set* s3 = new iegenlib::Set("{[0,i,2] : 0 <=i< N && x <=10}");


    std::vector<std::pair<int, iegenlib::Set*>>
            executionS {{0, s1}, {1, s2},{2,s3}};

    DominanceTree* dt = createDominanceTree(executionS);
    DominanceTree edt;

    int p1 = edt.push_Back({0, s1});
    int p2 =  edt.push_Back({1, s2});
    int p3 =  edt.push_Back({2, s3});

    edt.add_edge(p1, p2);
    edt.add_edge(p1, p3);

    EXPECT_TRUE(edt.equivalent(*dt));

}
TEST(SSATest, GetPrefixes){

    iegenlib::Set* s5 = new iegenlib::Set("{[2,t,1,p,1,q,0]:0<=t<M && p>10 && q>10}");
    iegenlib::Set* s6 = new iegenlib::Set("{[2,t,1,p,1,n,0]:0<=t<M && p>10 && n<=10 }");

    DominanceTree* edt;
    std::vector<Set*> v;
//    Set* s1 = new Set("{[2, t, 1, p, 1, q, 0] : t >= 0 && p - 11 >= 0 && q - 11 >= 0 && -t + M - 1 >= 0}");
    Set* s2 = new Set("{[2, t, 1, p, 1, q] : t >= 0 && p - 11 >= 0 && q - 11 >= 0 && M - 1 >= 0 && -t + M - 1 >= 0}");
    Set* s3 = new Set("{[2, t, 1, p] : t >= 0 && p - 11 >= 0 && M - 1 >= 0 && -t + M - 1 >= 0}");
    Set* s4 = new Set("{[2, t] : t >= 0 && M - 1 >= 0 && -t + M - 1 >= 0}");

    std::vector<Set*> expected_v = {s2, s3, s4};
    v = edt->getPrefixes(s5);

   // EXPECT_EQ(v, expected_v);
    for(int i=0;i<v.size();i++){
        EXPECT_EQ(v[i]->prettyPrintString(), expected_v[i]->prettyPrintString());
    }

}

TEST(SSATest, DominanceTreeTEST6){

    iegenlib::Set* s0 = new iegenlib::Set("{[0,i,0,j,0] : 0 <=i< N  && 0 <=j<M }");
    iegenlib::Set* s1 = new iegenlib::Set("{[1,i,0,j,0] : 0 <=i< N  && 0 <=j<M }");
    iegenlib::Set* s2 = new iegenlib::Set("{[2,i,0,j,0] : 0 <=i< N  && 0 <=j<M }");
    iegenlib::Set* s3 = new iegenlib::Set("{[3,i,0,j,0] : 0 <=i< N  && 0 <=j<M }");


    std::vector<std::pair<int, iegenlib::Set*>>
            executionS {{0, s0}, {1, s1},{2,s2},{3, s3}};

    DominanceTree* dt = createDominanceTree(executionS);
    DominanceTree edt;

}

TEST(SSATest, DominanceTreeTEST7){

    iegenlib::Set* s0 = new iegenlib::Set("{[0,i,0,j,0] : 0 <=i< N  && 0 <=j<M }");
    iegenlib::Set* s1 = new iegenlib::Set("{[0,i,0,j,1] : 0 <=i< N  && 0 <=j<M }");
    iegenlib::Set* s2 = new iegenlib::Set("{[0,i,0,j,2] : 0 <=i< N  && 0 <=j<M }");
    iegenlib::Set* s3 = new iegenlib::Set("{[0,i,0,j,3] : 0 <=i< N  && 0 <=j<M }");


    std::vector<std::pair<int, iegenlib::Set*>>
            executionS {{0, s0}, {1, s1},{2,s2},{3, s3}};

    DominanceTree* dt = createDominanceTree(executionS);
    DominanceTree edt;

}


TEST(SSATest, DominanceTreeTEST8){

    iegenlib::Set* s0 = new iegenlib::Set("{[0,i,0,j,0] : 0 <=i< N  && 0 <=j<M }");
    iegenlib::Set* s1 = new iegenlib::Set("{[0,i,1,j,0] : 0 <=i< N  && 0 <=j<M }");
    iegenlib::Set* s2 = new iegenlib::Set("{[1,i,0,j,0] : 0 <=i< N  && 0 <=j<M }");
    iegenlib::Set* s3 = new iegenlib::Set("{[1,i,0,j,1] : 0 <=i< N  && 0 <=j<M }");


    std::vector<std::pair<int, iegenlib::Set*>>
            executionS {{0, s0}, {1, s1},{2,s2},{3, s3}};

    DominanceTree* dt = createDominanceTree(executionS);
    DominanceTree edt;

}

TEST(SSATest, DominanceTreeTEST9){

    iegenlib::Set* s0 = new iegenlib::Set("{[0,i,0,j,0] : 0 <=i< N  && 0 <=j<M }");
    iegenlib::Set* s1 = new iegenlib::Set("{[0,i,1,j,0] : 0 <=i< N  && 0 <=j<M }");
    iegenlib::Set* s2 = new iegenlib::Set("{[1,i,0,j,0] : 0 <=i< N  && 0 <=j<M }");
    iegenlib::Set* s3 = new iegenlib::Set("{[1,i,0,j,1] : 0 <=i< N  && 0 <=j<M }");

    std::vector<std::pair<int, iegenlib::Set*>>
            executionS {{0, s0}, {1, s1},{2,s2},{3, s3}};

    DominanceTree* dt = createDominanceTree(executionS);
    DominanceTree* dt1;
    dt1 = findPredecessors(dt);
    DominanceTree edt;
    dt->DFCal();
}



TEST(SSATest, DominanceTreeTEST10){

    iegenlib::Set* s1 = new iegenlib::Set("{[0]}");
    iegenlib::Set* s2 = new iegenlib::Set("{[1]}");

    iegenlib::Set* s3 = new iegenlib::Set("{[2,t,0]: 0<=t<M}");
    iegenlib::Set* s4 = new iegenlib::Set("{[2,t,1,p,0]: 0<=t<M && p> 10 }");

    iegenlib::Set* s5 = new iegenlib::Set("{[2,t,1,p,1,q,0]:0<=t<M && p>10 && q>10}");
    iegenlib::Set* s6 = new iegenlib::Set("{[2,t,1,p,1,n,0]:0<=t<M && p>10 && n<=10 }");

    iegenlib::Set* s7 = new iegenlib::Set("{[2,t,1,p,2]: 0<=t<M && p>10}");
    iegenlib::Set* s8 = new iegenlib::Set("{[2,t,2,m,0]: 0<=t<M && m<=10}");

    iegenlib::Set* s9 = new iegenlib::Set("{[2,t,3]: 0<=t<M}");
    iegenlib::Set* s10 = new iegenlib::Set("{[2,t,4,s,0,r,0]: 0<=t<M && 0<=s<S && r>10}");

    iegenlib::Set* s11 = new iegenlib::Set("{[2,t,5]: 0<=t<M}");
    iegenlib::Set* s12 = new iegenlib::Set("{[3]}");


    std::vector<std::pair<int, iegenlib::Set*>>
            executionS {{0, s1}, {1, s2}, {2, s3}, {3,s4},{4,s5},
                        {5,s6},{6,s7},{7,s8},{8,s9},{9,s10}, {10,s11},
                        {11,s12}};

    DominanceTree* dt = createDominanceTree(executionS);
    DominanceTree* dt1 = findPredecessors(dt);
    dt1->DFCal();
}

TEST(SSATest, IsDominator5){

    iegenlib::Set* s9 = new iegenlib::Set("{[2,t,1]: 0<=t<M}");
    iegenlib::Set* s10 = new iegenlib::Set("{[2,t,3,s,0,r,0]: 0<=t<M && 0<=s<S && r>10}");

    bool status = SSA::isDominator(s9, s10);

    EXPECT_EQ(status, true);
}

TEST(SSATest, DominanceTreeTEST11){


    iegenlib::Set* s1 = new iegenlib::Set("{[0]}");
    iegenlib::Set* s2 = new iegenlib::Set("{[1]}");

    iegenlib::Set* s3 = new iegenlib::Set("{[2,t,0]: 0<=t<M}");
    iegenlib::Set* s4 = new iegenlib::Set("{[2,t,1,p,0]: 0<=t<M && p> 10 }");

    iegenlib::Set* s5 = new iegenlib::Set("{[2,t,1,p,1,q,0]:0<=t<M && p>10 && q>10}");
    iegenlib::Set* s6 = new iegenlib::Set("{[2,t,1,p,1,n,0]:0<=t<M && p>10 && n<=10 }");

    iegenlib::Set* s7 = new iegenlib::Set("{[2,t,1,p,2]: 0<=t<M && p>10}");
    iegenlib::Set* s8 = new iegenlib::Set("{[2,t,2,m,0]: 0<=t<M && m<=10}");

    iegenlib::Set* s9 = new iegenlib::Set("{[2,t,3]: 0<=t<M}");
    iegenlib::Set* s10 = new iegenlib::Set("{[2,t,4,s,0,r,0]: 0<=t<M && 0<=s<S && r>10}");

    iegenlib::Set* s11 = new iegenlib::Set("{[2,t,5]: 0<=t<M}");
    iegenlib::Set* s12 = new iegenlib::Set("{[3]}");


    std::vector<std::pair<int, iegenlib::Set*>>
            executionS {{0, s1}, {1, s2}, {2, s3}, {3,s4},{4,s5},
                        {5,s6},{6,s7},{7,s8},{8,s9},{9,s10}, {10,s11},
                        {11,s12}};

    DominanceTree* dt = createDominanceTree(executionS);
    DominanceTree* dt1 = findPredecessors(dt);
    dt1->DFCal();


    std::vector<std::map<string, std::vector<int>>> globals;
    std::map<string, std::vector<int>> a;
    std::map<string, std::vector<int>> c;
    std::vector<int>b {1,5};
    a.insert(make_pair("x", b));
    //c.insert(make_pair("y", b));

    globals.push_back(a);
    //globals.push_back(c);

    dt1->insertPhiNode(globals);

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

    std::string codeGen = computation->codeGen();
    computation->finalize();

    EXPECT_EQ("1","1");
    delete computation;
}
