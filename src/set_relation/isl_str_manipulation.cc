/*!
 * \file isl_str_manipulation.h
 *
 * \Implementations of isl string manipulator functions
 *
 * \date Started: 2016-03-11
 *
 * \authors Michelle Strout, Mahdi Soltan Mohammadi
 *
 * Copyright (c) 2016, University of Arizona <br>
 * All rights reserved. 
 * See ../../COPYING for details. <br>
 */

#include "isl_str_manipulation.h"

namespace iegenlib{


//****** ISL tuple correction functions

/*! This function extracts tuple variables from a Tuple Declaration string.
**  Tuple Declaration can be either of Relation or Set form
**  Input:  "[i1,col_tv3_] -> [i3,i4]" or
**          "[i1,col_tv3_,i3,i4]"  
**  Output: (pointer to)
**          {"i1", "col_tv3_", "i3", "i4"}
*/
std::string* tupVarsExtract(std::string tupDecl, int inArity, int outArity){

    int st=0, end=0, idxSt = 0;
    int nparts=1;
    int arity = inArity + outArity;
    std::string* tupVars = new std::string[arity];

    if( outArity != 0 ){
      nparts = 2;
      arity = inArity;
    }
    for(int p=0 ; p < nparts ; p++){
        st = tupDecl.find_first_of('[', st);
        end = tupDecl.find_first_of(']', st);
        std::string str = tupDecl.substr(st+1,end-st-1);

        int tst = 0,tend = 0 ;
        for(int i = 0 ; i < arity ; i++ ){

            tend = str.find_first_of(',', tst);
            tupVars[idxSt + i] = str.substr(tst,tend-tst);
            tst = tend + 1;
        }

        arity = outArity;
        idxSt = inArity;
        st = end + 1;
    }

    return tupVars;
}
/*! This function checks to see whether two Tuple Variables are equal
**  regardless of spacing before and after their names:
**  "  ip " == "ip"
*/
#define unsigned int uint
bool tvEqCheck(std::string tv1, std::string tv2){
    int nonSpSt = 0, nonSpFi = tv1.length()-1;
    for( uint i = 0; tv1[nonSpSt] == ' ' && i < tv1.length(); i++, nonSpSt++ );
    for( uint i = 0; tv1[nonSpFi] == ' ' && i < tv1.length(); i++, nonSpFi-- );
    tv1 = tv1.substr(nonSpSt,nonSpFi-nonSpSt+1);

    nonSpSt = 0;
    nonSpFi = tv2.length()-1;
    for( uint i = 0; tv2[nonSpSt] == ' ' && i < tv2.length(); i++, nonSpSt++ );
    for( uint i = 0; tv2[nonSpFi] == ' ' && i < tv2.length(); i++, nonSpFi-- );
    tv2 = tv2.substr(nonSpSt,nonSpFi-nonSpSt+1);

    if ( tv1 == tv2 ){
        return true;
    }

    return false;
}
/*! This function constructs equality constraints between Tuple Varialbes
**  that are replaced eachother by ISL. To do this, it takes in two
**  Tuple declaration, extracts their Tuple Variables, then creates
**  equalities for those that are replaced:
**      origTupDecl: [i1, i2] -> [1, i4]
**      islTupDecl : [col_tv1_, i2] -> [1, i2]
**      output:      (i1) - (col_tv1_) = 0 and (i4) - (i2) = 0
**  Note: noFirstAnd determines whether we should put "and" at the beginning of
**  the output string, which depends on original constraints being empty or not. 
*/
std::string missingEqs(std::string origTupDecl, std::string islTupDecl,
                              int inArity, int outArity, bool noFirstAnd){
    std::string eqs("");
    int arity = inArity + outArity;
    bool firstEq = true;
 
    std::string* origTupVars = tupVarsExtract(origTupDecl, inArity, outArity);
    std::string* islTupVars = tupVarsExtract(islTupDecl, inArity, outArity);

    for(int i = 0 ; i < arity ; i++){
        if( ! tvEqCheck(origTupVars[i] , islTupVars[i]) ){
            if(noFirstAnd && firstEq){
                eqs +=  " (" + origTupVars[i] +
                        ") - (" + islTupVars[i] + ") = 0";
                firstEq = false;
            } else {
                eqs +=  " and (" + origTupVars[i] +
                        ") - (" + islTupVars[i] + ") = 0";
            }
        
        }
    }

    delete[] origTupVars;
    delete[] islTupVars;

    return eqs;
}
/*! This function takes in a Set or Relation string and returns different 
**  parts of it in a srParts structure.
*/
srParts* getPartsFromStr(std::string str){
    srParts* parts = new srParts();
    int l1,l2,l3;

    l1 = str.find_first_of('{', 0);
    l2 = str.find_first_of(':', l1);
    if( l2 < 0 ){
        parts->symVars =     str.substr( 0 , l1-0 );
        l3 = str.find_first_of('}', l1);
        parts->tupDecl =     str.substr( l1+1 , l3-(l1+1)-1 );
        parts->constraints = "";
        return parts;
    }
    l3 = str.find_first_of('}', l2);

    parts->symVars =     str.substr( 0 , l1-0 );
    parts->tupDecl =     str.substr( l1+1 , l2-(l1+1) );
    parts->constraints = str.substr( l2+1 , l3-(l2+1) );

    return parts;
}
/*! The main function that restores uninted changes ISL library apply to
**  Tuple Declaration because of the equality constraints. ISL library replaces
**  tuple variables with their equal expression if one exists in the constraints:
**    input to isl   : [n] -> { [i] : i = n and i < 10 }
**    output from isl: { [n] : n < 10 }
**  This function replaces Tuple Declaration from ISL string with original one,
**  and creates missing equalities and puts them back into constraints.
*/
std::string revertISLTupDeclToOrig(std::string origStr, std::string islStr,
                              int inArity, int outArity){
    std::string correctedStr, eqsStr;
    bool noFirstAnd = false;
    
    srParts* origParts = getPartsFromStr(origStr);
    srParts* islParts = getPartsFromStr(islStr);

    if( islParts->constraints.length() == 0 ){
        noFirstAnd = true;
    }
    eqsStr = missingEqs( origParts->tupDecl , islParts->tupDecl ,
                                inArity, outArity, noFirstAnd );
    
    if( islParts->constraints.length() == 0 && eqsStr.length() == 0){
        correctedStr = islParts->symVars + islParts->sC +
                       origParts->tupDecl + islParts->eC;
    } else {
        correctedStr = islParts->symVars + islParts->sC +
                       origParts->tupDecl + islParts->sepC + 
                       islParts->constraints + eqsStr +islParts->eC;
    }
    delete origParts;
    delete islParts;

    return correctedStr;
}
/*! This function takes in a Set or Relation string and removes tuple variable
**  located in poTv position from Tuple Declaration of the string.
**     str         : [n] -> { [i1,i2,i3] : ... }
**     poTv        : 1
**     correctedStr: [n] -> { [i1,i3] : ... }
*/
std::string projectOutStrCorrection(std::string str, int poTv,
                                    int inArity, int outArity){
    std::string correctedStr, newTupDecl;
    int arity = inArity + outArity;
    
    srParts* parts = getPartsFromStr(str);
    
    std::string* origTupVars = tupVarsExtract(parts->tupDecl,
                                              inArity, outArity);
    newTupDecl = "[";
    for (int i = 0 ; i < arity ; i++){
        if( i == inArity && outArity != 0){
            newTupDecl += "] -> [";
        }
        if( i != poTv ){
            newTupDecl += origTupVars[i];
            if( i != arity-1 && i != inArity-1 && 
                !( ((i == arity-2)&&(poTv == arity-1)) ||
                   ((i == inArity-2)&&(poTv == inArity-1)) )
              ){
                newTupDecl += ", ";
            }
        }
    }
    newTupDecl += "] ";
  
    if (parts->constraints.length() == 0 ){
        correctedStr = parts->symVars + parts->sC + newTupDecl + parts->eC; 
    } else{
        correctedStr = parts->symVars + parts->sC + newTupDecl +
                       parts->sepC + parts->constraints + parts->eC;
    }

    delete parts;
    delete[] origTupVars;

    return correctedStr;
}
//****** ISL tuple correction End

//! This function takes a Set string and returns equivalent isl_set*
isl_set* islStringToSet( std::string relstr , isl_ctx *ctx )
{
  // load Relation r into ISL map
  isl_set* iset = isl_set_read_from_str(ctx, relstr.c_str());

  return iset;
}

/*! This function takes an isl_set* and returns equivalent Set string
** The function takes ownership of input argument 'iset'
*/
std::string islSetToString ( isl_set* iset , isl_ctx *ctx ) {
  // Get an isl printer and associate to an isl context
  isl_printer * ip = isl_printer_to_str(ctx);

  // get string back from ISL map
  isl_printer_set_output_format(ip, ISL_FORMAT_ISL);
  isl_printer_print_set(ip, iset);
  char *i_str = isl_printer_get_str(ip);
  std::string stringFromISL (i_str); 
  
  // clean-up
  isl_printer_flush(ip);
  isl_printer_free(ip);
  isl_set_free(iset);
  iset= NULL;
  free(i_str);

  return stringFromISL;
}

//! This function takes a Relation string and returns pointer to equ. isl_map
isl_map* islStringToMap( std::string relstr , isl_ctx *ctx )
{
  // load Relation r into ISL map
  isl_map* imap = isl_map_read_from_str(ctx, relstr.c_str());

  return imap;
}

/*! This function takes an isl_map* and returns pointer to equ. Relation string
** The function takes ownership of input argument 'imap'
*/
std::string islMapToString ( isl_map* imap , isl_ctx *ctx )
{
  // Get an isl printer and associate to an isl context
  isl_printer * ip = isl_printer_to_str(ctx);

  // get string back from ISL map
  isl_printer_set_output_format(ip , ISL_FORMAT_ISL);
  isl_printer_print_map(ip ,imap);
  char *i_str = isl_printer_get_str(ip);
  std::string stringFromISL (i_str); 
  
  // clean-up
  isl_printer_flush(ip);
  isl_printer_free(ip);
  isl_map_free(imap);
  imap= NULL;
  free(i_str);

  return stringFromISL;
}

}// iegenlib namespace
