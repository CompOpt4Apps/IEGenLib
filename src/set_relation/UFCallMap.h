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
 * This class can store UFcalls and their equ. VarTerm. 
 * We need this in functionality of creating affine sets out of
 * non-affine sets, where UFcalls get replaced with symbolic constants.
 * UFCallTerms and VarTerms are stored with coefficient = 1, however there is no
 * need to set coefficient when inserting a UFCallTerm or searching for
 * UFCallTerm or VarTerm in the map using find functions.
 */
class UFCallMap {
public:
    UFCallMap(){}
    ~UFCallMap(){
               mUFC2VarParam.clear();
               mVarParam2UFC.clear();
    }

    //! Copy constructor.
    UFCallMap( const UFCallMap& other ){ *this = other; }

    //! Assignment operator.
    UFCallMap& operator=( const UFCallMap& other );

    //! returns a string representing ufcterm as a symbolic constant
    string symUFC( std::string &ufcName );

    /*! Use this to insert a UFCallTerm to map.
    **  The function creates an VarTerm representing the UFC then,
    **  adds (ufc,vt) to mUFC2VarParam & adds (vt,str) to mVarParam2UFC
    **  The class does not own the object pointed by *ufcterm,
    **  so users are responsible for freeing the memory.
    **  There is no need to set coefficient of ufc, function insert a UFCallTerm
    **  with coefficient = 1 to the map ithout changing ufc objevt.
    */
    void insert( UFCallTerm *ufc);

    /*! Searches for ufcterm (with coefficient = 1) in the map. If ufcterm
    **  exists, it returns a pointer to equ. VarTerm, otherwise returns NULL.
    **  The class does not own the object pointed by ufcterm,
    **  and it is left unchanged.
    */
    VarTerm* find( UFCallTerm* ufc );

    /*! Searches for a VarTerm (with coefficient = 1) in the map. If VarTerm
    **  exists in the map, returns the pointer to equ. UFC,
    **  otherwise returns NULL. The class does not own the object
    **  pointed by symbol, and it is left unchanged.
    */
    UFCallTerm* find( VarTerm* symbol );

    //! prints the content of the map into a string, and returns it
    std::string toString();

private:
    std::map<UFCallTerm,VarTerm> mUFC2VarParam;
    std::map<VarTerm,UFCallTerm> mVarParam2UFC;
};

}

#endif
