/*!
 * \file util.h
 *
 * \brief Utilities for the IEGen project
 *
 * \date Started: 9/13/2010, but has been completely rewritten
 * # $Revision:: 622                $: last committed revision
 * # $Date:: 2013-01-18 13:11:32 -0#$: date of last committed revision
 * # $Author:: cathie               $: author of last committed revision
 *
 * \authors
 *
 * Copyright (c) 2012, Colorado State University <br>
 * All rights reserved. <br>
 * See ../../COPYING for details. <br>
 */

#include "jsonHelper.h"

//using jsoncons::json;
//using namespace iegenlib;

// Reads a list of UFCs from a json structure and stores them in the environment  
void addUFCs(json &ufcs){

  for (size_t j = 0; j < ufcs["UFS"].size(); ++j){

    bool bijective = false;
    if( ufcs["UFS"][j]["Bijective"].as<string>() == string("true") ){
      bijective = true;
    }
    iegenlib::MonotonicType monotonicity = iegenlib::Monotonic_NONE;
    if(ufcs["UFS"][j]["Monotonicity"].as<string>() == 
                            string("Monotonic_Nondecreasing")){
      monotonicity = iegenlib::Monotonic_Nondecreasing;
    } else if(ufcs["UFS"][j]["Monotonicity"].as<string>() == 
                                  string("Monotonic_Increasing")){
      monotonicity = iegenlib::Monotonic_Increasing;
    }

    iegenlib::appendCurrEnv(ufcs["UFS"][j]["Name"].as<string>(),// Name
         new Set(ufcs["UFS"][j]["Domain"].as<string>()),   // Domain 
         new Set(ufcs["UFS"][j]["Range"].as<string>()),    // Range
         bijective,                                              // Bijective?
         monotonicity                                            // Monotonicity?
                                );
  }
}


// Reads a list of universially quantified constraints from a json structure
// and stores them in the environment
void adduniQuantConstraints(json &uqCons){

  uniQuantConstraint uqConst;
  for (size_t j = 0; j < uqCons.size(); ++j){

    // Read user defined constraints based on universally quantified
    // expressions, then call addConsForUniversQuantExp to add them:
    //    Forall e1, e2: if ( e1 exOP e2 ) => ( UF1(e1) ufOP UF2(e2) )
    if( uqCons[j]["Type"].as<string>() == "UserDefPar2UFC"){
      uqConst.setType("UserDefPar2UFC");
      uqConst.setExpCompOp(uqCons[j]["Forall e1, e2: if e1 is? e2"].as<string>());
      uqConst.setUfCompOp(uqCons[j][" is? "].as<string>());
      uqConst.setUfSymbol1(uqCons[j]["then add: UFSymbol1?(e1)"].as<string>());
      uqConst.setUfSymbol2(uqCons[j]["UFSymbol2?(e2)"].as<string>() );
      iegenlib::addUniQuantConstraint(uqConst);
    }

    // read user defined relations between UFCs, then call 
    // addConsForUFCallRel to add related constraints:
    //    Forall e1, e2: if ( UF1(e1) ufOP UF2(e2) ) => ( e1 exOP e2 ) 
    else if( uqCons[j]["Type"].as<string>() == "UserDefUFC2Par"){
      uqConst.setType("UserDefUFC2Par");
      uqConst.setExpCompOp(uqCons[j]["then add: e1 is? e2"].as<string>());
      uqConst.setUfCompOp(uqCons[j][" is? "].as<string>() );
      uqConst.setUfSymbol1(uqCons[j]["Forall e1, e2: if UFSymbol1?(e1)"].as<string>());
      uqConst.setUfSymbol2(uqCons[j]["UFSymbol2?(e2)"].as<string>() );
      iegenlib::addUniQuantConstraint(uqConst);
    }
  }
}

// Reads iterators that we should not project from a json sructure
void notProjectIters(Relation* rel, std::set<int> &parallelTvs, json &np){

  iegenlib::TupleDecl td = rel->getTupleDecl();
  for (size_t j = 0; j < np.size(); ++j){
    string tvS = np[j].as<string>();
    int tvN = -1;
    for (unsigned int c = 0 ; c < td.getSize() ; c++){
      if( tvS == td.elemToString(c) ){
        tvN = c;
        break;
      }
    }
    parallelTvs.insert( tvN );
  }
}

