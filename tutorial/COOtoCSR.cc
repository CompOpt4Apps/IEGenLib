/*!
 * \file sparse_format_example.cc
 *
 * \brief This file contains sparse format examples for IEGenLib C++ API.
 *
 * \date Date Started: 10/6/2021
 *
 * \authors Catherine Olschanowsky
 *
 * Copyright (c) 2011-2013, Colorado State University <br>
 * Copyright (c) 2015, University of Arizona <br>
 * All rights reserved. <br>
 * See COPYING for details. <br>
 */

#include <iegenlib.h>
#include <iostream>

using iegenlib::Relation;
using iegenlib::Set;

int main(int ac, char **av) {

    // Set up our input relations
    Set * dns = new Set(
     "{[i,j] : 0<=i<NR and 0<=j<NC}");
    Relation * coo2dns = new Relation(
     "{[n]->[i,j]:row1(n)=i and col1(n) =j and 0<=n<NNZ}");
    Relation * dns2csr = new Relation(
     "{[i,j] ->[k]:rowptr(i)<=k<rowptr(i+ 1) and col2(k)=j and P(i,j)=k}");

    Relation * coo2csr = dns2csr->Compose(coo2dns);

    std::cout<< coo2csr->TransitiveClosure()->prettyPrintString() << "\n"; 

    // Step 1: Generate Computation
    /*Computation* coo2csrComp = new Computation();
    vector < pair<string, string> > dataReads;
    vector < pair<string, string> > dataWrites;*/

    // Step 2: Set up the parameters. Each of the UF that are in the
    // coo2dns relation are known and will come in as paramters.
    // Also, the dns set has NR and NC - parameters.
    // This will create the data macros that can be used by the 
    // statements we create below.


    // Step 3: The first statement we are adding will be to populate
    // P. 
    // Get the constraint involving P, there should only be one.
    // Get the tuple - generated code will use std::make_tuple, push_back
    // on a vector, and then std::sort. The tupe of this dataspace will be
    // std::vector<tuple> The declaration should not be created yet
/*    std::string insertP = "P.push_back(std::make_tuple(";
    insertP.append(// get the parameters to P)
    
    Stmt* Pstmt = new Stmt("P.push_back((row1(n),col1(n)));std::sort(p.begin(),P.end(),comparator",
                "{[k] : 0 <= k <= -2 + 2n}",
                "{[k]->[0, k, 0]}",
                 dataReads,
                 dataWrites
                 );  */

    // Step 4: This will be done in a loop over the constraints. We need
    // to keep track of which constraints we have attempted and make sure 
    // to stop if we keep trying the same ones and they do not work
    // LOOP TO DETECT COMPLETION OR STUCKNESS
       // LOOP OVER CONSTRAINTS
          // if all UFs are known/complete continue
          // if unknown UF count > 1 continue
          // if unknown UF count == 1
             // Step 4a: Create Statement (3 options)
             // Step 4b: Create Iteration to Index Mapping
             // Step 4c: Create Iteration Space
             // if(iteration space !ONTO domain(UF)
                // create negated statement
                // create iteration to Index Mapping
                // create iteration space, project out tuple variable
                // that prevents ONTO
                // schedule this statement 
                // add negated statement to computation
             // Step 4d: Add statement to Computation
             // Mark Constraint as known


    // Computation for Iterator should be complete
    // Will need to allocate dataspaces for codegen
    return 0;
}
