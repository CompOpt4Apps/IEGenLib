/*!
 * \file UFCallMap.cc
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

#include "UFCallMap.h"
#include "set_relation.h"
#include <util/util.h>
#include <iostream>

namespace iegenlib{

//! Returns a string representing ufcterm as a symbolic constant
// The result is in a format that isl and Omega+ libraries can handle
// It is going to look like:
// Original:    row( col(tv2 + 1) - 2 ) ->
// ConsSymbol:  row_col_tv2P1_M2_
string UFCallMap::symUFC( std::string &ufcName )
{
    int len = ufcName.length();
    std::string sym; 

    for (int i = 0 ; i < len ; i++){
        char c = ufcName[i];
        if( c != ' ' && c != ','){
            if( c == '(' || c == ')'){
                sym.append<int>( 1, '_');
            }
            else if( c == '+' ){
                sym.append<int>( 1, 'P');
            }
            else if( c == '-' ){
                sym.append<int>( 1, 'M');
            }
            else{
                sym.append<int>( 1, c);
            }
        }
    }

    return sym;
}


//! Inserts a UFC term to both of themaps.
//  The function creats an string representing the UFC as symbolic constant,
//  then,  adds (ufc,str) to mUFC2Str & adds (ufc,str) to mStr2UFC
//  It does not add repetitive UFCs
void UFCallMap::insert( UFCallTerm &ufcterm )
{
    if( find(ufcterm) != string("") ){
        return;
    }

    std::string symCons = ufcterm.toString();
    
    symCons = symUFC(symCons);
    
    mUFC2Str.insert ( std::pair<UFCallTerm,std::string>(ufcterm,symCons) );
    mStr2UFC.insert ( std::pair<std::string,UFCallTerm>(symCons,ufcterm) );
}

//! Searches for ufcterm in mUFC2Str. If it exists in the map, returns
// the equ. symbol, otherwise returns an empty string.
string UFCallMap::find( UFCallTerm &ufcterm )
{
    std::string symCons("");
    std::map<UFCallTerm,std::string>::iterator it;
    
    it = mUFC2Str.find(ufcterm);
    if (it != mUFC2Str.end()){
        symCons = it->second;
    }
    
    return symCons;
}

//! Searches for a symbol in mStr2UFC. If it exists in the map, returns the
//  equ. UFC, otherwise returns foo() (representing empty UFC)
UFCallTerm UFCallMap::find( string &symbol )
{
    std::map<std::string,UFCallTerm>::iterator it;
    UFCallTerm ufcall("foo", 0);

    it = mStr2UFC.find(symbol);
    if (it != mStr2UFC.end()){
        ufcall = it->second;
    }
    
    return ufcall;
}

// prints the content of the map into a string, and returns it
std::string UFCallMap::toString()
{
    std::stringstream ss;
    ss << "UFCallMap:" << std::endl;
    std::map<UFCallTerm,std::string>::iterator it;
    for (it=mUFC2Str.begin(); it!=mUFC2Str.end(); it++) {
        ss << "\tUFC = " << it->first.toString() 
           << "  ,  sym = " << it->second << std::endl;
    }
    return ss.str();
}

}  // end of namespace iegenlib
