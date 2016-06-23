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
            -I IEGENLIB_HOME/src IEGENLIB_HOME/build/src/libiegenlib.a -lisl -std=c++11

    You can also run following command after running "make install" for IEgenLIB:

       g++ -o EXECUTABLE simplifyDriver.cc 
           -I INSTALLATION_FOLDER/include/iegenlib cpp_api_example.cc
              INSTALLATION_FOLDER/lib/libiegenlib.a -lisl -std=c++11
   
    IEGENLIB_HOME indicates where you have your copy of IEgenLIB.
    For example if you are compiling this file in its original location that is 
    IEGENLIB_HOME, then you can run following to compile:

    g++ -o simplifyDriver simplifyDriver.cc -I src build/src/libiegenlib.a -lisl -std=c++11
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
 *        only once for relations that have same UFSs during one run
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
 
 *  We have demonstrated these steps in the simplify function
 *  This function reads information from a JSON file (inputFile), and applies
 *  the simplfication algorithm to the sets found in the file. 
 */


#include <iostream>
#include "iegenlib.h"
#include "parser/jsoncons/json.hpp"

using jsoncons::json;
using iegenlib::Set;
using iegenlib::Relation;
using namespace std;


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//! If you wish not to see the output, change the value: verbose = false;
bool verbose=true;

// Examples
void CSR_Gauss_Seidel_Example();
void CSR_ILU_Example();

// Utility function
bool printRelation(string msg, Relation *rel);
void EXPECT_EQ(string a, string b);
void EXPECT_EQ(Relation *a, Relation *b);
int str2int(string str);

// Reads information from a JSON file (inputFile), and applies
// the simplfication algorithm to the sets found in the file. 
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
    verbose && printRelation(string(buffer), rel_sim);

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

//----------------------- MAIN ---------------
int main(int argc, char **argv)
{

    CSR_Gauss_Seidel_Example();

    CSR_ILU_Example();

    return 0;
}

void CSR_Gauss_Seidel_Example(){

    verbose && cout<<"\n\nGauss-Seidel_CSR: \n\n";

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

  simplifiy(string("gs_csr.json"));

}

void CSR_ILU_Example(){


    verbose && cout<<"\n\nILU_CSR: \n\n";

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

  simplifiy(string("ilu_csr.json"));

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

int str2int(string str){
  int i;
  sscanf (str.c_str(),"%d",&i);
  return i;
}

