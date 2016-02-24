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

    TermPartOrdGraph g;
    VarTerm * v = new VarTerm( "N" );
    g.insertTerm( v );

    EXPECT_EQ("TermPartOrdGraph:\n\tDoneInsertingTerms = 0\n"
              "\tNumTerms = 1\n\tNonNegativeTerms = \n"
              "\tUFCallTerm2IntMap = \n\tTupleVarTerm2IntMap ="
              " \n\tVarTerm2IntMap = \n\t\tterm = N, id = 0\n", 
              g.toString());
    //std::cout << g.toString();
    
    delete v;
}

#pragma mark TermPartOrdGraphIsNonNegative
TEST(TermPartOrdGraphTest, TermPartOrdGraphIsNonNegative) {

    TermPartOrdGraph g;
    VarTerm * v = new VarTerm( "N" );
    g.insertTerm( v );
    g.termNonNegative( v );

    UFCallTerm* uf_call = new UFCallTerm("tau", 2); // tau(i,j)
    Exp *tau_arg_1 = new Exp();
    tau_arg_1->addTerm(new VarTerm("i"));
    uf_call->setParamExp(0,tau_arg_1);
    Exp *tau_arg_2 = new Exp();
    tau_arg_2->addTerm(new VarTerm("j"));
    uf_call->setParamExp(1,tau_arg_2);

    g.insertTerm( uf_call );
    
    //std::cout << g.toString();

    EXPECT_EQ(true, g.isNonNegative( v ));
    EXPECT_EQ(false, g.isNonNegative( uf_call ));
    
    delete v;
    delete uf_call;
}

#pragma mark TermPartOrdGraphUniqueTerms
TEST(TermPartOrdGraphTest, TermPartOrdGraphUniqueTerms) {

    TermPartOrdGraph g;
    
    // term = N
    VarTerm * v = new VarTerm( 3, "N" );
    g.insertTerm( v );
    g.termNonNegative( v );
    VarTerm * v2 = new VarTerm( -1, "N" );
    g.insertTerm( v2 );
    
    // Check that non-negative stayed the same.
    EXPECT_EQ(true, g.isNonNegative( v ));
    
    {
    // Check the size of the unique sets.
    std::set<UFCallTerm*> ufCallTermSet = g.getUniqueUFCallTerms();
    EXPECT_EQ(0,ufCallTermSet.size());
    std::set<Term*> termSet = g.getAllUniqueTerms();
    EXPECT_EQ(1,termSet.size());
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
    }
    g.insertTerm( uf_call );

    {
    // Check the size of the unique sets.
    std::set<UFCallTerm*> ufCallTermSet = g.getUniqueUFCallTerms();
    EXPECT_EQ(1,ufCallTermSet.size());
    std::set<Term*> termSet = g.getAllUniqueTerms();
    EXPECT_EQ(2,termSet.size());
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
    }
    g.insertTerm( uf_call2 );

    {
    // Check the size of the unique sets.
    std::set<UFCallTerm*> ufCallTermSet = g.getUniqueUFCallTerms();
    EXPECT_EQ(1,ufCallTermSet.size());
    std::set<Term*> termSet = g.getAllUniqueTerms();
    EXPECT_EQ(2,termSet.size());
    }

    // term = g(3 __tv3)
    UFCallTerm* uf_call3 = new UFCallTerm("g", 1);
    {
    Exp *g0 = new Exp();
    g0->addTerm(new TupleVarTerm(3, 3));
    uf_call3->setParamExp(0,g0);
    }
    g.insertTerm( uf_call3 );

    {
    // Check the size of the unique sets.
    std::set<UFCallTerm*> ufCallTermSet = g.getUniqueUFCallTerms();
    EXPECT_EQ(2,ufCallTermSet.size());
    std::set<Term*> termSet = g.getAllUniqueTerms();
    EXPECT_EQ(3,termSet.size());
    }

    // term = g(2 __tv3)
    UFCallTerm* uf_call4 = new UFCallTerm("g", 1);
    {
    Exp *g0 = new Exp();
    g0->addTerm(new TupleVarTerm(2, 3));
    uf_call4->setParamExp(0,g0);
    }
    g.insertTerm( uf_call4 );

    {
    // Check the size of the unique sets.
    std::set<UFCallTerm*> ufCallTermSet = g.getUniqueUFCallTerms();
    EXPECT_EQ(3,ufCallTermSet.size());
    std::set<Term*> termSet = g.getAllUniqueTerms();
    EXPECT_EQ(4,termSet.size());
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

    TermPartOrdGraph g;
    
    // term = N
    VarTerm * v = new VarTerm( 3, "N" );
    g.insertTerm( v );
    
    // term = tau(i,j)
    UFCallTerm* uf_call1 = new UFCallTerm("tau", 2);
    Exp *tau_arg_1 = new Exp();
    tau_arg_1->addTerm(new VarTerm("i"));
    uf_call1->setParamExp(0,tau_arg_1);
    Exp *tau_arg_2 = new Exp();
    tau_arg_2->addTerm(new VarTerm("j"));
    uf_call1->setParamExp(1,tau_arg_2);

    g.insertTerm( uf_call1 );

    // term = g(3 __tv3)
    UFCallTerm* uf_call2 = new UFCallTerm("g", 1);
    Exp* g0 = new Exp();
    g0->addTerm(new TupleVarTerm(3, 3));
    uf_call2->setParamExp(0,g0);
    
    g.insertTerm( uf_call2 );

    // term = g(2 __tv3)
    UFCallTerm* uf_call3 = new UFCallTerm("g", 1);
    g0 = new Exp();
    g0->addTerm(new TupleVarTerm(2, 3));
    uf_call3->setParamExp(0,g0);
    
    g.insertTerm( uf_call3 );


    g.doneInsertingTerms();

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
