/*!
 * \file UFCallMap.h
 *
 * \brief Interface of UFCallMap class
 *
 * \date Started: 2016-02-02
 *
 * \authors Michelle Strout, Mahdi Soltan Mohammadi
 *
 * Copyright (c) 2015-2016, University of Arizona <br>
 * All rights reserved. 
 * See ../../COPYING for details. <br>
 */
 
#ifndef UFCALLMAP_H_
#define UFCALLMAP_H_

#include "expression.h"
#include "TupleDecl.h"
#include <map>
#include <string>

namespace iegenlib{

class Set;

/*!
 * \class UFCallMap
 * This class can store UFcalls and their equ. symbolic constants. 
 * We need this in functionality of creating affine sets out of
 * non-affine sets, where UFcalls get replaced with symbolic constants.
 */
class UFCallMap {
public:
    UFCallMap(){}
    ~UFCallMap(){
               mUFC2Str.clear();
               mStr2UFC.clear();
    }

    //! Copy constructor.
    UFCallMap( const UFCallMap& other ){ *this = other; }

    //! returns a string representing ufcterm as a symbolic constant
    string symUFC( std::string &ufcName );

    //! Use this to insert a UFC term to maps.
    //  The function creats an string representing the UFC then,
    //  adds (ufc,str) to mUFC2Str & adds (ufc,str) to mStr2UFC
    void insert( UFCallTerm &ufcterm );

    //! Searches for ufcterm in mUFC2Str. If it exists in the map, returns
    // the equ. symbol, otherwise returns an empty string.
    string find( UFCallTerm &ufcterm );

    //! Searches for a symbol in mStr2UFC. If it exists in the map, returns the
    //  equ. UFC, otherwise returns foo() (representing empty UFC)
    UFCallTerm find( string &symbol );

    // prints the content of the map into a string, and returns it
    std::string toString();

private:
    std::map<UFCallTerm,string> mUFC2Str;
    std::map<string,UFCallTerm> mStr2UFC;
};

}

#endif
