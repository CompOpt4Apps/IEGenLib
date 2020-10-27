/*!
 * \file Computation.cc
 *
 * \brief Implementation of the Computation and Stmt classes.
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

#include "Computation.h"

#include <iostream>
#include <sstream>
#include <unordered_set>
#include <utility>
#include <vector>

#include "set_relation/set_relation.h"

namespace iegenlib {

/* Computation */

Computation::Computation(const Computation& other) { *this = other; }

Computation& Computation::operator=(const Computation& other) {
    this->dataSpaces = other.dataSpaces;
    this->stmtsInfoMap = other.stmtsInfoMap;
}

Stmt* Computation::addStmt(const Stmt& stmt) {
    stmtsInfoMap.emplace(numStmts, Stmt(stmt));
    Stmt* newStmt = getStmt(numStmts);
    numStmts++;
    return newStmt;
}

Stmt* Computation::getStmt(unsigned int index) {
    return &stmtsInfoMap.at(index);
}

void Computation::addDataSpace(std::string dataSpaceName) {
    dataSpaces.emplace(dataSpaceName);
}

std::unordered_set<std::string> Computation::getDataSpaces() const {
    return dataSpaces;
}

unsigned int Computation::getNumStmts() const { return numStmts; }

void Computation::printInfo() const {
    std::ostringstream stmtsOutput;
    std::ostringstream iterSpacesOutput;
    std::ostringstream execSchedulesOutput;
    std::ostringstream dataReadsOutput;
    std::ostringstream dataWritesOutput;
    std::ostringstream dataSpacesOutput;

    for (const auto& it : stmtsInfoMap) {
        std::string stmtName = "S" + std::to_string(it.first);
        // stmt source code
        stmtsOutput << stmtName << ": " << it.second.getStmtSourceCode()
                    << "\n";
        // iter spaces
        iterSpacesOutput << stmtName << ": "
                         << it.second.getIterationSpace()->prettyPrintString()
                         << "\n";
        // exec schedules
        execSchedulesOutput
            << stmtName << ": "
            << it.second.getExecutionSchedule()->prettyPrintString() << "\n";
        // data reads
        auto dataReads = it.second.getDataReads();
        dataReadsOutput << stmtName << ":";
        if (dataReads.empty()) {
            dataReadsOutput << " none";
        } else {
            dataReadsOutput << "{\n";
            for (const auto& read_it : dataReads) {
                dataReadsOutput << "    " << read_it.first << ": "
                                << read_it.second->prettyPrintString() << "\n";
            }
            dataReadsOutput << "}";
        }
        dataReadsOutput << "\n";
        // data writes
        auto dataWrites = it.second.getDataWrites();
        dataWritesOutput << stmtName << ":";
        if (dataWrites.empty()) {
            dataWritesOutput << " none";
        } else {
            dataWritesOutput << "{\n";
            for (const auto& write_it : dataWrites) {
                dataWritesOutput << "    " << write_it.first << ": "
                                 << write_it.second->prettyPrintString()
                                 << "\n";
            }
            dataWritesOutput << "}";
        }
        dataWritesOutput << "\n";
    }

    // data spaces
    dataSpacesOutput << "{";
    for (const auto& it : dataSpaces) {
        if (it != *dataSpaces.begin()) {
            dataSpacesOutput << ", ";
        }
        dataSpacesOutput << it;
    }
    dataSpacesOutput << "}\n";

    std::cout << "Statements:\n" << stmtsOutput.str();
    std::cout << "\nIteration spaces:\n" << iterSpacesOutput.str();
    std::cout << "\nExecution schedules:\n" << execSchedulesOutput.str();
    std::cout << "\nData spaces: " << dataSpacesOutput.str();
    std::cout << "\nArray reads:\n" << dataReadsOutput.str();
    std::cout << "\nArray writes:\n" << dataWritesOutput.str();
    std::cout << "\n";
}

bool Computation::isComplete() const {
    std::unordered_set<std::string> dataSpacesActuallyAccessed;
    for (const auto& stmtEntry : stmtsInfoMap) {
        // check completeness of each statement
        if (!stmtEntry.second.isComplete()) {
            return false;
        }

        // collect all data space accesses
        for (const auto& readInfo : stmtEntry.second.getDataReads()) {
            dataSpacesActuallyAccessed.emplace(readInfo.first);
        }
        for (const auto& writeInfo : stmtEntry.second.getDataWrites()) {
            dataSpacesActuallyAccessed.emplace(writeInfo.first);
        }
    }

    // check that list of data spaces matches those accessed in statements
    if (dataSpaces != dataSpacesActuallyAccessed) {
        return false;
    }

    return true;
}

void Computation::clear() {
    dataSpaces.clear();
    stmtsInfoMap.clear();
}

/* Stmt */

Stmt::Stmt(std::string stmtSourceCode, std::string iterationSpaceStr,
           std::string executionScheduleStr,
           std::vector<std::pair<std::string, std::string>> dataReadsStrs,
           std::vector<std::pair<std::string, std::string>> dataWritesStrs) {
    setStmtSourceCode(stmtSourceCode);
    setIterationSpace(iterationSpaceStr);
    setExecutionSchedule(executionScheduleStr);
    setDataReads(dataReadsStrs);
    setDataWrites(dataWritesStrs);
};

Stmt::Stmt(const Stmt& other) { *this = other; }

Stmt& Stmt::operator=(const Stmt& other) {
    this->stmtSourceCode = other.stmtSourceCode;
    this->iterationSpace = std::unique_ptr<Set>(new Set(*other.iterationSpace));
    this->executionSchedule =
        std::unique_ptr<Relation>(new Relation(*other.executionSchedule));
    for (const auto& readInfo : other.dataReads) {
        this->dataReads.push_back(
            {readInfo.first,
             std::unique_ptr<Relation>(new Relation(*readInfo.second))});
    }
    for (const auto& writeInfo : other.dataWrites) {
        this->dataWrites.push_back(
            {writeInfo.first,
             std::unique_ptr<Relation>(new Relation(*writeInfo.second))});
    }
}

bool Stmt::isComplete() const {
    return !stmtSourceCode.empty() && iterationSpace && executionSchedule;
}

std::string Stmt::getStmtSourceCode() const { return stmtSourceCode; }

void Stmt::setStmtSourceCode(std::string newStmtSourceCode) {
    this->stmtSourceCode = newStmtSourceCode;
}

Set* Stmt::getIterationSpace() const { return iterationSpace.get(); }

void Stmt::setIterationSpace(std::string newIterationSpaceStr) {
    this->iterationSpace = std::unique_ptr<Set>(new Set(newIterationSpaceStr));
}

Relation* Stmt::getExecutionSchedule() const { return executionSchedule.get(); }

void Stmt::setExecutionSchedule(std::string newExecutionScheduleStr) {
    this->executionSchedule =
        std::unique_ptr<Relation>(new Relation(newExecutionScheduleStr));
}

std::vector<std::pair<std::string, Relation*>> Stmt::getDataReads() const {
    std::vector<std::pair<std::string, Relation*>> result;
    for (const auto& readInfo : dataReads) {
        result.push_back({readInfo.first, readInfo.second.get()});
    }
    return result;
}

void Stmt::setDataReads(
    std::vector<std::pair<std::string, std::string>> dataReadsStrs) {
    dataReads.clear();
    for (const auto& readInfo : dataReadsStrs) {
        dataReads.push_back(
            {readInfo.first,
             std::unique_ptr<Relation>(new Relation(readInfo.second))});
    }
}

std::vector<std::pair<std::string, Relation*>> Stmt::getDataWrites() const {
    std::vector<std::pair<std::string, Relation*>> result;
    for (const auto& writeInfo : dataWrites) {
        result.push_back({writeInfo.first, writeInfo.second.get()});
    }
    return result;
}

void Stmt::setDataWrites(
    std::vector<std::pair<std::string, std::string>> dataWritesStrs) {
    dataWrites.clear();
    for (const auto& writeInfo : dataWritesStrs) {
        dataWrites.push_back(
            {writeInfo.first,
             std::unique_ptr<Relation>(new Relation(writeInfo.second))});
    }
}

}  // namespace iegenlib
