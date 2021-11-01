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

   // COO to Dense
    Relation * coo2dns = new Relation(
     "{[n]->[i,j]:row1(n)=i and col1(n) =j and 0<=n<NNZ}");

    Relation * dns2csr = new Relation(
     "{[i,j] ->[k]:rowptr(i)<=k<rowptr(i+ 1) and col2(k)=j and P(i,j)=k and 0<=i<NR and 0<=j<NC}");

    Relation * coo2csr = dns2csr->Compose(coo2dns);

    std::cout<< coo2csr->TransitiveClosure()->prettyPrintString() << "\n"; 

    // TODO
    // extract all known UFs
    /*StringIterator* knownUFsIter = coo2dns->getSymbolIterator();
    std::vector<string> knownUFs;
    std::cout << "Known UFS\n";
    while (knownUFsIter->hasNext()) {
       knownUFs.push_back( knownUFsIter->next() );
       std::cout << knownUFs.back() << std::endl;
    }
    // extract all unknown UFs
    StringIterator* unknownUFsIter = dns2csr->getSymbolIterator();
    std::vector<string> knownUFs;
    std::vector<Set*> domains;
    std::cout << "unknown UFS and their dowmains\n";
    while (unknownUFsIter->hasNext()) {
       unknownUFs.push_back( unknownUFsIter->next() );
       domains.push_back(coo2csr->ExtractUFDomain(unknownUFs.back())); 
       std::cout << unknownUFs.back() << std::endl;
    }*/

    
   
   /* // Get all constraints that involve P2
    std::vector<Expression> P2constraints = 
         dns2csr->getConstraintsUsingUF(unknownUFs.at(2));

    // foreach constraint
    // 2. sort into (a. self referential, b. depends on unknown UF, and c. other
    for(auto &constraint : P2constraints){
      if(constraint->usesUFMoreThanOnce()){
        // push it onto insertion constraints
      }else if(constraint->dependsOnUnknownUF()){
        // discard we can't use this right now
      }else{
        // turn into a statment and push onto statement list
      }
    }

    // At this point we should have a statement list
    // The next task is to determine the iteration space associated
    // With those statements
    */

    return 0;
}
