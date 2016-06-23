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
 * HOW TO USE INTERFACAE TO SIMPLIFICATION ALGORITHM for
 * simplifying constraints set containing Uninterpreted Function Calls:
 * 
 * (1) You need to define uninterpreted function calls (UFCs) that appear
 *        in constraints for iegenlib enviroment. The example in the main
 *        function shows how to do this. Note that, you need to this only once
 *        for sets that have same UFCs during one run
 *
 * (2) You need to put constraints in iegenlib Set (or Relation).
 * 
 * (3) Create a std::set that includes which tuple variables (loop iterators)
 *     we are not going to project out. 
 *
 * (4) Apply a heuristic to remove expensive constraints that
 *     is keeping us from projecting out an iterator:
         for instance: 
                        col(j) < n  would keep us from projecting 'j'
       We only remove constraints up to a maximum number determined by user.
 *
 * (5) You can add user defined constraints to the Sets as demonstrated
 *     in examples. 
 * 
 * (6) Use simplifyForPartialParallel function that is main interface for the
 *     algorithm. If Sets are not satisfiable the function would return NULL.
 *     Otherwise it would return the simplified result as iegenlib Set.
 * 
 * (7) Print out result (if not NULL) using toISLString() function. 
 
 *  We have demonstrated these steps in the simplify function
 *  This function reads information from a JSON file (inputFile), and applies
 *  the simplfication algorithm to the sets found in the file.
 */

#include <gtest/gtest.h>

#include <iegenlib.h>
#include "parser/jsoncons/json.hpp"

using jsoncons::json;
using namespace iegenlib;
using namespace std;

// Helper functions
bool printRelation(string msg, Relation *rel);
int str2int(string str);

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

// Reads information from a JSON file, and applies the simplfication
// to sets found in the file
void simplifiy(string inputFile)
{

  iegenlib::setCurrEnv();
  std::set<int> parallelTvs;
  // (0)
  // Read the data from inputFile
  ifstream in(inputFile);
  json data;
  in >> data;

  for(size_t p = 0; p < data.size(); ++p){    // Dependence relations (DR) found in the file
  for (size_t i = 0; i < data[p].size(); ++i){// Conjunctions found for one DR in the file

    // (1)
    // Introduce the uninterpreted function symbols to enviroment, and indicate
    // their domain, range, whether they are bijective, or monotonic.
    if( i == 0 ){  // Read these data only once. 
                   // They are stored in the first conjunction.
      for (size_t j = 0; j < data[p][i]["UFS"].size(); ++j){

        bool bijective = false;
        if( data[p][i]["UFS"][j]["Bijective"].as<string>() == string("true") ){
          bijective = true;
        }
        iegenlib::MonotonicType monotonicity = iegenlib::Monotonic_NONE;
        if(data[p][i]["UFS"][j]["Monotonicity"].as<string>() == 
                                     string("Monotonic_Nondecreasing")){
          monotonicity = iegenlib::Monotonic_Nondecreasing;
        } else if(data[p][i]["UFS"][j]["Monotonicity"].as<string>() == 
                                        string("Monotonic_Increasing")){
          monotonicity = iegenlib::Monotonic_Increasing;
        }

        iegenlib::appendCurrEnv(data[p][i]["UFS"][j]["Name"].as<string>(),// Name
            new Set(data[p][i]["UFS"][j]["Domain"].as<string>()),   // Domain 
            new Set(data[p][i]["UFS"][j]["Range"].as<string>()),    // Range
            bijective,                                              // Bijective?
            monotonicity                                            // Monotonicity?
                                );
      }
    }

    // (2)
    // Putting constraints in an iegenlib::Relation
    // Reading original set.
    Relation* rel = new Relation(data[p][i]["Relation"].as<string>());

    // Reading expected outputs
    Relation *ex_rel = NULL;
    string expected_str = data[p][i]["Expected"].as<string>();
    if ( expected_str != string("Not Satisfiable") ){
      ex_rel = new Relation(expected_str);
    }
    
    // (3)
    // Specify loops that are going to be parallelized, so we are not going to
    // project them out.
    if( i == 0 ){  // Read these data only once. 
                   // They are stored in the first conjunction.

      for (size_t j = 0; j < data[p][0]["Do Not Project"].size(); ++j){
        int tvN = str2int(data[p][0]["Do Not Project"][j].as<string>());
        parallelTvs.insert( tvN );
      }
    }
 
    // (4)
    // Applying heuristic for removing expensive iterators
    int numConstToRemove = str2int(data[p][0]["Remove Constraints"].as<string>());
    rel->RemoveExpensiveConsts(parallelTvs, numConstToRemove );

    // (5)
    // Add user defined constraints
    Relation *rel_extend;
    for (size_t j = 0; j < data[p][0]["User Defined"].size(); ++j){
 
      rel_extend = rel->addUFConstraints(
                    data[p][0]["User Defined"][j]["Func1"].as<string>(),
                    data[p][0]["User Defined"][j]["operator"].as<string>(),
                    data[p][0]["User Defined"][j]["Func2"].as<string>()
                                            );
      *rel = *rel_extend;
      delete rel_extend;

    }

    // (6)
    // Simplifyng the constraints relation
    Relation* rel_sim = rel->simplifyForPartialParallel(parallelTvs);

    // (7)
    // Print out results: if not satisfiable returns NULL
    char buffer [50];
    sprintf (buffer, "Relation #%d.%d simplified = ", int(p)+1, int(i)+1);
    // printRelation(string(buffer), rel_sim);

    // Verify the results 
    if( ex_rel != NULL && rel_sim != NULL) {
      EXPECT_EQ(ex_rel->toISLString(), rel_sim->toISLString());
    } else {
      EXPECT_EQ(ex_rel, rel_sim);
    }

    delete rel;   
    delete rel_sim;
  }
  } // End of p loop
}

/*! Dependence simplification for GS.
*/
TEST_F(ChillUsageTest, GS_CSR_DepSimplification)
{

/* Following is the Gauss-Seidel code. Dependence analysis of this code
   would identify 1 pair of read/write data accesses (in S1) that may
   produce data dependences. This pair produces two distinct conjunctions 
   considering the ordering of accesses (Flow and Anti dependence). Overall, 
   there are 2 distinct conjunctions for the complete dependence relation.
   
   We will apply simplififcation algorithm to these 2 conjunctions.

for (i=0; i < N; i++) {
    for (j=rowptr[i];j<rowptr[i + 1];j++) {
S1:     y[i] -= values[j]*y[colidx[j]];
    }
}

Source of data accesses:

1) a read in S1 (y[colidx[j]]);  a write in S1 (y[i]); 

*/

  simplifiy(string("../gs_csr.json"));

}

/*! Test cases CSR ILU code's data access dependencies
*/
TEST_F(ChillUsageTest, ILU_CSR_DepSimplification)
{

/* Following is the ILU0 code. Dependence analysis of this code
   would identify 8 pairs of read/write or write/write(in or between S1 & S2)
   data accesses that may be data dependences. Each pair produces two 
   distinct conjunctions considering the ordering of accesses. Overall there 
   are 16 distinct conjunctions for the complete dependence relation.
   
   We will apply simplififcation algorithm to all of these 16 conjunctions.

for(int i=0; i < n; i++)
{
  for(int k= row[i]; k < diag[i]; k++)
  {
S1: v[k] = v[k] / v[diag[col[k]]]; 
    tmp = v[k];

    int j1 = k + 1, j2 = diag[col[k]] + 1;

    while (j1 < row[i + 1] && j2 < row[col[k] + 1])
    {
      if (col[j1] < col[j2]) ++j1;
      else if (col[j2] < col[j1]) ++j2;
      else {
S2:     v[j1] -= tmp*v[j2]; 
        ++j1; ++j2;
      }
    }
  }
}

Source of data accesses:

1) a read in S1 (v[k]);  a write in S1 (v[k]); 
2) a read in S1 (v[diag[col[k]]]);  a write in S1 (v[k]);
3) a write in S1 (v[k]);  a write in S2 (v[j1]);
4) a write in S1 (v[k]);  a read in S2 (v[j2]);
5) a read in S1 (v[diag[col[k]]]);  a write in S2 (v[j1]);
6) a write in S2 (v[j1]);  a write in S2 (v[j1]);
7) a read in S1 (v[k]);  a write in S2 (v[j1]);
8) a read in S2 (v[j2]);  a write in S2 (v[j1]);

*/

  simplifiy(string("../ilu_csr.json"));

}

// Helper functions
bool printRelation(string msg, Relation *rel){

    if ( rel ) {

        cout<<"\n\n"<<msg<<rel->toISLString()<<"\n\n";
    } else {

        cout<<"\n\n"<<msg<<"Not Satisfiable"<<"\n\n";
    }

    return true;
}
int str2int(string str){
  int i;
  sscanf (str.c_str(),"%d",&i);
  return i;
}
