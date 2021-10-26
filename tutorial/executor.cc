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

   // Matrix Multiply for Tuowen's PLDI Submission
   Set * dense = new Set("{[i,k,j]:0<=i<N and 0<=k<N and 0<=j<N}");
   Relation * denseToC = new Relation("{[i,k,j]->[i,j]}"); 
   Relation * denseToB = new Relation("{[i,k,j]->[i,k]}"); 
   Relation * denseToA = new Relation("{[i,k,j]->[k,j]}"); 

   Relation * csrA = new Relation(
       "{[i,j] ->[n]:rowptra(i)<=n<rowptra(i+1) and cola(n)=j and Pa(i,j)=n}");
   Relation * csrB = new Relation(
       "{[i,j] ->[n]:rowptrb(i)<=n<rowptrb(i+1) and colb(n)=j and Pb(i,j)=n}");
  
   Relation * Arel = csrA->Compose(denseToA); 
   Relation * Brel = csrB->Compose(denseToB); 

   std::cout << "Arel: " << Arel->prettyPrintString() << std::endl;
   Relation * result = Arel->IntersectOnInputTuple(denseToC);
   result = Brel->IntersectOnInputTuple(result);
   Set * resultSet = result->ToSet();
   Set * actualResult = result->Apply(dense);
   std::cout<< "This is the new set!! " 
            <<  actualResult->prettyPrintString() << "\n"; 

   // The arity of the relation?? which is actually a set?
   std::vector<int> arity;
   arity.push_back(actualResult->arity());

   // The iteration space
   std::vector<std::string> iterSpaces;
   VisitorChangeUFsForOmega* vOmegaReplacer=new VisitorChangeUFsForOmega();
   actualResult->acceptVisitor(vOmegaReplacer);
   std::string omegaIterString =
             actualResult->toOmegaString(vOmegaReplacer->getUFCallDecls());
   iterSpaces.push_back(omegaIterString);
   std::cout << omegaIterString << std::endl;

  
   // Known Set -- we know nothing 
   Set* knownSet = new Set("{}");
   knownSet->acceptVisitor(vOmegaReplacer);
   std::string known =
       knownSet->toOmegaString(vOmegaReplacer->getUFCallDecls());

   // Finally, codegen
   std::cout << Computation::omegaCodeGenFromString(arity, iterSpaces, known);

   return 0;
}
