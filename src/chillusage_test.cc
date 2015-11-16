/*!
 * \file chillusage_test.cc
 *
 * \brief Tests the part of IEGenLib interface that will be needed in CHILL.
 *
 * \date Started: 8/16/13
 *
 * \authors Michelle Strout
 *
 * Copyright (c) 2013, Colorado State University <br>
 * Copyright (c) 2015, University of Arizona <br>
 * All rights reserved. <br>
 * See ../../COPYING for details. <br>
 */

#include <gtest/gtest.h>

#include <iegenlib.h>
using namespace iegenlib;


/*!
 * \class ChillUsageTest
 *
 * \brief Class to test CHILL usage of IEGenLib.
 *
 * This class holds gtest test cases that test the functionality in
 * iegenlib that will be needed by CHILL.
 */
class ChillUsageTest : public::testing::Test {
    public:
    protected:
    virtual void SetUp() {}
    virtual void TearDown() {}
};


/*! Seeing if we can do dependence simplification for GS.
*/
TEST_F(ChillUsageTest, GSDepSimplification)
{
    // From CHILL, Anand creates the full dependence relation
    // for the loop.
    Relation* full_dep = new Relation("{ [i,j] -> [i',j'] : "
                            "i<i' && i=col(j') and 0<=i and i<N and "
                            "0<=i' and i'<N "
                            "and idx(i) <= j and j<idx(i+1) "
                            "and idx(i’) <= j' and j' < idx(i’+1) }");

    EXPECT_EQ("{ [i, j] -> [i', j'] : i - col(j') = 0 && i >= 0 && i' >= 0 "
              "&& j - idx(i) >= 0 && j' - idx(i) >= 0 && -i + i' - 1 >= 0 && "
              "-i + N - 1 >= 0 && -j + idx(i + 1) - 1 >= 0 && -i' + N - 1 >= "
              "0 && -j' + idx(i + 1) - 1 >= 0 }", 
              full_dep->prettyPrintString() );
   
   /* MMS, 10/21/15, this doesn't work yet
    // project out j
    Relation* rel_2to1 = new Relation("{[x,y]->[x]}");
    //Relation* rel_1to2 = new Relation("{[x]->[x,y]}");
    EXPECT_EQ( "",
               rel_2to1->Compose(full_dep)->prettyPrintString());
*/

    delete full_dep;
}

/*! Tests modifying the access relations for loop coalescing.
*/
TEST_F(ChillUsageTest, LoopCoalescing)
{
    // from CHILL, Anand defines the access relations.
    // example: y[i] += a[j]*x[col[j]];
    // v0 stands for version 0
    Relation* r0_v0 = new Relation("{[i,j]->[i]}");
    Relation* r1_v0 = new Relation("{[i,j]->[j]}");
    Relation* r2_v0 = new Relation("{[i,j]->[r2] : r2=col(j)}");

    // from CHILL, Anand declares the uninterpreted function
    // that performs coalescing and defines the coalescing
    // transformation.
    iegenlib::setCurrEnv(); // Clears out the environment
    iegenlib::appendCurrEnv("c", // UF name
        // UF domain
        new Set("{[i,j]:0<=i && i <nrows && ja(i)<=j && j<ja(i+1) }"),
        // UF range
        new Set("{[k]:0<=k && k<nnz}"),
        // c function is bijective
        true );               
    Relation* T_coalesce = new Relation("{[i,j]->[k]:k = c(i,j)}");  
    // c for coalesce?  we talked about R but that was for inverse of above

    // then applies the transformation to each of the access relation
    Relation* T_coalesce_inv = T_coalesce->Inverse();
    EXPECT_EQ("{ [k] -> [i, j] : k - c(i, j) = 0 }",
        T_coalesce_inv->prettyPrintString() );

    Relation* r0_v1 = r0_v0->Compose(T_coalesce_inv);
    EXPECT_EQ("{ [k] -> [i] : i - c_inv(k)[0] = 0 }", 
        r0_v1->prettyPrintString() );

    Relation* r1_v1 = r1_v0->Compose(T_coalesce_inv);
    EXPECT_EQ("{ [k] -> [j] : j - c_inv(k)[1] = 0 }",
        r1_v1->prettyPrintString() );

    Relation* r2_v1 = r2_v0->Compose(T_coalesce_inv);
    EXPECT_EQ("{ [k] -> [r2] : r2 - col(c_inv(k)[1]) = 0 }",
        r2_v1->prettyPrintString() );


        
    ////// To get the updated access relation do the following:
    // Find tuple variable 1 as a function of tuple variables 0 through 0
    Exp* r0_v1_exp = r0_v1->findFunction(1,0,0);
    EXPECT_EQ("c_inv(k)[0]", 
        r0_v1_exp->prettyPrintString(r0_v1->getTupleDecl()) );
    
    Exp* r1_v1_exp = r1_v1->findFunction(1,0,0);
    EXPECT_EQ("c_inv(k)[1]", 
        r1_v1_exp->prettyPrintString(r0_v1->getTupleDecl()) );
    
    Exp* r2_v1_exp = r2_v1->findFunction(1,0,0);
    EXPECT_EQ("col(c_inv(k)[1])", 
        r2_v1_exp->prettyPrintString(r0_v1->getTupleDecl()) );
    
    delete r0_v0;
    delete r0_v1;
    delete r0_v1_exp;
    delete r1_v0;
    delete r1_v1;
    delete r1_v1_exp;
    delete r2_v0;
    delete r2_v1;
    delete r2_v1_exp;
    delete T_coalesce;
    delete T_coalesce_inv;
}

/*! Tests modifying the access relations for loop coalescing
    and indicating that k=j.
*/
TEST_F(ChillUsageTest, LoopCoalescingKequalJ)
{
    // from CHILL, Anand defines the access relations.
    // example: y[i] += a[j]*x[col[j]];
    // v0 stands for version 0
    Relation* r0_v0 = new Relation("{[i,j]->[i]}");
    Relation* r1_v0 = new Relation("{[i,j]->[j]}");
    Relation* r2_v0 = new Relation("{[i,j]->[r2] : r2=col(j)}");

    // from CHILL, Anand declares the uninterpreted function
    // that performs coalescing and defines the coalescing
    // transformation.
    iegenlib::setCurrEnv(); // Clears out the environment
    iegenlib::appendCurrEnv("c", // UF name
        // UF domain
        new Set("{[i,j]:0<=i && i <nrows && ja(i)<=j && j<ja(i+1) }"),
        // UF range
        new Set("{[k]:0<=k && k<nnz}"),
        // c function is bijective
        true );               
    Relation* T_coalesce = new Relation("{[i,j]->[k]:k = c(i,j) && k=j}");  

    // then applies the transformation to each of the access relation
    Relation* T_coalesce_inv = T_coalesce->Inverse();
    EXPECT_EQ("{ [k] -> [i, j] : k - j = 0 && k - c(i, j) = 0 }",
        T_coalesce_inv->prettyPrintString() );

    Relation* r0_v1 = r0_v0->Compose(T_coalesce_inv);
    EXPECT_EQ(
        "{ [k] -> [i] : k - c(c_inv(k)[0], k) = 0 && i - c_inv(k)[0] = 0 }", 
        r0_v1->prettyPrintString() );

    Relation* r1_v1 = r1_v0->Compose(T_coalesce_inv);
    EXPECT_EQ("{ [k] -> [j] : k - j = 0 && k - c(c_inv(k)[0], k) = 0 }",
        r1_v1->prettyPrintString() );

    Relation* r2_v1 = r2_v0->Compose(T_coalesce_inv);
    EXPECT_EQ("{ [k] -> [r2] : k - c(c_inv(k)[0], k) = 0 && r2 - col(k) = 0 }",
        r2_v1->prettyPrintString() );


        
    ////// To get the updated access relation do the following:
    // Find tuple variable 1 as a function of tuple variables 0 through 0
    Exp* r0_v1_exp = r0_v1->findFunction(1,0,0);
    EXPECT_EQ("c_inv(k)[0]", 
        r0_v1_exp->prettyPrintString(r0_v1->getTupleDecl()) );
    
    Exp* r1_v1_exp = r1_v1->findFunction(1,0,0);
    EXPECT_EQ("k", 
        r1_v1_exp->prettyPrintString(r0_v1->getTupleDecl()) );
    
    Exp* r2_v1_exp = r2_v1->findFunction(1,0,0);
    EXPECT_EQ("col(k)", 
        r2_v1_exp->prettyPrintString(r0_v1->getTupleDecl()) );
    
    delete r0_v0;
    delete r0_v1;
    delete r0_v1_exp;
    delete r1_v0;
    delete r1_v1;
    delete r1_v1_exp;
    delete r2_v0;
    delete r2_v1;
    delete r2_v1_exp;
    delete T_coalesce;
    delete T_coalesce_inv;
}

/*! Tests adding constraints between UFCalls such as
    forall e, index(e) <= diagptr(e).
*/
TEST_F(ChillUsageTest, AddUFConstraints)
{
    Relation* r1 = new Relation("{[i,j]->[k] : k=col(j) && 0<=i && i<N"
                                "&& index(i)<=j && j<index(i+1)}");
    Relation* r1add = r1->addUFConstraints("index", "<=", "diagptr");
    Relation* expect1 = new Relation("{[i,j]->[k] : k=col(j) && 0<=i && i<N"
                                     "&& index(i)<=j && j<index(i+1)"
                                     "&& index(i)<=diagptr(i)"
                                     "&& index(i+1)<=diagptr(i+1)}");

    //EXPECT_EQ(expect1->prettyPrintString(), r1add->prettyPrintString());
    
    delete r1;
    delete r1add;
    delete expect1;
}
