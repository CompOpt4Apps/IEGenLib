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

int main(int ac, char **av)
{
//  iegenlib::Set* set = new iegenlib::Set("{[a]: a=10}");
  Set* set = new Set("{[a]:0<a}");

  std::cout << set->prettyPrintString() << std::endl;

  return 0;
}
