/*!
 * \file TermPartOrdGraph_test.cc
 *
 * \brief Test for the TermPartOrdGraph class.
 *
 * \date Started: 11/20/15
 *
 * \authors Michelle Strout
 *
 * Copyright (c) 2015, University of Arizona<br>
 * All rights reserved. <br>
 * See ../../COPYING for details. <br>
 */

#include "expression.h"
#include "TermPartOrdGraph.h"

#include <gtest/gtest.h>
#include <utility>
#include <iostream>
#include <fstream>

using iegenlib::TermPartOrdGraph;
using iegenlib::TupleVarTerm;
using iegenlib::Term;
using iegenlib::VarTerm;
using iegenlib::UFCallTerm;
using iegenlib::TupleExpTerm;
using iegenlib::Exp;

 /*      //====== Queries ======
 *
 *      if (g.isNonNegative(uf)) { ... }
 *
 *      std::set<UFCallTerm*> ufTerms = g.getUniqueUFCallTerms();
 *      for (std::set<Term*>::const_iterator iter1 = ufTerms.begin();
 *              iter1!=ufTerms.end(); iter1++) {
 *          for (std::set<Term*>::const_iterator iter2 = ufTerms.begin();
 *                  iter2!=ufTerms.end(); iter2++) {
 *              if (g.isLT(*iter1, *iter2)) {
 *                  ...
 *              }
 *          }
 *      }
 */


#pragma mark TermPartOrdGraphToString
TEST(TermPartOrdGraphTest, TermPartOrdGraphToString) {

    // TermPartOrdGraph constructs PartOrdGraph based on a max number of terms
    //      TermPartOrdGraph g(maxNoOfTerms);
    TermPartOrdGraph g(3);
    VarTerm * v = new VarTerm( "N" );
    TupleVarTerm *i = new TupleVarTerm(1, 0);

    // Terms get added whenever we add ordering for new term
    g.insertLT(i,v);

    EXPECT_EQ("TermPartOrdGraph:\n\tNumTerms = 2\n\tNonNegativeTerms = \n"
              "\tUFCallTerm2IntMap = \n\tTupleVarTerm2IntMap = \n\t\tterm"
              " = __tv0, id = 0\n\tVarTerm2IntMap = \n\t\tterm = N, id = "
              "1\n\tTerm2IntMap = \nPartOrdGraph:\n\tmCurN = 2\tmMaxN = 3"
              "\n\t\t0\t1\n\t0\t=\t<\n\t1\t\t=\n\n", g.toString());
    //std::cout << g.toString();
    
    delete v;
}

#pragma mark TermPartOrdGraphIsNonNegative
TEST(TermPartOrdGraphTest, TermPartOrdGraphIsNonNegative) {

    TermPartOrdGraph g(10);
    VarTerm * v = new VarTerm( "N" );
    g.termNonNegative( v );

    UFCallTerm* uf_call = new UFCallTerm("tau", 2); // tau(i,j)
    Exp *tau_arg_1 = new Exp();
    tau_arg_1->addTerm(new VarTerm("i"));
    uf_call->setParamExp(0,tau_arg_1);
    Exp *tau_arg_2 = new Exp();
    tau_arg_2->addTerm(new VarTerm("j"));
    uf_call->setParamExp(1,tau_arg_2);

    //std::cout << g.toString();

    EXPECT_EQ(true, g.isNonNegative( v ));
    EXPECT_EQ(false, g.isNonNegative( uf_call ));
    
    delete v;
    delete uf_call;
}

#pragma mark TermPartOrdGraphUniqueTerms
TEST(TermPartOrdGraphTest, TermPartOrdGraphUniqueTerms) {

    TermPartOrdGraph g(10);
    
    // term = N
    VarTerm * v = new VarTerm( 3, "N" );
    g.termNonNegative( v );
    VarTerm * v2 = new VarTerm( -1, "N" );
    
    // Check that non-negative stayed the same.
    EXPECT_EQ(true, g.isNonNegative( v ));
    
    {
    // Check the size of the unique sets.
    std::set<UFCallTerm*> ufCallTermSet = g.getUniqueUFCallTerms();
    EXPECT_EQ(0,ufCallTermSet.size());
    }
    
    // term = tau(i,j)
    UFCallTerm* uf_call = new UFCallTerm("tau", 2);
    {
    Exp *tau_arg_1 = new Exp();
    tau_arg_1->addTerm(new VarTerm("i"));
    uf_call->setParamExp(0,tau_arg_1);
    Exp *tau_arg_2 = new Exp();
    tau_arg_2->addTerm(new VarTerm("j"));
    uf_call->setParamExp(1,tau_arg_2);
    g.termNonNegative( uf_call );
    }

    {
    // Check the size of the unique sets.
    std::set<UFCallTerm*> ufCallTermSet = g.getUniqueUFCallTerms();
    EXPECT_EQ(1,ufCallTermSet.size());
    }
        
    // Inserting same thing twice on purpose.
    // term = tau(i,j)
    UFCallTerm* uf_call2 = new UFCallTerm("tau", 2);
    {
    Exp *tau_arg_1 = new Exp();
    tau_arg_1->addTerm(new VarTerm("i"));
    uf_call2->setParamExp(0,tau_arg_1);
    Exp *tau_arg_2 = new Exp();
    tau_arg_2->addTerm(new VarTerm("j"));
    uf_call2->setParamExp(1,tau_arg_2);
    g.termNonNegative( uf_call2 );
    }

    {
    // Check the size of the unique sets.
    std::set<UFCallTerm*> ufCallTermSet = g.getUniqueUFCallTerms();
    EXPECT_EQ(1,ufCallTermSet.size());
    }

    // term = g(3 __tv3)
    UFCallTerm* uf_call3 = new UFCallTerm("g", 1);
    {
    Exp *g0 = new Exp();
    g0->addTerm(new TupleVarTerm(3, 3));
    uf_call3->setParamExp(0,g0);
    g.termNonNegative( uf_call3 );
    }

    {
    // Check the size of the unique sets.
    std::set<UFCallTerm*> ufCallTermSet = g.getUniqueUFCallTerms();
    EXPECT_EQ(2,ufCallTermSet.size());
    }

    // term = g(2 __tv3)
    UFCallTerm* uf_call4 = new UFCallTerm("g", 1);
    {
    Exp *g0 = new Exp();
    g0->addTerm(new TupleVarTerm(2, 3));
    uf_call4->setParamExp(0,g0);
    g.termNonNegative( uf_call4 );
    }

    {
    // Check the size of the unique sets.
    std::set<UFCallTerm*> ufCallTermSet = g.getUniqueUFCallTerms();
    EXPECT_EQ(3,ufCallTermSet.size());
    }

    // Have to clean up all terms that were put into part ord,
    // because TermPartOrdGraph doesn't own them.
    delete v;
    delete v2;
    delete uf_call;
    delete uf_call2;
    delete uf_call3;
    delete uf_call4;
  
}

#pragma mark TermPartOrdGraphPartOrd
TEST(TermPartOrdGraphTest, TermPartOrdGraphPartOrd) {

    TermPartOrdGraph g(10);
    
    // term = N
    VarTerm * v = new VarTerm( 3, "N" );
    
    // term = tau(i,j)
    UFCallTerm* uf_call1 = new UFCallTerm("tau", 2);
    Exp *tau_arg_1 = new Exp();
    tau_arg_1->addTerm(new VarTerm("i"));
    uf_call1->setParamExp(0,tau_arg_1);
    Exp *tau_arg_2 = new Exp();
    tau_arg_2->addTerm(new VarTerm("j"));
    uf_call1->setParamExp(1,tau_arg_2);

    // term = g(3 __tv3)
    UFCallTerm* uf_call2 = new UFCallTerm("g", 1);
    Exp* g0 = new Exp();
    g0->addTerm(new TupleVarTerm(3, 3));
    uf_call2->setParamExp(0,g0);

    // term = g(2 __tv3)
    UFCallTerm* uf_call3 = new UFCallTerm("g", 1);
    g0 = new Exp();
    g0->addTerm(new TupleVarTerm(2, 3));
    uf_call3->setParamExp(0,g0);

    // Put in relationships between terms
    // v < uf_call1, uf_call1 <= uf_call2, uf_call3 < uf_call1
    g.insertLT(v,uf_call1);
    g.insertLTE(uf_call1,uf_call2);
    g.insertLT(uf_call3,uf_call1);
    
    EXPECT_EQ(true, g.isLT(v,uf_call2));
    EXPECT_EQ(false, g.isLT(uf_call2,v));
    EXPECT_EQ(true, g.isLT(uf_call3,uf_call2));
    EXPECT_EQ(false, g.isEqual(uf_call3,uf_call2));

    delete v;
    delete uf_call1;
    delete uf_call2;
    delete uf_call3;

}

#pragma mark TermPartOrdGraphConstOrd
TEST(TermPartOrdGraphTest, TermPartOrdGraphConstOrd) {

    // In this test case we want to show that partial ordering can:
    //   1. Add orderings between constants (and non-constants) like 1 < 6
    //   2. Use these ordering to do transitive closures like: 
    //        ii <= 3 && 6 < ufcall(j)  => ii < ufcall(j)

    TermPartOrdGraph g(10);
    VarTerm * v = new VarTerm( "N" );
    g.termNonNegative( v );

    TupleVarTerm *ii = new TupleVarTerm(1, 0);
    TupleVarTerm *jj = new TupleVarTerm(1, 1);

    Term * c2 = new Term( 6 ); 

    Term * c4 = new Term( -6 );

    UFCallTerm* uf_call = new UFCallTerm("tau",1); // tau(__tv2)
    Exp *tau_arg_1 = new Exp();
    tau_arg_1->addTerm( new TupleVarTerm(1, 2) );
    uf_call->setParamExp(0,tau_arg_1);

    // Put in relationships between terms
    g.insertLT( c2 , uf_call );    // 6 < uf_call
    Term * c1 = new Term( 3 ); 

    g.insertLTE(ii , c1 );         // __tv0 <= 3
    Term * c3 = new Term( 3 ); // Unique contant terms get inserted only once.

    g.insertLTE( c3 , jj );        // 3 <=  __tv1


    // __tv0 <= 3 && 6 < uf_call => __tv0 < uf_call(j) ?
    EXPECT_EQ(true, g.isLT( ii , uf_call ));
    // __tv0 <= 3 && 3 <= __tv1 => __tv0 <= __tv1 ?
    EXPECT_EQ(true, g.isLTE( ii , jj ));
    // 3 < 6 ? 
    EXPECT_EQ(true, g.isLT(c1,c2));
    // 3 < -6 ?
    EXPECT_EQ(false, g.isLT(c1,c4));
    // 3 == 3 ?
    EXPECT_EQ(true, g.isEqual(c1,c3));

    EXPECT_EQ("TermPartOrdGraph:\n\tNumTerms = 7\n\tNonNegativeTerms = \n"
              "\t\tN\n\tUFCallTerm2IntMap = \n\t\tterm = tau(__tv2), id ="
              " 2\n\tTupleVarTerm2IntMap = \n\t\tterm = __tv0, id = 3\n\t"
              "\tterm = __tv1, id = 5\n\tVarTerm2IntMap = \n\t\tterm = N,"
              " id = 0\n\tTerm2IntMap = \n\t\tterm = -6, id = 6\n\t\tterm"
              " = 3, id = 4\n\t\tterm = 6, id = 1\nPartOrdGraph:\n\tmCurN"
              " = 7\tmMaxN = 10\n\t\t0\t1\t2\t3\t4\t5\t6\n\t0\t=\t\t\t\t\t"
              "\t\n\t1\t\t=\t<\t\t\t\t\n\t2\t\t\t=\t\t\t\t\n\t3\t\t<\t<\t"
              "=\t<=\t<=\t\n\t4\t\t<\t<\t\t=\t<=\t\n\t5\t\t\t\t\t\t=\t\n"
              "\t6\t\t<\t<\t\t<\t<\t=\n\n",  g.toString());
    
    delete v;
    delete ii;
    delete jj;
    delete uf_call;
    delete c1;
    delete c2;
    delete c3;
    delete c4;
}
