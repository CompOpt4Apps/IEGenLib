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

//! Assignment operator.
UFCallMap& UFCallMap::operator=( const UFCallMap& other) {

    mUFC2VarParam = other.mUFC2VarParam;
    mVarParam2UFC = other.mVarParam2UFC;

    return *this;
}

//! Returns a string representing ufcterm as a symbolic constant
// The result is in a format that isl and Omega+ libraries can handle
// It is going to look like:
// Original:    row( col(tv2 + 1) - 2 ) ->
// ConsSymbol:  row_col_tv2P1_M2_
string UFCallMap::symUFC( std::string &ufcName )
{
    int len = ufcName.length();
    std::stringstream ss;

    for (int i = 0 ; i < len ; i++){
        char c = ufcName[i];
        if( c != ' ' && c != ','){
            if( c == '(' || c == ')'){
                ss <<'_';
            }
            else if( c == '[' || c == ']'){
                ss <<'B';
            }
            else if( c == '+' ){
                ss <<'P';
            }
            else if( c == '-' ){
                ss <<'M';
            }
            else{
                ss <<c;
            }
        }
    }

    return (ss.str());
}

//! Inserts a UFC term to both of themaps.
//  The function creates an string representing the UFC as symbolic constant,
//  then, adds (ufc,vt) & (vt,ufc) to mUFC2VarParam & mVarParam2UFC
//  It does not add repetitive UFCs. The function does not own ufcterm.
void UFCallMap::insert( UFCallTerm *ufc )
{
    UFCallTerm* ufcterm = ( (UFCallTerm*)(ufc->clone()) );
    ufcterm->setCoefficient(1);
    if( find(ufcterm) ){
        return;
    }

    std::string symCons = ufcterm->toString();
    symCons = symUFC(symCons);
    VarTerm vt( 1 , symCons );

    mUFC2VarParam.insert ( std::pair<UFCallTerm,VarTerm>(*ufcterm,vt) );
    mVarParam2UFC.insert ( std::pair<VarTerm,UFCallTerm>(vt,*ufcterm) );

    delete ufcterm;
}

//! Searches for ufcterm in mUFC2VarParam. If it exists in the map, returns
// the equ. symbol, otherwise returns an empty string.
VarTerm* UFCallMap::find( UFCallTerm* ufc )
{
    UFCallTerm* ufcterm = ( (UFCallTerm*)(ufc->clone()) );
    ufcterm->setCoefficient(1);

    std::map<UFCallTerm,VarTerm>::iterator it;
    
    it = mUFC2VarParam.find(*ufcterm);

    delete ufcterm;
    if (it == mUFC2VarParam.end()){
        return NULL;
    }

    return ( (VarTerm*)(it->second).clone() );
}

//! Searches for a symbol in mVarParam2UFC. If it exists in the map, returns the
//  equ. UFC, otherwise returns foo() (representing empty UFC)
UFCallTerm* UFCallMap::find( VarTerm* vt )
{
    VarTerm* sym = ( (VarTerm*)(vt->clone()) );
    sym->setCoefficient(1);
    std::map<VarTerm,UFCallTerm>::iterator it;

    it = mVarParam2UFC.find(*sym);
    if (it == mVarParam2UFC.end()){
        return NULL;
    }
    delete sym;

    return ( (UFCallTerm*)(it->second).clone() );
}

// prints the content of the map into a string, and returns it
std::string UFCallMap::toString()
{
    std::stringstream ss;
    ss << "UFCallMap:" << std::endl;
    std::map<UFCallTerm,VarTerm>::iterator it;
    for (it=mUFC2VarParam.begin(); it!=mUFC2VarParam.end(); it++) {
        ss << "\tUFC = " << it->first.toString() 
           << "  ,  sym = " << it->second.toString() << std::endl;
    }
    return ss.str();
}

}  // end of namespace iegenlib
