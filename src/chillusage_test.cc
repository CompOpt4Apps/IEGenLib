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

    // CHILL creates the full dependence relation for the loop. And, Ananad
    // indicates monotinicity.

    iegenlib::setCurrEnv();
    iegenlib::appendCurrEnv("colidx",
            new Set("{[i]:0<=i &&i<nnz}"),      // Domain 
            new Set("{[j]:0<=j &&j<m}"),        // Range
            false,                              // Bijective?!
            iegenlib::Monotonic_NONE            // monotonicity
            );
    iegenlib::appendCurrEnv("rowptr",
        new Set("{[i]:0<=i &&i<m}"), 
        new Set("{[j]:0<=j &&j<nnz}"), false, iegenlib::Monotonic_NONE);


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



    //*** Simplifying flow dependence

    Set* flow_sim = flow->simplifyNeedsName();
// Print results
//   std::cout<<std::endl<< "Simp. Dep. = "<<flow_sim->toISLString()<<std::endl;
    EXPECT_EQ( ex_flow->toISLString() , flow_sim->toISLString() );


    //*** Simplifying Anti dependence

    Set* anti_sim = anti->simplifyNeedsName();
// Print results
//   std::cout<<std::endl<<"Simp. Dep. = "<<anti_sim->toISLString()<<std::endl;
    EXPECT_EQ( ex_anti->toISLString() , anti_sim->toISLString() );


    delete ex_flow;
    delete ex_anti;
    delete flow;
    delete anti;
    delete flow_sim;
    delete anti_sim;
}


/*! Test cases CSR ILU code's data access dependencies
*/
TEST_F(ChillUsageTest, ILU_CSR_DepSimplification)
{

    // CHILL creates the full dependence relation for the loop. And, Ananad
    // indicates monotinicity, and adds constraints like:
    //                                  forall e row(e) < diag(e)

    // Introduce the UFCalls to enviroment, and indicate their domain, range
    // whether they are bijective, or monotonic.
    iegenlib::setCurrEnv();
    iegenlib::appendCurrEnv("colidx",
            new Set("{[i]:0<=i &&i<nnz}"),      // Domain 
            new Set("{[j]:0<=j &&j<m}"),        // Range
            false,                              // Bijective?!
            iegenlib::Monotonic_NONE            // monotonicity
            );
    iegenlib::appendCurrEnv("rowptr",
        new Set("{[i]:0<=i &&i<m}"), 
        new Set("{[j]:0<=j &&j<nnz}"), false, iegenlib::Monotonic_Increasing);
    iegenlib::appendCurrEnv("diagptr",
        new Set("{[i]:0<=i &&i<m}"), 
        new Set("{[j]:0<=j &&j<nnz}"), false, iegenlib::Monotonic_Increasing);


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

/*
    //*** Simplifying flow dependence

    Set* F1_sim = F1->simplifyNeedsName();
// Print results
//   std::cout<<std::endl<< "F1 Simp. Dep. = "<<F1_sim->toISLString()<<std::endl;
//    EXPECT_EQ( ex_flow->toISLString() , flow_sim->toISLString() );
*/

    Set *F2 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: ip < i &&"
                " k < j1 && j1 < rowptr(1+i) && j2 < rowptr(1+colidx(k)) &&"
          " diagptr(colidx(k)) < j2 && 0 <= i && i < m && rowptr(i) <= k &&"
  " k < diagptr(i) && rowptr(i) <= diagptr(i) && rowptr(i) < rowptr(1+i) &&"
               " diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp)) &&"
           " j1p < rowptr(1+ip) && kp < diagptr(ip) && 0 <= ip && ip < m &&"
                         " rowptr(ip) <= kp && rowptr(ip) <= diagptr(ip) &&"
                    " rowptr(ip) < rowptr(1+ip) && k = diagptr(colidx(kp)) }");

    Set *A2 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: i < ip &&"
                " k < j1 && j1 < rowptr(1+i) && j2 < rowptr(1+colidx(k)) &&"
          " diagptr(colidx(k)) < j2 && 0 <= i && i < m && rowptr(i) <= k &&"
  " k < diagptr(i) && rowptr(i) <= diagptr(i) && rowptr(i) < rowptr(1+i) &&"
               " diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp)) &&"
           " j1p < rowptr(1+ip) && kp < diagptr(ip) && 0 <= ip && ip < m &&"
                         " rowptr(ip) <= kp && rowptr(ip) <= diagptr(ip) &&"
                    " rowptr(ip) < rowptr(1+ip) && k = diagptr(colidx(kp))}");

/*
    //*** Simplifying flow dependence

    Set* F2_sim = F2->simplifyNeedsName();
// Print results
   std::cout<<std::endl<< "F2 Simp. Dep. = "<<F2_sim->toISLString()<<std::endl;
//    EXPECT_EQ( ex_flow->toISLString() , flow_sim->toISLString() );
*/


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


    //*** Simplifying flow dependence

    Set* F8_sim;
    // First add in user defined constraints
    Set* temp = F8->addUFConstraints("rowptr","<=", "diagptr");

    F8_sim = temp->simplifyNeedsName();

// Print results
//   std::cout<<"\n\n"<< "F8 Simp. Dep. = "<<F8_sim->toISLString()<<"\n\n\n";
//    EXPECT_EQ( ex_F8->toISLString() , F8_sim->toISLString() );



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


