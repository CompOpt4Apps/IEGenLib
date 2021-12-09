
/*!
 * \file VisitorChangeUFsForOmega.h
 *
 * \brief Visitor classes  that makes modifications to UF call terms so that they are
 * acceptable for what Omega supports and a class that replaces all instances
 * of omega compliant UFs with more precise original UF. 
 *
 * Intended for use on sets/relations. Also gathers information needed to pass
 * along to Omega codegen, such as #define macros.
 
 *
 * \date Started: 10/09/20
 *
 * \authors Anna Rift, Tobi Popoola
 *
 * Copyright (c) 2020, University of Arizona <br>
 * Copyright (c) 2020, Boise State University <br>
 * All rights reserved. <br>
 * See ../../COPYING for details. <br>
 */

#ifndef VISITORCHANGEUFSFOROMEGA_H
#define VISITORCHANGEUFSFOROMEGA_H

#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "Visitor.h"
#include "TupleDecl.h"
#include "expression.h"
#include "set_relation.h"


namespace iegenlib{


/*!
 * \class VisitorChangeUFsForOmega
*/
class VisitorChangeUFsForOmega : public Visitor {
   private:
    
    // List of flattened ufcall terms and where it will
    // be substituted in the tuple declaration.
    // For example 
    // Initial Set: {[n,k]:} 
    // FlatUF,1
    // Final Set: {[n,t1,k]
    std::vector<std::pair<UFCallTerm*,int>> flatUfTupleMap;
    
    Conjunction* currentConjunction ; 
        
    // Stores each replaced UFCallTerm with 
    // original UFCalll
    std::map<std::string,UFCallTerm*> ufMap;
	   
    //! UF call (re)definition macros
    std::map<std::string, std::string> macros;
    //! declarations of UF calls needed by Omega parser
    std::set<std::string> ufCallDecls;
    //! UF calls that we've seen before and do not need to rename if encountered
    //! again; mapping from call as string -> new assigned name
    std::map<std::string, std::string> knownUFs;

    std::vector<std::string> arrayAccessUFs;

    //! next number to use in creating unique function names
    int nextFuncReplacementNumber;
    //! stored tuple decl for variable retrieval
    TupleDecl currentTupleDecl;

    //! Information stored for tuple variable
    //! assignments.
    std::map<int,std::string> tupleAssignments;

   public:
    //! Construct a new VisitorChangeUFsForOmega
    VisitorChangeUFsForOmega();
    //! Destructor
    ~VisitorChangeUFsForOmega();

    //! Completely reset state of Visitor for re-use, including freeing memory.
    //! This method should NOT be used between visiting connected
    //! Sets/Relations, for instance those which share some UF calls you want
    //! the Visitor to remember.
    void reset();

    //! Partially reset Visitor state so that it is prepared for use with
    //! additional interrelated Sets/Relations, such as statements in a
    //! Computation.
    void prepareForNext();

    //! Get tuple assignments. A tuple is initially assigned
    //! to zero, unless there is an equality constraint
    //! involving that tuple variable and constants
    std::map<int,std::string>& getTupleAssignments();

    //! Get the UF call macros required for the code corresponding to the
    //! set/relation to function correctly, as a string
    std::map<std::string, std::string>* getUFMacros();

    //! Gets a map from omega compliant uf name back to 
    //! IEGenlib UFCallTerm.
    std::map<std::string, UFCallTerm*>& getUFMap();

    //! Get the declarations of UF calls needed by Omega parser
    std::set<std::string> getUFCallDecls();

    void preVisitConjunction(Conjunction* c);
    
    void postVisitConjunction(Conjunction*);

    void postVisitUFCallTerm(UFCallTerm*);
    
    void preVisitUFCallTerm(UFCallTerm*);

    void preVisitExp(iegenlib::Exp * e);
    
    void postVisitSet(iegenlib::Set*);

    void preVisitRelation(iegenlib::Relation*);
};


/**
 * VisitorChangeOmegaUF, this reverses the transfformation 
 * of VisitorChangeUFsForOmega. It replaces all occurence 
 * of modified ufs back to their original uf call
 */
class VisitorChangeOmegaUF:public Visitor {
private:
    // Stores each replaced UFCallTerm with 
    // original UFCalll
    std::map<std::string,UFCallTerm*> ufMap;
public:
    explicit VisitorChangeOmegaUF(std::map<std::string,UFCallTerm*> ufMap):
	   ufMap(ufMap){} 
    void postVisitUFCallTerm(UFCallTerm*);
};	

}

#endif
