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
 *     g++ -o cpp_api_example -I../iegen/include -L../iegen/lib -liegenlib cpp_api_example.cc
 *
 * \date Date Started: 3/11/11
 * # $Revision:: 623                $: last committed revision
 * # $Date:: 2013-01-18 13:12:02 -0#$: date of last committed revision
 * # $Author:: cathie               $: author of last committed revision
 *
 * \authors Alan LaMielle, Michelle Strout
 *
 * Copyright (c) 2011, 2012, Colorado State University <br>
 * All rights reserved. <br>
 * See COPYING for details. <br>
 */

#include <iostream>
#include <iegenlib.h>


int main(int ac, char **av)
{
  iegenlib::Set* set = new iegenlib::Set("{[a]: a=10}");

  std::cout << set->prettyPrintString() << std::endl;

  return 0;
}
