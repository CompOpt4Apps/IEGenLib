/*!
 * \file UFCallMapAndBounds_test.cc
 *
 * \brief Test for the UFCallMapAndBounds class.
 *
 * \date Started: 7/30/2013
 *
 * \authors Michelle Strout and Mark Heim
 *
 * Copyright (c) 2013, Colorado State University <br>
 * All rights reserved. <br>
 * See ../../COPYING for details. <br>
 */

#include "expression.h"
#include "UFCallMapAndBounds.h"
#include "set_relation.h"

#include <gtest/gtest.h>
#include <utility>
#include <iostream>
#include <fstream>

using iegenlib::UFCallMapAndBounds;
using iegenlib::Exp;
using iegenlib::TupleVarTerm;
using iegenlib::Term;
using iegenlib::VarTerm;
using iegenlib::UFCallTerm;
using iegenlib::Set;
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

#pragma mark UFCallMapAndBoundsDomainWithTupleVars
// Test the UFCallMapAndBounds methods related to bounding by domain.
// There should be no temporary variables introduced.
// In this example the parameter expressions should use tuple vars.
TEST(UFCallMapAndBoundsTest, UFCallMapAndBoundsDomainWithTupleVars) {

    Set* domain = new Set("{[j] : lb_j <= j && j <= ub_j } ");
    Set* range = new Set("{[i] : lb_i <= i && i <= ub_i } ");

    // declare f and g as non-bijective functions
    iegenlib::setCurrEnv();
    iegenlib::appendCurrEnv("f", domain, range, false,
                            iegenlib::Monotonic_NONE);

    // Assume original set is
    // {[x,y] : x = f(N)}
    Set* original_set = new Set("{[x,y] : x = f(x+y) && 4=f(N)}");
    UFCallMapAndBounds ufcall_map(original_set->getTupleDecl());

    // Construct the UFCallTerm f(x+y)
    Exp *e1 = new Exp();  // x+y
    e1->addTerm(new TupleVarTerm(0));
    e1->addTerm(new TupleVarTerm(1));
        
    UFCallTerm* ufcallptr = new UFCallTerm("f", 1);
    ufcallptr->setParamExp(0,e1);

    // boundByDomain is what we are testing
    ufcall_map.boundByDomain(ufcallptr);
    
    // boundByDomain should modify the constraints UFCallMapAndBounds 
    // is maintaining.
    EXPECT_EQ(0u, ufcall_map.numTempVars());
    Set* constraints = ufcall_map.cloneConstraints();
    Set* expected = new Set("{[x,y] : lb_j<=x+y && x+y<=ub_j}");
    EXPECT_EQ(expected->toString(), constraints->toString());
 
    // Construct the UFCallTerm f(N)
    e1 = new Exp();  // N
    e1->addTerm(new VarTerm("N"));
        
    UFCallTerm* ufcallptr2 = new UFCallTerm("f", 1);
    ufcallptr2->setParamExp(0,e1);

     // boundByDomain is what we are testing
    ufcall_map.boundByDomain(ufcallptr2);
    
    // boundByDomain should modify the constraints UFCallMapAndBounds 
    // is maintaining.
    EXPECT_EQ(0u, ufcall_map.numTempVars());
    delete constraints;
    constraints = ufcall_map.cloneConstraints();
    delete expected;
    expected = new Set("{[x,y]: lb_j<=x+y && x+y<=ub_j && lb_j<=N && N<=ub_j}");
    EXPECT_EQ(expected->toString(), constraints->toString());
   
    delete constraints;
    delete expected;
    delete original_set;
    delete ufcallptr;
    delete ufcallptr2;
}

#pragma mark UFCallMapAndBoundsDomain2D
// Test the UFCallMapAndBounds methods related to bounding by domain.
// There should be no temporary variables introduced.
// Here the domain is 2D.
TEST(UFCallMapAndBoundsTest, UFCallMapAndBoundsDomain2D) {

    Set* range = new Set("{[0,t] : lb_t <= t && t <= ub_t } "
                         "union {[1,s] : lb_s <= s && s <= ub_s }");

    Set* domain = new Set("{[p,v] : lb_p <= p && p <= ub_p && lb_v <= v "
                         "&& v <= ub_v } "
                         "union {[q,w] : lb_q <= q && q <= ub_q "
                         "&& lb_w <= w && w <= ub_w }");

    // declare f and g as non-bijective functions
    iegenlib::setCurrEnv();
    iegenlib::appendCurrEnv("f", domain, range, false,
                            iegenlib::Monotonic_NONE);

    // Assume original set is
    // {[x,y] : x = f(N,y)}
    Set* original_set = new Set("{[x,y] : x = f(N,y)}");
    UFCallMapAndBounds ufcall_map(original_set->getTupleDecl());

    // Construct the UFCallTerm f(N,y)
    Exp *e1 = new Exp();  // N
    e1->addTerm(new VarTerm("N"));
        
    UFCallTerm* ufcallptr = new UFCallTerm("f", 2);
    ufcallptr->setParamExp(0,e1);
    e1 = new Exp(); // y
    e1->addTerm(new TupleVarTerm(7,1));
    ufcallptr->setParamExp(1,e1);

    // boundByDomain is what we are testing
    ufcall_map.boundByDomain(ufcallptr);
    
    // boundByDomain should modify the constraints UFCallMapAndBounds 
    // is maintaining.
    EXPECT_EQ(0u, ufcall_map.numTempVars());
    Set* constraints = ufcall_map.cloneConstraints();
    Set* expected = new Set("{[x,y] : lb_p <= N && N <= ub_p && lb_v <= 7 y "
                         "&& 7 y <= ub_v } "
                         "union {[x,y] : lb_q <= N && N <= ub_q "
                         "&& lb_w <= 7 y && 7 y <= ub_w }");
    EXPECT_EQ(expected->toString(), constraints->toString());
    

    delete constraints;
    delete expected;
    delete original_set;
    delete ufcallptr;
}

#pragma mark UFCallMapAndBoundsRange1D
// Range is 1D.  One temporary variable should be created.
TEST(UFCallMapAndBoundsTest, UFCallMapAndBoundsRange1D) {

    Set* range = new Set("{[i] : lb_i <= i && i <= ub_i } ");

    Set* domain = new Set("{[p,v] : lb_p <= p && p <= ub_p && lb_v <= v "
                         "&& v <= ub_v } "
                         "union {[q,w] : lb_q <= q && q <= ub_q "
                         "&& lb_w <= w && w <= ub_w }");

    // declare f as a non-bijective function.    
    iegenlib::setCurrEnv();
    iegenlib::appendCurrEnv("f", domain, range, false,
                            iegenlib::Monotonic_NONE);

    // Assume original set is
    // {[x,y] : 7 + x = f(N,y)}
    Set* original_set = new Set("{[x,y] : x = f(N,y)}");
    UFCallMapAndBounds ufcall_map(original_set->getTupleDecl());

    // Construct the UFCallTerm f(N,7 y)
    Exp *e1 = new Exp();  // N
    e1->addTerm(new VarTerm("N"));
        
    UFCallTerm* ufcallptr = new UFCallTerm("f", 2);
    ufcallptr->setParamExp(0,e1);
    e1 = new Exp(); // y
    e1->addTerm(new TupleVarTerm(7,1));
    ufcallptr->setParamExp(1,e1);
    
    // bound by range, method we are testing
    TupleExpTerm* t = ufcall_map.boundByRange( ufcallptr );
    delete t;

    // boundByRange should modify the constraints UFCallMapAndBounds 
    // is maintaining.  It should introduce one temporary.
    EXPECT_EQ(1u, ufcall_map.numTempVars());
    Set* constraints = ufcall_map.cloneConstraints();
    Set* expected 
        = new Set("{[x,y,tv2] : lb_i <= tv2 && tv2 <= ub_i }");
    EXPECT_EQ(expected->toString(), constraints->toString());
    UFCallTerm* fetched_ufcall = ufcall_map.cloneUFCall(2); // query tv2
    EXPECT_EQ("f(N, 7 __tv1)", fetched_ufcall->toString() );

    delete original_set;
    delete expected;
    delete ufcallptr;
    delete constraints;
    delete fetched_ufcall;
}

#pragma mark UFCallMapAndBoundsRange2D
// Test the UFCallMapAndBounds methods that generate bounds for
// a multi-dimensional expression.
TEST(UFCallMapAndBoundsTest, UFCallMapAndBoundsRange2D) {

    Set* range = new Set("{[0,t] : lb_t <= t && t <= ub_t } "
                         "union {[1,s] : lb_s <= s && s <= ub_s }");

    Set* domain = new Set("{[p,v] : lb_p <= p && p <= ub_p && lb_v <= v "
                         "&& v <= ub_v } "
                         "union {[q,w] : lb_q <= q && q <= ub_q "
                         "&& lb_w <= w && w <= ub_w }");

    // declare f as a non-bijective function.    
    iegenlib::setCurrEnv();
    iegenlib::appendCurrEnv("f", domain, range, false,
                            iegenlib::Monotonic_NONE);

    // Assume original set is
    // {[x,y] : (7 + x,y) = f(N,y)}
    Set* original_set = new Set("{[x,y] : (7+x,y) = f(N,y)}");
    UFCallMapAndBounds ufcall_map(original_set->getTupleDecl());

    // Construct the UFCallTerm f(N,7 y)
    Exp *e1 = new Exp();  // N
    e1->addTerm(new VarTerm("N"));
        
    UFCallTerm* ufcallptr = new UFCallTerm("f", 2);
    ufcallptr->setParamExp(0,e1);
    e1 = new Exp(); // y
    e1->addTerm(new TupleVarTerm(7,1));
    ufcallptr->setParamExp(1,e1);
    
    // bound by range, method we are testing
    TupleExpTerm* t = ufcall_map.boundByRange( ufcallptr );
    delete t;

    // boundByRange should modify the constraints UFCallMapAndBounds 
    // is maintaining.  It should introduce two temporaries.
    EXPECT_EQ(2u, ufcall_map.numTempVars());
    Set* constraints = ufcall_map.cloneConstraints();
    Set* expected = new Set("{[x,y,tv2,tv3] : "
                    "tv2=0 && lb_t <= tv3 && tv3 <= ub_t}"
                    " union {[x,y,tv2,tv3] : "
                    "tv2=1 && lb_s <= tv3 && tv3 <= ub_s }");
    EXPECT_EQ(expected->toString(), constraints->toString());
    UFCallTerm* fetched_ufcall = ufcall_map.cloneUFCall(2); // tv2 = ?
    //EXPECT_EQ("f(N, 7 __tv1)", fetched_ufcall->toString() ); 
    EXPECT_EQ("f(N, 7 __tv1)[0]", fetched_ufcall->toString() );

    delete original_set;
    delete expected;
    delete ufcallptr;
    delete fetched_ufcall;
    delete constraints;
}
