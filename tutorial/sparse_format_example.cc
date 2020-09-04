/*!
 * \file sparse_format_example.cc
 *
 * \brief This file contains sparse format examples for IEGenLib C++ API.
 *
 * \date Date Started: 8/29/20
 *
 * \authors Floriana Ciaglia, Anna Rift, Shivani Singh
 *
 * Copyright (c) 2011-2013, Colorado State University <br>
 * Copyright (c) 2015, University of Arizona <br>
 * All rights reserved. <br>
 * See COPYING for details. <br>
 */

#include <iostream>
#include <iegenlib.h>

using iegenlib::Set;
using iegenlib::Relation;

int main(int ac, char **av)
{
   // Test COO WRT DENSE
   std::cout << "\n=> Starting example COO_WRT_DENSE\n\n";

   Relation *r_coo = new Relation("{[n] -> [i,j] : row(n) = i && col(n) = j "
                             "&& 0 <= n && n < NNZ && Dense(i,j) > 0}");
   std::cout << "COO relation:\n" << r_coo->prettyPrintString()<<"\n";

   delete r_coo; 

  // Test CSR WRT DENSE

   std::cout << "\n=> Starting example CSR_WRT_DENSE\n\n";
   Relation *r_csr = new Relation("{[n] -> [i,j] :n >= rptr(i) "
                             "&& n < rptr(i+1) && col(n) = j "
                             "&& 0 <= n && n < NNZ && Dense(i,j) > 0}");
   std::cout << "CSR relation:\n" << r_csr->prettyPrintString()<<"\n";
   delete r_csr;

  return 0;
}
