/*!
 * \file cpp_api_example.cc
 *
 * \brief This file is a small example of using the IEGenLib C++ API.  
 *
 * To compile, execute the following command from the root of the project:
 *     cd ..
 *     make install     // or change the iegen/ subdir below to iegen-MACOSX/
 *                      //                                  or iegen-LINUX64/
 *     cd src
 *     g++ -o cpp_api_example -I../iegen/include/iegenlib -L../iegen/lib -liegenlib cpp_api_example.cc
 *
 * \date Date Started: 3/11/11
 *
 * \authors Alan LaMielle, Michelle Strout, Cathie Olschanowsky
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
