/*!
 * \file simplifyForPartialParallelDriver.cc
 *
 * This file is a driver for using simplifyForPartialParallel 
 * and pertaining functions in IEgenLib.
 * These functions can be used to carry out simplification of 
 * data dependence relation in non-affine relations containing
 * uninterpreted function calls.
 *
 * To compile, after building IEgenLib with:

       ./configure    
       make

    Run the following command with YOUR OWN ADDRESSES:

        g++ -o EXECUTABLE simplifyDriver.cc 
            -I IEGENLIB_HOME/src IEGENLIB_HOME/build/src/libiegenlib.a -lisl

    You can also run following command after running "make install" for IEgenLIB:

       g++ -o EXECUTABLE simplifyDriver.cc 
           -I INSTALLATION_FOLDER/include/iegenlib cpp_api_example.cc
              INSTALLATION_FOLDER/lib/libiegenlib.a -lisl
   
    IEGENLIB_HOME indicates where you have your copy of IEgenLIB.
    For example if you are compiling this file in its original location that is 
    IEGENLIB_HOME, then you can run following to compile:

    g++ -o simplifyDriver simplifyDriver.cc -I src build/src/libiegenlib.a -lisl
 *     
 *
 * \date Date Started: 3/21/2016
 *
 * \authors Michelle Strout, Mahdi Soltan Mohammadi
 *
 * Copyright (c) 2016, University of Arizona <br>
 * All rights reserved. <br>
 * See COPYING for details. <br>
 * 
 *
 * HOW TO USE INTERFACAE TO SIMPLIFICATION ALGORITHM for
 * simplifying constraints relation containing Uninterpreted Function Calls:
 * 
 * (1) You need to define uninterpreted function calls (UFCs) that appear
 *        in constraints for iegenlib enviroment. The example in the main
 *        function shows how to do this. Note that, you need to do this
 *        only once for relations that have same UFCs during one run
 *
 * (2) You need to put constraints in iegenlib Relation (or Set).
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
 * (5) You can add user defined constraints to the relation as demonstrated
 *     in examples. 
 * 
 * (6) Use simplifyForPartialParallel function that is main interface for the
 *     algorithm. If relation is not satisfiable the function would return NULL.
 *     Otherwise it would return the simplified result as iegenlib Relation.
 * 
 * (7) Print out result (if not NULL) using toISLString() function.  
 
 *  We have demonstrated these steps with two example codes: ILU0 and Gauss-Seidel
 */


#include <iostream>
#include "iegenlib.h"
using iegenlib::Set;
using iegenlib::Relation;
using namespace std;


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//! If you wish not to see the output, change the value: verbose = false;
bool verbose=true;

// Examples
void Gauss_Seidel_Example();
void ILU0_Example();

// Utility function
bool printRelation(string msg, Relation *rel);
void EXPECT_EQ(string a, string b);
void EXPECT_EQ(Relation *a, Relation *b);

//----------------------- MAIN ---------------
int main(int argc, char **argv)
{

    Gauss_Seidel_Example();

    ILU0_Example();

    return 0;
}



void Gauss_Seidel_Example(){


    verbose && cout<<"\n\n Gauss-Seidel Example: \n\n";

/* Following is the Gauss-Seidel code. Dependence analysis of this code
   would identify 1 pair of read/write (inbetween S1)
   data accesses that may be a data dependence. This pair produces two 
   distinct conjunctions considering the ordering of accesses. Overall there 
   are 2 distinct conjunctions for the complete dependence relation.
   
   We will apply simplififcation algorithm to these 2 conjunctions.

for (i=0; i < N; i++) {
    for (j=rowptr[i];j<rowptr[i + 1];j++) {
S1:     y[i] -= values[j]*y[colidx[j]];
    }
}
*/

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
        new Set("{[j]:0<=j &&j<nnz}"), false, iegenlib::Monotonic_NONE);


    // (2)
    // Putting constraints in an iegenlib::Relation
    // Flow dependence checking for: read in S1 (y[colidx[j]]); write in S1 (y[i]);

    Relation* flow = new Relation("{ [i,j] -> [ip,jp] : i<ip && i=colidx(jp) "
                     "&& 0 <= i && i < m && 0 <= ip && ip < m "
                        "&& rowptr(i) <= j && j < rowptr(i+1) "
                   "&& rowptr(ip) <= jp && jp < rowptr(ip+1) }");

    // Expected outputs 
    Relation* ex_flow = new Relation("{ [i] -> [ip,jp] : i=colidx(jp) "
                              "&& colidx(jp) < ip && ip < m-1 "
                                  "&& rowptr(i) < rowptr(i+1) "
                    "&& rowptr(ip) <= jp && jp < rowptr(ip+1) "
     "&& colidx(jp) >= 0 && rowptr(i) >= 0 && rowptr(ip) >= 0 "
                  "&& rowptr(i+1) < nnz && rowptr(ip+1) < nnz}");


    // (3)
    // Specify loops that are going to be parallelized, so we are not going to
    // project them out.
    std::set<int> parallelTvs;
    parallelTvs.insert(0);
    parallelTvs.insert(2);

    // (4)
    // Applying heuristic for removing expensive iterators
    flow->RemoveExpensiveConsts(parallelTvs, 0);

    // (5)
    // Add user defined constraints

    // (6)
    // Simplifyng the constraints relation
    Relation* flow_sim = flow->simplifyForPartialParallel(parallelTvs);

    // (7)
    // Print out results: if not satisfiable returns NULL
    verbose && printRelation(string("Flow simplified = "), flow_sim);

    // Verify the results 
    if ( ex_flow != NULL && flow_sim != NULL) {
        EXPECT_EQ(ex_flow->toISLString(), flow_sim->toISLString());
    } else {
        EXPECT_EQ(ex_flow, flow_sim);
    }


    // (2)
    // Putting constraints in an iegenlib::Relation
    // Anti dependence checking for: read in S1 (y[colidx[j]]); write in S1 (y[i]);

    Relation* anti = new Relation("{ [i,j] -> [ip,jp] : ip<i && i=colidx(jp) "
                     "&& 0 <= i && i < m && 0 <= ip && ip < m "
                        "&& rowptr(i) <= j && j < rowptr(i+1) "
                   "&& rowptr(ip) <= jp && jp < rowptr(ip+1) }");


    Relation* ex_anti = new Relation("{ [i] -> [ip,jp] : i=colidx(jp) "
                               "&& 0 <= ip && ip < colidx(jp) "
                                  "&& rowptr(i) < rowptr(i+1) "
                    "&& rowptr(ip) <= jp && jp < rowptr(ip+1) "
     "&& colidx(jp) < m-1 && rowptr(i) >= 0 && rowptr(ip) >= 0 "
                  "&& rowptr(i+1) < nnz && rowptr(ip+1) < nnz}");


    // (4)
    // Applying heuristic for removing expensive iterators
    flow->RemoveExpensiveConsts(parallelTvs, 0);

    // (5)
    // Add user defined constraints

    // (6)
    // Simplifyng the constraints relation
    Relation* anti_sim = anti->simplifyForPartialParallel(parallelTvs);

    // (7)
    // Print out results: if not satisfiable returns NULL
    verbose && printRelation(string("Anti simplified = "), anti_sim);

    // Verify the results 
    if ( ex_anti != NULL && anti_sim != NULL) {
        EXPECT_EQ(ex_anti->toISLString(), anti_sim->toISLString());
    } else {
        EXPECT_EQ(ex_anti, anti_sim);
    }


    delete ex_flow;
    delete ex_anti;
    delete flow;
    delete anti;
    delete flow_sim;
    delete anti_sim;
}

void ILU0_Example(){


    verbose && cout<<"\n\n ILU0 Example: \n\n";

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
*/

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


/* The first pair of possible data access dependences is comming from:
   a read in S1 (v[k]);  a write in S1 (v[k]);
   A flow (F1) or anti (A1) dependence can exist for these access pairs.

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
*/

    // (2)
    // Putting constraints in an iegenlib::Relation
    // Anti dependence checking for: read in S1 (v[k]); write in S1 (v[k]);

    Relation *A1 = new Relation("[m] -> {[i,k,j1,j2] -> [ip,kp,j1p,j2p]: ip < i"
                                   " && 0 <= i && i < m1"
                                  " && 0 <= ip && ip < m"
                           " && rowptr(i) <= k && k < diagptr(i)"
                         " && rowptr(ip) <= kp && kp < diagptr(ip)"
                                   " && k < j1 && j1 < rowptr(1+i)"
                                 " && kp < j1p && j1p < rowptr(1+ip)"
                  " && diagptr(colidx(k)) < j2 && j2 < rowptr(1+colidx(k))"
                " && diagptr(colidx(kp)) < j2p && j2p < rowptr(1+colidx(kp))"
                             " && colidx(j1) = colidx(j2)"
                             " && colidx(j1p) = colidx(j2p)"

                                     " && k = kp }");

    // expected output  (for testing purposes)
    Relation *ex_A1 = NULL;

    // (3) 
    // Specify loops that are going to be parallelized, so we are not going to
    // project them out. Here "i" and "ip"
    std::set<int> parallelTvs;
    parallelTvs.insert(0);
    parallelTvs.insert(4);

    // (4)
    // Applying heuristic for removing expensive iterators
    A1->RemoveExpensiveConsts(parallelTvs, 2);

    // (5)
    // How to add user defined constraint
    Relation *A1_extend = A1->addUFConstraints("rowptr","<=", "diagptr");


    // (6)
    // Simplifyng the constraints relation
    Relation *A1_sim = A1_extend->simplifyForPartialParallel(parallelTvs);


    // (7)
    // Print out results: if not satisfiable returns NULL
    verbose && printRelation(string("A1 simplified = "), A1_sim);

    // Verify the results 
    if ( ex_A1 != NULL && A1_sim != NULL) {
        EXPECT_EQ(ex_A1->toISLString(), A1_sim->toISLString());
    } else {
        EXPECT_EQ(ex_A1, A1_sim);
    }


    // (2)
    // Putting constraints in an iegenlib::Relation
    // Flow dependence checking for: read in S1 (v[k]); write in S1 (v[k]);

    Relation *F1 = new Relation("[m] -> {[i,k,j1,j2] -> [ip,kp,j1p,j2p]: i < ip"
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

                                     " && k = kp}");

    Relation *ex_F1 = NULL;


    // (4)
    // Applying heuristic for removing expensive iterators
    F1->RemoveExpensiveConsts(parallelTvs, 2);

    // (5)
    // Adding user defined constraint
    Relation *F1_extend = F1->addUFConstraints("rowptr","<=", "diagptr");

    // (6)
    // Simplifyng the constraints relation
    Relation *F1_sim = F1_extend->simplifyForPartialParallel(parallelTvs);

    // (7)
    // Print out results
    verbose && printRelation(string("F1 simplified = "), F1_sim);

    // Verify the results 
    if ( ex_F1 != NULL && F1_sim != NULL) {
        EXPECT_EQ(ex_F1->toISLString(), F1_sim->toISLString());
    } else {
        EXPECT_EQ(ex_F1, F1_sim);
    }

    delete A1;
    delete F1;
    delete A1_extend;
    delete A1_sim;
    delete F1_extend;
    delete F1_sim;

    // ----------------------------


/* The second pair of possible data access dependences is comming from:
   a read in S1 (v[diag[col[k]]]);  a write in S1 (v[k]);
   A flow (F2) or anti (A2) dependence can exist for these access pairs.

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
*/

    Relation *A2 = new Relation("[m] -> {[i,k,j1,j2] -> [ip,kp,j1p,j2p]: ip < i"
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

                                     " && k = diagptr(colidx(kp)) }");

    Relation *ex_A2 = NULL;

    Relation *F2 = new Relation("[m] -> {[i,k,j1,j2] -> [ip,kp,j1p,j2p]: i < ip"
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

                                     " && k = diagptr(colidx(kp))}");

    Relation *ex_F2 = NULL;


    //--- Simplifying Anti dependence

    // (4)
    // Applying heuristic for removing expensive iterators
    A2->RemoveExpensiveConsts(parallelTvs, 2);

    // Adding user defined constraint
    Relation *A2_extend = A2->addUFConstraints("rowptr","<=", "diagptr");

    // Simplifyng the constraints relation
    Relation *A2_sim = A2_extend->simplifyForPartialParallel(parallelTvs);

    // (7)
    // Print out results
    verbose && printRelation(string("A2 simplified = "), A2_sim);

    // Verify the results 
    if ( ex_A2 != NULL && A2_sim != NULL) {
        EXPECT_EQ(ex_A2->toISLString(), A2_sim->toISLString());
    } else {
        EXPECT_EQ(ex_A2, A2_sim);
    }


    //--- Simplifying flow dependence

    // (4)
    // Applying heuristic for removing expensive iterators
    F2->RemoveExpensiveConsts(parallelTvs, 2);

    // Adding user defined constraint
    Relation *F2_extend = F2->addUFConstraints("rowptr","<=", "diagptr");

    // Simplifyng the constraints relation
    Relation *F2_sim = F2_extend->simplifyForPartialParallel(parallelTvs);

    // (7)
    // Print out results
    verbose && printRelation(string("F2 simplified = "), F2_sim);

    // Verify the results 
    if ( ex_F2 != NULL && F2_sim != NULL) {
        EXPECT_EQ(ex_F2->toISLString(), F2_sim->toISLString());
    } else {
        EXPECT_EQ(ex_F2, F2_sim);
    }

    delete A2;
    delete F2;
    delete A2_extend;
    delete A2_sim;
    delete F2_extend;
    delete F2_sim;

    // ----------------------------


/* The third pair of possible data access dependences is comming from:
   a write in S1 (v[k]);  a write in S2 (v[j1]);
   One of two possible output dependences can exist: 
       F3 (write_S1 before write_S2) or A3 (write_S2 before write_S1)

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
*/

    Relation *A3 = new Relation("[m] -> {[i,k,j1,j2] -> [ip,kp,j1p,j2p]: ip < i"
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

                                     " && k = j1p}");

    Relation *ex_A3 = NULL;

    Relation *F3 = new Relation("[m] -> {[i,k,j1,j2] -> [ip,kp,j1p,j2p]: i < ip"
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

                                     " && k = j1p}");

    Relation *ex_F3 = NULL;

    //--- Simplifying Anti dependence

    // (4)
    // Applying heuristic for removing expensive iterators
    A3->RemoveExpensiveConsts(parallelTvs, 2);

    // Adding user defined constraint
    Relation *A3_extend = A3->addUFConstraints("rowptr","<=", "diagptr");

    // Simplifyng the constraints relation
    Relation *A3_sim = A3_extend->simplifyForPartialParallel(parallelTvs);

    // (7)
    // Print out results
    verbose && printRelation(string("A3 simplified = "), A3_sim);

    // Verify the results 
    if ( ex_A3 != NULL && A3_sim != NULL) {
        EXPECT_EQ(ex_A3->toISLString(), A3_sim->toISLString());
    } else {
        EXPECT_EQ(ex_A3, A3_sim);
    }


    //--- Simplifying flow dependence

    // (4)
    // Applying heuristic for removing expensive iterators
    F3->RemoveExpensiveConsts(parallelTvs, 2);

    // Adding user defined constraint
    Relation *F3_extend = F3->addUFConstraints("rowptr","<=", "diagptr");

    // Simplifyng the constraints relation
    Relation *F3_sim = F3_extend->simplifyForPartialParallel(parallelTvs);

    // (7)
    // Print out results
    verbose && printRelation(string("F3 simplified = "), F3_sim);

    // Verify the results 
    if ( ex_F3 != NULL && F3_sim != NULL) {
        EXPECT_EQ(ex_F3->toISLString(), F3_sim->toISLString());
    } else {
        EXPECT_EQ(ex_F3, F3_sim);
    }


    delete A3;
    delete F3;
    delete A3_extend;
    delete A3_sim;
    delete F3_extend;
    delete F3_sim;

    // ----------------------------

/* The fourth pair of possible data access dependences is comming from:
   a write in S1 (v[k]);  a read in S2 (v[j2]);
   A flow (F4) or anti (A4) dependence can exist for these access pairs.

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
*/

    Relation *A4 = new Relation("[m] -> {[i,k,j1,j2] -> [ip,kp,j1p,j2p]: ip < i"
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

                                     " && k = j2p}");

    Relation *ex_A4 = NULL;

    Relation *F4 = new Relation("[m] -> {[i,k,j1,j2] -> [ip,kp,j1p,j2p]: i < ip"
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

                                     " && k = j2p}");

    Relation *ex_F4 = NULL;


    //--- Simplifying Anit dependence

    // (4)
    // Applying heuristic for removing expensive iterators
    A4->RemoveExpensiveConsts(parallelTvs, 2);

    // Adding user defined constraint
    Relation *A4_extend = A4->addUFConstraints("rowptr","<=", "diagptr");

    // Simplifyng the constraints relation
    Relation *A4_sim = A4_extend->simplifyForPartialParallel(parallelTvs);

    // (7)
    // Print out results
    verbose && printRelation(string("A4 simplified = "), A4_sim);

    // Verify the results 
    if ( ex_A4 != NULL && A4_sim != NULL) {
        EXPECT_EQ(ex_A4->toISLString(), A4_sim->toISLString());
    } else {
        EXPECT_EQ(ex_A4, A4_sim);
    }


    //--- Simplifying Flow dependence

    // (4)
    // Applying heuristic for removing expensive iterators
    F4->RemoveExpensiveConsts(parallelTvs, 2);

    // Adding user defined constraint
    Relation *F4_extend = F4->addUFConstraints("rowptr","<=", "diagptr");

    // Simplifyng the constraints relation
    Relation *F4_sim = F4_extend->simplifyForPartialParallel(parallelTvs);

    // (7)
    // Print out results
    verbose && printRelation(string("F4 simplified = "), F4_sim);

    // Verify the results 
    if ( ex_F4 != NULL && F4_sim != NULL) {
        EXPECT_EQ(ex_F4->toISLString(), F4_sim->toISLString());
    } else {
        EXPECT_EQ(ex_F4, F4_sim);
    }

    delete A4;
    delete F4;
    delete A4_extend;
    delete A4_sim;
    delete F4_extend;
    delete F4_sim;

    // ----------------------------

/* The fifth pair of possible data access dependences is comming from:
   a read in S1 (v[diag[col[k]]]);  a write in S2 (v[j1]);
   A flow (F5) or anti (A5) dependence can exist for these access pairs.

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
*/

    Relation *A5 = new Relation("[m] -> {[i,k,j1,j2] -> [ip,kp,j1p,j2p]: ip < i"
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

                                     " && j1 = diagptr(colidx(kp)) }");

    Relation *ex_A5 = new Relation("[ m, nnz ] -> { [i, k] -> [ip, kp] : i = colidx(kp) &&"
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


    Relation *F5 = new Relation("[m] -> {[i,k,j1,j2] -> [ip,kp,j1p,j2p]: i < ip"
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

                                     " && j1 = diagptr(colidx(k))}");

    Relation *ex_F5 = new Relation("[ m, nnz ] -> { [i, k] -> [ip, kp] : i = colidx(k) &&"
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


    //--- Simplifying Anti dependence

    // (4)
    // Applying heuristic for removing expensive iterators
    A5->RemoveExpensiveConsts(parallelTvs, 2);

    // Adding user defined constraint
    Relation *A5_extend = A5->addUFConstraints("rowptr","<=", "diagptr");

    // Simplifyng the constraints relation
    Relation *A5_sim = A5_extend->simplifyForPartialParallel(parallelTvs);

    // (7)
    // Print out results
    verbose && printRelation(string("A5 simplified = "), A5_sim);

    // Verify the results 
    if ( ex_A5 != NULL && A5_sim != NULL) {
//        EXPECT_EQ(ex_A5->toISLString(), A5_sim->toISLString());
    } else {
//        EXPECT_EQ(ex_A5, A5_sim);
    }


    //--- Simplifying FLow dependence

    // (4)
    // Applying heuristic for removing expensive iterators
    F5->RemoveExpensiveConsts(parallelTvs, 2);

    // Adding user defined constraint
    Relation *F5_extend = F5->addUFConstraints("rowptr","<=", "diagptr");

    // Simplifyng the constraints relation
    Relation *F5_sim = F5_extend->simplifyForPartialParallel(parallelTvs);

    // (7)
    // Print out results
    verbose && printRelation(string("F5 simplified = "), F5_sim);

    // Verify the results 
    if ( ex_F5 != NULL && F5_sim != NULL) {
//        EXPECT_EQ(ex_F5->toISLString(), F5_sim->toISLString());
    } else {
//        EXPECT_EQ(ex_F5, F5_sim);
    }

    delete A5;
    delete F5;
    delete A5_extend;
    delete A5_sim;
    delete F5_extend;
    delete F5_sim;

    // ----------------------------

/* The sixth pair of possible data access dependences is comming from:
   a write in S2 (v[j1]);  a write in S2 (v[j1]);
   Depending on ordering one of two possible output dependences can exists.

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
*/

    Relation *A6 = new Relation("[m] -> {[i,k,j1,j2] -> [ip,kp,j1p,j2p]: ip < i"
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

                                     " && j1 = j1p}");


    Relation *ex_A6 = NULL;

    Relation *F6 = new Relation("[m] -> {[i,k,j1,j2] -> [ip,kp,j1p,j2p]: i < ip"
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

                                     " && j1 = j1p}");

    Relation *ex_F6 = NULL;


    //--- Simplifying Anti dependence

    // (4)
    // Applying heuristic for removing expensive iterators
    A6->RemoveExpensiveConsts(parallelTvs, 2);

    // Adding user defined constraint
    Relation *A6_extend = A6->addUFConstraints("rowptr","<=", "diagptr");

    // Simplifyng the constraints relation
    Relation *A6_sim = A6_extend->simplifyForPartialParallel(parallelTvs);

    // (7)
    // Print out results
    verbose && printRelation(string("A6 simplified = "), A6_sim);

    // Verify the results 
    if ( ex_A6 != NULL && A6_sim != NULL) {
        EXPECT_EQ(ex_A6->toISLString(), A6_sim->toISLString());
    } else {
        EXPECT_EQ(ex_A6, A6_sim);
    }


    //--- Simplifying Flow dependence

    // (4)
    // Applying heuristic for removing expensive iterators
    F6->RemoveExpensiveConsts(parallelTvs, 2);

    // Adding user defined constraint
    Relation *F6_extend = F6->addUFConstraints("rowptr","<=", "diagptr");

    // Simplifyng the constraints relation
    Relation *F6_sim = F6_extend->simplifyForPartialParallel(parallelTvs);

    // (7)
    // Print out results
    verbose && printRelation(string("F6 simplified = "), F6_sim);

    // Verify the results 
    if ( ex_F6 != NULL && F6_sim != NULL) {
        EXPECT_EQ(ex_F6->toISLString(), F6_sim->toISLString());
    } else {
        EXPECT_EQ(ex_F6, F6_sim);
    }


    delete A6;
    delete F6;
    delete A6_extend;
    delete A6_sim;
    delete F6_extend;
    delete F6_sim;

    // ----------------------------

/* The seventh pair of possible data access dependences is comming from:
   a read in S1 (v[k]);  a write in S2 (v[j1]);
   A flow (F7) or anti (A7) dependence can exist for these access pairs.

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
*/

    Relation *A7 = new Relation("[m] -> {[i,k,j1,j2] -> [ip,kp,j1p,j2p]: ip < i"
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

                                     " && j1 = kp}");

    Relation *ex_A7 = NULL;

    Relation *F7 = new Relation("[m] -> {[i,k,j1,j2] -> [ip,kp,j1p,j2p]: i < ip"
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

                                     " && j1 = kp}");

    Relation *ex_F7 = NULL;

    //--- Simplifying Anti dependence

    // (4)
    // Applying heuristic for removing expensive iterators
    A7->RemoveExpensiveConsts(parallelTvs, 2);

    // (5)
    // Adding user defined constraint
    Relation *A7_extend = A7->addUFConstraints("rowptr","<=", "diagptr");

    // (6)
    // Simplifyng the constraints relation
    Relation *A7_sim = A7_extend->simplifyForPartialParallel(parallelTvs);

    // (7)
    // Print out results
    verbose && printRelation(string("A7 simplified = "), A7_sim);

    // Verify the results 
    if ( ex_A7 != NULL && A7_sim != NULL) {
        EXPECT_EQ(ex_A7->toISLString(), A7_sim->toISLString());
    } else {
        EXPECT_EQ(ex_A7, A7_sim);
    }


    //--- Simplifying flow dependence

    // (4)
    // Applying heuristic for removing expensive iterators
    F7->RemoveExpensiveConsts(parallelTvs, 2);

    // (5)
    // Adding user defined constraint
    Relation *F7_extend = F7->addUFConstraints("rowptr","<=", "diagptr");

    // (6)
    // Simplifyng the constraints relation
    Relation *F7_sim = F7_extend->simplifyForPartialParallel(parallelTvs);

    // (7)
    // Print out results
    verbose && printRelation(string("F7 simplified = "), F7_sim);

    // Verify the results 
    if ( ex_F7 != NULL && F7_sim != NULL) {
        EXPECT_EQ(ex_F7->toISLString(), F7_sim->toISLString());
    } else {
        EXPECT_EQ(ex_F7, F7_sim);
    }

    delete A7;
    delete F7;
    delete A7_extend;
    delete A7_sim;
    delete F7_extend;
    delete F7_sim;

    // ----------------------------

/* The eighth pair of possible data access dependences is comming from:
   a read in S2 (v[j2]);  a write in S2 (v[j1]);
   A flow (F8) or anti (A8) dependence can exist for these access pairs.

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
*/

    Relation *A8 = new Relation("[m] -> {[i,k,j1,j2] -> [ip,kp,j1p,j2p]: ip < i"
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

    Relation *ex_A8 = new Relation("[ m, nnz ] -> { [i, k] -> [ip, kp] : "
                            " i = colidx(kp) &&  colidx(kp)+1 < m &&"
               " rowptr(i) <= k && k <= rowptr(i + 1)-2 && k < diagptr(i) &&"
                                   " 0 <= ip && ip < colidx(kp) &&"
                          " rowptr(ip) <= kp && kp < diagptr(ip) &&"

" colidx(k) >= 0 && rowptr(i) >= 0 && rowptr(ip) >= 0 &&"
" rowptr(colidx(k)) >= 0 && rowptr(colidx(kp)) >= 0 &&"
" nnz >= diagptr(colidx(k) + 1) &&"
" nnz >= diagptr(colidx(kp) + 1) && nnz >= rowptr(colidx(k) + 1) &&"
" nnz >= rowptr(colidx(kp) + 1) && diagptr(i + 1) >= rowptr(i + 1) &&"
" diagptr(ip + 1) >= rowptr(ip + 1) &&"
" diagptr(colidx(k)) >= rowptr(colidx(k)) &&"
" diagptr(colidx(k) + 1) + 1 >= 0 &&"
" diagptr(colidx(kp)) >= rowptr(colidx(kp)) &&"
" diagptr(colidx(kp) + 1) + 1 >= 0 && rowptr(colidx(k) + 1) + 1 >= 0 &&"
" rowptr(colidx(kp) + 1) + 1 >= 0 && rowptr(colidx(kp) + 1) + 2 >= k &&"
" rowptr(ip + 1) >= kp + 2 && m >= colidx(k) + 2 && nnz > diagptr(i) &&"
" nnz > diagptr(i + 1) && nnz > diagptr(ip) && nnz > diagptr(ip + 1) &&"
" nnz > diagptr(colidx(k)) + 1 &&"
" -diagptr(colidx(k)) + rowptr(colidx(k) + 1) + 2 >= 0 &&"
" diagptr(colidx(k) + 1) + 8 >= rowptr(colidx(k) + 1) &&"
" rowptr(i + 1) >= diagptr(colidx(kp)) + 2 &&"
" rowptr(colidx(kp) + 1) + 2 >= diagptr(colidx(kp)) &&"
" diagptr(colidx(kp) + 1) + 8 >=  rowptr(colidx(kp) + 1) }");

    string ex_A8_str = ex_A8->toISLString();

    Relation *F8 = new Relation("[m] -> {[i,k,j1,j2] -> [ip,kp,j1p,j2p]: i < ip"
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

    Relation *ex_F8 = new Relation("[ m, nnz ] -> { [i, k] -> [ip, kp] : "
                                     " i = colidx(kp) &&"
               " rowptr(i) <= k && k <= rowptr(i + 1)-2 && k < diagptr(i) &&"
                             " colidx(kp) < ip && ip < m-1 &&"
                            " rowptr(ip) <= kp && kp < diagptr(ip) &&"

" colidx(k) >= 0 && colidx(kp) >= 0 && rowptr(i) >= 0 && rowptr(ip) >= 0 &&"
" rowptr(colidx(k)) >= 0 && rowptr(colidx(kp)) >= 0 &&"
" nnz >= diagptr(colidx(k) + 1) &&"
" nnz >= diagptr(colidx(kp) + 1) && nnz >= rowptr(colidx(k) + 1) &&"
" nnz >= rowptr(colidx(kp) + 1) && diagptr(i + 1) >= rowptr(i + 1) &&"
" diagptr(ip + 1) >= rowptr(ip + 1) &&"
" diagptr(colidx(k)) >= rowptr(colidx(k)) &&"
" diagptr(colidx(k) + 1) + 1 >= 0 &&"
" diagptr(colidx(kp)) >= rowptr(colidx(kp)) &&"
" diagptr(colidx(kp) + 1) + 1 >= 0 && rowptr(colidx(k) + 1) + 1 >= 0 &&"
" rowptr(colidx(kp) + 1) + 1 >= 0 && rowptr(colidx(kp) + 1) + 2 >= k &&"
" rowptr(ip + 1) >= kp + 2 && m >= colidx(k) + 2 && nnz > diagptr(i) &&"
" nnz > diagptr(i + 1) && nnz > diagptr(ip) && nnz > diagptr(ip + 1) &&"
" nnz > diagptr(colidx(k)) + 1 &&"
" -diagptr(colidx(k)) + rowptr(colidx(k) + 1) + 2 >= 0 &&"
" diagptr(colidx(k) + 1) + 8 >= rowptr(colidx(k) + 1) &&"
" rowptr(i + 1) >= diagptr(colidx(kp)) + 2 &&"
" rowptr(colidx(kp) + 1) + 2 >= diagptr(colidx(kp)) &&"
" diagptr(colidx(kp) + 1) + 8 >=  rowptr(colidx(kp) + 1) }");

    //--- Simplifying anti dependence

    // (4)
    // Applying heuristic for removing expensive iterators
    A8->RemoveExpensiveConsts(parallelTvs, 2);

    // (5)
    // Adding user defined constraint
    Relation *A8_extend = A8->addUFConstraints("rowptr","<=", "diagptr");

    // (6)
    // Simplifyng the constraints relation
    Relation *A8_sim = A8_extend->simplifyForPartialParallel(parallelTvs);

    // (7)
    // Print out results
    verbose && printRelation(string("A8 simplified = "), A8_sim);

    // Verify the results 
    if ( ex_A8 != NULL && A8_sim != NULL) {
        EXPECT_EQ(ex_A8->toISLString(), A8_sim->toISLString());
    } else {
        EXPECT_EQ(ex_A8, A8_sim);
    }

    //--- Simplifying flow dependence

    // (4)
    // Applying heuristic for removing expensive iterators
    F8->RemoveExpensiveConsts(parallelTvs, 2);

    // (5)
    // Adding user defined constraint
    Relation *F8_extend = F8->addUFConstraints("rowptr","<=", "diagptr");

    // (6)
    // Simplifyng the constraints relation
    Relation *F8_sim = F8_extend->simplifyForPartialParallel(parallelTvs);

    // (7)
    // Print out results
    verbose && printRelation(string("F8 simplified = "), F8_sim);

    // Verify the results 
    if ( ex_F8 != NULL && F8_sim != NULL) {
        EXPECT_EQ(ex_F8->toISLString(), F8_sim->toISLString());
    } else {
        EXPECT_EQ(ex_F8, F8_sim);
    }

    delete A8;
    delete F8;
    delete A8_extend;
    delete A8_sim;
    delete F8_extend;
    delete F8_sim;
}


bool printRelation(string msg, Relation *rel){

    if ( rel ) {

        cout<<"\n\n"<<msg<<rel->toISLString()<<"\n\n";
    } else {

        cout<<"\n\n"<<msg<<"Not Satisfiable"<<"\n\n";
    }

    return true;
}

void EXPECT_EQ(string a, string b){

    if( a != b ){

        cout<<"\n\nExpected: "<<a;
        cout<<"\n\nActual:"<< b <<"\n\n";
    }
}

void EXPECT_EQ(Relation *a, Relation *b){

    if( a != b ){
        cout<<"\n\nIncorrect results: Expected or Actual is NULL.\n\n";
    }
}


