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

using iegenlib::TupleVarTerm;
using iegenlib::Term;
using iegenlib::VarTerm;
using iegenlib::UFCallTerm;
using iegenlib::TupleExpTerm;


#pragma mark UFCallMapAndBoundsDomainNoTupleVars
// Test the UFCallMapAndBounds methods related to bounding by domain.
// There should be no temporary variables introduced.
// In this example the parameter expressions do not use tuple vars.
TEST(UFCallMapAndBoundsTest, UFCallMapAndBoundsDomainNoTupleVars) {

    Set* domain = new Set("{[j] : lb_j <= j && j <= ub_j } ");
    Set* range = new Set("{[i] : lb_i <= i && i <= ub_i } ");

    // declare f and g as non-bijective functions
    iegenlib::setCurrEnv();
    iegenlib::appendCurrEnv("f", domain, range, false, 
                            iegenlib::Monotonic_NONE);

    // Assume original set is
    // {[x,y] : x = f(N)}
    Set* original_set = new Set("{[x,y] : x = f(N)}");
    UFCallMapAndBounds ufcall_map(original_set->getTupleDecl());

    // Construct the UFCallTerm f(N)
    Exp *e1 = new Exp();  // N
    e1->addTerm(new VarTerm("N"));
        
    UFCallTerm* ufcallptr = new UFCallTerm("f", 1);
    ufcallptr->setParamExp(0,e1);

    // boundByDomain is what we are testing
    ufcall_map.boundByDomain(ufcallptr);
    
    // boundByDomain should modify the constraints UFCallMapAndBounds 
    // is maintaining.
    EXPECT_EQ(0u, ufcall_map.numTempVars());
    Set* constraints = ufcall_map.cloneConstraints();
    Set* expected = new Set("{[x,y] : lb_j<=N && N<=ub_j}");
    EXPECT_EQ(expected->toString(), constraints->toString());
    

    delete constraints;
    delete expected;
    delete original_set;
    delete ufcallptr;
}

