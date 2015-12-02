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

/*      TermPartOrdGraph g;
 *      UFCallTerm uf = new UFCallTerm( ... );
 *      TupleVarTerm tv = new TupleVarTerm( ... );
 *      g.insertTerm( uf );
 *      g.insertTerm( tv );
 *      g.insertTerm( uf );     // Will do object comparison and only keep one.
 *      
 *      g.doneInsertingTerms();
 *
 *      // Insertions of orderings can only come after doneInsertingTerms.
 *      g.insertLTE( uf, tv);   // (*uf) <= (*tv)
 *      g.insertLT( uf, tv);    // (*uf) < (*tv)
 *      g.insertEqual( uf,tv ); // Will cause an error in this case.
 *
 *      // Indicating a term is non-negative can happen at anytime
 *      // after the term has been inserted.
 *      g.termNonNegative( tv );
 *
 *      //====== Queries ======
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
// Test the toString method.
TEST(TermPartOrdGraphTest, TermPartOrdGraphToString) {

    TermPartOrdGraph g;
    VarTerm * v = new VarTerm( "N" );
    g.insertTerm( v );

    EXPECT_EQ("TermPartOrdGraph:\n\tmDoneInsertingTerms = 0\n"
              "\tmNumTerms = 1\n\tmNonNegativeTerms = \n"
              "\tmUFCallTerm2IntMap = \n\tmTupleVarTerm2IntMap ="
              " \n\tmVarTerm2IntMap = \n\t\tterm = N, id = 0\n", 
              g.toString());
    //std::cout << g.toString();
    
    delete v;
}

#pragma mark TermPartOrdGraphIsNonNegative
// Test the toString method.
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
// Test the toString method.
TEST(TermPartOrdGraphTest, TermPartOrdGraphUniqueTerms) {

    TermPartOrdGraph g;
    
    // term = N
    VarTerm * v = new VarTerm( 3, "N" );
    g.insertTerm( v );
    delete v;
    g.termNonNegative( v );
    VarTerm * v2 = new VarTerm( -1, "N" );
    g.insertTerm( v2 );
    delete v2;
    
    // Check that non-negative stayed the same.
    EXPECT_EQ(true, g.isNonNegative( v ));
    
    {
    // Check the size of the unique sets.
    std::set<UFCallTerm*> ufCallTermSet = g.getUniqueUFCallTerms();
    EXPECT_EQ(0,ufCallTermSet.size());
    std::set<Term*> termSet = g.getAllUniqueTerms();
    EXPECT_EQ(1,termSet.size());
    }
    
    {
    // term = tau(i,j)
    UFCallTerm* uf_call = new UFCallTerm("tau", 2);
    Exp *tau_arg_1 = new Exp();
    tau_arg_1->addTerm(new VarTerm("i"));
    uf_call->setParamExp(0,tau_arg_1);
    Exp *tau_arg_2 = new Exp();
    tau_arg_2->addTerm(new VarTerm("j"));
    uf_call->setParamExp(1,tau_arg_2);

    g.insertTerm( uf_call );
    delete uf_call;
    }

    {
    // Check the size of the unique sets.
    std::set<UFCallTerm*> ufCallTermSet = g.getUniqueUFCallTerms();
    EXPECT_EQ(1,ufCallTermSet.size());
    std::set<Term*> termSet = g.getAllUniqueTerms();
    EXPECT_EQ(2,termSet.size());
    }
        
    { // Inserting same thing twice on purpose.
    // term = tau(i,j)
    UFCallTerm* uf_call = new UFCallTerm("tau", 2);
    Exp *tau_arg_1 = new Exp();
    tau_arg_1->addTerm(new VarTerm("i"));
    uf_call->setParamExp(0,tau_arg_1);
    Exp *tau_arg_2 = new Exp();
    tau_arg_2->addTerm(new VarTerm("j"));
    uf_call->setParamExp(1,tau_arg_2);

    g.insertTerm( uf_call );
    delete uf_call;
    }

    {
    // Check the size of the unique sets.
    std::set<UFCallTerm*> ufCallTermSet = g.getUniqueUFCallTerms();
    EXPECT_EQ(1,ufCallTermSet.size());
    std::set<Term*> termSet = g.getAllUniqueTerms();
    EXPECT_EQ(2,termSet.size());
    }

    {
    // term = g(3 __tv3)
    UFCallTerm* uf_call = new UFCallTerm("g", 1);
    Exp *g0 = new Exp();
    g0->addTerm(new TupleVarTerm(3, 3));
    uf_call->setParamExp(0,g0);
    
    g.insertTerm( uf_call );
    delete uf_call;
    }

    {
    // Check the size of the unique sets.
    std::set<UFCallTerm*> ufCallTermSet = g.getUniqueUFCallTerms();
    EXPECT_EQ(2,ufCallTermSet.size());
    std::set<Term*> termSet = g.getAllUniqueTerms();
    EXPECT_EQ(3,termSet.size());
    }

    {
    // term = g(2 __tv3)
    UFCallTerm* uf_call = new UFCallTerm("g", 1);
    Exp *g0 = new Exp();
    g0->addTerm(new TupleVarTerm(2, 3));
    uf_call->setParamExp(0,g0);
    
    g.insertTerm( uf_call );
    delete uf_call;
    }

    {
    // Check the size of the unique sets.
    std::set<UFCallTerm*> ufCallTermSet = g.getUniqueUFCallTerms();
    EXPECT_EQ(3,ufCallTermSet.size());
    std::set<Term*> termSet = g.getAllUniqueTerms();
    EXPECT_EQ(4,termSet.size());
    }

    std::cout << g.toString();

}
