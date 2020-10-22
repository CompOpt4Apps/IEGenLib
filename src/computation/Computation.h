/*!
 * \file Computation.h
 *
 * \brief Declarations for the Computation and StmtInfo classes.
 *
 * The Computation class is the SPF representation of a logical computation.
 * It contains a StmtInfo class for each statement, which in turn contains
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

#include <map>
#include <memory>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "set_relation/set_relation.h"

namespace iegenlib {

class StmtInfo;

/*!
 * \class Computation
 *
 * \brief SPF representation of a computation (group of statements such as a
 * function).
 */
class Computation {
   public:
    //! Print out all the information represented in this Computation for
    //! debug purposes
    void printInfo();

    //! Clear all data from this Computation
    void clear();

    //! Get the  data spaces used in this Computation
    std::unordered_set<std::string> getDataSpaces();

    //! Get the source code for a given statement
    std::string getStmtSource(unsigned int stmtNumber);

    //! Set the source code for a given statement
    void setStmtSource(unsigned int stmtNumber, std::string newSource);

    //! Get the iteration space (pretty printed) for a statement
    std::string getIterSpace(unsigned int stmtNumber);

    //! Set the iteration space for a statement
    void setIterSpace(unsigned int stmtNumber,
                      std::string newIterationSpaceStr);

    //! Get the execution schedule (pretty printed) for a statement
    std::string getExecSched(unsigned int stmtNumber);

    //! Set the execution schedule for a statement
    void setExecSched(unsigned int stmtNumber,
                      std::string newExecutionScheduleStr);

    //! Get the reads for a statement
    std::vector<std::pair<std::string, std::string>> getDataReads(
        unsigned int stmtNumber);

    //! Add a read to a statement
    void addDataRead(unsigned int stmtNumber, unsigned int index,
                     std::string dataSpace, std::string readRelStr);

    //! Remove a read from a statement
    void removeDataRead(unsigned int stmtNumber, unsigned int index);

    //! Get the writes for a statement
    std::vector<std::pair<std::string, std::string>> getDataWrites(
        unsigned int stmtNumber);

    //! Add a write to a statement
    void addDataWrite(unsigned int stmtNumber, unsigned int index,
                      std::string dataSpace, std::string writeRelStr);

    //! Remove a write from a statement
    void removeDataWrite(unsigned int stmtNumber, unsigned int index);

   private:
    //! Data spaces accessed in the computation
    std::unordered_set<std::string> dataSpaces;
    //! Map of statement names (numbers) -> the statement's corresponding
    //! information
    std::map<unsigned int, StmtInfo> stmtsInfoMap;
};

/*!
 * \class StmtInfo
 *
 * \brief Information attached to a statement represented as mathematical
 * objects.
 */
class StmtInfo {
   public:
    //! Construct a StmtInfo given strings that will be used to construct each
    //! set/relation.
    StmtInfo(std::string stmtSourceCode, std::string iterationSpaceStr,
             std::string executionScheduleStr,
             std::vector<std::pair<std::string, std::string>> dataReadsStrs,
             std::vector<std::pair<std::string, std::string>> dataWritesStrs);

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
