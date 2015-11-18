/*!
 * \file chillusage_test.cc
 *
 * \brief Tests the part of IEGenLib interface that will be needed in CHILL.
 *
 * \date Started: 8/16/13
 * # $Revision:: 391                $: last committed revision
 * # $Date:: 5/30/12                $: date of last committed revision
 * # $Author:: heim                 $: author of last committed revision
 *
 * \authors Michelle Strout
 *
 * Copyright (c) 2013, Colorado State University <br>
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


//*****************************************************************************

// Testing isUFSArg: is a tuple variable argument to a UFS? (with its index)

TEST_F(ChillUsageTest, ISUFSARG) {

    iegenlib::setCurrEnv();
    iegenlib::appendCurrEnv("col",
        new Set("{[i]:0<=i &&i<n}"), 
        new Set("{[j]:0<=j &&j<n}"), true);
    iegenlib::appendCurrEnv("idx",
        new Set("{[i]:0<=i &&i<n}"), 
        new Set("{[j]:0<=j &&j<n}"), true);
    iegenlib::appendCurrEnv("row",
        new Set("{[i]:0<=i &&i<n}"), 
        new Set("{[j]:0<=j &&j<n}"), true);
    iegenlib::appendCurrEnv("diag",
        new Set("{[i]:0<=i &&i<n}"), 
        new Set("{[j]:0<=j &&j<n}"), true);

    Relation *r1 = new Relation("[n] -> { [i,j] -> [ip,jp] : i = col(jp) and i < ip "
                             "and 0 <= i and i < n and idx(i) <= j and j < idx(i+1) "
                 "and 0 <= ip and ip < n and idx(ip) <= jp and jp < idx(ip+1) }");

    Relation *r2 = new Relation("[n] -> { [i,k,j1,j2] -> [ip,kp,jp1,jp2] : i < ip"
               " and j1 = jp2 and 0 <= i and i < n and 0 <= ip and ip <= n and "
               "k+1 <= j1 and j1 < row(i+1) and kp+1 <= jp1 and jp1 < row(ip+1) and "
               "diag(col(k))+1 <= j2 and j2 < row(col(k)+1) and diag(col(kp))+1 <="
               " jp2 and jp2 < row(col(kp)+1) and row(i) <= k and k < diag(i) "
               "and row(ip) <= kp and kp < diag(ip) }");

   //  Test case for r2
   string org_tup("11001100");
   string ins_tup("00000000");
 
   int ar = r2->arity();
   for(int i = 0 ; i < ar ; i++)
   {
     if ( r2->isUFSArg(i) )
     {
       ins_tup[i] = '1';
//       std::cout << std::endl <<i << "   is UFS arg" << std::endl;
     }
     else
     {
       ins_tup[i] = '0';
//       std::cout << std::endl <<i << "   is not UFS arg" << std::endl;
     }
   }
   
    EXPECT_EQ( org_tup , ins_tup );

   delete r1;
   delete r2;
}

//*****************************************************************************

//*****************************************************************************
// Testing project_out: project out tuple variable # tvar

TEST_F(ChillUsageTest, PROJECT_OUT) {

    iegenlib::setCurrEnv();
    iegenlib::appendCurrEnv("col",
        new Set("{[i]:0<=i &&i<n}"), 
        new Set("{[j]:0<=j &&j<n}"), true);
    iegenlib::appendCurrEnv("idx",
        new Set("{[i]:0<=i &&i<n}"), 
        new Set("{[j]:0<=j &&j<n}"), true);
    iegenlib::appendCurrEnv("row",
        new Set("{[i]:0<=i &&i<n}"), 
        new Set("{[j]:0<=j &&j<n}"), true);
    iegenlib::appendCurrEnv("diag",
        new Set("{[i]:0<=i &&i<n}"), 
        new Set("{[j]:0<=j &&j<n}"), true);

    Relation *r1 = new Relation("[n] -> { [i,j] -> [ip,jp] : i = col(jp) and i < ip "
                             "and 0 <= i and i < n and idx(i) <= j and j < idx(i+1) "
                 "and 0 <= ip and ip < n and idx(ip) <= jp and jp < idx(ip+1) }");

    Relation *r2 = new Relation("[n] -> { [i,k,j1,j2] -> [ip,kp,jp1,jp2] : i < ip"
               " and j1 = jp2 and 0 <= i and i < n and 0 <= ip and ip <= n and "
               "k+1 <= j1 and j1 < row(i+1) and kp+1 <= jp1 and jp1 < row(ip+1) and "
               "diag(col(k))+1 <= j2 and j2 < row(col(k)+1) and diag(col(kp))+1 <="
               " jp2 and jp2 < row(col(kp)+1) and row(i) <= k and k < diag(i) "
               "and row(ip) <= kp and kp < diag(ip) }");

    Set *s2 = new Set("[n] -> { [i,k,j1,j2,ip,kp,jp1,jp2] : i < ip"
               " and j1 = jp2 and 0 <= i and i < n and 0 <= ip and ip <= n and "
               "k+1 <= j1 and j1 < row(i+1) and kp+1 <= jp1 and jp1 < row(ip+1) and "
               "diag(col(k))+1 <= j2 and j2 < row(col(k)+1) and diag(col(kp))+1 <="
               " jp2 and jp2 < row(col(kp)+1) and row(i) <= k and k < diag(i) "
               "and row(ip) <= kp and kp < diag(ip) }");
 
   int iar = r2->inArity(), ar = r2->arity();
   for(int i = ar-1 ; i >= 0 ; i--)
   {
     if (i == 0 || i == iar)
       continue;

     if ( !r2->isUFSArg(i) )
     {     
       r2->project_out(i);
     }
   }

   int arS = s2->arity();
   for(int i = arS-1 ; i >= 0 ; i--)
   {
     if ( !s2->isUFSArg(i) )
     {     
       s2->project_out(i);
     }
   }

//   std::cout << std::endl << "r2.pr = " << r2->prettyPrintString() << std::endl;
//   std::cout << std::endl << "s2.pr = " << s2->prettyPrintString() << std::endl;

	delete r1;
	delete r2;
	delete s2;
}

//************************************************************************************

