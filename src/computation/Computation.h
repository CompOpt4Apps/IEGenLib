/*!
 * \file Computation.h
 *
 * \brief Declarations for the Computation and StmtInfo structs.
 *
 * The Computation struct is the SPF representation of a logical computation.
 * It contains a StmtInfo struct for each statement, which in turn contains
 * information about that statement as mathematical objects.
 * Originally part of spf-ie.
 *
 * \date Started: 10/09/20
 *
 * \authors Anna Rift
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

struct StmtInfo;

/*!
 * \struct Computation
 *
 * \brief SPF representation of a computation (group of statements such as a
 * function).
 */
struct Computation {
    //! Print out all the information represented in this Computation for
    //! debug purposes
    void printInfo();

    //! Data spaces accessed in the computation
    std::unordered_set<std::string> dataSpaces;
    //! Map of statement names -> the statement's corresponding information
    std::map<std::string, StmtInfo> stmtsInfoMap;
};

/*!
 * \struct StmtInfo
 *
 * \brief Information attached to a statement represented as mathematical
 * objects.
 */
struct StmtInfo {
    //! Construct a StmtInfo given strings that will be used to construct each
    //! set/relation.
    StmtInfo(
        std::string stmtSourceCode, std::string iterationSpaceStr,
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
    std::vector<std::pair<std::string, std::unique_ptr<Relation>>>
        dataReads;
    //! Write dependences of a statement, pairing data space name to relation
    std::vector<std::pair<std::string, std::unique_ptr<Relation>>>
        dataWrites;
};

}  // namespace iegenlib

#endif
