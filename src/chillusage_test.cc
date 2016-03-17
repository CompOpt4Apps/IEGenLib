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
        true,
        // no monotonicity
        Monotonic_NONE );               
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
        true,
        // no monotonicity
        Monotonic_NONE );               
               
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

/*! Seeing if we can do dependence simplification for GS.
*/
TEST_F(ChillUsageTest, GS_CSR_DepSimplification)
{
    iegenlib::setCurrEnv();
    iegenlib::appendCurrEnv("colidx",
        new Set("{[i]:0<=i &&i<nnz}"), 
        new Set("{[j]:0<=j &&j<m}"), true, iegenlib::Monotonic_Increasing);
    iegenlib::appendCurrEnv("rowptr",
        new Set("{[i]:0<=i &&i<m}"), 
        new Set("{[j]:0<=j &&j<nnz}"), true, iegenlib::Monotonic_Increasing);

    // From CHILL, Anand creates the full dependence relation
    // for the loop.
    Set* flow = new Set("{ [i,ip,j,jp] : i<ip && i=colidx(jp) "
                     "&& 0 <= i && i < m && 0 <= ip && ip < m "
                        "&& rowptr(i) <= j && j < rowptr(i+1) "
                   "&& rowptr(ip) <= jp && jp < rowptr(ip+1) }");

    Set* anti = new Set("{ [i,ip,j,jp] : ip<i && i=colidx(jp) "
                     "&& 0 <= i && i < m && 0 <= ip && ip < m "
                        "&& rowptr(i) <= j && j < rowptr(i+1) "
                   "&& rowptr(ip) <= jp && jp < rowptr(ip+1) }");


    // Expected outputs 
    Set* ex_flow = new Set("{ [i,ip,jp] : i=colidx(jp) "
                              "&& colidx(jp) < ip && ip < m-1 "
                                  "&& rowptr(i) < rowptr(i+1) "
                    "&& rowptr(ip) <= jp && jp < rowptr(ip+1) "
     "&& colidx(jp) >= 0 && rowptr(i) >= 0 && rowptr(ip) >= 0 "
                  "&& rowptr(i+1) < nnz && rowptr(ip+1) < nnz}");

    Set* ex_anti = new Set("{ [i,ip,jp] : i=colidx(jp) "
                               "&& 0 <= ip && ip < colidx(jp) "
                                  "&& rowptr(i) < rowptr(i+1) "
                    "&& rowptr(ip) <= jp && jp < rowptr(ip+1) "
     "&& colidx(jp) < m-1 && rowptr(i) >= 0 && rowptr(ip) >= 0 "
                  "&& rowptr(i+1) < nnz && rowptr(ip+1) < nnz}");


    // Simplifying Flow dependence by projecting out j
    Set *ts;
    ts = flow->projectOut(2);    // 2 == index of 'j'
    if ( ts ){                   // Did we project out 'j': YES!
        delete flow;               // removing old cs
        flow = ts;
    }

// Print results
//   std::cout<<std::endl<< "Simp. Dep. = "<<flow->toISLString()<<std::endl;
    EXPECT_EQ( ex_flow->toISLString() , flow->toISLString() );



    // Simplifying Anti dependence by projecting out j
    ts = anti->projectOut(2);    // 2 == index of 'j'
    if ( ts ){                   // Did we project out 'j': YES!
        delete anti;             // removing old cs
        anti = ts;
    }

// Print results
//   std::cout<<std::endl<<"Simp. Dep. = "<<anti->toISLString()<<std::endl;
    EXPECT_EQ( ex_anti->toISLString() , anti->toISLString() );


    delete ex_flow;
    delete ex_anti;
    delete flow;
    delete anti;
}


/*! Test cases CSR ILU code's data access dependencies
*/
TEST_F(ChillUsageTest, ILU_CSR_DepSimplification)
{
    iegenlib::setCurrEnv();
    iegenlib::appendCurrEnv("colidx",
        new Set("{[i]:0<=i &&i<nnz}"), 
        new Set("{[j]:0<=j &&j<m}"), true, iegenlib::Monotonic_Increasing);
    iegenlib::appendCurrEnv("rowptr",
        new Set("{[i]:0<=i &&i<m}"), 
        new Set("{[j]:0<=j &&j<nnz}"), true, iegenlib::Monotonic_Increasing);
    iegenlib::appendCurrEnv("diagptr",
        new Set("{[i]:0<=i &&i<m}"), 
        new Set("{[j]:0<=j &&j<nnz}"), true, iegenlib::Monotonic_Increasing);


    Set *F1 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: ip < i &&"
                " k < j1 && j1 < rowptr(1+i) && j2 < rowptr(1+colidx(k)) &&"
          " diagptr(colidx(k)) < j2 && 0 <= i && i < m && rowptr(i) <= k &&"
  " k < diagptr(i) && rowptr(i) <= diagptr(i) && rowptr(i) < rowptr(1+i) &&"
               " diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp)) &&"
           " j1p < rowptr(1+ip) && kp < diagptr(ip) && 0 <= ip && ip < m &&"
                         " rowptr(ip) <= kp && rowptr(ip) <= diagptr(ip) &&"
                                     " rowptr(ip) < rowptr(1+ip) && k = kp}");

    Set *A1 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: i < ip &&"
                " k < j1 && j1 < rowptr(1+i) && j2 < rowptr(1+colidx(k)) &&"
          " diagptr(colidx(k)) < j2 && 0 <= i && i < m && rowptr(i) <= k &&"
  " k < diagptr(i) && rowptr(i) <= diagptr(i) && rowptr(i) < rowptr(1+i) &&"
               " diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp)) &&"
           " j1p < rowptr(1+ip) && kp < diagptr(ip) && 0 <= ip && ip < m &&"
                         " rowptr(ip) <= kp && rowptr(ip) <= diagptr(ip) &&"
                                     " rowptr(ip) < rowptr(1+ip) && k = kp}");


    Set *F2 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: ip < i &&"
                " k < j1 && j1 < rowptr(1+i) && j2 < rowptr(1+colidx(k)) &&"
          " diagptr(colidx(k)) < j2 && 0 <= i && i < m && rowptr(i) <= k &&"
  " k < diagptr(i) && rowptr(i) <= diagptr(i) && rowptr(i) < rowptr(1+i) &&"
               " diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp)) &&"
           " j1p < rowptr(1+ip) && kp < diagptr(ip) && 0 <= ip && ip < m &&"
                         " rowptr(ip) <= kp && rowptr(ip) <= diagptr(ip) &&"
                    " rowptr(ip) < rowptr(1+ip) && k = diagptr(colidx(kp))}");

    Set *A2 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: i < ip &&"
                " k < j1 && j1 < rowptr(1+i) && j2 < rowptr(1+colidx(k)) &&"
          " diagptr(colidx(k)) < j2 && 0 <= i && i < m && rowptr(i) <= k &&"
  " k < diagptr(i) && rowptr(i) <= diagptr(i) && rowptr(i) < rowptr(1+i) &&"
               " diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp)) &&"
           " j1p < rowptr(1+ip) && kp < diagptr(ip) && 0 <= ip && ip < m &&"
                         " rowptr(ip) <= kp && rowptr(ip) <= diagptr(ip) &&"
                    " rowptr(ip) < rowptr(1+ip) && k = diagptr(colidx(kp))}");



    Set *F3 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: ip < i &&"
                " k < j1 && j1 < rowptr(1+i) && j2 < rowptr(1+colidx(k)) &&"
          " diagptr(colidx(k)) < j2 && 0 <= i && i < m && rowptr(i) <= k &&"
  " k < diagptr(i) && rowptr(i) <= diagptr(i) && rowptr(i) < rowptr(1+i) &&"
               " diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp)) &&"
           " j1p < rowptr(1+ip) && kp < diagptr(ip) && 0 <= ip && ip < m &&"
                         " rowptr(ip) <= kp && rowptr(ip) <= diagptr(ip) &&"
                                    " rowptr(ip) < rowptr(1+ip) && k = j1p}");

    Set *A3 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: i < ip &&"
                " k < j1 && j1 < rowptr(1+i) && j2 < rowptr(1+colidx(k)) &&"
          " diagptr(colidx(k)) < j2 && 0 <= i && i < m && rowptr(i) <= k &&"
  " k < diagptr(i) && rowptr(i) <= diagptr(i) && rowptr(i) < rowptr(1+i) &&"
               " diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp)) &&"
           " j1p < rowptr(1+ip) && kp < diagptr(ip) && 0 <= ip && ip < m &&"
                         " rowptr(ip) <= kp && rowptr(ip) <= diagptr(ip) &&"
                                    " rowptr(ip) < rowptr(1+ip) && k = j1p}");


    Set *F4 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: ip < i &&"
                " k < j1 && j1 < rowptr(1+i) && j2 < rowptr(1+colidx(k)) &&"
          " diagptr(colidx(k)) < j2 && 0 <= i && i < m && rowptr(i) <= k &&"
  " k < diagptr(i) && rowptr(i) <= diagptr(i) && rowptr(i) < rowptr(1+i) &&"
               " diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp)) &&"
           " j1p < rowptr(1+ip) && kp < diagptr(ip) && 0 <= ip && ip < m &&"
                         " rowptr(ip) <= kp && rowptr(ip) <= diagptr(ip) &&"
                                    " rowptr(ip) < rowptr(1+ip) && k = j2p}");

    Set *A4 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: i < ip &&"
                " k < j1 && j1 < rowptr(1+i) && j2 < rowptr(1+colidx(k)) &&"
          " diagptr(colidx(k)) < j2 && 0 <= i && i < m && rowptr(i) <= k &&"
  " k < diagptr(i) && rowptr(i) <= diagptr(i) && rowptr(i) < rowptr(1+i) &&"
               " diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp)) &&"
           " j1p < rowptr(1+ip) && kp < diagptr(ip) && 0 <= ip && ip < m &&"
                         " rowptr(ip) <= kp && rowptr(ip) <= diagptr(ip) &&"
                                    " rowptr(ip) < rowptr(1+ip) && k = j2p}");


    Set *F5 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: ip < i &&"
                " k < j1 && j1 < rowptr(1+i) && j2 < rowptr(1+colidx(k)) &&"
          " diagptr(colidx(k)) < j2 && 0 <= i && i < m && rowptr(i) <= k &&"
  " k < diagptr(i) && rowptr(i) <= diagptr(i) && rowptr(i) < rowptr(1+i) &&"
               " diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp)) &&"
           " j1p < rowptr(1+ip) && kp < diagptr(ip) && 0 <= ip && ip < m &&"
                         " rowptr(ip) <= kp && rowptr(ip) <= diagptr(ip) &&"
                   " rowptr(ip) < rowptr(1+ip) && diagptr(colidx(k)) = j1p}");

    Set *A5 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: i < ip &&"
                " k < j1 && j1 < rowptr(1+i) && j2 < rowptr(1+colidx(k)) &&"
          " diagptr(colidx(k)) < j2 && 0 <= i && i < m && rowptr(i) <= k &&"
  " k < diagptr(i) && rowptr(i) <= diagptr(i) && rowptr(i) < rowptr(1+i) &&"
               " diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp)) &&"
           " j1p < rowptr(1+ip) && kp < diagptr(ip) && 0 <= ip && ip < m &&"
                         " rowptr(ip) <= kp && rowptr(ip) <= diagptr(ip) &&"
                   " rowptr(ip) < rowptr(1+ip) && diagptr(colidx(k)) = j1p}");



    Set *F6 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: ip < i &&"
                " k < j1 && j1 < rowptr(1+i) && j2 < rowptr(1+colidx(k)) &&"
          " diagptr(colidx(k)) < j2 && 0 <= i && i < m && rowptr(i) <= k &&"
  " k < diagptr(i) && rowptr(i) <= diagptr(i) && rowptr(i) < rowptr(1+i) &&"
               " diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp)) &&"
           " j1p < rowptr(1+ip) && kp < diagptr(ip) && 0 <= ip && ip < m &&"
                         " rowptr(ip) <= kp && rowptr(ip) <= diagptr(ip) &&"
                                   " rowptr(ip) < rowptr(1+ip) && j1 = j1p}");

    Set *A6 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: i < ip &&"
                " k < j1 && j1 < rowptr(1+i) && j2 < rowptr(1+colidx(k)) &&"
          " diagptr(colidx(k)) < j2 && 0 <= i && i < m && rowptr(i) <= k &&"
  " k < diagptr(i) && rowptr(i) <= diagptr(i) && rowptr(i) < rowptr(1+i) &&"
               " diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp)) &&"
           " j1p < rowptr(1+ip) && kp < diagptr(ip) && 0 <= ip && ip < m &&"
                         " rowptr(ip) <= kp && rowptr(ip) <= diagptr(ip) &&"
                                   " rowptr(ip) < rowptr(1+ip) && j1 = j1p}");



    Set *F7 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: ip < i &&"
                " k < j1 && j1 < rowptr(1+i) && j2 < rowptr(1+colidx(k)) &&"
          " diagptr(colidx(k)) < j2 && 0 <= i && i < m && rowptr(i) <= k &&"
  " k < diagptr(i) && rowptr(i) <= diagptr(i) && rowptr(i) < rowptr(1+i) &&"
               " diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp)) &&"
           " j1p < rowptr(1+ip) && kp < diagptr(ip) && 0 <= ip && ip < m &&"
                         " rowptr(ip) <= kp && rowptr(ip) <= diagptr(ip) &&"
                                    " rowptr(ip) < rowptr(1+ip) && j1 = kp}");

    Set *A7 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: i < ip &&"
                " k < j1 && j1 < rowptr(1+i) && j2 < rowptr(1+colidx(k)) &&"
          " diagptr(colidx(k)) < j2 && 0 <= i && i < m && rowptr(i) <= k &&"
  " k < diagptr(i) && rowptr(i) <= diagptr(i) && rowptr(i) < rowptr(1+i) &&"
               " diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp)) &&"
           " j1p < rowptr(1+ip) && kp < diagptr(ip) && 0 <= ip && ip < m &&"
                         " rowptr(ip) <= kp && rowptr(ip) <= diagptr(ip) &&"
                                    " rowptr(ip) < rowptr(1+ip) && j1 = kp}");



   Set *F8 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: ip < i &&"
                " k < j1 && j1 < rowptr(1+i) && j2 < rowptr(1+colidx(k)) &&"
          " diagptr(colidx(k)) < j2 && 0 <= i && i < m && rowptr(i) <= k &&"
  " k < diagptr(i) && rowptr(i) <= diagptr(i) && rowptr(i) < rowptr(1+i) &&"
               " diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp)) &&"
           " j1p < rowptr(1+ip) && kp < diagptr(ip) && 0 <= ip && ip < m &&"
                         " rowptr(ip) <= kp && rowptr(ip) <= diagptr(ip) &&"
                                   " rowptr(ip) < rowptr(1+ip) && j1 = j2p}");

    Set *A8 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: i < ip &&"
                " k < j1 && j1 < rowptr(1+i) && j2 < rowptr(1+colidx(k)) &&"
          " diagptr(colidx(k)) < j2 && 0 <= i && i < m && rowptr(i) <= k &&"
  " k < diagptr(i) && rowptr(i) <= diagptr(i) && rowptr(i) < rowptr(1+i) &&"
               " diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp)) &&"
           " j1p < rowptr(1+ip) && kp < diagptr(ip) && 0 <= ip && ip < m &&"
                         " rowptr(ip) <= kp && rowptr(ip) <= diagptr(ip) &&"
                                   " rowptr(ip) < rowptr(1+ip) && j1 = j2p}");


    Set *cs = new Set(*A1);
    Set *ts;
    ts = cs->projectOut(7);    // 7 == index of 'j2p'
    if ( ts ){                 // Did we project out 'j2p': YES!
        delete cs;             // removing old cs
        cs = ts;
    }

    ts = cs->projectOut(6);    // 7 == index of 'j2'
    if ( ts ){                 // Did we project out 'j2': YES!
        delete cs;             // removing old cs
        cs = ts;
    }

    ts = cs->projectOut(5);    // 7 == index of 'j1p'
    if ( ts ){                 // Did we project out 'j1p': YES!
        delete cs;             // removing old cs
        cs = ts;
    }

    ts = cs->projectOut(4);    // 7 == index of 'j1'
    if ( ts ){                 // Did we project out 'j1': YES!
        delete cs;             // removing old cs
        cs = ts;
    }

// Print results
//   std::cout << std::endl << "Simp. Dep. = " << cs->toISLString() << std::endl;

//    EXPECT_EQ( ex_cs->toISLString() , cs->toISLString() );



    delete cs;
//    delete ex_cs;

    delete F1;
    delete A1;
    delete F2;
    delete A2;
    delete F3;
    delete A3;
    delete F4;
    delete A4;
    delete F5;
    delete A5;
    delete F6;
    delete A6;
    delete F7;
    delete A7;
    delete F8;
    delete A8;
}


