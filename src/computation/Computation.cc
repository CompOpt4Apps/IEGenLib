/*!
 * \file Computation.cc
 *
 * \brief Implementation of the Computation and StmtInfo classes.
 *
 * The Computation struct is the SPF representation of a logical computation.
 * It contains a StmtInfo struct for each statement, which in turn contains
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

void Computation::printInfo() {
    std::ostringstream stmts;
    std::ostringstream iterSpaces;
    std::ostringstream execSchedules;
    std::ostringstream dataReads;
    std::ostringstream dataWrites;
    std::ostringstream dataSpacesOutput;

    for (const auto& it : stmtsInfoMap) {
        std::string stmtName = "S" + std::to_string(it.first);
        stmts << stmtName << ": " << it.second.stmtSourceCode << "\n";
        iterSpaces << stmtName << ": "
                   << it.second.iterationSpace->prettyPrintString() << "\n";
        execSchedules << stmtName << ": "
                      << it.second.executionSchedule->prettyPrintString()
                      << "\n";
        dataReads << stmtName << ":";
        if (it.second.dataReads.empty()) {
            dataReads << " none";
        } else {
            dataReads << "{\n";
            for (const auto& read_it : it.second.dataReads) {
                dataReads << "    " << read_it.first << ": "
                          << read_it.second->prettyPrintString() << "\n";
            }
            dataReads << "}";
        }
        dataReads << "\n";
        dataWrites << stmtName << ":";
        if (it.second.dataWrites.empty()) {
            dataWrites << " none";
        } else {
            dataWrites << "{\n";
            for (const auto& write_it : it.second.dataWrites) {
                dataWrites << "    " << write_it.first << ": "
                           << write_it.second->prettyPrintString() << "\n";
            }
            dataWrites << "}";
        }
        dataWrites << "\n";
    }
    dataSpacesOutput << "{";
    for (const auto& it : dataSpaces) {
        if (it != *dataSpaces.begin()) {
            dataSpacesOutput << ", ";
        }
        dataSpacesOutput << it;
    }
    dataSpacesOutput << "}\n";

    std::cout << "Statements:\n" << stmts.str();
    std::cout << "\nIteration spaces:\n" << iterSpaces.str();
    std::cout << "\nExecution schedules:\n" << execSchedules.str();
    std::cout << "\nData spaces: " << dataSpacesOutput.str();
    std::cout << "\nArray reads:\n" << dataReads.str();
    std::cout << "\nArray writes:\n" << dataWrites.str();
    std::cout << "\n";
}

void Computation::clear() {
    dataSpaces.clear();
    stmtsInfoMap.clear();
}

std::unordered_set<std::string> Computation::getDataSpaces() {
    return dataSpaces;
}

std::string Computation::getStmtSource(unsigned int stmtNumber) {
    return stmtsInfoMap.at(stmtNumber).stmtSourceCode;
}

void Computation::setStmtSource(unsigned int stmtNumber, std::string source) {
    stmtsInfoMap.at(stmtNumber).stmtSourceCode = source;
}

std::string Computation::getIterSpace(unsigned int stmtNumber) {
    return stmtsInfoMap.at(stmtNumber).iterationSpace->prettyPrintString();
}

void Computation::setIterSpace(unsigned int stmtNumber,
                               std::string newIterationSpaceStr) {
    stmtsInfoMap.at(stmtNumber).iterationSpace =
        std::unique_ptr<Set>(new Set(newIterationSpaceStr));
}

std::string Computation::getExecSched(unsigned int stmtNumber) {
    return stmtsInfoMap.at(stmtNumber).executionSchedule->prettyPrintString();
}

void Computation::setExecSched(unsigned int stmtNumber,
                               std::string newExecutionScheduleStr) {
    stmtsInfoMap.at(stmtNumber).executionSchedule =
        std::unique_ptr<Relation>(new Relation(newExecutionScheduleStr));
}

std::vector<std::pair<std::string, std::string>> Computation::getDataReads(
    unsigned int stmtNumber) {
    std::vector<std::pair<std::string, std::string>> result;
    for (const auto& readInfo : stmtsInfoMap.at(stmtNumber).dataReads) {
        result.push_back(
            {readInfo.first, readInfo.second->prettyPrintString()});
    }
    return result;
}

void Computation::addDataRead(unsigned int stmtNumber, unsigned int index,
                              std::string dataSpace, std::string readRelStr) {
    dataSpaces.emplace(dataSpace);
    stmtsInfoMap.at(stmtNumber)
        .dataReads.insert(
            stmtsInfoMap.at(stmtNumber).dataReads.begin() + index,
            {dataSpace, std::unique_ptr<Relation>(new Relation(readRelStr))});
}

void Computation::removeDataRead(unsigned int stmtNumber, unsigned int index) {
    stmtsInfoMap.at(stmtNumber)
        .dataReads.erase(stmtsInfoMap.at(stmtNumber).dataReads.begin() + index);
}

std::vector<std::pair<std::string, std::string>> Computation::getDataWrites(
    unsigned int stmtNumber) {
    std::vector<std::pair<std::string, std::string>> result;
    for (const auto& writeInfo : stmtsInfoMap.at(stmtNumber).dataWrites) {
        result.push_back(
            {writeInfo.first, writeInfo.second->prettyPrintString()});
    }
    return result;
}

void Computation::addDataWrite(unsigned int stmtNumber, unsigned int index,
                               std::string dataSpace, std::string writeRelStr) {
    dataSpaces.emplace(dataSpace);
    stmtsInfoMap.at(stmtNumber)
        .dataWrites.insert(
            stmtsInfoMap.at(stmtNumber).dataWrites.begin() + index,
            {dataSpace, std::unique_ptr<Relation>(new Relation(writeRelStr))});
}

void Computation::removeDataWrite(unsigned int stmtNumber, unsigned int index) {
    stmtsInfoMap.at(stmtNumber)
        .dataWrites.erase(stmtsInfoMap.at(stmtNumber).dataWrites.begin() +
                          index);
}

/* StmtInfo */

StmtInfo::StmtInfo(
    std::string stmtSourceCode, std::string iterationSpaceStr,
    std::string executionScheduleStr,
    std::vector<std::pair<std::string, std::string>> dataReadsStrs,
    std::vector<std::pair<std::string, std::string>> dataWritesStrs)
    : stmtSourceCode(stmtSourceCode) {
    iterationSpace = std::unique_ptr<Set>(new Set(iterationSpaceStr));
    executionSchedule =
        std::unique_ptr<Relation>(new Relation(executionScheduleStr));
    for (const auto& readInfo : dataReadsStrs) {
        dataReads.push_back(
            {readInfo.first,
             std::unique_ptr<Relation>(new Relation(readInfo.second))});
    }
    for (const auto& writeInfo : dataWritesStrs) {
        dataWrites.push_back(
            {writeInfo.first,
             std::unique_ptr<Relation>(new Relation(writeInfo.second))});
    }
};

}  // namespace iegenlib
