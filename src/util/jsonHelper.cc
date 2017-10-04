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
void addUniQuantRules(json &uqCons);
// Reads iterators that we should not project from a json sructure
void notProjectIters(Relation* rel, std::set<int> &parallelTvs, json &np);
