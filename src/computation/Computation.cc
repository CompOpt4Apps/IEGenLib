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
        stmts << it.first << ": " << it.second.stmtSourceCode << "\n";
        iterSpaces << it.first << ": "
                   << it.second.iterationSpace->prettyPrintString() << "\n";
        execSchedules << it.first << ": "
                      << it.second.executionSchedule->prettyPrintString()
                      << "\n";
        dataReads << it.first << ":";
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
        dataWrites << it.first << ":";
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

/* StmtInfo */

StmtInfo::StmtInfo(
    std::string stmtSourceCode, std::string iterationSpaceStr,
    std::string executionScheduleStr,
    std::vector<std::pair<std::string, std::string>> dataReadsStrs,
    std::vector<std::pair<std::string, std::string>> dataWritesStrs)
    : stmtSourceCode(stmtSourceCode) {
    iterationSpace = std::make_unique<Set>(iterationSpaceStr);
    executionSchedule =
        std::make_unique<Relation>(executionScheduleStr);
    for (const auto& readInfo : dataReadsStrs) {
        dataReads.push_back(
            {readInfo.first,
             std::make_unique<Relation>(readInfo.second)});
    }
        for (const auto& writeInfo : dataWritesStrs) {
            dataWrites.push_back(
                {writeInfo.first,
                 std::make_unique<Relation>(writeInfo.second)});
        }
};

}  // namespace iegenlib
