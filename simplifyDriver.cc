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
 * simplifying constraints set containing Uninterpreted Function Calls:
 * 
 * (1) You need to define uninterpreted function calls (UFCs) that appear
 *        in constraints for iegenlib enviroment. The example in the main
 *        function shows how to do this. Note that, you need to do this
 *        only once for sets that have same UFCs during one run
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
 
 *  First example shows each step clearly.
 */


#include <iostream>
#include "iegenlib.h"
using iegenlib::Set;
using iegenlib::Relation;
using namespace std;


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//! If you wish not to see the output, change the value: verbose = false;
bool verbose=true;


void EXPECT_EQ(string a, string b);


int main(int argc, char **argv)
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
                             " && colidx(j1) = colidx(j2)"
                             " && colidx(j1p) = colidx(j2p)"

                                     " && k = kp }");

    // expected output  (for testing purposes)
    string ex_A1_str("Not Satisfiable");

    // (3) 
    // Specify loops that are going to be parallelized, so we are not going to
    // project them out. Here "i" and "ip"
    std::set<int> parallelTvs;
    parallelTvs.insert(0);
    parallelTvs.insert(1);


    // (4)
    // Applying heuristic for removing expensive iterators
    A1->RemoveExpensiveConsts(parallelTvs, 2);

    // (5)
    // How to add user defined constraint
    Set* A1_extend = A1->addUFConstraints("rowptr","<=", "diagptr");


    // (6)
    // Simplifyng the constraints set
    Set* A1_sim = A1_extend->simplifyForPartialParallel(parallelTvs);


    // (7)
    // Print out results
    // If set is not satisfiable simplifyForPartialParallel is going to return
    // NULL, we should check this before getting result's string with
    // toISLString. 
    string A1_sim_str("Not Satisfiable");
    if( A1_sim ){
        A1_sim_str = A1_sim->toISLString();
    }
    verbose && cout<<"\n\nA1 simplified = "<<A1_sim_str<<"\n\n";    

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
                             " && colidx(j1) = colidx(j2)"
                             " && colidx(j1p) = colidx(j2p)"

                                     " && k = kp}");

    string ex_F1_str("Not Satisfiable");


    // (4)
    // Applying heuristic for removing expensive iterators
    F1->RemoveExpensiveConsts(parallelTvs, 2);

    // Adding user defined constraint
    Set* F1_extend = F1->addUFConstraints("rowptr","<=", "diagptr");

    // Simplifyng the constraints set
    Set* F1_sim = F1_extend->simplifyForPartialParallel(parallelTvs);

    string F1_sim_str("Not Satisfiable");
    if( F1_sim ){
        F1_sim_str = F1_sim->toISLString();
    }

    verbose && cout<<"\n\nF1 simplified = "<<F1_sim_str<<"\n\n";

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
                             " && colidx(j1) = colidx(j2)"
                             " && colidx(j1p) = colidx(j2p)"

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
                             " && colidx(j1) = colidx(j2)"
                             " && colidx(j1p) = colidx(j2p)"

                                     " && k = diagptr(colidx(kp))}");

    string ex_F2_str("Not Satisfiable");

    // (4)
    // Applying heuristic for removing expensive iterators
    A2->RemoveExpensiveConsts(parallelTvs, 2);

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
    verbose && cout<<"\n\nA2 simplified = "<<A2_sim_str<<"\n\n"; 

    EXPECT_EQ( ex_A2_str , A2_sim_str );


    // (4)
    // Applying heuristic for removing expensive iterators
    F2->RemoveExpensiveConsts(parallelTvs, 2);


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
    verbose && cout<<"\n\nF2 simplified = "<<F2_sim_str<<"\n\n"; 

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
                             " && colidx(j1) = colidx(j2)"
                             " && colidx(j1p) = colidx(j2p)"

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
                             " && colidx(j1) = colidx(j2)"
                             " && colidx(j1p) = colidx(j2p)"

                                     " && k = j1p}");

    string ex_F3_str("");


    // (4)
    // Applying heuristic for removing expensive iterators
    A3->RemoveExpensiveConsts(parallelTvs, 2);


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
    verbose && cout<<"\n\nA3 simplified = "<<A3_sim_str<<"\n\n"; 

    EXPECT_EQ( ex_A3_str , A3_sim_str );


    // (4)
    // Applying heuristic for removing expensive iterators
    F3->RemoveExpensiveConsts(parallelTvs, 2);


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
    verbose && cout<<"\n\nF3 simplified = "<<F3_sim_str<<"\n\n"; 

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
                             " && colidx(j1) = colidx(j2)"
                             " && colidx(j1p) = colidx(j2p)"

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
                             " && colidx(j1) = colidx(j2)"
                             " && colidx(j1p) = colidx(j2p)"

                                     " && k = j2p}");

    string ex_F4_str("Not Satisfiable");


    // (4)
    // Applying heuristic for removing expensive iterators
    A4->RemoveExpensiveConsts(parallelTvs, 2);

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
    verbose && cout<<"\n\nA4 simplified = "<<A4_sim_str<<"\n\n"; 

    EXPECT_EQ( ex_A4_str , A4_sim_str );

    // (4)
    // Applying heuristic for removing expensive iterators
    F4->RemoveExpensiveConsts(parallelTvs, 2);

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
    verbose && cout<<"\n\nF4 simplified = "<<F4_sim_str<<"\n\n"; 
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
                             " && colidx(j1) = colidx(j2)"
                             " && colidx(j1p) = colidx(j2p)"

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
                             " && colidx(j1) = colidx(j2)"
                             " && colidx(j1p) = colidx(j2p)"

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


    // (4)
    // Applying heuristic for removing expensive iterators
    A5->RemoveExpensiveConsts(parallelTvs, 2);


    //--- Simplifying flow dependence
    // Adding user defined constraint
    Set* A5_extend = A5->addUFConstraints("rowptr","<=", "diagptr");

    // Simplifyng the constraints set
    Set* A5_sim = A5_extend->simplifyForPartialParallel(parallelTvs);

    string A5_sim_str("Not Satisfiable");
    if( A5_sim ){
        A5_sim_str = A5_sim->toISLString();
    }

    // (7)
    // Print out results
    verbose && cout<<"\n\nA5 simplified = "<<A5_sim_str<<"\n\n"; 
//    EXPECT_EQ( ex_A5_str , A5_sim_str );


    // (4)
    // Applying heuristic for removing expensive iterators
    F5->RemoveExpensiveConsts(parallelTvs, 2);


    //--- Simplifying Anti dependence
    // Adding user defined constraint
    Set* F5_extend = F5->addUFConstraints("rowptr","<=", "diagptr");

    // Simplifyng the constraints set
    Set* F5_sim = F5_extend->simplifyForPartialParallel(parallelTvs);

    string F5_sim_str("Not Satisfiable");
    if( F5_sim ){
        F5_sim_str = F5_sim->toISLString();
    }
    // (7)
    // Print out results
    verbose && cout<<"\n\nF5 simplified = "<<F5_sim_str<<"\n\n";

//    EXPECT_EQ( ex_F5_str , F5_sim_str );

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
                             " && colidx(j1) = colidx(j2)"
                             " && colidx(j1p) = colidx(j2p)"

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
                             " && colidx(j1) = colidx(j2)"
                             " && colidx(j1p) = colidx(j2p)"

                                     " && j1 = j1p}");

    string ex_F6_str("Not Satisfiable");

    // (4)
    // Applying heuristic for removing expensive iterators
    A6->RemoveExpensiveConsts(parallelTvs, 2);

    //--- Simplifying flow dependence
    // Adding user defined constraint
    Set* A6_extend = A6->addUFConstraints("rowptr","<=", "diagptr");

    // Simplifyng the constraints set
    Set* A6_sim = A6_extend->simplifyForPartialParallel(parallelTvs);

    string A6_sim_str("Not Satisfiable");
    if( A6_sim ){
        A6_sim_str = A6_sim->toISLString();
    }

    // (7)
    // Print out results
    verbose && cout<<"\n\nA6 simplified = "<<A6_sim_str<<"\n\n"; 

    EXPECT_EQ( ex_A6_str , A6_sim_str );


    // (4)
    // Applying heuristic for removing expensive iterators
    F6->RemoveExpensiveConsts(parallelTvs, 2);


    //--- Simplifying Anti dependence
    // Adding user defined constraint
    Set* F6_extend = F6->addUFConstraints("rowptr","<=", "diagptr");

    // Simplifyng the constraints set
    Set* F6_sim = F6_extend->simplifyForPartialParallel(parallelTvs);

    string F6_sim_str("Not Satisfiable");
    if( F6_sim ){
        F6_sim_str = F6_sim->toISLString();
    }

    // (7)
    // Print out results
    verbose && cout<<"\n\nF6 simplified = "<<F6_sim_str<<"\n\n";

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
                             " && colidx(j1) = colidx(j2)"
                             " && colidx(j1p) = colidx(j2p)"

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
                             " && colidx(j1) = colidx(j2)"
                             " && colidx(j1p) = colidx(j2p)"

                                     " && j1 = kp}");

    string ex_F7_str("Not Satisfiable");


    // (4)
    // Applying heuristic for removing expensive iterators
    A7->RemoveExpensiveConsts(parallelTvs, 2);


    //--- Simplifying flow dependence
    // Adding user defined constraint
    Set* A7_extend = A7->addUFConstraints("rowptr","<=", "diagptr");

    // Simplifyng the constraints set
    Set* A7_sim = A7_extend->simplifyForPartialParallel(parallelTvs);

    string A7_sim_str("Not Satisfiable");
    if( A7_sim ){
        A7_sim_str = A7_sim->toISLString();
    }

    // (7)
    // Print out results
    verbose && cout<<"\n\nA7 simplified = "<<A7_sim_str<<"\n\n"; 

    EXPECT_EQ( ex_A7_str , A7_sim_str );


    // (4)
    // Applying heuristic for removing expensive iterators
    F7->RemoveExpensiveConsts(parallelTvs, 2);


    //--- Simplifying Anti dependence
    // Adding user defined constraint
    Set* F7_extend = F7->addUFConstraints("rowptr","<=", "diagptr");

    // Simplifyng the constraints set
    Set* F7_sim = F7_extend->simplifyForPartialParallel(parallelTvs);

    string F7_sim_str("Not Satisfiable");
    if( F7_sim ){
        F7_sim_str = F7_sim->toISLString();
    }

    // (7)
    // Print out results
    verbose && cout<<"\n\nF7 simplified = "<<F7_sim_str<<"\n\n"; 

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
                             " && colidx(j1) = colidx(j2)"
                             " && colidx(j1p) = colidx(j2p)"

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
" && nnz - diagptr(ip + 1) - 1 >= 0 && nnz - diagptr(colidx(k)) - 2 >= 0"
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

    // (4)
    // Applying heuristic for removing expensive iterators
    A8->RemoveExpensiveConsts(parallelTvs, 2);


    //--- Simplifying anti dependence
    // Adding user defined constraint
    Set* A8_extend = A8->addUFConstraints("rowptr","<=", "diagptr");

    // Simplifyng the constraints set
    Set* A8_sim = A8_extend->simplifyForPartialParallel(parallelTvs);

    string A8_sim_str("Not Satisfiable");
    if( A8_sim ){
        A8_sim_str = A8_sim->toISLString();
    }

    // (7)
    // Print out results
    verbose && cout<<"\n\nA8 simplified = "<<A8_sim_str<<"\n\n"; 

    EXPECT_EQ( ex_A8_str , A8_sim_str );


    // (4)
    // Applying heuristic for removing expensive iterators
    F8->RemoveExpensiveConsts(parallelTvs, 2);

    // (5)
    // Adding user defined constraint
    Set* F8_extend = F8->addUFConstraints("rowptr","<=", "diagptr");

    // (6)
    // Simplifyng the constraints set
    Set* F8_sim = F8_extend->simplifyForPartialParallel(parallelTvs);

    string F8_sim_str("Not Satisfiable");
    if( F8_sim ){
        F8_sim_str = F8_sim->toISLString();
    }

    // (7)
    // Print out results
    verbose && cout<<"\n\nF8 simplified = "<<F8_sim_str<<"\n\n"; 

    EXPECT_EQ( ex_F8_str , F8_sim_str );

    delete A8;
    delete F8;
    delete A8_extend;
    delete A8_sim;
    delete F8_extend;
    delete F8_sim;

    return 0;
}


void EXPECT_EQ(string a, string b){

    if( a != b ){

        cout<<"\n\nExpected: "<<a;
        cout<<"\n\nActual:"<< b <<"\n\n";
    }
}



