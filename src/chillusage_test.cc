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
 *
 *
 * HOW TO USE INTERFACAE TO SIMPLIFICATION ALGORITHM:
 * 
 * (1) You need to define uninterpreted function calls (UFCs) that appear
 *        in constraints for iegenlib enviroment. The example in the main
 *        function shows how to do this. Note that, you need to this only once
 *        for sets that have same UFCs during one run
 *
 * (2) You need to put constraints in iegenlib Set (or Relation).
 *
 * (3) You can add user defined constraints to the Sets as demonstrated
 *     in examples. 
 * 
 * (4) Create a std::set that includes which tuple variables (loop iterators)
 *     we are not going to project out. 
 * 
 * (5) Use simplifyForPartialParallel function that is main interface for the
 *     algorithm. If Sets are not satisfiable the function would return NULL.
 *     Otherwise it would return the simplified result as iegenlib Set.
 * 
 * (6) Print out result (if not NULL) using toISLString() function. 
 
 *  First example shows each step clearly.
 */

#include <gtest/gtest.h>

#include <iegenlib.h>
using namespace iegenlib;
using namespace std;

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

    // Specify loops that are going to be parallelized, so we are not going to
    // project them out.
    std::set<int> parallelTvs;
    parallelTvs.insert(0);
    parallelTvs.insert(1);

    //*** Simplifying flow dependence

    Set* flow_sim = flow->simplifyForPartialParallel(parallelTvs);
// Print results
//   std::cout<<std::endl<< "Simp. Dep. = "<<flow_sim->toISLString()<<std::endl;
    EXPECT_EQ( ex_flow->toISLString() , flow_sim->toISLString() );


    //*** Simplifying Anti dependence

    Set* anti_sim = anti->simplifyForPartialParallel(parallelTvs);
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
    // (1)
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


    // (2)
    // Putting constraints in an iegenlib::Set
    // Data access dependence from ILU CSR code

    Set *A1 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: ip < i"
                                   " && 0 <= i && i < m"
                                  " && 0 <= ip && ip < m"
                           " && rowptr(i) <= k && k < diagptr(i)"
                         " && rowptr(ip) <= kp && kp < diagptr(ip)"
                                   " && k < j1 && j1 < rowptr(1+i)"
                                 " && kp < j1p && j1p < rowptr(1+ip)"
                  " && diagptr(colidx(k)) < j2 && j2 < rowptr(1+colidx(k))"
                " && diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp))"

                                     " && k = kp }");

    // expected output  (for testing purposes)
    string ex_A1_str("Not Satisfiable");


    // (3)
    // How to add user defined constraint
    Set* A1_extend = A1->addUFConstraints("rowptr","<=", "diagptr");


    // (4) 
    // Specify loops that are going to be parallelized, so we are not going to
    // project them out. Here "i" and "ip"
    std::set<int> parallelTvs;
    parallelTvs.insert(0);
    parallelTvs.insert(1);


    // (5)
    // Simplifyng the constraints set

    Set* A1_sim = A1_extend->simplifyForPartialParallel(parallelTvs);


    // (6)
    // Print out results
    // If set is not satisfiable simplifyForPartialParallel is going to return
    // NULL, we should check this before getting result's string with
    // toISLString. 
    string A1_sim_str("Not Satisfiable");
    if( A1_sim ){
        A1_sim_str = A1_sim->toISLString();
    }
    // cout<<"\n\nA1 simplified = "<<A1_sim_str<<"\n\n";    

    // For testing purposes
    EXPECT_EQ( ex_A1_str , A1_sim_str );


    Set *F1 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: i < ip"
                                   " && 0 <= i && i < m"
                                  " && 0 <= ip && ip < m"
                           " && rowptr(i) <= k && k < diagptr(i)"
                         " && rowptr(ip) <= kp && kp < diagptr(ip)"
                                   " && k < j1 && j1 < rowptr(1+i)"
                                 " && kp < j1p && j1p < rowptr(1+ip)"
                  " && diagptr(colidx(k)) < j2 && j2 < rowptr(1+colidx(k))"
                " && diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp))"

                                     " && k = kp}");

    string ex_F1_str("Not Satisfiable");


    // Adding user defined constraint
    Set* F1_extend = F1->addUFConstraints("rowptr","<=", "diagptr");

    // Simplifyng the constraints set
    Set* F1_sim = F1_extend->simplifyForPartialParallel(parallelTvs);

    string F1_sim_str("Not Satisfiable");
    if( F1_sim ){
        F1_sim_str = F1_sim->toISLString();
    }

    // cout<<"\n\nF1 simplified = "<<F1_sim_str<<"\n\n";

    EXPECT_EQ( ex_A1_str , A1_sim_str );

    delete A1;
    delete F1;
    delete A1_extend;
    delete A1_sim;
    delete F1_extend;
    delete F1_sim;

    // ----------------------------

    Set *A2 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: ip < i"
                                   " && 0 <= i && i < m"
                                  " && 0 <= ip && ip < m"
                           " && rowptr(i) <= k && k < diagptr(i)"
                         " && rowptr(ip) <= kp && kp < diagptr(ip)"
                                   " && k < j1 && j1 < rowptr(1+i)"
                                 " && kp < j1p && j1p < rowptr(1+ip)"
                  " && diagptr(colidx(k)) < j2 && j2 < rowptr(1+colidx(k))"
                " && diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp))"

                                     " && k = diagptr(colidx(kp)) }");

    string ex_A2_str("Not Satisfiable");

    Set *F2 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: i < ip"
                                   " && 0 <= i && i < m"
                                  " && 0 <= ip && ip < m"
                           " && rowptr(i) <= k && k < diagptr(i)"
                         " && rowptr(ip) <= kp && kp < diagptr(ip)"
                                   " && k < j1 && j1 < rowptr(1+i)"
                                 " && kp < j1p && j1p < rowptr(1+ip)"
                  " && diagptr(colidx(k)) < j2 && j2 < rowptr(1+colidx(k))"
                " && diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp))"

                                     " && k = diagptr(colidx(kp))}");

    string ex_F2_str("Not Satisfiable");

    //--- Simplifying flow dependence
    // Adding user defined constraint
    Set* A2_extend = A2->addUFConstraints("rowptr","<=", "diagptr");

    // Simplifyng the constraints set
    Set* A2_sim = A2_extend->simplifyForPartialParallel(parallelTvs);

    string A2_sim_str("Not Satisfiable");
    if( A2_sim ){
        A2_sim_str = A2_sim->toISLString();
    }

    // (6)
    // Print out results
    // cout<<"\n\nA2 simplified = "<<A2_sim_str<<"\n\n"; 

    EXPECT_EQ( ex_A2_str , A2_sim_str );


    //--- Simplifying Anti dependence
    // Adding user defined constraint
    Set* F2_extend = F2->addUFConstraints("rowptr","<=", "diagptr");

    // Simplifyng the constraints set
    Set* F2_sim = F2_extend->simplifyForPartialParallel(parallelTvs);

    string F2_sim_str("Not Satisfiable");
    if( F2_sim ){
        F2_sim_str = F2_sim->toISLString();
    }

    // (6)
    // Print out results
    // cout<<"\n\nF2 simplified = "<<F2_sim_str<<"\n\n"; 

    EXPECT_EQ( ex_F2_str , F2_sim_str );

    delete A2;
    delete F2;
    delete A2_extend;
    delete A2_sim;
    delete F2_extend;
    delete F2_sim;

    // ----------------------------

    Set *A3 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: ip < i"
                                   " && 0 <= i && i < m"
                                  " && 0 <= ip && ip < m"
                           " && rowptr(i) <= k && k < diagptr(i)"
                         " && rowptr(ip) <= kp && kp < diagptr(ip)"
                                   " && k < j1 && j1 < rowptr(1+i)"
                                 " && kp < j1p && j1p < rowptr(1+ip)"
                  " && diagptr(colidx(k)) < j2 && j2 < rowptr(1+colidx(k))"
                " && diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp))"

                                     " && k = j1p}");

    string ex_A3_str("Not Satisfiable");

    Set *F3 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: i < ip"
                                   " && 0 <= i && i < m"
                                  " && 0 <= ip && ip < m"
                           " && rowptr(i) <= k && k < diagptr(i)"
                         " && rowptr(ip) <= kp && kp < diagptr(ip)"
                                   " && k < j1 && j1 < rowptr(1+i)"
                                 " && kp < j1p && j1p < rowptr(1+ip)"
                  " && diagptr(colidx(k)) < j2 && j2 < rowptr(1+colidx(k))"
                " && diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp))"

                                     " && k = j1p}");

    string ex_F3_str("");


    //--- Simplifying flow dependence
    // Adding user defined constraint
    Set* A3_extend = A3->addUFConstraints("rowptr","<=", "diagptr");

    // Simplifyng the constraints set
    Set* A3_sim = A3_extend->simplifyForPartialParallel(parallelTvs);

    string A3_sim_str("Not Satisfiable");
    if( A3_sim ){
        A3_sim_str = A3_sim->toISLString();
    }

    // (6)
    // Print out results
    // cout<<"\n\nA3 simplified = "<<A3_sim_str<<"\n\n"; 

    EXPECT_EQ( ex_A3_str , A3_sim_str );


    //--- Simplifying Anti dependence
    // Adding user defined constraint
    Set* F3_extend = F3->addUFConstraints("rowptr","<=", "diagptr");

    // Simplifyng the constraints set
    Set* F3_sim = F3_extend->simplifyForPartialParallel(parallelTvs);

    string F3_sim_str("Not Satisfiable");
    if( F3_sim ){
        F3_sim_str = F3_sim->toISLString();
    }

    // (6)
    // Print out results
    // cout<<"\n\nF3 simplified = "<<F3_sim_str<<"\n\n"; 

//    EXPECT_EQ( ex_F3_str , F3_sim_str );

    delete A3;
    delete F3;
    delete A3_extend;
    delete A3_sim;
    delete F3_extend;
    delete F3_sim;

    // ----------------------------

    Set *A4 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: ip < i"
                                   " && 0 <= i && i < m"
                                  " && 0 <= ip && ip < m"
                           " && rowptr(i) <= k && k < diagptr(i)"
                         " && rowptr(ip) <= kp && kp < diagptr(ip)"
                                   " && k < j1 && j1 < rowptr(1+i)"
                                 " && kp < j1p && j1p < rowptr(1+ip)"
                  " && diagptr(colidx(k)) < j2 && j2 < rowptr(1+colidx(k))"
                " && diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp))"

                                     " && k = j2p}");

    string ex_A4_str("Not Satisfiable");

    Set *F4 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: i < ip"
                                   " && 0 <= i && i < m"
                                  " && 0 <= ip && ip < m"
                           " && rowptr(i) <= k && k < diagptr(i)"
                         " && rowptr(ip) <= kp && kp < diagptr(ip)"
                                   " && k < j1 && j1 < rowptr(1+i)"
                                 " && kp < j1p && j1p < rowptr(1+ip)"
                  " && diagptr(colidx(k)) < j2 && j2 < rowptr(1+colidx(k))"
                " && diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp))"

                                     " && k = j2p}");

    string ex_F4_str("Not Satisfiable");

    //--- Simplifying flow dependence
    // Adding user defined constraint
    Set* A4_extend = A4->addUFConstraints("rowptr","<=", "diagptr");

    // Simplifyng the constraints set
    Set* A4_sim = A4_extend->simplifyForPartialParallel(parallelTvs);

    string A4_sim_str("Not Satisfiable");
    if( A4_sim ){
        A4_sim_str = A4_sim->toISLString();
    }

    // (6)
    // Print out results
    // cout<<"\n\nA4 simplified = "<<A4_sim_str<<"\n\n"; 

    EXPECT_EQ( ex_A4_str , A4_sim_str );


    //--- Simplifying Anti dependence
    // Adding user defined constraint
    Set* F4_extend = F4->addUFConstraints("rowptr","<=", "diagptr");

    // Simplifyng the constraints set
    Set* F4_sim = F4_extend->simplifyForPartialParallel(parallelTvs);

    string F4_sim_str("Not Satisfiable");
    if( F4_sim ){
        F4_sim_str = F4_sim->toISLString();
    }

    // (6)
    // Print out results
    // cout<<"\n\nF4 simplified = "<<F4_sim_str<<"\n\n"; 

    EXPECT_EQ( ex_F4_str , F4_sim_str );

    delete A4;
    delete F4;
    delete A4_extend;
    delete A4_sim;
    delete F4_extend;
    delete F4_sim;

    // ----------------------------


    Set *A5 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: ip < i"
                                   " && 0 <= i && i < m"
                                  " && 0 <= ip && ip < m"
                           " && rowptr(i) <= k && k < diagptr(i)"
                         " && rowptr(ip) <= kp && kp < diagptr(ip)"
                                   " && k < j1 && j1 < rowptr(1+i)"
                                 " && kp < j1p && j1p < rowptr(1+ip)"
                  " && diagptr(colidx(k)) < j2 && j2 < rowptr(1+colidx(k))"
                " && diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp))"

                                     " && j1 = diagptr(colidx(kp)) }");

    Set *ex_A5 = new Set("[ m, nnz ] -> { [i, ip, k, kp] : i = colidx(kp) &&"
" ip >= 0 && k >= 0 && kp >= 0 && colidx(k) >= 0 && colidx(kp) >= 0 &&"
" diagptr(i) >= 0 && diagptr(i + 1) >= 0 && diagptr(ip) >= 0 &&"
" diagptr(ip + 1) >= 0 && diagptr(colidx(k)) >= 0 &&"
" diagptr(colidx(kp)) >= 0 && rowptr(i) >= 0 && rowptr(i + 1) >= 0 &&"
" rowptr(ip) >= 0 && rowptr(ip + 1) >= 0 && rowptr(colidx(k)) >= 0 &&"
" rowptr(colidx(kp)) >= 0 && k - rowptr(i) >= 0 && kp - rowptr(ip) >= 0 &&"
" nnz - diagptr(colidx(k) + 1) >= 0 && nnz - diagptr(colidx(kp) + 1) >= 0 &&"
" nnz - rowptr(colidx(k) + 1) >= 0 && nnz - rowptr(colidx(kp) + 1) >= 0 &&"
" diagptr(i + 1) - rowptr(i + 1) >= 0 && diagptr(ip + 1) - rowptr(ip + 1) >= 0 &&"
" diagptr(colidx(k)) - rowptr(colidx(k)) >= 0 && diagptr(colidx(k) + 1) + 1 >= 0 &&"
" diagptr(colidx(kp)) - rowptr(colidx(kp)) >= 0 && diagptr(colidx(kp) + 1) + 1 >= 0 &&"
" rowptr(colidx(k) + 1) + 1 >= 0 && rowptr(colidx(kp) + 1) + 1 >= 0 &&"
" -ip + m - 2 >= 0 && -ip + colidx(kp) - 1 >= 0 && -k + nnz - 1 >= 0 &&"
" -k + diagptr(i) - 1 >= 0 && -k + diagptr(colidx(kp)) - 1 >= 0 &&"
" -kp + nnz - 1 >= 0 && -kp + diagptr(ip) - 1 >= 0 &&"
" -kp + rowptr(ip + 1) - 2 >= 0 && m - colidx(k) - 2 >= 0 &&"
" m - colidx(kp) - 2 >= 0 && nnz - diagptr(i) - 1 >= 0 &&"
" nnz - diagptr(i + 1) - 1 >= 0 && nnz - diagptr(ip) - 1 >= 0 &&"
" nnz - diagptr(ip + 1) - 1 >= 0 && nnz - diagptr(colidx(k)) - 1 >= 0 &&"
" nnz - diagptr(colidx(kp)) - 1 >= 0 && nnz - rowptr(i) - 1 >= 0 &&"
" nnz - rowptr(i + 1) - 1 >= 0 && nnz - rowptr(ip) - 1 >= 0 &&"
" nnz - rowptr(ip + 1) - 1 >= 0 && nnz - rowptr(colidx(k)) - 1 >= 0 &&"
" nnz - rowptr(colidx(kp)) - 1 >= 0 &&"
" -diagptr(colidx(k)) + rowptr(colidx(k) + 1) + 3 >= 0 &&"
" diagptr(colidx(k) + 1) - rowptr(colidx(k) + 1) + 10 >= 0 &&"
" -diagptr(colidx(kp)) + rowptr(i + 1) - 1 >= 0 &&"
" -diagptr(colidx(kp)) + rowptr(colidx(kp) + 1) + 3 >= 0 &&"
" diagptr(colidx(kp) + 1) - rowptr(colidx(kp) + 1) + 10 >= 0 }");

    string ex_A5_str = ex_A5->toISLString();

    Set *F5 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: i < ip"
                                   " && 0 <= i && i < m"
                                  " && 0 <= ip && ip < m"
                           " && rowptr(i) <= k && k < diagptr(i)"
                         " && rowptr(ip) <= kp && kp < diagptr(ip)"
                                   " && k < j1 && j1 < rowptr(1+i)"
                                 " && kp < j1p && j1p < rowptr(1+ip)"
                  " && diagptr(colidx(k)) < j2 && j2 < rowptr(1+colidx(k))"
                " && diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp))"

                                     " && j1 = diagptr(colidx(k))}");

    Set *ex_F5 = new Set("[ m, nnz ] -> { [i, ip, k, kp] : i = colidx(k) &&"
" ip >= 0 && k >= 0 && kp >= 0 && colidx(k) >= 0 && colidx(kp) >= 0 &&"
" diagptr(i) >= 0 && diagptr(i + 1) >= 0 && diagptr(ip) >= 0 &&"
" diagptr(ip + 1) >= 0 && diagptr(colidx(k)) >= 0 && diagptr(colidx(kp)) >= 0 &&"
" rowptr(i) >= 0 && rowptr(i + 1) >= 0 && rowptr(ip) >= 0 &&"
" rowptr(ip + 1) >= 0 && rowptr(colidx(k)) >= 0 && rowptr(colidx(kp)) >= 0 &&"
" k - rowptr(i) >= 0 && kp - rowptr(ip) >= 0 &&"
" nnz - diagptr(colidx(k) + 1) >= 0 && nnz - diagptr(colidx(kp) + 1) >= 0 &&"
" nnz - rowptr(colidx(k) + 1) >= 0 && nnz - rowptr(colidx(kp) + 1) >= 0 &&"
" diagptr(i + 1) >= rowptr(i + 1) && diagptr(ip + 1) >= rowptr(ip + 1) &&"
" diagptr(colidx(k)) - rowptr(colidx(k)) >= 0 &&"
" diagptr(colidx(k) + 1) + 1 >= 0 &&"
" diagptr(colidx(kp)) - rowptr(colidx(kp)) >= 0 &&"
" diagptr(colidx(kp) + 1) + 1 >= 0 && rowptr(colidx(k) + 1) + 1 >= 0 &&"
" rowptr(colidx(kp) + 1) + 1 >= 0 && -ip + m - 2 >= 0 &&"
" ip > colidx(k) && nnz > k && -k + diagptr(i) - 1 >= 0 &&"
" -k + diagptr(colidx(k)) - 1 >= 0 && -kp + nnz - 1 >= 0 &&"
" -kp + diagptr(ip) - 1 >= 0 && -kp + rowptr(ip + 1) - 2 >= 0 &&"
" m - colidx(k) - 2 >= 0 && m - colidx(kp) - 2 >= 0 &&"
" nnz - diagptr(i) - 1 >= 0 && nnz - diagptr(i + 1) - 1 >= 0 &&"
" nnz - diagptr(ip) - 1 >= 0 && nnz - diagptr(ip + 1) - 1 >= 0 &&"
" nnz - diagptr(colidx(k)) - 1 >= 0 && nnz - diagptr(colidx(kp)) - 1 >= 0 &&"
" nnz - rowptr(i) - 1 >= 0 && nnz - rowptr(i + 1) - 1 >= 0 &&"
" nnz - rowptr(ip) - 1 >= 0 && nnz - rowptr(ip + 1) - 1 >= 0 &&"
" nnz - rowptr(colidx(k)) - 1 >= 0 && nnz - rowptr(colidx(kp)) - 1 >= 0 &&"
" -diagptr(colidx(k)) + rowptr(i + 1) - 1 >= 0 &&"
" -diagptr(colidx(k)) + rowptr(colidx(k) + 1) + 3 >= 0 &&"
" diagptr(colidx(k) + 1) - rowptr(colidx(k) + 1) + 10 >= 0 &&"
" -diagptr(colidx(kp)) + rowptr(colidx(kp) + 1) + 3 >= 0 &&"
" diagptr(colidx(kp) + 1) - rowptr(colidx(kp) + 1) + 10 >= 0 }");

    string ex_F5_str = ex_F5->toISLString();

    //--- Simplifying flow dependence
    // Adding user defined constraint
    Set* A5_extend = A5->addUFConstraints("rowptr","<=", "diagptr");

    // Simplifyng the constraints set
    Set* A5_sim = A5_extend->simplifyForPartialParallel(parallelTvs);

    string A5_sim_str("Not Satisfiable");
    if( A5_sim ){
        A5_sim_str = A5_sim->toISLString();
    }

    // (6)
    // Print out results
    // cout<<"\n\nA5 simplified = "<<A5_sim_str<<"\n\n"; 
    EXPECT_EQ( ex_A5_str , A5_sim_str );


    //--- Simplifying Anti dependence
    // Adding user defined constraint
    Set* F5_extend = F5->addUFConstraints("rowptr","<=", "diagptr");

    // Simplifyng the constraints set
    Set* F5_sim = F5_extend->simplifyForPartialParallel(parallelTvs);

    string F5_sim_str("Not Satisfiable");
    if( F5_sim ){
        F5_sim_str = F5_sim->toISLString();
    }
    // (6)
    // Print out results
    // cout<<"\n\nF5 simplified = "<<F5_sim_str<<"\n\n";

    EXPECT_EQ( ex_F5_str , F5_sim_str );

    delete A5;
    delete F5;
    delete A5_extend;
    delete A5_sim;
    delete F5_extend;
    delete F5_sim;

    // ----------------------------


    Set *A6 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: ip < i"
                                   " && 0 <= i && i < m"
                                  " && 0 <= ip && ip < m"
                           " && rowptr(i) <= k && k < diagptr(i)"
                         " && rowptr(ip) <= kp && kp < diagptr(ip)"
                                   " && k < j1 && j1 < rowptr(1+i)"
                                 " && kp < j1p && j1p < rowptr(1+ip)"
                  " && diagptr(colidx(k)) < j2 && j2 < rowptr(1+colidx(k))"
                " && diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp))"

                                     " && j1 = j1p}");


    string ex_A6_str("Not Satisfiable");

    Set *F6 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: i < ip"
                                   " && 0 <= i && i < m"
                                  " && 0 <= ip && ip < m"
                           " && rowptr(i) <= k && k < diagptr(i)"
                         " && rowptr(ip) <= kp && kp < diagptr(ip)"
                                   " && k < j1 && j1 < rowptr(1+i)"
                                 " && kp < j1p && j1p < rowptr(1+ip)"
                  " && diagptr(colidx(k)) < j2 && j2 < rowptr(1+colidx(k))"
                " && diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp))"

                                     " && j1 = j1p}");

    string ex_F6_str("Not Satisfiable");


    //--- Simplifying flow dependence
    // Adding user defined constraint
    Set* A6_extend = A6->addUFConstraints("rowptr","<=", "diagptr");

    // Simplifyng the constraints set
    Set* A6_sim = A6_extend->simplifyForPartialParallel(parallelTvs);

    string A6_sim_str("Not Satisfiable");
    if( A6_sim ){
        A6_sim_str = A6_sim->toISLString();
    }

    // (6)
    // Print out results
    // cout<<"\n\nA6 simplified = "<<A6_sim_str<<"\n\n"; 

    EXPECT_EQ( ex_A6_str , A6_sim_str );


    //--- Simplifying Anti dependence
    // Adding user defined constraint
    Set* F6_extend = F6->addUFConstraints("rowptr","<=", "diagptr");

    // Simplifyng the constraints set
    Set* F6_sim = F6_extend->simplifyForPartialParallel(parallelTvs);

    string F6_sim_str("Not Satisfiable");
    if( F6_sim ){
        F6_sim_str = F6_sim->toISLString();
    }

    // (6)
    // Print out results
    // cout<<"\n\nF6 simplified = "<<F6_sim_str<<"\n\n";

    EXPECT_EQ( ex_F6_str , F6_sim_str );


    delete A6;
    delete F6;
    delete A6_extend;
    delete A6_sim;
    delete F6_extend;
    delete F6_sim;

    // ----------------------------


    Set *A7 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: ip < i"
                                   " && 0 <= i && i < m"
                                  " && 0 <= ip && ip < m"
                           " && rowptr(i) <= k && k < diagptr(i)"
                         " && rowptr(ip) <= kp && kp < diagptr(ip)"
                                   " && k < j1 && j1 < rowptr(1+i)"
                                 " && kp < j1p && j1p < rowptr(1+ip)"
                  " && diagptr(colidx(k)) < j2 && j2 < rowptr(1+colidx(k))"
                " && diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp))"

                                     " && j1 = kp}");

    string ex_A7_str("Not Satisfiable");

    Set *F7 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: i < ip"
                                   " && 0 <= i && i < m"
                                  " && 0 <= ip && ip < m"
                           " && rowptr(i) <= k && k < diagptr(i)"
                         " && rowptr(ip) <= kp && kp < diagptr(ip)"
                                   " && k < j1 && j1 < rowptr(1+i)"
                                 " && kp < j1p && j1p < rowptr(1+ip)"
                  " && diagptr(colidx(k)) < j2 && j2 < rowptr(1+colidx(k))"
                " && diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp))"

                                     " && j1 = kp}");

    string ex_F7_str("Not Satisfiable");

    //--- Simplifying flow dependence
    // Adding user defined constraint
    Set* A7_extend = A7->addUFConstraints("rowptr","<=", "diagptr");

    // Simplifyng the constraints set
    Set* A7_sim = A7_extend->simplifyForPartialParallel(parallelTvs);

    string A7_sim_str("Not Satisfiable");
    if( A7_sim ){
        A7_sim_str = A7_sim->toISLString();
    }

    // (6)
    // Print out results
    // cout<<"\n\nA7 simplified = "<<A7_sim_str<<"\n\n"; 

    EXPECT_EQ( ex_A7_str , A7_sim_str );


    //--- Simplifying Anti dependence
    // Adding user defined constraint
    Set* F7_extend = F7->addUFConstraints("rowptr","<=", "diagptr");

    // Simplifyng the constraints set
    Set* F7_sim = F7_extend->simplifyForPartialParallel(parallelTvs);

    string F7_sim_str("Not Satisfiable");
    if( F7_sim ){
        F7_sim_str = F7_sim->toISLString();
    }

    // (6)
    // Print out results
    // cout<<"\n\nF7 simplified = "<<F7_sim_str<<"\n\n"; 

    EXPECT_EQ( ex_F7_str , F7_sim_str );

    delete A7;
    delete F7;
    delete A7_extend;
    delete A7_sim;
    delete F7_extend;
    delete F7_sim;

    // ----------------------------


    Set *A8 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: ip < i"
                                   " && 0 <= i && i < m"
                                  " && 0 <= ip && ip < m"
                           " && rowptr(i) <= k && k < diagptr(i)"
                         " && rowptr(ip) <= kp && kp < diagptr(ip)"
                                   " && k < j1 && j1 < rowptr(1+i)"
                                 " && kp < j1p && j1p < rowptr(1+ip)"
                  " && diagptr(colidx(k)) < j2 && j2 < rowptr(1+colidx(k))"
                " && diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp))"

                                     " && j1 = j2p}");

    Set *ex_A8 = new Set("[ m, nnz ] -> { [i, ip, k, kp] : i - colidx(kp) = 0"
" && ip >= 0 && colidx(k) >= 0 && rowptr(i) >= 0 && rowptr(ip) >= 0"
" && rowptr(colidx(k)) >= 0 && rowptr(colidx(kp)) >= 0"
" && k - rowptr(i) >= 0 && kp - rowptr(ip) >= 0"
" && nnz - diagptr(colidx(k) + 1) >= 0 && nnz - diagptr(colidx(kp) + 1) >= 0"
" && nnz - rowptr(colidx(k) + 1) >= 0 && nnz - rowptr(colidx(kp) + 1) >= 0"
" && diagptr(i + 1) - rowptr(i + 1) >= 0"
" && diagptr(ip + 1) - rowptr(ip + 1) >= 0"
" && diagptr(colidx(k)) - rowptr(colidx(k)) >= 0"
" && diagptr(colidx(k) + 1) + 1 >= 0"
" && diagptr(colidx(kp)) - rowptr(colidx(kp)) >= 0"
" && diagptr(colidx(kp) + 1) + 1 >= 0 && rowptr(colidx(k) + 1) + 1 >= 0"
" && rowptr(colidx(kp) + 1) + 1 >= 0"
" && -ip + colidx(kp) - 1 >= 0"
" && -k + diagptr(i) - 1 >= 0 && -k + rowptr(i + 1) - 2 >= 0"
" && -k + rowptr(colidx(kp) + 1) + 3 >= 0 && -kp + diagptr(ip) - 1 >= 0"
" && -kp + rowptr(ip + 1) - 2 >= 0 && m - colidx(k) - 2 >= 0"
" && m - colidx(kp) - 2 >= 0 && nnz - diagptr(i) - 1 >= 0"
" && nnz - diagptr(i + 1) - 1 >= 0 && nnz - diagptr(ip) - 1 >= 0"
" && nnz - diagptr(ip + 1) - 1 >= 0 && nnz - diagptr(colidx(k)) - 1 >= 0"
" && -diagptr(colidx(k)) + rowptr(colidx(k) + 1) + 3 >= 0"
" && diagptr(colidx(k) + 1) - rowptr(colidx(k) + 1) + 10 >= 0"
" && -diagptr(colidx(kp)) + rowptr(i + 1) - 2 >= 0"
" && -diagptr(colidx(kp)) + rowptr(colidx(kp) + 1) + 3 >= 0"
" && diagptr(colidx(kp) + 1) - rowptr(colidx(kp) + 1) + 10 >= 0 }");

    string ex_A8_str = ex_A8->toISLString();

    Set *F8 = new Set("[m] -> {[i,ip,k,kp,j1,j1p,j2,j2p]: i < ip"
                                   " && 0 <= i && i < m"
                                  " && 0 <= ip && ip < m"
                           " && rowptr(i) <= k && k < diagptr(i)"
                         " && rowptr(ip) <= kp && kp < diagptr(ip)"
                                   " && k < j1 && j1 < rowptr(1+i)"
                                 " && kp < j1p && j1p < rowptr(1+ip)"
                  " && diagptr(colidx(k)) < j2 && j2 < rowptr(1+colidx(k))"
                " && diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp))"
                             " && colidx(j1) = colidx(j2)"
                             " && colidx(j1p) = colidx(j2p)"
                                     " && j1 = j2p}");

    Set *ex_F8 = new Set("[ m, nnz ] -> { [i, ip, k, kp] : i = colidx(kp) &&"
" colidx(k) >= 0 && colidx(kp) >= 0 && rowptr(i) >= 0 && rowptr(ip) >= 0 &&"
" rowptr(colidx(k)) >= 0 && rowptr(colidx(kp)) >= 0 && k >= rowptr(i) &&"
" kp >= rowptr(ip) && nnz >= diagptr(colidx(k) + 1) &&"
" nnz >= diagptr(colidx(kp) + 1) && nnz >= rowptr(colidx(k) + 1) &&"
" nnz >= rowptr(colidx(kp) + 1) && diagptr(i + 1) >= rowptr(i + 1) &&"
" diagptr(ip + 1) >= rowptr(ip + 1) &&"
" diagptr(colidx(k)) >= rowptr(colidx(k)) &&"
" diagptr(colidx(k) + 1) + 1 >= 0 &&"
" diagptr(colidx(kp)) >= rowptr(colidx(kp)) &&"
" diagptr(colidx(kp) + 1) + 1 >= 0 && rowptr(colidx(k) + 1) + 1 >= 0 &&"
" rowptr(colidx(kp) + 1) + 1 >= 0 &&  m >= ip + 2 && ip > colidx(kp) &&"
" diagptr(i) > k && rowptr(i + 1) >= k + 2 &&"
" rowptr(colidx(kp) + 1) + 3 >= k && diagptr(ip) >= kp + 1 &&"
" rowptr(ip + 1) >= kp + 2 && m >= colidx(k) + 2 && nnz > diagptr(i) &&"
" nnz > diagptr(i + 1) && nnz > diagptr(ip) && nnz > diagptr(ip + 1) &&"
" nnz > diagptr(colidx(k)) + 1 &&"
" -diagptr(colidx(k)) + rowptr(colidx(k) + 1) + 3 >= 0 &&"
" diagptr(colidx(k) + 1) + 10 >= rowptr(colidx(k) + 1) &&"
" rowptr(i + 1) >= diagptr(colidx(kp)) + 2 &&"
" rowptr(colidx(kp) + 1) + 3 >= diagptr(colidx(kp)) &&"
" diagptr(colidx(kp) + 1) + 10 >=  rowptr(colidx(kp) + 1) }");

    string ex_F8_str = ex_F8->toISLString();

    //--- Simplifying flow dependence
    // Adding user defined constraint
    Set* A8_extend = A8->addUFConstraints("rowptr","<=", "diagptr");

    // Simplifyng the constraints set
    Set* A8_sim = A8_extend->simplifyForPartialParallel(parallelTvs);

    string A8_sim_str("Not Satisfiable");
    if( A8_sim ){
        A8_sim_str = A8_sim->toISLString();
    }

    // (6)
    // Print out results
    // cout<<"\n\nA8 simplified = "<<A8_sim_str<<"\n\n"; 

    EXPECT_EQ( ex_A8_str , A8_sim_str );


    // Applying heuristic III.A           /////////////////////////////////////
    F8->RemoveExpensiveConsts(parallelTvs, 2);

    //--- Simplifying Anti dependence
    // Adding user defined constraint
    Set* F8_extend = F8->addUFConstraints("rowptr","<=", "diagptr");

    // Simplifyng the constraints set
    Set* F8_sim = F8_extend->simplifyForPartialParallel(parallelTvs);

    string F8_sim_str("Not Satisfiable");
    if( F8_sim ){
        F8_sim_str = F8_sim->toISLString();
    }

    // (6)
    // Print out results
    // cout<<"\n\nF8 simplified = "<<F8_sim_str<<"\n\n"; 

    EXPECT_EQ( ex_F8_str , F8_sim_str );

//    delete A8;
    delete F8;
//    delete A8_extend;
//    delete A8_sim;
    delete F8_extend;
    delete F8_sim;
}
