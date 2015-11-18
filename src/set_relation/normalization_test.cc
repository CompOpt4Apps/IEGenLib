/*!
 * \file normalization_test.cc
 *
 * \brief Normalization tests on Sets and Relations.
 *
 * This file is to begin to test the Set and Relation classes, along with the
 * related Conjunction class, for normal form to facilitate equality testing.
 *
 * \date Started: 6/24/13
 * # $Revision:: 685                $: last committed revision
 * # $Date:: 2013-06-20 15:42:27 -0#$: date of last committed revision
 * # $Author::               $: author of last committed revision
 *
 * \authors Barbara Kreaseck and Michelle Strout
 *
 * Copyright (c) 201r32, Colorado State University <br>
 * All rights reserved. <br>
 * See ../../COPYING for details. <br>
 */

#include "set_relation.h"
#include "expression.h"
#include <util/util.h>
#include <gtest/gtest.h>

#include <isl/set.h>   // ISL Sets
#include <isl/map.h>   // ISL Relations

#include <util/UnionFindUniverse.h>

#include <utility>
#include <fstream>
#include <iostream>

// Exp and Term classes
using iegenlib::Exp;
using iegenlib::Term;
using iegenlib::UFCallTerm;
using iegenlib::TupleVarTerm;
using iegenlib::VarTerm;
using iegenlib::TupleDecl;

// set and relation classes
using iegenlib::Conjunction;
using iegenlib::SparseConstraints;
using iegenlib::Set;
using iegenlib::Relation;
using iegenlib::Environment;
using iegenlib::UninterpFunc;
using iegenlib::TupleExpTerm;

// helper function to eventually be located elsewhere
// runs the Relation through ISL and returns the resulting string

string getRelationStringFromISL(string rstr) {

    // Sending r thru ISL and returning resulting string
    
    // Get an isl context
    isl_ctx *ctx;
    ctx = isl_ctx_alloc();
    
    // Get an isl printer and associate to an isl context
    isl_printer * ip = NULL;
    ip = isl_printer_to_str(ctx);
    
    // load Relation r into ISL map
    isl_map* imap = NULL;
    imap = isl_map_read_from_str(ctx, rstr.c_str());
    
    // get string back from ISL map
    char * cstr;
    isl_printer_set_output_format(ip , ISL_FORMAT_ISL);
    isl_printer_print_map(ip ,imap);
    cstr=isl_printer_get_str(ip);
    string stringFromISL = cstr;
    
    // clean-up
    isl_printer_flush(ip);
    isl_printer_free(ip);
    free(cstr);
    isl_map_free(imap);
    imap= NULL;
    isl_ctx_free(ctx);  

	return stringFromISL;
}

// helper function to eventually be located elsewhere
// runs the Relation through ISL using isl_basic_map 
// and returns the resulting string

string getRelationStringFromBasicISL(string rstr) {

    // Sending r thru ISL and returning resulting string
    
    // Get an isl context
    isl_ctx *ctx;
    ctx = isl_ctx_alloc();
    
    // Get an isl printer and associate to an isl context
    isl_printer * ip = NULL;
    ip = isl_printer_to_str(ctx);
    
    // load Relation r into ISL map
    isl_basic_map* imap = NULL;
    imap = isl_basic_map_read_from_str(ctx, rstr.c_str());
    
    // get string back from ISL map
    char * cstr;
    isl_printer_set_output_format(ip , ISL_FORMAT_ISL);
    isl_printer_print_basic_map(ip ,imap);
    cstr=isl_printer_get_str(ip);
    string stringFromISL = cstr;
    
    // clean-up
    isl_printer_flush(ip);
    isl_printer_free(ip);
    free(cstr);
    isl_basic_map_free(imap);
    imap= NULL;
    isl_ctx_free(ctx);  

	return stringFromISL;
}


// first try at creating a normalized string form of a relation with UFCalls
// ... might be too specialized, but have to start somewhere ...
// string normalize(Relation *r) {
//
// }

// This is just a test setup class that is used in the other tests in this
// file.  We could create a data structure that many of the tests use.
class NormalizationTest:public::testing::Test {
public:
   // data structure declaration
    std::string root_dir;
    std::string dot_data;
protected:
   // method that sets up the data structure
   void SetUp();

   void TearDown() { }
};

void NormalizationTest::SetUp() { 

}

#pragma mark SimpleEqualities
// Test the simple equalities
TEST_F(NormalizationTest, SimpleEqualities) {

    // conjunction {[a,b,c] -> [d] : a=d && b=d && c=d }
    Exp a_minus_d;
    a_minus_d.addTerm(new VarTerm("a"));
    a_minus_d.addTerm(new VarTerm(-1,"d"));
   
    Exp b_minus_d;
    b_minus_d.addTerm(new VarTerm("b"));
    b_minus_d.addTerm(new VarTerm(-1,"d"));

    Exp c_minus_d;
    c_minus_d.addTerm(new VarTerm("c"));
    c_minus_d.addTerm(new VarTerm(-1,"d"));
    
    TupleDecl tdecl1(4);
    tdecl1.setTupleElem(0,"a");
    tdecl1.setTupleElem(1,"b");
    tdecl1.setTupleElem(2,"c");
    tdecl1.setTupleElem(3,"d");
    
    Conjunction *conj1r;
    conj1r = new Conjunction(4,3);
    conj1r->setTupleDecl(tdecl1);            // tdecl1 adopted
    conj1r->addEquality(a_minus_d.clone());
    conj1r->addEquality(b_minus_d.clone());
    conj1r->addEquality(c_minus_d.clone());
    conj1r->substituteTupleDecl();
    
    EXPECT_EQ("{ [a, b, c] -> [d] : __tv0 - __tv3 = 0 && __tv1 - __tv3 = 0 && "
              "__tv2 - __tv3 = 0 }", conj1r->toString());
    
    EXPECT_EQ("{ [a, b, c] -> [d] : a - d = 0 && b - d = 0 && c - d = 0 }", 
              conj1r->prettyPrintString());

	Relation* expected  = new Relation("{ [a, b, c] -> [d] : a - d = 0 && "
	                         "b - d = 0 && c - d = 0 }");
	conj1r->normalize();

	EXPECT_EQ(expected->toString(), conj1r->toString());
	EXPECT_EQ(expected->prettyPrintString(), conj1r->prettyPrintString());

	delete conj1r;
	delete expected;
}

#pragma mark SimpleEqOutOfOrder
TEST_F(NormalizationTest, SimpleEqOutOfOrder) {

    // conjunction {[a,b,c] -> [d] : d=a && d=b && d=c }
    Exp d_minus_a;
    d_minus_a.addTerm(new VarTerm("d"));
    d_minus_a.addTerm(new VarTerm(-1,"a"));
   
    Exp d_minus_b;
    d_minus_b.addTerm(new VarTerm("d"));
    d_minus_b.addTerm(new VarTerm(-1,"b"));

    Exp d_minus_c;
    d_minus_c.addTerm(new VarTerm("d"));
    d_minus_c.addTerm(new VarTerm(-1,"c"));
    
    TupleDecl tdecl1(4);
    tdecl1.setTupleElem(0,"a");
    tdecl1.setTupleElem(1,"b");
    tdecl1.setTupleElem(2,"c");
    tdecl1.setTupleElem(3,"d");
    
    Conjunction *conj1r;
    conj1r = new Conjunction(4,3);
    conj1r->setTupleDecl(tdecl1);            // tdecl1 adopted
    conj1r->addEquality(d_minus_a.clone());
    conj1r->addEquality(d_minus_b.clone());
    conj1r->addEquality(d_minus_c.clone());
    conj1r->substituteTupleDecl();
    
    EXPECT_EQ("{ [a, b, c] -> [d] : __tv0 - __tv3 = 0 && __tv1 - __tv3 = 0 && "
              "__tv2 - __tv3 = 0 }", conj1r->toString()); // fails
	
    EXPECT_EQ("{ [a, b, c] -> [d] : a - d = 0 && b - d = 0 && c - d = 0 }", 
            conj1r->prettyPrintString()); // passes

	Relation* expected  = new Relation("{ [a, b, c] -> [d] : a - d = 0 && "
	                         "b - d = 0 && c - d = 0 }");
	conj1r->normalize();
	EXPECT_EQ(expected->toString(), conj1r->toString());
	EXPECT_EQ(expected->prettyPrintString(), conj1r->prettyPrintString());

	delete conj1r;
}

#pragma mark SimpleEqOutOfOrderRelation
TEST_F(NormalizationTest, SimpleEqOutOfOrderRelation) {

    Relation *r1 = new Relation("{[a,b,c] -> [d] : d=a && d=b && d=c }");
    Relation *r2 = new Relation("{[a,b,c] -> [d] : b=a && c=b && d=c }");
    Relation *r3 = new Relation("{[a,a,a] -> [a] }");
    Relation *r4 = new Relation("{[a,a,b] -> [b] : b=a }");
    Relation *r5 = new Relation("{[a,a,b] -> [a] : b=a }");


    // toString() output checks
    EXPECT_EQ("{ [a, b, c] -> [d] : "
              "__tv0 - __tv3 = 0 && __tv1 - __tv3 = 0 && __tv2 - __tv3 = 0 }", 
              r1->toString());
    EXPECT_EQ("{ [a, b, c] -> [d] : "
              "__tv0 - __tv1 = 0 && __tv1 - __tv2 = 0 && __tv2 - __tv3 = 0 }", 
              r2->toString());
    EXPECT_EQ("{ [a, a, a] -> [a] : "
              "__tv0 - __tv1 = 0 && __tv0 - __tv2 = 0 && __tv0 - __tv3 = 0 }", 
              r3->toString());
    EXPECT_EQ("{ [a, a, b] -> [b] : "
              "__tv0 - __tv1 = 0 && __tv0 - __tv2 = 0 && __tv2 - __tv3 = 0 }", 
              r4->toString());
    EXPECT_EQ("{ [a, a, b] -> [a] : "
              "__tv0 - __tv1 = 0 && __tv0 - __tv2 = 0 && __tv0 - __tv3 = 0 }", 
              r5->toString());

    // prettyPrintString() output checks
    EXPECT_EQ("{ [a, b, c] -> [d] : a - d = 0 && b - d = 0 && c - d = 0 }", 
              r1->prettyPrintString());
    EXPECT_EQ("{ [a, b, c] -> [d] : a - b = 0 && b - c = 0 && c - d = 0 }", 
              r2->prettyPrintString());
    EXPECT_EQ("{ [a, a, a] -> [a] : a - a = 0 && a - a = 0 && a - a = 0 }", 
              r3->prettyPrintString());
    EXPECT_EQ("{ [a, a, b] -> [b] : a - a = 0 && a - b = 0 && b - b = 0 }", 
              r4->prettyPrintString());
    EXPECT_EQ("{ [a, a, b] -> [a] : a - a = 0 && a - b = 0 && a - a = 0 }", 
              r5->prettyPrintString());


	r1->normalize();
	r2->normalize();
	r3->normalize();
	r4->normalize();
	r5->normalize();

    EXPECT_EQ("{ [a, b, c] -> [d] : "
              "__tv0 - __tv1 = 0 && __tv0 - __tv2 = 0 && __tv0 - __tv3 = 0 }", 
              r1->toString());
    EXPECT_EQ("{ [a, b, c] -> [d] : "
              "__tv0 - __tv1 = 0 && __tv0 - __tv2 = 0 && __tv0 - __tv3 = 0 }", 
              r2->toString());
    EXPECT_EQ("{ [a, a, a] -> [a] : "
              "__tv0 - __tv1 = 0 && __tv0 - __tv2 = 0 && __tv0 - __tv3 = 0 }", 
              r3->toString());
    EXPECT_EQ("{ [a, a, b] -> [b] : "
              "__tv0 - __tv1 = 0 && __tv0 - __tv2 = 0 && __tv0 - __tv3 = 0 }", 
              r4->toString());
    EXPECT_EQ("{ [a, a, b] -> [a] : "
              "__tv0 - __tv1 = 0 && __tv0 - __tv2 = 0 && __tv0 - __tv3 = 0 }", 
              r5->toString());

    EXPECT_EQ("{ [a, b, c] -> [d] : a - b = 0 && a - c = 0 && a - d = 0 }", 
              r1->prettyPrintString());
    EXPECT_EQ("{ [a, b, c] -> [d] : a - b = 0 && a - c = 0 && a - d = 0 }", 
              r2->prettyPrintString());
    EXPECT_EQ("{ [a, a, a] -> [a] : a - a = 0 && a - a = 0 && a - a = 0 }", 
              r3->prettyPrintString());
    EXPECT_EQ("{ [a, a, b] -> [b] : a - a = 0 && a - b = 0 && a - b = 0 }", 
              r4->prettyPrintString());
    EXPECT_EQ("{ [a, a, b] -> [a] : a - a = 0 && a - b = 0 && a - a = 0 }", 
              r5->prettyPrintString());
    
    
    //OR

	ASSERT_TRUE ((*r1) == (*r2));
	ASSERT_TRUE ((*r1) == (*r3));
	ASSERT_TRUE ((*r1) == (*r4));
	ASSERT_TRUE ((*r1) == (*r5));
	


	delete r1;
	delete r2;
	delete r3;
	delete r4;
	delete r5;
	
}

#pragma mark EqUFParamRelation
// differently-named equal variables and 
// differently-named equal variables sent as parameters to UFCall
TEST_F(NormalizationTest, EqUFParamRelation) {

    iegenlib::setCurrEnv();
    iegenlib::appendCurrEnv("theta",
        new Set("{[i]:0<=i &&i<N}"), 
        new Set("{[j]:0<=j &&j<P}"), true);

    Relation *r1 = new Relation("{[tstep,i] -> [s0,t,i1,x] : "
                   "tstep = s0 && i = i1 && t = theta(i) && x = 0 }");

    Relation *r2 = new Relation("{[tstep,i] -> [tstep,t,i1,x1] : "
                   "i = i1 && t = theta(i1) && x1 = 0 }");

    // checking toString() results
    EXPECT_EQ("{ [tstep, i] -> [s0, t, i1, x] : __tv5 = 0 && "
                   "__tv0 - __tv2 = 0 && "
                   "__tv1 - __tv4 = 0 && __tv3 - theta(__tv1) = 0 }",
                   r1->toString());

    EXPECT_EQ("{ [tstep, i] -> [tstep, t, i1, x1] : __tv5 = 0 && "
                   "__tv0 - __tv2 = 0 && "
                   "__tv1 - __tv4 = 0 && __tv3 - theta(__tv4) = 0 }",
                   r2->toString());

    r1->normalize();
//std::cout << "r1 = " << r1->toString() << std::endl;

    r2->normalize();
//std::cout << "r2 = " << r2->toString() << std::endl;

	ASSERT_TRUE((*r1) == (*r2));  // works!
	
	delete r1;
	delete r2;
}

#pragma mark EqUFParamRelation2
// differently-named equal variables sent as parameters to UFCall
// same as EqUFParamRelation, but with two arguments in the call
TEST_F(NormalizationTest, EqUFParamRelation2) {
    
    iegenlib::setCurrEnv();
    iegenlib::appendCurrEnv("theta",
        new Set("{[i,j]:0<=i &&i<M && 0<=j &&j<P}"),
        new Set("{[i]:0<=i &&i<N}"), true);

    Relation *r1 = new Relation("{[tstep,i] -> [s0,t,i1,x] : "
                   "tstep = s0 && i = i1 && t = theta(0,i) && x = 0 }");

    Relation *r2 = new Relation("{[tstep,i] -> [s0,t,i1,x] : "
                   "tstep = s0 && i = i1 && t = theta(0,i1) && x = 0 }");

    r1->normalize();
//std::cout << "r1 = " << r1->toString() << std::endl;

    r2->normalize();
//std::cout << "r2 = " << r2->toString() << std::endl;


	ASSERT_TRUE((*r1) == (*r2));  // works!
	
	delete r1;
	delete r2;
}

#pragma mark EqUFSignatureRelation
// differently-named equal variables sent as parameters to differently 
// named UFCall same as EqUFParamRelation2, but with differently-named UFCalls
TEST_F(NormalizationTest, EqUFSignatureRelation) {


    iegenlib::setCurrEnv();
    iegenlib::appendCurrEnv("theta",
        new Set("{[i,j]:0<=i &&i<M && 0<=j &&j<P}"),
        new Set("{[i]:0<=i &&i<N}"), true);
    iegenlib::appendCurrEnv("sigma",
        new Set("{[i,j]:0<=i &&i<M && 0<=j &&j<P}"),
        new Set("{[i]:0<=i &&i<N}"), true);

    Relation *r1 = new Relation("{[tstep,i] -> [0,s0,1,t,0,i1,x] : "
                   "tstep = s0 && i = i1 && t = theta(0,i) && x = 0 }");

    Relation *r2 = new Relation("{[tstep,i] -> [0,s0,1,t,0,i1,x] : "
                   "tstep = s0 && i = i1 && t = sigma(0,i1) && x = 0 }");

    r1->normalize();
//std::cout << "r1 = " << r1->toString() << std::endl;

    r2->normalize();
//std::cout << "r2 = " << r2->toString() << std::endl;

	ASSERT_FALSE((*r1) == (*r2));  // works!

	delete r1;
	delete r2;
}

#pragma mark NestedUFCalls1D1D1D
// nested UFCalls with single dimension arity for range and 
// domain 1D -> 1D -> 1D
TEST_F(NormalizationTest, NestedUFCalls1D1D1D) {

    iegenlib::setCurrEnv();
    
    iegenlib::appendCurrEnv("left",
        new Set("{[i]:0<=i &&i<K}"),
        new Set("{[i]:0<=i &&i<M}"), true);
    iegenlib::appendCurrEnv("sigma",
        new Set("{[i]:0<=i &&i<M}"),
        new Set("{[i]:0<=i &&i<N}"), true);
	
    Set *r1 = new Set("{[s,i1,s2,e] : "
                   "s = s2 && i1 = sigma(left(e)) }");

    Set *r2 = new Set("{[s,i1,s,f] : "
                   "i1 = sigma(left(f)) }");

    EXPECT_EQ("{ [s, i1, s2, e] : "
              "__tv0 - __tv2 = 0 && __tv1 - sigma(left(__tv3)) = 0 }",
              r1->toString());
 
    EXPECT_EQ("{ [s, i1, s, f] : "
              "__tv0 - __tv2 = 0 && __tv1 - sigma(left(__tv3)) = 0 }",
              r2->toString());
             
    // passes because the e and f are in same tuple location
	ASSERT_TRUE((*r1) == (*r2));  

    r1->normalize();
//std::cout << "r1 = " << r1->toString() << std::endl;

    r2->normalize();
//std::cout << "r2 = " << r2->toString() << std::endl;

	EXPECT_TRUE((*r1) == (*r2));  // works!
	
	delete r1;
	delete r2;

}

#pragma mark NestedUFCalls1D2D1D
// nested UFCalls with 1D -> 2D ->1D
TEST_F(NormalizationTest, NestedUFCalls1D2D1D) {

    /* // ------------------------------------------------------------
    // OLD CODE ... keeping for the string routines ... 
    // don't have to look them up again
    
    // 1) i1 = u1           ... add this one back in as a constraint
    fromISL1.erase(fromISL1.length()-1,1); // remove last character ... '}'
    fromISL1 += string("and i1 = u1 ");
    fromISL1 += string("}");
    
    // 2) u1 = sigma(u0)    ... replace all "u1" with "sigma(u0)" 
    int pos;
    std::string target = "u1";
    std::string replacement = "sigma(u0)";
    while ((pos = fromISL1.find(target)) != std::string::npos) {
        fromISL1.replace(pos, target.length(), replacement);
    }

    // 3) u0 = left(e)      ... replace all "u0" with "left(e)"   
    Set* r1mod = new Set(fromISL1.c_str());
	target = "u0";
	replacement = "left(e)";
    while ((pos = fromISL1.find(target)) != std::string::npos) {
        fromISL1.replace(pos, target.length(), replacement);
    }
    
    */ // -----------------------------------------------------------
    
    iegenlib::setCurrEnv();
    
    iegenlib::appendCurrEnv("left",
        new Set("{[i]:0<=i &&i<K}"),
        new Set("{[i,j]:0<=i &&i<M && 0<=j &&j<P}"), true);
    iegenlib::appendCurrEnv("sigma",
        new Set("{[i,j]:0<=i &&i<M && 0<=j &&j<P}"),
        new Set("{[i]:0<=i &&i<N}"), true);

    Set *r1 = new Set("{[s,i1,s2,e] : "
                   "s = s2 && i1 = sigma(left(e)) }");

    Set *r2 = new Set("{[s,i1,s,f] : "
                   "i1 = sigma(left(f)) }");

    EXPECT_EQ("{ [s, i1, s2, e] : "
              "__tv0 - __tv2 = 0 && __tv1 - sigma(left(__tv3)) = 0 }",
              r1->toString());
 
    EXPECT_EQ("{ [s, i1, s, f] : "
              "__tv0 - __tv2 = 0 && __tv1 - sigma(left(__tv3)) = 0 }",
              r2->toString());
             
    // passes because the e and f are in same tuple location
	ASSERT_TRUE((*r1) == (*r2));  


    r1->normalize();
//std::cout << "r1 = " << r1->toString() << std::endl;
	EXPECT_EQ("{ [s, i1, s2, e] : __tv0 - __tv2 = 0 && "
	          "__tv1 - sigma(left(__tv3)[0], left(__tv3)[1]) = 0 "
	          "&& __tv1 >= 0 && "
	          "__tv3 >= 0 && left(__tv3)[0] >= 0 && left(__tv3)[1] >= 0 && "
	          "-__tv1 + N - 1 >= 0 && -__tv3 + K - 1 >= 0 && "
	          "M - left(__tv3)[0] - 1 >= 0 && P - left(__tv3)[1] - 1 >= 0 }",
	          r1->toString());

    r2->normalize();
//std::cout << "r2 = " << r2->toString() << std::endl;
	EXPECT_EQ("{ [s, i1, s, f] : __tv0 - __tv2 = 0 && "
	          "__tv1 - sigma(left(__tv3)[0], left(__tv3)[1]) = 0 "
	          "&& __tv1 >= 0 && "
	          "__tv3 >= 0 && left(__tv3)[0] >= 0 && left(__tv3)[1] >= 0 && "
	          "-__tv1 + N - 1 >= 0 && -__tv3 + K - 1 >= 0 && "
	          "M - left(__tv3)[0] - 1 >= 0 && P - left(__tv3)[1] - 1 >= 0 }",
	          r2->toString());

	EXPECT_TRUE((*r1) == (*r2));
	
	delete r1;
	delete r2;

}

#pragma mark NestedUFCallsEqUFCalls
// nested UFCalls that are equal to UFCalls
TEST_F(NormalizationTest, NestedUFCallsEqUFCalls) {

    iegenlib::setCurrEnv();
    
    iegenlib::appendCurrEnv("left",
        new Set("{[i]:0<=i &&i<K}"),
        new Set("{[i]:0<=i &&i<M}"), true);
    iegenlib::appendCurrEnv("sigma",
        new Set("{[j]:0<=j &&j<P}"),
        new Set("{[i]:0<=i &&i<N}"), true);
    iegenlib::appendCurrEnv("sl",
        new Set("{[j]:0<=j &&j<Q}"),
        new Set("{[i]:0<=i &&i<R}"), true);

    Set *r1 = new Set("{[i,j,e,f] : "
                   "i = j && e = f && sl(i) = sigma(left(e)) }");

    Set *r2 = new Set("{[i,j,e,f] : "
                   "i = j && e = f && sl(j) = sigma(left(f)) }");

    EXPECT_EQ("{ [i, j, e, f] : __tv0 - __tv1 = 0 && __tv2 - __tv3 = 0 && "
              "sigma(left(__tv2)) - sl(__tv0) = 0 }",
              r1->toString());
 
    EXPECT_EQ("{ [i, j, e, f] : __tv0 - __tv1 = 0 && __tv2 - __tv3 = 0 && "
              "sigma(left(__tv3)) - sl(__tv1) = 0 }",
              r2->toString());
              
    // before normalization constraints are different
    EXPECT_FALSE((*r1) == (*r2));
             
    r1->normalize();
//std::cout << "r1 = " << r1->toString() << std::endl;

    r2->normalize();
//std::cout << "r2 = " << r2->toString() << std::endl;

    // after normalization all constraints are equivalent
	EXPECT_TRUE((*r1) == (*r2));
	
	delete r1;
	delete r2;

}

#pragma mark EqUFSignatureEqParams
// differently-named equal variables sent as parameters to same-named UFCall
// includes UFCall signature boundaries
TEST_F(NormalizationTest, EqUFSignatureEqParams) {

    iegenlib::setCurrEnv();
    
    iegenlib::appendCurrEnv("theta",
        new Set("{[i,j]:0<=i &&i<P && 0<=j &&j<=N}"),
        new Set("{[i]:0<=i &&i<M}"), true);

    Set *r1 = new Set("{[tstep,i,s0,t,i1,x] : "
                   "tstep = s0 && i = i1 && t = theta(0,i1) && x = 0 "
                   "&& 1 <= theta(0,i1) && theta(0,i1) <= M && 1 <= i1 && i1 <= N}");

    Set *r2 = new Set("{[tstep,i,s0,t,i1,x] : "
                   "tstep = s0 && i = i1 && t = theta(0,i) && x = 0 "
                   "&& 1 <= theta(0,i) && theta(0,i) <= M && 1 <= i && i <= N}");

    EXPECT_EQ("{ [tstep, i, s0, t, i1, x] : __tv5 = 0 && __tv0 - __tv2 = 0 && "
              "__tv1 - __tv4 = 0 && __tv3 - theta(0, __tv4) = 0 "
              "&& -__tv4 + N >= 0 && "
              "__tv4 - 1 >= 0 && M - theta(0, __tv4) >= 0 && theta(0, __tv4) - 1 >= 0 }",
              r1->toString());
 
    EXPECT_EQ("{ [tstep, i, s0, t, i1, x] : __tv5 = 0 && __tv0 - __tv2 = 0 && "
              "__tv1 - __tv4 = 0 && __tv3 - theta(0, __tv1) = 0 "
              "&& -__tv1 + N >= 0 && "
              "__tv1 - 1 >= 0 && M - theta(0, __tv1) >= 0 && theta(0, __tv1) - 1 >= 0 }",
              r2->toString());

    // before normalization constraints are different
    EXPECT_FALSE((*r1) == (*r2));
                          
    r1->normalize();
//std::cout << "r1 = " << r1->toString() << std::endl;

    r2->normalize();
//std::cout << "r2 = " << r2->toString() << std::endl;

	ASSERT_TRUE((*r1) == (*r2));  // works!
	
	delete r1;
	delete r2;

}

#pragma mark AffineExpWithUFCall
TEST_F(NormalizationTest, AffineExpWithUFCall) {

  {
    iegenlib::setCurrEnv();
    
    iegenlib::appendCurrEnv("f",
        new Set("{[i]:0<=i &&i<P}"),
        new Set("{[i]:0<=i &&i<M}"), true);

    Set *r1 = new Set("{[x,d] : d = f(x) && f(x) + x = f(x) + x}");

    EXPECT_EQ("{ [x, d] : __tv1 - f(__tv0) = 0 }",
              r1->toString()); 

	// for r1:   Since IEGenLib already is able to recognize that f(x)+x=f(x)+x
	//           is redundant, leaving only d=f(x) as a constraint, no need
	//           to normalize() it, but still sending it through to show
	//           that normalize() brings in function range and domain
	//           constraints
		
	r1->normalize();
//std::cout << "r1 = " << r1->toString() << std::endl;

    EXPECT_EQ("{ [x, d] : __tv1 - f(__tv0) = 0 && __tv0 >= 0 && __tv1 >= 0 && "
              "-__tv0 + P - 1 >= 0 && -__tv1 + M - 1 >= 0 }",
              r1->toString());
	
    delete r1;
  }
  
  {
    iegenlib::setCurrEnv();
    
    iegenlib::appendCurrEnv("f",
        new Set("{[i]:0<=i &&i<P}"),
        new Set("{[i]:0<=i &&i<M}"), true);

    Set *r2 = new Set("{[x] : f(x) - f(3) = 0 && x = 3}");

    EXPECT_EQ("{ [x] : __tv0 - 3 = 0 && f(3) - f(__tv0) = 0 }",
              r2->toString());
             
    r2->normalize();
//std::cout << "r2 = " << r2->toString() << std::endl;

    EXPECT_EQ("{ [3] : f(3) - f(__tv0) = 0 && f(3) >= 0 && "
                 "P - 4 >= 0 && M - f(3) - 1 >= 0 }",
              r2->toString());
    // Should we still have [x] rather than the [3] in the toString()
    // for r2 after normalize()?  No we want to recognize when we have
    // constants.  One thing we should do is when we have all constants in
    // the tuple, then all the other constraints should go away.
    // Made this IEGRTWO-107.
    	
	delete r2;
  }
}

#pragma mark EqAffineExpParamForUFCalls
// {[i,j,x] : j = i + 2 && f(j-1) = x} vs {[i,j,x] : i = j - 2 && f(i+1) = x}
TEST_F(NormalizationTest, EqAffineExpParamForUFCalls) {

    iegenlib::setCurrEnv();
    
    iegenlib::appendCurrEnv("f",
        new Set("{[i]:0<=i &&i<P}"),
        new Set("{[i]:0<=i &&i<M}"), true);

    Set *r1 = new Set("{[i,j,x] : j = i + 2 && f(j-1) = x }");

    Set *r2 = new Set("{[i,j,x] : i = j - 2 && f(i+1) = x }");

    EXPECT_EQ("{ [i, j, x] : __tv2 - f(__tv1 - 1) = 0 "
              "&& __tv0 - __tv1 + 2 = 0 }",
              r1->toString());
 
    EXPECT_EQ("{ [i, j, x] : __tv2 - f(__tv0 + 1) = 0 "
              "&& __tv0 - __tv1 + 2 = 0 }",
              r2->toString());
    
// problems with r1->normalize()
    
/*
    Conjunction::ufCallsToTempVars: e     = __tv2 - f(__tv1 - 1)
	Exp::ufCallsToTempVars callTerm = -f(__tv1 - 1)
	UFCallTerm::ufCallsToTempVars: init  mod_ufcall = f(__tv1 - 1)
	tempvars for  UFCallTerm::ufCallsToTempVars= ( __tv3 )
	retval for  UFCallTerm::ufCallsToTempVars= -__tv3
	Exp::ufCallsToTempVars mod_call = -__tv3
	Conjunction::ufCallsToTempVars: mod_e = __tv2 - f(__tv1 - 1)
	Conjunction::ufCallsToTempVars: e     = __tv0 - __tv1 + 2
	Conjunction::ufCallsToTempVars: mod_e = __tv0 - __tv1 + 2
	Conjunction::normalize: fromStep1 = [ M, P ] -> { [i, j, x, tv3] : x - tv3 = 0 && i - j + 2 = 0 && tv3 >= 0 && -j + P >= 0 && j - 1 >= 0 && -tv3 + M - 1 >= 0 }
	Conjunction::normalize: fromISl = [M, P] -> { [i, 2 + i, x, x] : x >= 0 and i <= -2 + P and i >= -1 and x <= -1 + M }
	unknown file: Failure
	C++ exception with description 
		"syntax error, unexpected PLUS, expecting RBRACKET or COMMA at '+', line 1
		" thrown in the test body.
*/
             
//    r1->normalize();
//std::cout << "r1 = " << r1->toString() << std::endl;

//    r2->normalize();
//std::cout << "r2 = " << r2->toString() << std::endl;

    
//	  ASSERT_TRUE((*r1) == (*r2));  // works!

	
	delete r1;
	delete r2;

}


#pragma mark boundTupleExpRange
// IEGRTWO-85 when bounding by the range
// [a,b] = f(c,d), creating bounds for expressions a and b
TEST_F(NormalizationTest, boundTupleExpRange) {
    {
        Set* range = new Set("{[4,t] : lb_t <= t && t <= ub_t } "
                             "union {[3,s] : lb_s <= s && s <= ub_s }");
                             
        // a = 7 + x
        Exp* aExp = new Exp();
        aExp->addTerm( new VarTerm("x") );
        aExp->addTerm( new Term(7) );
        // b = y
        Exp* bExp = new Exp();
        bExp->addTerm( new VarTerm("y") );
        
        // tuple expression [a,b]
        TupleExpTerm outexps(2);
        outexps.setExpElem(0,aExp);
        outexps.setExpElem(1,bExp);

        // call the method being tested
        Set* boundSet = range->boundTupleExp( outexps );
        
        Set* expected = new Set("{[] : 7+x = 4 && lb_t <= y && y <= ub_t} "
                                "union "
                                "{[] : 7+x = 3 && lb_s <= y && y <= ub_s}");

        EXPECT_EQ(expected->toString(), boundSet->toString());
        
        delete range;
        delete boundSet;
        delete expected;
    }
    {
        Set* range = new Set("{[p,v] : lb_p <= p && p <= ub_p && lb_v <= v "
                             "&& v <= ub_v } "
                             "union {[q,w] : lb_q <= q && q <= ub_q "
                             "&& lb_w <= w && w <= ub_w }");
                             
        // a = 7 + x
        Exp* aExp = new Exp();
        aExp->addTerm( new VarTerm("x") );
        aExp->addTerm( new Term(7) );
        // b = y
        Exp* bExp = new Exp();
        bExp->addTerm( new VarTerm("y") );
        
        // tuple expression [a,b]
        TupleExpTerm outexps(2);
        outexps.setExpElem(0,aExp);
        outexps.setExpElem(1,bExp);

        // call the method being tested
        Set* boundSet = range->boundTupleExp( outexps );
        
        Set* expected = new Set("{[] : lb_p<=7+x && 7+x<=ub_p "
                                "&& lb_v <= y && y <= ub_v} "
                                "union "
                                "{[] : 7+x <= ub_q && lb_q<=7+x "
                                "&& lb_w <= y && y <= ub_w}");

        EXPECT_EQ(expected->toString(), boundSet->toString());
        
        delete range;
        delete boundSet;
        delete expected;
    }
}

#pragma mark boundTupleExpDomain
// IEGRTWO-85 when bounding by the domain
// [a,b] = f(c,d), creating bounds for expressions c and d
TEST_F(NormalizationTest, boundTupleExpDomain) {
    {
        Set* range = new Set("{[0,t] : lb_t <= t && t <= ub_t } "
                             "union {[1,s] : lb_s <= s && s <= ub_s }");
                             
        // c = 7 + x
        Exp* cExp = new Exp();
        cExp->addTerm( new VarTerm("x") );
        cExp->addTerm( new Term(7) );
        // d = y
        Exp* dExp = new Exp();
        dExp->addTerm( new VarTerm("y") );
        
        // tuple expression [c,d]
        TupleExpTerm param_exps(2);
        param_exps.setExpElem(0,cExp);
        param_exps.setExpElem(1,dExp);
        
        // call the method being tested
        Set* boundSet = range->boundTupleExp( param_exps );
        
        Set* expected = new Set("{[] : 7+x = 0 "
                                "&& lb_t <= y && y <= ub_t} "
                                "union "
                                "{[] : 7+x = 1 "
                                "&& lb_s <= y && y <= ub_s}");

        EXPECT_EQ(expected->toString(), boundSet->toString());
                
        delete range;
        delete boundSet;
        delete expected;
    }
    {
        Set* range = new Set("{[p,v] : lb_p <= p && p <= ub_p && lb_v <= v "
                             "&& v <= ub_v } "
                             "union {[q,w] : lb_q <= q && q <= ub_q "
                             "&& lb_w <= w && w <= ub_w }");
                             
        // c = 7 + x
        Exp* cExp = new Exp();
        cExp->addTerm( new VarTerm("x") );
        cExp->addTerm( new Term(7) );
        // d = y
        Exp* dExp = new Exp();
        dExp->addTerm( new VarTerm("y") );
        
        // tuple expression [c,d]
        TupleExpTerm param_exps(2);
        param_exps.setExpElem(0,cExp);
        param_exps.setExpElem(1,dExp);

        // call the method being tested
        Set* boundSet = range->boundTupleExp( param_exps );
        
        Set* expected = new Set("{[] : lb_p<=7+x && 7+x<=ub_p "
                                "&& lb_v <= y && y <= ub_v} "
                                "union "
                                "{[] : 7+x <= ub_q && lb_q<=7+x "
                                "&& lb_w <= y && y <= ub_w}");

        EXPECT_EQ(expected->toString(), boundSet->toString());
        
        delete range;
        delete boundSet;
        delete expected;
    }
}

#pragma mark buildingNormalize
// IEGRTWO-87 using this test case to build up normalize routine.
TEST_F(NormalizationTest, buildingNormalize) {
    iegenlib::setCurrEnv();
    iegenlib::appendCurrEnv("tau",
        new Set("{[i]:0<=i &&i<N}"), 
        new Set("{[i,j]:0<=i &&i<M && 0<=j &&j<P}"), true);

/* MMS, keep for historical reasons
    // When normalize only does step 0 it just groups together
    // indexed UFCalls.
    {
    Set* s = new Set("{[i,j,k] : i=tau(k)[0] && j=tau(k)[1]}");
    EXPECT_EQ("{ [i, j, k] : __tv0 - tau(__tv2)[0] = 0 "
                "&& __tv1 - tau(__tv2)[1] = 0 }", s->toString() );
    s->normalize();
    Set* expected = new Set("{[i,j,k] : (i,j)=tau(k)}");
    EXPECT_EQ(expected->toString(),s->toString());
    delete s;
    delete expected;
    }

    {
    // When normalize does step 0 and step 1 it creates
    // a set with affine constraints that can be sent to ISL.
    Set* s = new Set("{[i,j,k] : i=tau(k)[0] && j=tau(k)[1]}");
    EXPECT_EQ("{ [i, j, k] : __tv0 - tau(__tv2)[0] = 0 "
                "&& __tv1 - tau(__tv2)[1] = 0 }", s->toString() );
    s->normalize();
    Set* expected = new Set("{[i,j,k,tv3,tv4] : 0<=tv3 && tv3<M && "
                            "0<=tv4 && tv4<P "
                            " && 0<=k && k<N && tv3=i && tv4=j}");
    EXPECT_EQ(expected->toString(),s->toString());
    delete s;
    delete expected;
    }
   
     {
    // When normalize does step 0 and step 1 and step 2 it creates
    // a set with affine constraints and sends it to ISL.
    Set* s = new Set("{[i,j,k] : i=tau(k)[0] && j=tau(k)[1]}");
    EXPECT_EQ("{ [i, j, k] : __tv0 - tau(__tv2)[0] = 0 "
                "&& __tv1 - tau(__tv2)[1] = 0 }", s->toString() );
    s->normalize();
    Set* expected = new Set("{ [i, j, k, i, j] : k >= 0 && i >= 0 && "
                            "j >= 0 && k <= -1 + N && i <= -1 + M && "
                            "j <= -1 + P }");
    EXPECT_EQ(expected->toString(),s->toString());

    delete s;
    delete expected;
    }
*/
     {
    // When normalize does steps 0 through 3 it creates
    // a set with affine constraints and sends it through ISL and
    // re-establishes the original UFCalls
    Set* s = new Set("{[i,j,k] : i=tau(k)[0] && j=tau(k)[1]}");
    EXPECT_EQ("{ [i, j, k] : __tv0 - tau(__tv2)[0] = 0 "
                "&& __tv1 - tau(__tv2)[1] = 0 }", s->toString() );
    s->normalize();
    Set* expected = new Set("{ [i, j, k] : i - tau(k)[0] = 0 && "
                            "j - tau(k)[1] = 0 && i >= 0 && j >= 0 && "
                            "k >= 0 && -i + M - 1 >= 0 && -j + P - 1 >= 0 && "
                            "-k + N - 1 >= 0 }");

    EXPECT_EQ(expected->toString(),s->toString());

    delete s;
    delete expected;
    }
   
}

#pragma mark ufCallsToTempVars
// MMS 11/6/15, What does this method do?
TEST_F(NormalizationTest, ufCallsToTempVars) {
    Set *r1 = new Set("{[i,j,x] : j = i + 2 && f(j-1) = x }");
    EXPECT_EQ("{ [i, j, x] : x - f(j - 1) = 0 && i - j + 2 = 0 }",
              r1->prettyPrintString());
    EXPECT_EQ("{ [i, j, x] : __tv2 - f(__tv1 - 1) = 0 && __tv0 - "
              "__tv1 + 2 = 0 }",
              r1->toString());

    // Initialize an environment for the function f.    
    iegenlib::appendCurrEnv("f",
        new Set("{[i]:0<=i &&i<G}"), new Set("{[i]:0<=i &&i<G}"), true);
    
    // Iterate over all the conjunctions in our relation to determine
    // what the ufCallsToTempVars is doing.
    iegenlib::UFCallMapAndBounds ufcallmap(r1->getTupleDecl());
    for (std::list<Conjunction*>::iterator i=r1->mConjunctions.begin();
            i != r1->mConjunctions.end(); i++) {
         (*i)->ufCallsToTempVars(ufcallmap);
    }
    EXPECT_EQ("{ [i, j, x] : __tv2 - __tv3 = 0 && __tv0 - "
              "__tv1 + 2 = 0 }",
              r1->toString());
    
    // FIXME: the below segfaults because those routines add in new tuple
    // variables and then pretty print can't find their actual names.
    //EXPECT_EQ("",r1->prettyPrintString());
    
    delete r1;
}
