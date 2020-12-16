/*!
 * \file Computation.h
 *
 * \brief Declarations for the Computation and Stmt classes.
 *
 * The Computation class is the SPF representation of a logical computation.
 * It contains a Stmt class for each statement, which in turn contains
 * information about that statement as mathematical objects.
 * Originally part of spf-ie.
 *
 * \date Started: 10/09/20
 *
 * \authors Anna Rift
 *
 * Copyright (c) 2020, University of Arizona <br>
 * Copyright (c) 2020, Boise State University <br>
 * All rights reserved. <br>
 * See ../../COPYING for details. <br>
 */

#ifndef COMPUTATION_H_
#define COMPUTATION_H_

#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "set_relation/Visitor.h"
#include "set_relation/environment.h"
#include "set_relation/set_relation.h"

namespace iegenlib {

class Stmt;

/*!
 * \class VisitorFindUFReplacements
 *
 * \brief Visitor used to build a list of replacement names/formats for UF calls
 * that Omega doesn't support.
 */
class VisitorFindUFReplacements : public Visitor {
   private:
    //! next number to use in creating unique names
    int nextReplacementNumber = 0;
    //! 'from' and 'to' string for each UF call macro we need
    std::map<std::string, std::string>* macros;

   public:
    VisitorFindUFReplacements(std::map<std::string, std::string>* iMacros) {
        macros = iMacros;
    }

    void preVisitUFCallTerm(UFCallTerm* callTerm) {
        // set new function name
        std::string replacementName =
            callTerm->name() + "_" + std::to_string(nextReplacementNumber++);
        callTerm->setName(replacementName);

        // set up macro outputs
        std::ostringstream os_replaceFrom;
        std::ostringstream os_replaceTo;
        os_replaceFrom << replacementName << "(";
        os_replaceTo << callTerm->name() << "(";

        // process every parameter
        bool pastFirstParam = false;
        int paramNumber = 0;
        bool haveAddedToOutput;
        bool haveAddedToInput;
        Exp* paramExp;
        std::vector<Term*> termsToSave;
        unsigned int i;
        for (i = 0; i < callTerm->numArgs(); ++i) {
            // loop through all terms, adding them into the 'to' and 'from'
            // appropriately
            haveAddedToInput = false;
            haveAddedToOutput = false;
            if (pastFirstParam) {
                os_replaceTo << ",";
            }
            paramExp = callTerm->getParamExp(i);
            std::list<Term*> originalTerms = paramExp->getTermList();
            for (const auto& term : originalTerms) {
                if (term->isConst()) {
                    // add the term to the function call, without making an
                    // input param for it
                    os_replaceTo << (haveAddedToOutput ? "+" : "") << "("
                                 << term->toString() << ")";
                } else {
                    // add the term to both the input and output function call
                    os_replaceFrom
                        << ((pastFirstParam || haveAddedToInput) ? "," : "")
                        << "p" << paramNumber;
                    os_replaceTo << (haveAddedToOutput ? "+" : "") << "p"
                                 << paramNumber;
                    termsToSave.push_back(std::move(term->clone()));
                    paramNumber++;
                    haveAddedToInput = true;
                }
                haveAddedToOutput = true;
            }
            pastFirstParam = true;
        }

        // rewrite argument list, one arg per term in original call args
        callTerm->resetNumArgs(termsToSave.size());
        i = 0;
        for (const auto& savedTerm : termsToSave) {
            Exp* newParamExp = new Exp();
            newParamExp->addTerm(savedTerm);
            callTerm->setParamExp(i, newParamExp);
            i++;
        }

        // complete outputs for this UF call
        os_replaceFrom << ")";
        os_replaceTo << ")";
        macros->emplace(os_replaceFrom.str(), os_replaceTo.str());
    }
};

/*!
 * \class Computation
 *
 * \brief SPF representation of a computation (group of statements such as a
 * function).
 */
class Computation {
   public:
    //! Construct an empty Computation
    Computation(){};

    //! Copy constructor
    Computation(const Computation& other);

    //! Assignment operator (copy)
    Computation& operator=(const Computation& other);

    //! Equality operator
    bool operator==(const Computation& other) const;

    //! Add a statement to this Computation and get a pointer to it.
    //! Statements are numbered sequentially from 0 as they are inserted
    Stmt* addStmt(const Stmt& stmt);
    //! Get a statement by index
    Stmt* getStmt(unsigned int index);

    //! Add a data space to this Computation
    void addDataSpace(std::string dataSpaceName);
    //! Get data spaces
    std::unordered_set<std::string> getDataSpaces() const;

    //! Get the number of statements in this Computation
    unsigned int getNumStmts() const;

    //! Print out all the information represented in this Computation for
    //! debug purposes
    void printInfo() const;

    //! Get whether this Computation's required information is filled out,
    //! including that of the Stmts it contains
    bool isComplete() const;

    //! Clear all data from this Computation
    void clear();

    //! Environment used by this Computation
    Environment env;

    //! Method generates c code.
    std::string codeGen();

   private:
    //! Information on all statements in the Computation
    std::vector<Stmt> stmts;
    //! Data spaces accessed in the computation
    std::unordered_set<std::string> dataSpaces;
};

/*!
 * \class Stmt
 *
 * \brief Information attached to a statement represented as mathematical
 * objects.
 */
class Stmt {
   public:
    //! Construct an empty Stmt
    Stmt(){};

    //! Construct a complete Stmt, given strings that will be used to
    //! construct each set/relation.
    Stmt(std::string stmtSourceCode, std::string iterationSpaceStr,
         std::string executionScheduleStr,
         std::vector<std::pair<std::string, std::string>> dataReadsStrs,
         std::vector<std::pair<std::string, std::string>> dataWritesStrs);

    //! Copy constructor
    Stmt(const Stmt& other);

    //! Assignment operator (copy)
    Stmt& operator=(const Stmt& other);

    //! Equality operator
    //! Checks equality, NOT mathematical equivalence
    bool operator==(const Stmt& other) const;

    //! Get whether or not all necessary information for this Stmt is set
    bool isComplete() const;

    //! Get the source code of the statement
    std::string getStmtSourceCode() const;
    //! Set the source code of the statement
    void setStmtSourceCode(std::string newStmtSourceCode);

    //! Get the iteration space Set
    Set* getIterationSpace() const;
    //! Set the iteration space, constructing it from the given string
    void setIterationSpace(std::string newIterationSpaceStr);

    //! Get the execution schedule Relation
    Relation* getExecutionSchedule() const;
    //! Set the execution schedule, constructing it from the given string
    void setExecutionSchedule(std::string newExecutionScheduleStr);

    //! Add a data read
    void addRead(std::string dataSpace, std::string relationStr);
    //! Get the number of data reads for this statement
    unsigned int getNumReads() const;
    //! Get a data read's data space by index
    std::string getReadDataSpace(unsigned int index) const;
    //! Get a data read's relation by index
    Relation* getReadRelation(unsigned int index) const;

    //! Add a data write
    void addWrite(std::string dataSpace, std::string relationStr);
    //! Get the number of data writes for this statement
    unsigned int getNumWrites() const;
    //! Get a data write's data space by index
    std::string getWriteDataSpace(unsigned int index) const;
    //! Get a data write's relation by index
    Relation* getWriteRelation(unsigned int index) const;

   private:
    //! Source code of the statement, for debugging purposes
    std::string stmtSourceCode;
    //! Iteration space of a statement
    std::unique_ptr<Set> iterationSpace;
    //! Execution schedule of a single statement
    std::unique_ptr<Relation> executionSchedule;
    //! Read dependences of a statement, pairing data space name to relation
    std::vector<std::pair<std::string, std::unique_ptr<Relation>>> dataReads;
    //! Write dependences of a statement, pairing data space name to relation
    std::vector<std::pair<std::string, std::unique_ptr<Relation>>> dataWrites;
};

}  // namespace iegenlib

#endif
