/*!
 * \file Computation.cc
 *
 * \brief Implementation of the Computation and Stmt classes, and supporting
 * classes.
 *
 * The Computation class is the SPF representation of a logical computation.
 * It contains a Stmt class for each statement, which in turn contains
 * information about that statement as mathematical objects.
 * Originally part of spf-ie.
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

#include "Computation.h"
#include "util.h"

#include <code_gen/parser/parser.h>
#include <codegen.h>


#include <algorithm>
#include <set>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>
#include <map>

#include "set_relation/set_relation.h"

//! Base string for use in name prefixing
#define NAME_PREFIX_BASE "_iegen_"

//! String delimiter expected on either side of data space names
#define DATA_SPACE_DELIMITER "$"

//! String added when renaming dataspace to avoid name conflicts
#define DATA_RENAME_STR "__w__"

namespace iegenlib {

/* Computation */
unsigned int Computation::numRenames = 0;
unsigned int Computation::dataRenameCnt = 0;

Computation::Computation() {}

Computation::~Computation() {
    clear();
}

Computation::Computation(const Computation& other) { *this = other; }

Computation& Computation::operator=(const Computation& other) {
    this->stmts = other.stmts;
    this->dataSpaces = other.dataSpaces;
    this->parameters = other.parameters;
    this->returnValues = other.returnValues;
    this->transformationLists = other.transformationLists;

    return *this;
}

bool Computation::operator==(const Computation& other) const {
    return (this->stmts == other.stmts &&
            this->dataSpaces == other.dataSpaces &&
            this->parameters == other.parameters &&
            this->returnValues == other.returnValues &&
            this->transformationLists == other.transformationLists);
}

Computation* Computation::getUniquelyNamedClone() const {
    std::string namePrefix = NAME_PREFIX_BASE + std::to_string(numRenames++);
    Computation* prefixedCopy = new Computation();

    // prefix all data in the Computation and insert it to the new one
    for (auto& stmt : this->stmts) {
        prefixedCopy->addStmt(stmt->getUniquelyNamedClone(namePrefix, this->getDataSpaces()));
    }
    for (auto& space : this->dataSpaces) {
        prefixedCopy->addDataSpace(getPrefixedDataSpaceName(space, namePrefix));
    }
    for (auto& param : this->parameters) {
        prefixedCopy->addParameter(getPrefixedDataSpaceName(param.first, namePrefix), param.second);
    }
    for (auto& retVal : this->returnValues) {
        // only prefix values that are data space names, avoid trying to prefix
        // literals
        prefixedCopy->addReturnValue(
            (retVal.second ? getPrefixedDataSpaceName(retVal.first, namePrefix) : retVal.first), retVal.second);
    }

    return prefixedCopy;
}

void Computation::resetNumRenames() {
    Computation::numRenames = 0;
}

std::string Computation::getPrefixedDataSpaceName(const std::string& originalName, const std::string& prefix) {
    return DATA_SPACE_DELIMITER + prefix + originalName.substr(1);
}

void Computation::updateDataSpaceVersions(Stmt* stmt) {
    std::vector<std::pair<std::string, std::string>> writes;
    for (int i = 0; i < stmt->getNumWrites(); i++) {
        std::string write = stmt->getWriteDataSpace(i);
        if (write == "" || write[0] != '$') { continue; }
        if (isWrittenTo(write) == -1) { continue; }

        std::string newWrite = getDataSpaceRename(write);
        writes.push_back({write, newWrite});

        addDataSpace(newWrite);

        // Rename in the reads of the new statement
        stmt->replaceDataSpaceReads(write, newWrite);
    }

    for (int i = getNumStmts() - 1; i >= 0; i--) {
        Stmt* currStmt = getStmt(i);
        for (auto it = writes.begin(); it != writes.end(); ++it) {
            bool foundWrite = false;
            for (int j = 0; j < currStmt->getNumWrites(); j++) {
                std::string write = currStmt->getWriteDataSpace(j);
                if (write.compare(it->first) == 0) { foundWrite = true; break; }
            }
            if (foundWrite) {
                // Rename in the writes of the current statement
                currStmt->replaceDataSpaceWrites(it->first, it->second);
                it = writes.erase(it);
                if (it == writes.end()) { break; }
                continue;
            }
            // Rename throughout the entire statment
            currStmt->replaceDataSpace(it->first, it->second);
        }
        if (writes.empty()) { break; }
    }
}

void Computation::updateDataDependencies(Stmt* stmt) {
    std::vector<std::string> reads;
    for (int i = 0; i < stmt->getNumReads(); i++) {
        reads.push_back(stmt->getReadDataSpace(i));
    }

    for (int i = getNumStmts() - 1; i >= 0; i--)  {
        Stmt* currStmt = getStmt(i);
        for (int j = 0; j < currStmt->getNumWrites(); j++) {
            std::string write = currStmt->getWriteDataSpace(j);
            for (auto it = reads.begin(); it != reads.end(); it++) {
                if (isRenameOf(write, *it) || isRenameOf(*it, write)) { std::cerr << write << " is rename of " << (*it) << std::endl;}
                if (write.compare(*it) == 0) {
                    currStmt->addOutDependency(getNumStmts());
                    stmt->addInDependency(i);
                    it = reads.erase(it);
                    if (it == reads.end()) { break; }
                }
            }
            if (reads.empty()) { break; }
        }
        if (reads.empty()) { break; }
    }
}


void Computation::addStmt(Stmt* stmt) {
    updateDataSpaceVersions(stmt);
    updateDataDependencies(stmt);
    stmts.push_back(stmt);
    transformationLists.push_back({});
}

Stmt* Computation::getStmt(unsigned int index) const { return stmts.at(index); }

unsigned int Computation::getNumStmts() const { return stmts.size(); }

void Computation::addDataSpace(std::string dataSpaceName) {
    assertValidDataSpaceName(dataSpaceName);
    dataSpaces.emplace(dataSpaceName);
}

std::unordered_set<std::string> Computation::getDataSpaces() const {
    return dataSpaces;
}

bool Computation::isDataSpace(std::string name) const {
    return dataSpaces.count(name) > 0;
}

void Computation::addParameter(std::string paramName, std::string paramType) {
    assertValidDataSpaceName(paramName);
    parameters.push_back({paramName, paramType});
    // parameters are automatically available as data spaces to the Computation
    addDataSpace(paramName);
}

std::string Computation::getParameterName(unsigned int index) const {
    return parameters.at(index).first;
}

std::string Computation::getParameterType(unsigned int index) const {
    return parameters.at(index).second;
}

bool Computation::isParameter(std::string dataSpaceName) const {
    auto pos = std::find_if(parameters.begin(), parameters.end(),
                            [&dataSpaceName](const std::pair<std::string, std::string>& param) {
                                return dataSpaceName.compare(param.first) == 0;
                            });
    return pos != parameters.end();
}

unsigned int Computation::getNumParams() const {
    return parameters.size();
}

void Computation::addReturnValue(std::string name) {
    returnValues.push_back({name, this->isDataSpace(name)});
}

void Computation::addReturnValue(std::string name, bool isDataSpace) {
    returnValues.push_back({name, isDataSpace});
}

std::vector<std::string> Computation::getReturnValues() const {
    std::vector<std::string> names;
    for (const auto& retVal : returnValues) {
        names.push_back(retVal.first);
    }
    return names;
}

bool Computation::isReturnValue(std::string dataSpaceName) const {
    auto pos = std::find_if(returnValues.begin(), returnValues.end(),
                            [&dataSpaceName](const std::pair<std::string, bool>& retVal) {
                                return dataSpaceName.compare(retVal.first) == 0;
                            });
    return pos != returnValues.end();
}

unsigned int Computation::getNumReturnValues() const {
    return returnValues.size();
}

void Computation::printInfo() const {
    std::ostringstream stmtsOutput;
    std::ostringstream iterSpacesOutput;
    std::ostringstream execSchedulesOutput;
    std::ostringstream dataReadsOutput;
    std::ostringstream dataWritesOutput;
    std::ostringstream dataSpacesOutput;

    int stmtNum = 0;
    for (const auto& it : stmts) {
        std::string stmtName = "S" + std::to_string(stmtNum);
        // stmt source code
        stmtsOutput << stmtName << ": " << (*it).getStmtSourceCode() << "\n";
        // iter spaces
        iterSpacesOutput << stmtName << ": "
                         << (*it).getIterationSpace()->prettyPrintString() << "\n";
        // exec schedules
        execSchedulesOutput << stmtName << ": "
                            << (*it).getExecutionSchedule()->prettyPrintString()
                            << "\n";
        // data reads
        unsigned int numReads = (*it).getNumReads();
        dataReadsOutput << stmtName << ":";
        if (numReads == 0) {
            dataReadsOutput << " none";
        } else {
            dataReadsOutput << "{\n";
            for (unsigned int i = 0; i < numReads; ++i) {
                dataReadsOutput << "    " << (*it).getReadDataSpace(i) << ": "
                                << (*it).getReadRelation(i)->prettyPrintString()
                                << "\n";
            }
            dataReadsOutput << "}";
        }
        dataReadsOutput << "\n";
        // data writes
        unsigned int numWrites = (*it).getNumWrites();
        dataWritesOutput << stmtName << ":";
        if (numWrites == 0) {
            dataWritesOutput << " none";
        } else {
            dataWritesOutput << "{\n";
            for (unsigned int i = 0; i < numWrites; ++i) {
                dataWritesOutput << "    " << (*it).getReadDataSpace(i) << ": "
                                 << (*it).getReadRelation(i)->prettyPrintString()
                                 << "\n";
            }
            dataWritesOutput << "}";
        }
        dataWritesOutput << "\n";
        stmtNum++;
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
    for (const auto& stmt : stmts) {
        // check completeness of each statement
        if (!stmt->isComplete()) {
            return false;
        }

        // collect all data space accesses
        for (unsigned int i = 0; i < stmt->getNumReads(); ++i) {
            dataSpacesActuallyAccessed.emplace(stmt->getReadDataSpace(i));
        }
        for (unsigned int i = 0; i < stmt->getNumWrites(); ++i) {
            dataSpacesActuallyAccessed.emplace(stmt->getWriteDataSpace(i));
        }
    }

    // check that list of data spaces matches those accessed in statements
    if (dataSpaces != dataSpacesActuallyAccessed) {
        return false;
    }

    return true;
}

void Computation::clear() {
    for (auto& stmt : stmts) {
        delete stmt;
    }
    stmts.clear();
    dataSpaces.clear();
}

AppendComputationResult Computation::appendComputation(
  const Computation* other, std::string surroundingIterDomainStr,
  std::string surroundingExecScheduleStr,
  const std::vector<std::string>& arguments){

    Set* surroundingIterDomain = new Set(surroundingIterDomainStr);
    Relation* surroundingExecSchedule = new Relation(surroundingExecScheduleStr);

    // create a working copy of the appendee
    Computation* toAppend = other->getUniquelyNamedClone();
    const unsigned int numArgs = arguments.size();

    // gather append context's information
    const TupleDecl surroundingExecTuple =
        surroundingExecSchedule->getTupleDecl();
    const unsigned int surroundingExecInArity =
        surroundingExecSchedule->inArity();
    const unsigned int surroundingExecOutArity =
        surroundingExecSchedule->outArity();
    if (surroundingExecSchedule->getNumConjuncts() != 1) {
        throw assert_exception(
            "Surrounding execution schedule should have exactly 1 "
            "Conjunction.");
    }
    const TupleDecl surroundingIterTuple =
        surroundingIterDomain->getTupleDecl();
    const int surroundingIterArity = surroundingIterDomain->arity();
    const bool surroundingContextHasIterators =
        !surroundingIterTuple.elemIsConst(0);
    if (surroundingIterDomain->getNumConjuncts() != 1) {
        throw assert_exception(
            "Surrounding iteration domain should have exactly 1 Conjunction.");
    }

    // ensure that arguments match parameter list length
    if (numArgs != toAppend->getNumParams()) {
        throw assert_exception(
            "Incorrect number of parameters specified for appendComputation -- "
            "expected " +
            std::to_string(toAppend->getNumParams()) + ", got " +
            std::to_string(numArgs));
    }

    // Insert declarations+assignment of (would-have-been if not for inlining)
    // parameter values, at the beginning of the appendee.
    // Assignment of a parameter i will be like:
    // [type of i] [name of i] = [name of argument i from passed-in list];
    // Insertion is done by prepending statements one at a time in reverse
    // order.

    // Keep track of the number of writes. 
    // Need this for the correct execution order
    int idx = 0;
    // Used to track number of statements not including parameter
    // declarations. Used for paramter reassignments
    int numStmts = toAppend->getNumStmts();
    for (int i = ((signed int)numArgs) - 1; i >= 0; --i) {
        std::string param = toAppend->getParameterName(i);
        // Get the first write to the parameter
        int firstWrite = toAppend->isWrittenTo(param);
        // Only rename the parameter if there is a first write
        std::string newParam = param;
        if (firstWrite != -1) {
            newParam = getDataSpaceRename(param);
            toAppend->addDataSpace(newParam);
        }

        Stmt* paramDeclStmt = new Stmt();

        paramDeclStmt->setStmtSourceCode(newParam + " = " + arguments[i] + ";");
        paramDeclStmt->setIterationSpace("{[0]}");
        paramDeclStmt->setExecutionSchedule("{[0]->[" + std::to_string(idx) +
                                            "]}");
        idx++;
        // If passed-in argument is a data space, mark it as being read
        // (otherwise it is a literal)
        if (this->isDataSpace(arguments[i])) {
            paramDeclStmt->addRead(arguments[i], "{[0]->[0]}");
        }
        paramDeclStmt->addWrite(newParam, "{[0]->[0]}");

        // Rename the parameter up to its first write in toAppend
        for (int j = 0; j < firstWrite; j++) {
            toAppend->getStmt(j)->replaceDataSpace(param, newParam);
        }
        // Rename in the reads of the first write
        if (firstWrite != -1) { toAppend->getStmt(firstWrite)->replaceDataSpaceReads(param, newParam); }

        // Add the statement
        toAppend->stmts.insert(toAppend->stmts.begin(), paramDeclStmt);

        // If the argument is active out, reassign the toAssign parameter to it
        if (toAppend->getParameterType(i).find("&") != std::string::npos) {
            Stmt* paramReassignStmt = new Stmt();
            paramReassignStmt->setStmtSourceCode(arguments[i] + " = " +
                                                 param + ";");
            paramReassignStmt->setIterationSpace("{[0]}");
            paramReassignStmt->setExecutionSchedule("{[0]->[" + std::to_string(numStmts++) +
                                            "]}");
            // If passed-in argument is a data space, mark it as being written
            // (otherwise it is a literal)
            if (this->isDataSpace(arguments[i])) {
                paramReassignStmt->addWrite(arguments[i], "{[0]->[0]}");
            }
            paramReassignStmt->addRead(param, "{[0]->[0]}");
            toAppend->addStmt(paramReassignStmt);
        }
    }

    // add (already name prefixed) data spaces from appendee to appender
    for (const auto& dataSpace : toAppend->getDataSpaces()) {
        this->addDataSpace(dataSpace);
    }

    // calculate indexes/offsets for execution tuple modifications
    // value to offset 0th position of appended schedule tuples by
    int offsetValue =
        surroundingExecTuple.elemConstVal(surroundingExecSchedule->arity() - 1);
    // Keep track of the latest execution schedule position used.
    int latestTupleValue = offsetValue;

    // construct and insert an adjusted version of each statement of appendee,
    // including parameter declaration statements
    unsigned int remainingParamDeclStmts = idx;
    bool processingOriginalStmts = false;
    for (unsigned int stmtNum = 0; stmtNum < toAppend->getNumStmts();
         ++stmtNum) {
        // once we've finished processing prepended parameter declaration
        // statements, increase the offset for remaining (original) statements
        // by the number of prepended statements
        if (!processingOriginalStmts) {
            if (remainingParamDeclStmts == 0) {
                offsetValue += idx;
                processingOriginalStmts = true;
            } else {
                remainingParamDeclStmts--;
            }
        }

        // statement to copy and modify from the appendee
        Stmt* appendeeStmt = toAppend->getStmt(stmtNum);
        // new statement built up from appendee which will be inserted
        Stmt* newStmt = new Stmt();

        /* Source code */
        newStmt->setStmtSourceCode(appendeeStmt->getStmtSourceCode());

        /* Iteration domain */
        // collect information about current iteration space
        const Set* appendIterSpace = appendeeStmt->getIterationSpace();
        const int appendIterArity = appendIterSpace->arity();
        TupleDecl appendIterTuple = appendIterSpace->getTupleDecl();

        // construct new iteration space
        Set* newIterSpace;
        if (!surroundingContextHasIterators) {
            // if iteration domain for surrounding context is a singleton, leave
            // append iteration domain as-is
            newIterSpace = new Set(*appendIterSpace);
        } else if (appendIterTuple.elemIsConst(0)) {
            // if iteration domain to append is a singleton, adopt surrounding
            // iteration domain
            newIterSpace = new Set(*surroundingIterDomain);
        } else {
            // if neither iteration domain is trivial, combine the two

            // collect pieces for a new iteration space
            // construct tuple with desired iterators after appending
            TupleDecl newIterTuple =
                TupleDecl(surroundingIterArity + appendIterArity);
            for (unsigned int i = 0; i < surroundingIterArity; ++i) {
                newIterTuple.copyTupleElem(surroundingIterTuple, i, i);
            }
            for (unsigned int i = 0; i < appendIterArity; ++i) {
                newIterTuple.copyTupleElem(appendIterTuple, i,
                                           surroundingIterArity + i);
            }

            // add constraints from surroundings
            Conjunction* newIterSpaceConj = new Conjunction(newIterTuple);
            newIterSpaceConj->copyConstraintsFrom(
                *surroundingIterDomain->conjunctionBegin());

            // remap constraints from appendee iteration space to fit new tuple,
            // then copy them in
            Conjunction* shiftedAppendIterSpaceConj =
                new Conjunction(**appendIterSpace->conjunctionBegin());
            std::vector<int> shiftAppendeeIters;
            for (unsigned int i = 0; i < appendIterArity; ++i) {
                shiftAppendeeIters.push_back(surroundingIterArity + i);
            }
            shiftedAppendIterSpaceConj->remapTupleVars(shiftAppendeeIters);
            newIterSpaceConj->copyConstraintsFrom(shiftedAppendIterSpaceConj);
            delete shiftedAppendIterSpaceConj;

            // construct the new iteration space
            newIterSpace = new Set(newIterTuple);
            newIterSpace->addConjunction(newIterSpaceConj);
        }
        newStmt->setIterationSpace(newIterSpace);

        /* Execution schedule */
        // original execution schedule for statement to be appended
        const Relation* appendExecSchedule = appendeeStmt->getExecutionSchedule();
        TupleDecl appendExecTuple = appendExecSchedule->getTupleDecl();
        int appendExecInArity = appendExecSchedule->inArity();
        int appendExecOutArity = appendExecSchedule->outArity();

        // construct new execution schedule tuple
        int newExecInArity = surroundingExecInArity + appendExecInArity;
        // Subtract space for '0' iterator placeholder, if present. Only counted
        // once because if neither one has real iterators, a '0' will be used.
        if (!surroundingContextHasIterators || appendExecTuple.elemIsConst(0)) {
            newExecInArity -= 1;
        }
        int newExecOutArity = surroundingExecSchedule->outArity() +
                              appendExecSchedule->outArity() - 1;
        TupleDecl newExecTuple = TupleDecl(newExecInArity + newExecOutArity);

        unsigned int currentTuplePos = 0;
        // insert iterators from surrounding context
        for (int i = 0; i < surroundingExecSchedule->inArity(); ++i) {
            // skip '0' iterator placeholder
            if (i == 0 && surroundingExecTuple.elemIsConst(0)) {
                continue;
            }
            newExecTuple.copyTupleElem(surroundingExecTuple, i,
                                       currentTuplePos++);
        }
        // insert iterators from appended statement
        for (int i = 0; i < appendExecInArity; ++i) {
            // skip '0' iterator placeholder
            if (i == 0 && appendExecTuple.elemIsConst(0)) {
                continue;
            }
            newExecTuple.copyTupleElem(appendExecTuple, i, currentTuplePos++);
        }
        // if neither surroundings nor appended stmt have iterators, insert the
        // placeholder '0' iterator
        if (currentTuplePos == 0) {
            newExecTuple.setTupleElem(currentTuplePos++, 0);
        }
        // insert surrounding schedule tuple elements except the last one, which
        // must be combined with first append tuple value
        for (int i = surroundingExecSchedule->inArity();
             i < surroundingExecSchedule->arity() - 1; ++i) {
            newExecTuple.copyTupleElem(surroundingExecTuple, i,
                                       currentTuplePos++);
        }
        // offset and insert first append tuple value
        latestTupleValue =
            appendExecTuple.elemConstVal(appendExecInArity) + offsetValue;
        newExecTuple.setTupleElem(currentTuplePos++, latestTupleValue);
        // insert remaining append tuple values
        for (int i = appendExecInArity + 1; i < appendExecTuple.size(); ++i) {
            newExecTuple.copyTupleElem(appendExecTuple, i, currentTuplePos++);
        }

        // create new execution schedule Relation using the new tuple
        Relation* newExecSchedule = new Relation(newExecInArity, newExecOutArity);
        Conjunction* newExecScheduleConj = new Conjunction(newExecTuple);
        newExecScheduleConj->setInArity(newExecInArity);
        // if there are iterators, add equality constraints between iterators in input and output tuples
        if (!newExecTuple.elemIsConst(0)) {
            for (unsigned int i = 0; i < newExecInArity; ++i) {
                Exp* equality = new Exp();
                equality->setEquality();
                equality->addTerm(new TupleVarTerm(1, i));
                equality->addTerm(new TupleVarTerm(-1, newExecInArity + i*2 + 1));
                newExecScheduleConj->addEquality(equality);
            }
        }
        newExecSchedule->addConjunction(newExecScheduleConj);
        newStmt->setExecutionSchedule(newExecSchedule);

        /* Data reads */
        for (unsigned int i = 0; i < appendeeStmt->getNumReads(); ++i) {
            Relation* appendeeReadRel = appendeeStmt->getReadRelation(i);
            Relation* newReadRel;
            // only need to adjust if there are iterators in surrounding context
            if (surroundingContextHasIterators) {
                const int oldAppendInArity = appendeeReadRel->inArity();
                const int oldAppendOutArity = appendeeReadRel->outArity();
                // shift appendee relation tuple to make room for new iterators
                int shiftDistance = surroundingIterArity;
                if (appendeeReadRel->getTupleDecl().elemIsConst(0)) {
                    // no need to shift out a 0, it can be written over
                    shiftDistance -= 1;
                }
                std::vector<int> shiftReadRel;
                for (unsigned int pos = 0; pos < appendeeReadRel->arity();
                     ++pos) {
                    shiftReadRel.push_back(shiftDistance + pos);
                }
                appendeeReadRel->remapTupleVars(shiftReadRel);
                appendeeReadRel->SetinArity(shiftDistance + oldAppendInArity);

                // insert new iterators into tuple
                TupleDecl shiftedAppendeeReadTuple =
                    appendeeReadRel->getTupleDecl();
                for (unsigned int pos = 0; pos < surroundingIterArity; ++pos) {
                    shiftedAppendeeReadTuple.copyTupleElem(surroundingIterTuple,
                                                           pos, pos);
                }

                // construct new read relation
                newReadRel = new Relation(oldAppendInArity + shiftDistance, oldAppendOutArity);
                Conjunction* newReadRelConj = new Conjunction(shiftedAppendeeReadTuple);
                newReadRelConj->setInArity(newReadRel->inArity());
                newReadRelConj->copyConstraintsFrom(*appendeeReadRel->conjunctionBegin());
                newReadRel->addConjunction(newReadRelConj);
            } else {
                newReadRel = new Relation(*appendeeReadRel);
            }

            // add the new read using the new Relation
            newStmt->addRead(appendeeStmt->getReadDataSpace(i), newReadRel);
        }


        /* Data writes */
        for (unsigned int i = 0; i < appendeeStmt->getNumWrites(); ++i) {
            Relation* appendeeWriteRel = appendeeStmt->getWriteRelation(i);
            Relation* newWriteRel;
            // only need to adjust if there are iterators in surrounding context
            if (surroundingContextHasIterators) {
                const int oldAppendInArity = appendeeWriteRel->inArity();
                const int oldAppendOutArity = appendeeWriteRel->outArity();
                // shift appendee relation tuple to make room for new iterators
                int shiftDistance = surroundingIterArity;
                if (appendeeWriteRel->getTupleDecl().elemIsConst(0)) {
                    // no need to shift out a 0, it can be written over
                    shiftDistance -= 1;
                }
                std::vector<int> shiftWriteRel;
                for (unsigned int pos = 0; pos < appendeeWriteRel->arity();
                     ++pos) {
                    shiftWriteRel.push_back(shiftDistance + pos);
                }
                appendeeWriteRel->remapTupleVars(shiftWriteRel);
                appendeeWriteRel->SetinArity(shiftDistance + oldAppendInArity);

                // insert new iterators into tuple
                TupleDecl shiftedAppendeeWriteTuple =
                    appendeeWriteRel->getTupleDecl();
                for (unsigned int pos = 0; pos < surroundingIterArity; ++pos) {
                    shiftedAppendeeWriteTuple.copyTupleElem(surroundingIterTuple,
                                                           pos, pos);
                }

                // construct new write relation
                newWriteRel = new Relation(oldAppendInArity + shiftDistance, oldAppendOutArity);
                Conjunction* newWriteRelConj = new Conjunction(shiftedAppendeeWriteTuple);
                newWriteRelConj->setInArity(newWriteRel->inArity());
                newWriteRelConj->copyConstraintsFrom(*appendeeWriteRel->conjunctionBegin());
                newWriteRel->addConjunction(newWriteRelConj);
            } else {
                newWriteRel = new Relation(*appendeeWriteRel);
            }

            // add the new write using the new Relation
            newStmt->addWrite(appendeeStmt->getWriteDataSpace(i), newWriteRel);
        }

        // add the adapted statement into this Computation
        this->addStmt(newStmt);
    }

    // collect append result information to return
    AppendComputationResult result;
    result.tuplePosition = latestTupleValue;
    result.returnValues = toAppend->getReturnValues();

    delete surroundingIterDomain;
    delete surroundingExecSchedule;
    delete toAppend;

    return result;
}

//! Function compares two statement pair 
//  and returns the true if a is lexicographically 
//  lower in order to b. 
bool Computation::activeStatementComparator(const std::pair<int,Set*>& a, 
		    const std::pair<int,Set*>& b){
    return (*a.second) < (*b.second);
}


//! Function reschudles statment s1 to come before 
//  statement s2
//  \param s1 First statement Id
//  \param s2 Second statement Id.
void Computation::reschedule(int s1, int s2){
    if (s1 >= stmts.size() || s1 < 0 || s2 >= stmts.size()
		    || s2 < 0){
        throw assert_exception("s1 & s2 must be in bounds ");
    }

    if (s1 == s2){
        throw assert_exception("s1 cannot be s2"); 
    }
    // newIS is a list of pairs of statement id
    // to their transformed spaces.
    std::vector<std::pair<int,Set*> > newIS;
    
    int i = 0;
    std::vector<Set*> transformedSpaces = applyTransformations();
    Set * s1Set = nullptr;
    Set * s2Set = nullptr;
    for(Set* set : transformedSpaces){
	newIS.push_back(std::make_pair(i,set));
	if(s1 == i){
	    s1Set = set;
	}
	if(s2 == i){
	    s2Set = set;
	}
        i++;
    }
    // Sort by lexicographical order
    std::sort(newIS.begin(),newIS.end(),
		    Computation::activeStatementComparator);
    
    //TODO: take another pass to correctly rename 
    //      variables in the algorithm. 

    int splitLevel = 0;
   
    TupleDecl s1Tuple =  s1Set->getTupleDecl();
    TupleDecl s2Tuple =  s2Set->getTupleDecl();
    int s1PVal = -1;
    int s2PVal = -1;
    for(i  = 0 ; i < s1Tuple.getSize(); i++){
        if (s1Tuple.elemToString(i,true)!=
                s2Tuple.elemToString(i,true)){
	    splitLevel = i;
            s1PVal = s1Tuple.elemConstVal(i);
	    s2PVal = s2Tuple.elemConstVal(i);  
	    break;
	}
    }

    // Create output and input tuple decl 
    // that will be used for reschedule 
    // transformations.
    std::string prefix = "t";
    TupleDecl newInputTuple (s1Tuple.size());
    TupleDecl newOutputTuple (s1Tuple.size());
    for(int j = 0; j < newInputTuple.size(); j++){
        newInputTuple.setTupleElem(j,prefix+std::to_string(j));
	std::string outTupleVar= prefix+std::to_string(j);
	// Use a different tuple variable for output
	// if it is the split level.
	// Example:
	// {[t0,t1,t2,t3] -> [t0,t1p,t2,t3]} 
	// t1p for a split level of 1
	if(j == splitLevel){
            outTupleVar+="p";
	}
	newOutputTuple.setTupleElem(j,outTupleVar);
    }
    
    
    int newS1PositionVal = s2Tuple.elemConstVal(splitLevel) - 1;
    if (newS1PositionVal < 0){ 
        newS1PositionVal  = 0;
    }

    // Create constraint for new position of S1
    std::string constraint = newOutputTuple.elemVarString(splitLevel)
	    +" + "+ newInputTuple.elemVarString(splitLevel)+ 
	     " - "+ std::to_string(s1PVal)+ " = "+
	     std::to_string(newS1PositionVal);  
    
    // Construct new reiation for s1
    std::string rString ="{"+ newInputTuple.toString(true,0,false) + "->"
			   +newOutputTuple.toString(true,0,false) +
			   +": "+constraint+" }";
    Relation * rS1 = 
	    new Relation(rString);
    // Adds s1's transformation
    addTransformation(s1,rS1); 

    // Now go through each statement and update 
    // statment's siblings at that level.
    // Should we add unit transformation to statements
    // not affected by reschedule operation.
    for(std::pair<int,Set*> p : newIS ){
        // Ignore s1 
	if (p.first == s1)
            continue;
        // Check if this statement is a sibling
	// to s1 and s2.
	if (splitLevel - 1 >= 0 && 
	    p.second->getTupleDecl().elemToString(splitLevel-1)!=
	    s1Tuple.elemToString(splitLevel-1)){
            continue;
        }
	
        int val = p.second->getTupleDecl().elemConstVal(splitLevel);     
	// if s1 is lexicagrphically less than s2
        if (s1PVal < s2PVal){
	    if (val <= newS1PositionVal && val > s1PVal){
	        constraint = newOutputTuple.elemVarString(splitLevel)
	 	        +" = "+ newInputTuple.elemVarString(splitLevel)+ 
		        " - 1";  
	        std::string rSiString =  
		        "{"+ newInputTuple.toString(true,0,false) + "->"
			    +newOutputTuple.toString(true,0,false) +
			    +": "+constraint+" }";
	        Relation* rSi = new Relation(rSiString); 
                addTransformation(p.first,rSi);
	    }
	} else{
	    // if s1 is lexicographically greater than s2
            if (val >= s2PVal && val < s1PVal){
	        constraint = newOutputTuple.elemVarString(splitLevel)
	 	        +" = "+ newInputTuple.elemVarString(splitLevel)+ 
		        " + 1";  
	        std::string rSiString =  
		        "{"+ newInputTuple.toString(true,0,false) + "->"
			    +newOutputTuple.toString(true,0,false) +
			    +": "+constraint+" }";
	        Relation* rSi = new Relation(rSiString); 
                addTransformation(p.first,rSi);
	        
	    }
	}	
    }
     
    
    for(Set* set : transformedSpaces){
        delete set;
    }
}



//! Function fuses two statements at some level 
//  in their execution schedule. In the resulting 
//  computation after fusion, S1 will be ordered before
//  S2, other statements at that level will be adjusted accordingly
//  so reflect the new changes.
//
//  \param s1        first statement id
//  \param s2        second statement id
//  \param fuseLevel fuseLevel at which to fuse S1 and S2 
//  
//
//  Example S0: {[0,i,0,j,0] | stuff}; S1:{[1,i,0,j,0] | stuff}
//  fuse(S0,S1,2);
//  Result
//  S0: {[0,i,0,j,0] | stuff}; S1:{[0,i,1,j,0] | stuff}
void Computation::fuse (int s1, int s2, int fuseLevel){
    if (s1 >= stmts.size() || s1 < 0 || s2 >= stmts.size()
		    || s2 < 0){
        throw assert_exception("s1 & s2 must be in bounds ");
    }

    if (s1 == s2){
        throw assert_exception("s1 cannot be s2"); 
    }
    // newIS is a list of pairs of statement id
    // to their transformed spaces.
    std::vector<std::pair<int,Set*> > newIS;
    
    int i = 0;
    std::vector<Set*> transformedSpaces = applyTransformations();
    Set * s1Set = nullptr;
    Set * s2Set = nullptr;
    for(Set* set : transformedSpaces){
	newIS.push_back(std::make_pair(i,set));
	if(s1 == i){
	    s1Set = set;
	}
	if(s2 == i){
	    s2Set = set;
	}
        i++;
    }
    // Sort by lexicographical order
    std::sort(newIS.begin(),newIS.end(),
		    Computation::activeStatementComparator);
    
    // Find Split level for S1 and s2 Set
    // A split point is the point where
    // S1 and S2 diverges
    // Example S0: {[1,i,0,j,0] | stuff}; S1:{[1,i,1,j,0] | stuff}
    // Split level is at 2
    //
    int splitLevel = 0;
    TupleDecl s1Tuple =  s1Set->getTupleDecl();
    TupleDecl s2Tuple =  s2Set->getTupleDecl();
    
    // Fuse level must be constant tuple variable.
    if(!s1Tuple.elemIsConst(fuseLevel) ||
		    !s2Tuple.elemIsConst(fuseLevel)){
        throw assert_exception("fuse level must be constant tuple"
			" variable in s1 and s2");
    }
    int s1PVal = -1;
    int s2PVal = -1;
    
    for(i  = 0 ; i < s1Tuple.getSize(); i++){
        if (s1Tuple.elemToString(i,true)!=
                s2Tuple.elemToString(i,true)){
	    splitLevel = i;
            s1PVal = s1Tuple.elemConstVal(i);
	    s2PVal = s2Tuple.elemConstVal(i);  
	    break;
	}
    }

    // If Split level is the same as the level
    // of fusion, then fusion is not possible 
    if (splitLevel == fuseLevel){
        throw assert_exception("s1 & s2 statements are fused.");
    }
    
     
    //Create input and output tuple declarations
    std::string prefix = "t";
    TupleDecl newInputTuple (s1Tuple.size());
    TupleDecl newOutputTuple (s1Tuple.size());
    
    TupleDecl s2newOutputTuple (s2Tuple.size());
    TupleDecl s2newInputTuple (s2Tuple.size());
    for(int j = 0; j < newInputTuple.size(); j++){
        newInputTuple.setTupleElem(j,prefix+std::to_string(j));
	s2newInputTuple.setTupleElem(j,prefix+std::to_string(j));
	std::string outTupleVar= prefix+std::to_string(j);
	// Use a different tuple variable for output
	// if it is the split level or level to be fused
	// at. The split level and level to be fused at
	// are important parts to make the transformation
	// work.
	// Example:
	// {[t0,t1,t2,t3] -> [t0,t1p,t2,t3p]} 
	// t1p for a split level of 1 and t3p 
	// for fusion level 3.
	if(j == splitLevel || j == fuseLevel){
            outTupleVar+="p";
	}
	newOutputTuple.setTupleElem(j,outTupleVar);
        if ( j >= splitLevel && j <= fuseLevel){
	    s2newOutputTuple.setTupleElem(j,prefix+std::to_string(j)+"p");
	}else{
	    s2newOutputTuple.setTupleElem(j,prefix+std::to_string(j));
	}
    }
    


    // Create new relation to fuse S2 with S1 
    // S2 will be ordered after S1 at fuse level
    
    // First constraint places S2 and S1 on the 
    // same split level. This involves transforming
    // S2 to have the same value of s1 at split level
    
    // S2 must have the same schedule with S1 from 
    // split level up until just before fuse level
    std::string constraint =  
	    s2newOutputTuple.elemVarString(splitLevel)
	    +" + "+ s2newInputTuple.elemVarString(splitLevel)+ 
	     " - "+ std::to_string(s2PVal)+ " = "+
	     std::to_string(s1PVal); 
    
    for(int tuplIndex = splitLevel+1; tuplIndex < fuseLevel; tuplIndex++){
        // Ignore non constant tuple variables
	if(!s2Tuple.elemIsConst(tuplIndex) || 
			!s1Tuple.elemIsConst(tuplIndex)){
	
            constraint += " && "+ s2newOutputTuple.elemVarString(tuplIndex)
	 	        +" = "+ s2newInputTuple.elemVarString(tuplIndex) ;
	    continue;
	}
	constraint+=" && "+ 
            s2newOutputTuple.elemVarString(tuplIndex)
	    +" + "+ s2newInputTuple.elemVarString(tuplIndex)+ 
	     " - "+ std::to_string(s2Tuple.elemConstVal(tuplIndex))
	     + " = "+
	     std::to_string(s1Tuple.elemConstVal(tuplIndex));
    }   
 
    // Second constraint places S2 just after S1 on 
    // fuse level. Note later we will have to update
    // all siblings of fuse level to reflect this 
    // insertion
    constraint+= " && "+ 
            s2newOutputTuple.elemVarString(fuseLevel)
	    +" + "+ s2newInputTuple.elemVarString(fuseLevel)+ 
	     " - "+ std::to_string(s2Tuple.elemConstVal(fuseLevel))
	     + " = "+
	     std::to_string(s1Tuple.elemConstVal(fuseLevel)) + " + 1" ;
    

    // Construct new reiation for s2
    std::string rString ="{"+ s2newInputTuple.toString(true,0,false) + "->"
			   +s2newOutputTuple.toString(true,0,false) +
			   +": "+constraint+" }";
    Relation * rS2 = 
	  new Relation(rString);
    // Adds s1's transformation
    addTransformation(s2,rS2); 
   
    // Now go through each statement and update 
    // statment's siblings at split level
    // Should we add unit transformation to statements
    // not affected by reschedule operation.
    for(std::pair<int,Set*> p : newIS ){
        // Check if this statement is a sibling
	// to s1 and s2.
	if (splitLevel - 1 >= 0 && 
	    p.second->getTupleDecl().elemToString(splitLevel-1)!=
	    s1Tuple.elemToString(splitLevel-1)){
            continue;
        }
	
        int val = p.second->getTupleDecl().elemConstVal(splitLevel);     
	
	// If we find another relation that has exactly
	// the same value on splitLevel, we ignore updating
	// siblings at split level. 	
	// Example
	// "{[0,t1,0,t2,0]:stuff}",
	// "{[0,t1,1,t2,0]:stuff}",
	// "{[1,t1,0,t2,0]:stuff}",
	// "{[1,t1,0,t2,1]:stuff}"
	//
	// Fusion of S3 with S1 will take out 
	// S1 and place it beofore S3. The problem
	// is that we cant decrement the tuple constant
	// of S2 and S3 so we need to ignore updates
	// to siblings in this case. The check below
	// ensures this corner case is considered.
	if (val == s2PVal && p.first != s2){
	    break;
	} 	

	// if s1 is lexicagrphically less than s2
        if (s1PVal < s2PVal){
	    if (val > s2PVal){
	        constraint = newOutputTuple.elemVarString(splitLevel)
	 	        +" = "+ newInputTuple.elemVarString(splitLevel)+ 
		        " - 1 && "+ newOutputTuple.elemVarString(fuseLevel)
	 	        +" = "+ newInputTuple.elemVarString(fuseLevel);  
	        std::string rSiString =  
		        "{"+ newInputTuple.toString(true,0,false) + "->"
			    +newOutputTuple.toString(true,0,false) +
			    +": "+constraint+" }";
	        
		Relation* rSi = new Relation(rSiString); 
                addTransformation(p.first,rSi);
	    }
	} else{
	    // if s1 is lexicographically greater than s2
            if (val > s2PVal && val <= s1PVal){
	        constraint = newOutputTuple.elemVarString(splitLevel)
	 	        +" = "+ newInputTuple.elemVarString(splitLevel)+ 
		        " - 1 && "+ newOutputTuple.elemVarString(fuseLevel)
	 	        +" = "+ newInputTuple.elemVarString(fuseLevel) ;  
	        std::string rSiString =  
		        "{"+ newInputTuple.toString(true,0,false) + "->"
			    +newOutputTuple.toString(true,0,false) +
			    +": "+constraint+" }";
	        Relation* rSi = new Relation(rSiString); 
                addTransformation(p.first,rSi);
	        
	    }
	}	
    }
    
    
    
    // Now go through each statement and update 
    // statment's siblings at fuse  level. This involves
    // transforming every statement after S1 
    for(std::pair<int,Set*> p : newIS ){
        // Ignore s1 
	if (p.first == s2)
            continue;
        // Check if this statement is a sibling
	// to s1 and s2.
	if (fuseLevel - 1 >= 0 && 
	    p.second->getTupleDecl().elemToString(fuseLevel-1)!=
	    s1Tuple.elemToString(fuseLevel-1)){
            continue;
        }
	
        int val = p.second->getTupleDecl().elemConstVal(fuseLevel);     
        if (val > s1PVal){
	        constraint = newOutputTuple.elemVarString(fuseLevel)
	 	        +" = "+ newInputTuple.elemVarString(fuseLevel)+ 
		        " + 1 && "+ newOutputTuple.elemVarString(splitLevel)
	 	        +" = "+ newInputTuple.elemVarString(splitLevel);  
	        std::string rSiString =  
		        "{"+ newInputTuple.toString(true,0,false) + "->"
			    +newOutputTuple.toString(true,0,false) +
			    +": "+constraint+" }";
	        Relation* rSi = new Relation(rSiString); 
                addTransformation(p.first,rSi);
        }
    }
    
    for(Set* set : transformedSpaces){
        delete set;
    }
}

void Computation::padExecutionSchedules() {
    // Get the max arity
    int maxArity = 0;
    for (int j = 0; j < getNumStmts(); j++) {
        int tmp = getStmt(j)->getExecutionSchedule()->outArity();
        if (tmp > maxArity) { maxArity = tmp; }
    }

    // Pads iteration spaces by padding the execution schedule
    // The execution schedule is applied during applyTransformations()
    // Generate composition relation of the form
    // '{[a0,a1,...,an] -> [a0,a1,...am] : an+1 = 0 && an+2 = 0 && ... && am = 0}'
    // where m > n
    std::vector<iegenlib::Relation*> compositions;
    for (int i = 1; i <= maxArity; i++) {
        std::stringstream ss;
        ss << "{[";
        for (int j = 0; j < i; j++) {
            ss << "a" << j;
            if (j != i - 1) { ss << ","; }
        }
        ss << "] -> [";
        for (int j = 0; j < maxArity; j++) {
            ss << "a" << j;
            if (j != maxArity - 1) { ss << ","; }
        }
        ss << "] : ";
        for (int j = i; j < maxArity; j++) {
            ss << "a" << j << " = 0";
            if (j != maxArity - 1) { ss << " && "; }
        }
        ss << "}";
        compositions.push_back(new Relation(ss.str()));
//        std::cerr << "Composition Relation " << i << ": "
//                  << compositions.back()->getString() << std::endl;
    }

    // Apply composition to generate new execution schedule for each statement
    for (int i = 0; i < getNumStmts(); i++) {
        Stmt* stmt = getStmt(i);
        Relation* sched = stmt->getExecutionSchedule();
        Relation* newSched = compositions[sched->outArity() - 1]->Compose(sched);
//        std::cerr << "New Execution Schedule " << i << ": "
//                  << newSched->getString() << std::endl;
        stmt->setExecutionSchedule(newSched);
    }
}

bool Computation::consistentSetArity(const std::vector<Set*>& sets) {
    if (sets.size() == 0) { return true; }
    int arity = sets[0]->arity();
    for (Set* set : sets) { if (set->arity() != arity) return false; }
    return true;
}

std::string Computation::toDotString2() {
    //TODO: Deal with disjunction of cunjunctions later.
    
    padExecutionSchedules();

    // newIS is a list of pairs of statement id
    // to their transformed spaces.
    std::vector<std::pair<int,Set*> > newIS;

    std::vector<Set*> transformedSpaces = applyTransformations();
    if (!consistentSetArity(transformedSpaces)) {
        std::cerr << "Iteration spaces do not have a consistent arity" << std::endl;
        std::cerr << "Aborting toDotString()" << std::endl;
        return "";
    }
     
    std::ostringstream edges;

    bool visited[getNumStmts()];
    for (int i = 0; i < getNumStmts(); i++) { visited[i] = false; }

    for (int i = 0; i < getNumStmts(); i++) {
        if (visited[i]) { continue; }

        std::queue<int> toVisit;
        toVisit.push(i);
        while (!toVisit.empty()) {
            int idx = toVisit.front();
            toVisit.pop();
            if (visited[idx]) { continue; }

            Stmt* stmt = getStmt(idx);
            std::vector<int> outDeps = stmt->getOutDependencies();
            // If we are only using the variable once, juust skip to its use statement
            while (outDeps.size() == 1) {
                int newIdx = outDeps[0];
                stmt = getStmt(newIdx);
                outDeps = stmt->getOutDependencies();
                std::cerr << "Unused: " << idx << std::endl;
                idx = newIdx;
            }

            // From the final use statement, back track to collect all backwards dependencies
            std::vector<int> inDeps;
            std::queue<int> depQueue;
            for (int dep : stmt->getInDependencies()) { depQueue.push(dep); }
            while (!depQueue.empty()) {
                int idx2 = depQueue.front();
                depQueue.pop();
                Stmt* depStmt = getStmt(idx2);
                // If it only has one write, ignore it and keep going back
                if (depStmt->getOutDependencies().size() == 1) {
                    visited[idx2] = true;
                    for (int dep : depStmt->getInDependencies()) {
                        depQueue.push(dep);
                    }
                // Otherwise recognize as a dependence
                } else { inDeps.push_back(idx2); }
            }

            for (int j : inDeps) { std::cerr << j << ", ";} std::cerr << std::endl;
            // Remove duplicate dependencies
            std::sort(inDeps.begin(), inDeps.end());
            inDeps.erase(std::unique(inDeps.begin(), inDeps.end()), inDeps.end());
            for (int j : inDeps) { std::cerr << j << ", ";} std::cerr << std::endl;

            Set* sched = transformedSpaces[idx];
            newIS.push_back(std::make_pair(idx, sched));
    
            // Collect all writes
            std::string color;
            std::stringstream label;
            label << sched->prettyPrintString() << "\\n ";
            for (int j = 0; j < stmt->getNumWrites(); j++) {
                std::string write = stmt->getWriteDataSpace(j);
                if (j > 0) { label << ", "; }
                label << write;
                color = getDataSpaceDotColor(write);
            }

/*            edges << "S" << idx
                      << "[" << generateDotLabel(label.str())
                      << "][shape=Mrecord][style=bold]  [color="
                      << color << "];\n";*/

/*            size_t start_pos = getStmt(i)
                                   ->getReadRelation(data_read_index)
                                   ->getString()
                                   .rfind("[");
            size_t end_pos = getStmt(i)
                                 ->getReadRelation(data_read_index)
                                 ->getString()
                                 .rfind("]");*/
    
            // Connect each backwards dependency to this statement
            for (int dep : inDeps) {
                edges << "S" << dep << "->" << "S" << idx
                   << "[" << generateDotLabel("0")
/*                          stmt
                          ->getReadRelation(data_read_index)
                          ->getString()
                          .substr(start_pos + 1, end_pos - start_pos - 1))*/
                   << "][color=" << color << "]\n";
            }
            std::cerr << "Used: " << idx << std::endl;
            visited[idx] = true;

            // Add our next statements to visit
            for (int dep : outDeps) { toVisit.push(dep); }
        }
    }

    // Sort by lexicographical order
    std::sort(newIS.begin(),newIS.end(),
		    Computation::activeStatementComparator);  
    
    int maxLevel = newIS[0].second->arity();
    std::vector<std::vector<Set*>> projectedIS(maxLevel);
    
    for(int i = 0 ; i < maxLevel; i++ ){
        projectedIS[i] = std::vector<Set*>(newIS.size());
    }
   
    //TODO: Move deleting of active statement's set pointers to
    //      toDotScan. This is because overlapping 
    //      statements might result to new disjoint
    //      active iteration spaces of the same statement.
    //      and these disjoint active statements cannot
    //      be seen at this point. Or use unique pointers
    //      to help delete when sets get out of scope. 
    //      Easier way. :) 
    for(int i = 0; i < newIS.size(); i++){
        if (maxLevel > 0)
            // Replace all '$' because iegenlib throw a fit
            projectedIS[maxLevel-1][i] = new Set(replaceInString(newIS[i].second->getString(), "$", ""));
	//Perform projections for each column
	for (int j = maxLevel -1; j >= 1 ; j--) {
	    projectedIS[j -1][i] = projectedIS[j][i]->projectOut(j);   
       	}
    }

    // Generates Nodes
    std::ostringstream ss;
    ss << "digraph dataFlowGraph_1{ \n";
    toDotScan2(newIS,0,ss,projectedIS);
    // Added edges
    ss << edges.str() << "}";

    for(int j = 0 ; j < maxLevel ; j++)
	for (int k = 0 ; k <  newIS.size(); k++)
	    delete projectedIS[j][k];
    for(Set* set : transformedSpaces){
        delete set;
    }

    return ss.str();
}

//! Function returns a dot string representing nesting
//  and loop carrie dependency. Internally it uses
//  a lite version of polyhedral scanning to generate
//  subgraphs in the dot file.
//
std::string Computation::toDotString(){
    //TODO: Deal with disjunction of cunjunctions later.
    
    padExecutionSchedules();

    // newIS is a list of pairs of statement id
    // to their transformed spaces.
    std::vector<std::pair<int,Set*> > newIS;

    int i = 0;
    std::vector<Set*> transformedSpaces = applyTransformations();
    if (!consistentSetArity(transformedSpaces)) {
        std::cerr << "Iteration spaces do not have a consistent arity" << std::endl;
        std::cerr << "Aborting toDotString()" << std::endl;
        return "";
    }
    for(Set* set : transformedSpaces){
	newIS.push_back(std::make_pair(i,set));
        i++;
    }

    // Sort by lexicographical order
    std::sort(newIS.begin(),newIS.end(),
		    Computation::activeStatementComparator);
     i = 0;
    for (auto thing : newIS) {
        if (i != thing.first) { std::cerr << i << " " << thing.first << std::endl; }
        i++;
    }
   
    int maxLevel = newIS[0].second->arity();
    std::vector<std::vector<Set*> > projectedIS(maxLevel);
    
    for(int i = 0 ; i < maxLevel; i++ ){
        projectedIS[i] = std::vector<Set*>(newIS.size());
    }
    //TODO: Move deleting of active statement's set pointers to
    //      toDotScan. This is because overlapping 
    //      statements might result to new disjoint
    //      active iteration spaces of the same statement.
    //      and these disjoint active statements cannot
    //      be seen at this point. Or use unique pointers
    //      to help delete when sets get out of scope. 
    //      Easier way. :) 
    for(int i = 0; i < stmts.size(); i++){
        if (maxLevel > 0)
            // Replace all '$' because iegenlib throw a fit
            projectedIS[maxLevel-1][i] = new Set(replaceInString(newIS[i].second->getString(), "$", ""));
	//Perform projections for each column
	for (int j = maxLevel -1; j >= 1 ; j --) {
	    projectedIS[j -1][i] = projectedIS[j][i]->projectOut(j);   
       	}
    }
    std::ostringstream ss;
    ss << "digraph dataFlowGraph_1{ \n";
    toDotScan(newIS,0,ss,projectedIS);
    // Cleanup.
    /*
    for (int i = 0; i < stmts.size(); i++){
        delete newIS[i].second;
	for(int j = 0 ; j < maxLevel; j++)
            delete projectedIS[j][i];
    }*/
    // Code section from shivani for adding 
    // data nodes
    std::vector<string>
        data_spaces;  // Maintains the list of dataspaces already created

    // Adding the participating dataspaces for each statement and mapping out
    // the read and write access.
    for (int i = 0; i < getNumStmts(); i++) {
        int n = getStmt(i)->getNumWrites();
        if (n != 1) { std::cerr << "Num Writes: " << n << std::endl;
            std::cerr << "Stmt: " << getStmt(i)->getStmtSourceCode() << std::endl; }
        // Iterates over the read-DataSpaces
        for (int data_read_index = 0;
             data_read_index < getStmt(i)->getNumReads(); data_read_index++) {
            string readDataSpace =
                getStmt(i)->getReadDataSpace(data_read_index);
            // Set node color from data space type
            std::string color = isParameter(readDataSpace) ? "purple" :
                                isReturnValue(readDataSpace) ? "red" :
                                "grey";
            // Check to make sure the data space is not created if it already
            // exists
            if (!(std::count(data_spaces.begin(), data_spaces.end(),
                             readDataSpace))) {
                // Creates data space
                ss
                    << "\"" << readDataSpace << "\"["
                    << generateDotLabel(readDataSpace)
                    << "] [shape=box][style=bold][color=" << color << "];\n";

                data_spaces.push_back(readDataSpace);
            }

            size_t start_pos = getStmt(i)
                                   ->getReadRelation(data_read_index)
                                   ->getString()
                                   .rfind("[");
            size_t end_pos = getStmt(i)
                                 ->getReadRelation(data_read_index)
                                 ->getString()
                                 .rfind("]");

            ss << "\t\t\"" << readDataSpace << "\"->"
                    << "S" << i << "["
                    << generateDotLabel(
                           getStmt(i)
                           ->getReadRelation(data_read_index)
                           ->getString()
                           .substr(start_pos + 1, end_pos - start_pos - 1))
                    << "][color=" << color << "]"
                    << "\n";
        }

        // Iterates over the write-DataSpaces
        for (int data_write_index = 0;
             data_write_index < getStmt(i)->getNumWrites();
             data_write_index++) {
            string writeDataSpace =
                getStmt(i)->getWriteDataSpace(data_write_index);
            // Set node color from data space type
            std::string color = isParameter(writeDataSpace) ? "purple" :
                                isReturnValue(writeDataSpace) ? "red" :
                                "grey";
            // Check to make sure the data space is not created if it already
            // exists
            if (!(std::count(data_spaces.begin(), data_spaces.end(),
                             writeDataSpace))) {
                ss
                    << "\"" << writeDataSpace << "\"["
                    << generateDotLabel(writeDataSpace)
                    << "] [shape=box][style=bold][color=" << color << "];\n";

                data_spaces.push_back(writeDataSpace);
            }

            size_t start_pos = getStmt(i)
                                   ->getWriteRelation(data_write_index)
                                   ->getString()
                                   .rfind("[");
            size_t end_pos = getStmt(i)
                                 ->getWriteRelation(data_write_index)
                                 ->getString()
                                 .rfind("]");

            ss << "\t\t"
                    << "S" << i << "->\"" << writeDataSpace << "\"["
                    << generateDotLabel(
                           getStmt(i)
                           ->getWriteRelation(data_write_index)
                           ->getString()
                           .substr(start_pos + 1, end_pos - start_pos - 1))
                    << "][color=" << color << "]"
                    << "\n";
        }
    }

    ss << "}"; 
    // Clean up memory.
    for(int j = 0 ; j < maxLevel ; j++)
	for (int k = 0 ; k <  stmts.size(); k++)
	    delete projectedIS[j][k];
    for(Set* set : transformedSpaces){
        delete set;
    }
    return ss.str();
}

void Computation::addTransformation(unsigned int stmtIndex, Relation* rel) {
    transformationLists.at(stmtIndex).emplace_back(rel);
}

std::vector<Set*> Computation::applyTransformations() const {
    std::vector<Set*> transformedSchedules;
    for (int stmtNum = 0; stmtNum < this->getNumStmts(); ++stmtNum) {
        Stmt* currentStmt = getStmt(stmtNum);
        Set* schedule = currentStmt->getExecutionSchedule()->Apply(currentStmt->getIterationSpace());
        // apply transformations in order, chaining together outputs and inputs
        for (Relation* transformation : transformationLists.at(stmtNum)) {
            std::cerr << "Transformation " << transformation->getString() << std::endl;
            Set* scheduleAfterTransformation = transformation->Apply(schedule);
            delete schedule;
            schedule = scheduleAfterTransformation;
        }
        transformedSchedules.push_back(schedule);
    }
    return transformedSchedules;
}

//! param  activeStmt is assumed to be sorted lexicographically
std::vector<std::vector<std::pair<int,Set*> > > Computation::split
	(int level, std::vector<std::pair<int,Set*> >& activeStmt){
   std::map<std::string,std::vector<std::pair<int,Set*> > > grouping;
   
   for(std::pair<int,Set*> s : activeStmt){
      if(s.second->getTupleDecl().elemIsConst(level)){
          grouping[std::to_string(
			  s.second->getTupleDecl().elemConstVal(level))].push_back(s); 
	     	  
      }else {
	  // This will be expanded further to use constraints;
          grouping["t"].push_back(s);
      }  
   }
   std::vector<std::vector <std::pair<int,Set*> > > res;
   for( auto k : grouping){
       //Next iteration of the algorithm will be
       //focused on this section. 
       res.push_back(k.second);
   }
   return res;
}

// TODO: Alternative to cnt
static int cntr = 0;
void Computation::toDotScan2(std::vector<std::pair<int,Set*>> &activeStmts, int level,
	       std::ostringstream& ss ,
	       std::vector<std::vector<Set*> >&projectedIS){
    if(activeStmts.size() == 1){
        cntr++;
	std::string stmIter = activeStmts[0].second->prettyPrintString();

        Stmt* stmt = getStmt(activeStmts[0].first);
        std::string write = "No Write";
        if (stmt->getNumWrites() > 0) { write = stmt->getWriteDataSpace(0); }

        ss << "S" << activeStmts[0].first
           << "[" << generateDotLabel({stmIter, "\\n ",
              //getStmt(activeStmts[0].first)->getStmtSourceCode()})
              write})
           << "][shape=Mrecord][style=bold]  [color="
           << getDataSpaceDotColor(write) << "];\n";
        return;
    }
    std::vector<std::vector< std::pair <int, Set*> > > bins=
	    split(level,activeStmts);
    if(bins.size() > 1 && level > 0 && level <= projectedIS.size() ){
	std::string domainIter = projectedIS[level-1]
		[cntr]->prettyPrintString();

        ss << "subgraph cluster"<< level << " {\n"
           << "style = filled;\n"
           << " color = \"\";\n"
           << generateDotLabel({"Domain :", domainIter})
           << " \n";
    }

    for(auto active : bins){
        toDotScan(active,level+1,ss,projectedIS);
    }
    if(bins.size() > 1 && level > 0 && level <= projectedIS.size() ){
        ss << "}\n"; 
    }
}

//! Lite version of polyhedra scanning to generate 
//! toDot Clusters
void Computation::toDotScan(std::vector<std::pair<int,Set*>> &activeStmts, int level,
	       std::ostringstream& ss ,
	       std::vector<std::vector<Set*> >&projectedIS){
    if(activeStmts.size() == 1){
	std::string stmIter = activeStmts[0].second->prettyPrintString();
       
        ss << "S" << activeStmts[0].first
           << "[" << generateDotLabel({stmIter, "\\n ",
              //getStmt(activeStmts[0].first)->getStmtSourceCode()})
              "S", std::to_string(activeStmts[0].first)})
           << "][shape=Mrecord][style=bold]  [color=grey];\n";
        return;
    }
    std::vector<std::vector< std::pair <int, Set*> > > bins=
	    split(level,activeStmts);
    if(bins.size() > 1 && level > 0 && level <= projectedIS.size() ){
	std::string domainIter = projectedIS[level-1]
		[activeStmts[0].first]->prettyPrintString();

        ss << "subgraph cluster"<< level << " {\n"
           << "style = filled;\n"
           << " color = \"\";\n"
           << generateDotLabel({"Domain :", domainIter})
           << " \n";
    }

    for(auto active : bins){
        toDotScan(active,level+1,ss,projectedIS);
    }
    if(bins.size() > 1 && level > 0 && level <= projectedIS.size() ){
        ss << "}\n"; 
    }
}

std::string Computation::codeGen(Set* knownConstraints) {
    std::ostringstream generatedCode;
    std::vector<std::string> iterSpaces;
    std::vector<int> arity;

    // convert sets/relations to Omega format for use in codegen, and
    // collect statement macro definitions
    VisitorChangeUFsForOmega* vOmegaReplacer = new VisitorChangeUFsForOmega();
    std::ostringstream stmtMacroUndefs;
    std::ostringstream stmtMacroDefs;
    int stmtCount = 0;
    std::vector<Set*> newIS = applyTransformations();
    if (!consistentSetArity(newIS)) {
        std::cerr << "Iteration spaces do not have a consistent arity" << std::endl;
        std::cerr << "Aborting codeGen()" << std::endl;
        return "";
    }

    //Create a string for each statements iteration space
    for (const auto& stmt : stmts) {

	    // new Codegen would require an application
	    // be performed first before the set is sent
	    // to omega. This is a temporary solution to
	    // circumvent Omega's schedulling bug.
        Set * iterSpace = newIS[stmtCount];
	    iterSpace->acceptVisitor(vOmegaReplacer);
	    std::string tupleString =
            iterSpace->getTupleDecl().toString();
        // Stmt Macro:
        stmtMacroUndefs << "#undef s" << stmtCount << "\n";
        stmtMacroDefs << "#define s" << stmtCount << "(" << tupleString
                      << ")   "
                      << iegenlib::replaceInString(stmt->getStmtSourceCode(), DATA_SPACE_DELIMITER,
                                               "")
                      << " \n";
        stmtCount++;

        std::string omegaIterString =
            iterSpace->toOmegaString(vOmegaReplacer->getUFCallDecls());
        std::cout << "\nOmegaIterString: " << omegaIterString << std::endl;

        iterSpaces.push_back(omegaIterString);
        
        // Use identity transformation instead.
        //transforms.push_back(omega::Identity(iterSpace->arity()));
        arity.push_back(iterSpace->arity());
        
    	delete iterSpace;
    }

    // define necessary macros collected from statements
    std::ostringstream UFMacroUndefs;
    std::ostringstream UFMacroDefs;
    for (const auto& macro : *vOmegaReplacer->getUFMacros()) {
        UFMacroUndefs << "#undef " << macro.first << "\n";
        UFMacroDefs << "#define " << macro.first << " " << macro.second << "\n";
    }
    generatedCode << stmtMacroUndefs.str() << stmtMacroDefs.str() << "\n";
    generatedCode << UFMacroUndefs.str() << UFMacroDefs.str() << "\n";

    // initialize tuple variables
    std::ostringstream TupleAssignments;
    for(const auto& tupleA: vOmegaReplacer->getTupleAssignments()){
        TupleAssignments << "t" << (tupleA.first + 1) << " = "
		<< tupleA.second << "; \n";
    }
    generatedCode << TupleAssignments.str() << "\n";


    // convert set of known constraints to Omega format
    Set* modifiedKnown;
    if (knownConstraints) {
        modifiedKnown = new Set(*knownConstraints);
    } else {
        modifiedKnown = new Set("{}");
    }
    modifiedKnown->acceptVisitor(vOmegaReplacer);
    std::string omegaKnownString =
        modifiedKnown->toOmegaString(vOmegaReplacer->getUFCallDecls());
    delete modifiedKnown;

    delete vOmegaReplacer;

    generatedCode << omegaCodeGenFromString(arity, iterSpaces, omegaKnownString);

    // undefine macros, which are now extraneous
    generatedCode << stmtMacroUndefs.str() << UFMacroUndefs.str();

    return generatedCode.str();
}

//
std::string Computation::omegaCodeGenFromString(std::vector<int> relationArity, std::vector<std::string> iterSpacesStr, std::string known){
    std::ostringstream generatedCode;
    std::vector<omega::Relation> iterSpaces;
    std::vector<omega::Relation> transforms;
    
    for(int i=0; i<iterSpacesStr.size(); i++){
        std::string omegaIterString = iterSpacesStr[i];
        omega::Relation* omegaIterSpace =
            omega::parser::ParseRelation(iegenlib::replaceInString(omegaIterString, DATA_SPACE_DELIMITER, ""));

        iterSpaces.push_back(omega::copy(*omegaIterSpace));
        transforms.push_back(omega::Identity(relationArity[i]));
	    
        delete omegaIterSpace;
    } 
    
    omega::Relation* omegaKnown = omega::parser::ParseRelation(known);
    
    // do actual Omega CodeGen
    try {
        omega::CodeGen cg(transforms, iterSpaces, omega::copy(*omegaKnown));
        omega::CG_result* cgr = cg.buildAST();
        if (cgr) {
            generatedCode << cgr->printString() << "\n";
            delete cgr;
        } else {
            generatedCode << "/* empty */\n";
        }
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    delete omegaKnown;

    return generatedCode.str();
}


std::string Computation::toOmegaString() {
    std::ostringstream omegaString;

    // convert sets/relations to Omega format for use in codegen, and
    // collect statement macro definitions
    VisitorChangeUFsForOmega* vOmegaReplacer = new VisitorChangeUFsForOmega();
    int stmtCount = 0;
    for (const auto& stmt : stmts) {
        omegaString << "s" << stmtCount << "\n";
        omegaString << stmt->getStmtSourceCode() << "\n";
        stmtCount++;

	// new Codegen would require an application
	// be performed first before the set is sent
	// to omega. This is a temporary solution to
	// circumvent Omega's schedulling bug.
        Set * iterSpace = stmt->getExecutionSchedule()->
		Apply(stmt->getIterationSpace());
	iterSpace->acceptVisitor(vOmegaReplacer);


        std::string omegaIterString =
            iterSpace->toOmegaString(vOmegaReplacer->getUFCallDecls());
        omegaString << "Domain\n";
	omegaString << omegaIterString << "\n";

	delete iterSpace;
    }

    delete vOmegaReplacer;
    return omegaString.str();
}

bool Computation::assertValidDataSpaceName(const std::string& name) {
    if (!(name.length() >= 3 && std::string(1, name.front()) == DATA_SPACE_DELIMITER
            && std::string(1, name.back()) == DATA_SPACE_DELIMITER)){// || name.find('.') != std::string::npos) {
        std::stringstream msg;
        msg << "Data space names must be nonempty, surrounded in " << DATA_SPACE_DELIMITER
            << ", and not contain '.'\nError triggered for data space: " << name;
        throw assert_exception(msg.str());
    }
}

int Computation::isWrittenTo(std::string dataSpace){
    if (dataSpace == "") { return -1; }
    if (dataSpace[0] != '$') { dataSpace = "$"+dataSpace+"$"; }
 
    for(int i = 0; i < getNumStmts(); i++) {
        for(int data_write_index = 0; 
                data_write_index < getStmt(i)->getNumWrites(); 
                data_write_index++) {
            
            string writeDataSpace = getStmt(i)->getWriteDataSpace(data_write_index);
            if(dataSpace.compare(writeDataSpace) == 0){
                return i;
            }
        }
    }
    return -1;
}

void Computation::replaceDataSpaceName(std::string original, std::string newString){
    if (original == "") { return; }

    std::string searchString;
    std::string replacedString; 
    if(original.at(0) == '$'){
        searchString = original;
        replacedString = newString;
    }
    else{
        searchString = "$"+original+"$";
        replacedString = "$"+newString+"$";
    }
    for(int i = 0; i < getNumStmts(); i++) {
        getStmt(i)->replaceDataSpace(searchString, replacedString);
    }
    //This is being used with appendComputation so we don't need
    //to save a copy     
    dataSpaces.erase(searchString);
    
    //Rename return values as well
    for(auto it = returnValues.begin(); it != returnValues.end(); it++){
        std::string origReturnValue = (*it).first; 
        (*it).first = iegenlib::replaceInString(origReturnValue, original, newString);
    }
}

std::string Computation::getDataSpaceRename(std::string dataSpaceName) {
    if (dataSpaceName == "") { return ""; }
    dataSpaceName = trimDataSpaceName(dataSpaceName);
    return "$" + dataSpaceName + DATA_RENAME_STR + std::to_string(dataRenameCnt++) + "$";
}

std::string Computation::trimDataSpaceName(std::string dataSpace) {
    return dataSpace == "" || dataSpace[0] != '$' ? dataSpace :
           dataSpace.substr(1, dataSpace.length() - 2);
}

bool Computation::isRenameOf(std::string original, std::string rename) {
    original = trimDataSpaceName(original) + DATA_RENAME_STR;
    rename = trimDataSpaceName(rename);
    return rename.find(original) == 0; 
}

std::string Computation::getDataSpaceDotColor(std::string dataSpaceName) {
    return isParameter(dataSpaceName) ? "purple" : isReturnValue(dataSpaceName) ? "red" : "grey";
}

/* Stmt */

Stmt::~Stmt() {
    iterationSpace.reset();
    executionSchedule.reset();
    for (auto& read : dataReads) {
        read.second.reset();
    }
    dataReads.clear();
    for (auto& write : dataWrites) {
        write.second.reset();
    }
    dataWrites.clear();
}

bool Stmt::replaceDataSpaceReads(std::string searchString, std::string replacedString) {
    if (searchString == "") { return false; }
    if (searchString[0] != '$') { searchString = "$" + searchString + "$"; }
    if (replacedString != "" && replacedString[0] != '$') { replacedString = "$" + replacedString + "$"; }
   
    std::string oldSourceCode = getStmtSourceCode();
    std::stringstream newSourceCode;
    size_t eqPos = oldSourceCode.find("="), semiPos = oldSourceCode.find(";");
    while (eqPos != std::string::npos) {
        if (semiPos == std::string::npos) { semiPos = oldSourceCode.length() - 1; }
        newSourceCode << oldSourceCode.substr(0, eqPos)
                      << iegenlib::replaceInString(oldSourceCode.substr(eqPos, semiPos - eqPos + 1),
                                                   searchString, replacedString);
        oldSourceCode = oldSourceCode.substr(semiPos + 1);
        eqPos = oldSourceCode.find("=");
        semiPos = oldSourceCode.find(";");
    }
    newSourceCode << oldSourceCode;
    setStmtSourceCode(newSourceCode.str());

    for(auto& read: dataReads){
       if (read.first.compare(searchString) == 0) { read.first = replacedString; }
    }
    
    // TODO: should I do this?
    std::string iterSpaceStr = iterationSpace->prettyPrintString();
    std::string execScheduleStr = executionSchedule->prettyPrintString();

    iterSpaceStr = replaceInString(iterSpaceStr, searchString, replacedString);
    execScheduleStr = replaceInString(execScheduleStr, searchString, replacedString);

    // use modified strings to construct new values
    setIterationSpace(iterSpaceStr);
    setExecutionSchedule(execScheduleStr);

    return getStmtSourceCode().compare(newSourceCode.str()) != 0;
}

bool Stmt::replaceDataSpaceWrites(std::string searchString, std::string replacedString) {
    if (searchString == "") { return false; }
    if (searchString[0] != '$') { searchString = "$" + searchString + "$"; }
    if (replacedString != "" && replacedString[0] != '$') { replacedString = "$" + replacedString + "$"; }

    std::string oldSourceCode = getStmtSourceCode();
    std::stringstream newSourceCode;
    size_t eqPos = oldSourceCode.find("="), semiPos = oldSourceCode.find(";");
    while (eqPos != std::string::npos) {
        if (semiPos == std::string::npos) { semiPos = oldSourceCode.length() - 1; }
        newSourceCode << iegenlib::replaceInString(oldSourceCode.substr(0, eqPos),
                                                   searchString, replacedString)
                      << oldSourceCode.substr(eqPos, semiPos - eqPos + 1);
        oldSourceCode = oldSourceCode.substr(semiPos + 1);
        eqPos = oldSourceCode.find("=");
        semiPos = oldSourceCode.find(";");
    }
    newSourceCode << oldSourceCode;
    setStmtSourceCode(newSourceCode.str());

    for(auto& write: dataWrites){
        if (write.first.compare(searchString) == 0) { write.first = replacedString; }
    }

    // TODO: should I do this?
    std::string iterSpaceStr = iterationSpace->prettyPrintString();
    std::string execScheduleStr = executionSchedule->prettyPrintString();

    iterSpaceStr = replaceInString(iterSpaceStr, searchString, replacedString);
    execScheduleStr = replaceInString(execScheduleStr, searchString, replacedString);

    // use modified strings to construct new values
    setIterationSpace(iterSpaceStr);
    setExecutionSchedule(execScheduleStr);

    return getStmtSourceCode().compare(newSourceCode.str()) != 0;
}

void Stmt::replaceDataSpace(std::string searchString, std::string replacedString){
    if (searchString == "") { return; } 

    std::string oldSourceCode = getStmtSourceCode();
    std::string newSourceCode;
    newSourceCode = iegenlib::replaceInString(oldSourceCode, searchString, replacedString);
    setStmtSourceCode(newSourceCode);

    for(auto& write: dataWrites){
       write.first = iegenlib::replaceInString(write.first, searchString, replacedString);
    }
    for(auto& read: dataReads){
       read.first = iegenlib::replaceInString(read.first, searchString, replacedString);
    }

    std::string iterSpaceStr = iterationSpace->prettyPrintString();
    std::string execScheduleStr = executionSchedule->prettyPrintString();
    
    iterSpaceStr = replaceInString(iterSpaceStr, searchString, replacedString);
    execScheduleStr = replaceInString(execScheduleStr, searchString, replacedString);
    
    // use modified strings to construct new values
    setIterationSpace(iterSpaceStr);
    setExecutionSchedule(execScheduleStr);
}

Stmt::Stmt(std::string stmtSourceCode, std::string iterationSpaceStr,
           std::string executionScheduleStr,
           std::vector<std::pair<std::string, std::string>> dataReadsStrs,
           std::vector<std::pair<std::string, std::string>> dataWritesStrs) {
    setStmtSourceCode(stmtSourceCode);
    setIterationSpace(iterationSpaceStr);
    setExecutionSchedule(executionScheduleStr);
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
    return *this;
}

bool Stmt::operator==(const Stmt& other) const {
    // compare source code, iter space and exec schedule
    if (!(this->stmtSourceCode == other.stmtSourceCode &&
          *this->iterationSpace == *other.iterationSpace &&
          *this->executionSchedule == *other.executionSchedule)) {
        return false;
    }

    // compare data accesses, first by number then contents
    if (this->dataReads.size() != other.dataReads.size() ||
        this->dataWrites.size() != other.dataWrites.size()) {
        return false;
    }
    for (auto i = 0; i < this->dataReads.size(); ++i) {
        if (!(this->dataReads[i].first == other.dataReads[i].first &&
              *this->dataReads[i].second == *other.dataReads[i].second)) {
            return false;
        }
    }
    for (auto i = 0; i < this->dataWrites.size(); ++i) {
        if (!(this->dataWrites[i].first == other.dataWrites[i].first &&
              *this->dataWrites[i].second == *other.dataWrites[i].second)) {
            return false;
        }
    }

    return true;
}

Stmt* Stmt::getUniquelyNamedClone(const std::string& prefix, const std::unordered_set<std::string>& dataSpaceNames) const {
    Stmt* prefixedCopy = new Stmt(*this);

    // modify reads and writes
    for (auto& read : prefixedCopy->dataReads) {
        read.first = Computation::getPrefixedDataSpaceName(read.first, prefix);
    }
    for (auto& write : prefixedCopy->dataWrites) {
        write.first = Computation::getPrefixedDataSpaceName(write.first, prefix);
    }

    // replace data space names in statement source code, iteration space, execution schedule
    std::string srcCode = prefixedCopy->stmtSourceCode;
    std::string iterSpaceStr = prefixedCopy->iterationSpace->prettyPrintString();
    std::string execScheduleStr = prefixedCopy->executionSchedule->prettyPrintString();
    for (const string& originalName : dataSpaceNames) {
        srcCode = replaceInString(srcCode, originalName,
                                  Computation::getPrefixedDataSpaceName(originalName, prefix));
        iterSpaceStr = replaceInString(iterSpaceStr, originalName,
                                       Computation::getPrefixedDataSpaceName(originalName, prefix));
        execScheduleStr = replaceInString(execScheduleStr, originalName,
                                          Computation::getPrefixedDataSpaceName(originalName, prefix));
    }
    // use modified strings to construct new values
    prefixedCopy->setStmtSourceCode(srcCode);
    prefixedCopy->setIterationSpace(iterSpaceStr);
    prefixedCopy->setExecutionSchedule(execScheduleStr);

    return prefixedCopy;
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

void Stmt::setIterationSpace(Set* newIterationSpace) {
    this->iterationSpace = std::unique_ptr<Set>(newIterationSpace);
}

Relation* Stmt::getExecutionSchedule() const { return executionSchedule.get(); }

void Stmt::setExecutionSchedule(std::string newExecutionScheduleStr) {
    this->executionSchedule =
            std::unique_ptr<Relation>(new Relation(newExecutionScheduleStr));
}

void Stmt::setExecutionSchedule(Relation* newExecutionSchedule) {
    this->executionSchedule =
            std::unique_ptr<Relation>(newExecutionSchedule);
}

void Stmt::addRead(std::string dataSpace, std::string relationStr) {
    dataReads.push_back(
        {dataSpace, std::unique_ptr<Relation>(new Relation(relationStr))});
}

void Stmt::addRead(std::string dataSpace, Relation* relation) {
    dataReads.push_back(
        {dataSpace, std::unique_ptr<Relation>(relation)});
}

unsigned int Stmt::getNumReads() const { return dataReads.size(); }

std::string Stmt::getReadDataSpace(unsigned int index) const {
    return dataReads.at(index).first;
}

Relation* Stmt::getReadRelation(unsigned int index) const {
    return dataReads.at(index).second.get();
}

void Stmt::addWrite(std::string dataSpace, std::string relationStr) {
    dataWrites.push_back(
        {dataSpace, std::unique_ptr<Relation>(new Relation(relationStr))});
}

void Stmt::addWrite(std::string dataSpace, Relation* relation) {
    dataWrites.push_back(
        {dataSpace, std::unique_ptr<Relation>(relation)});
}

unsigned int Stmt::getNumWrites() const { return dataWrites.size(); }

std::string Stmt::getWriteDataSpace(unsigned int index) const {
    return dataWrites.at(index).first;
}

Relation* Stmt::getWriteRelation(unsigned int index) const {
    return dataWrites.at(index).second.get();
}

/* VisitorChangeUFsForOmega */

VisitorChangeUFsForOmega::VisitorChangeUFsForOmega() { reset(); }

VisitorChangeUFsForOmega::~VisitorChangeUFsForOmega() { reset(); }

void VisitorChangeUFsForOmega::reset() {
    ufCallDecls.clear();
    macros.clear();
    knownUFs.clear();
    nextFuncReplacementNumber = 0;
    currentTupleDecl = NULL;
}

void VisitorChangeUFsForOmega::prepareForNext() {
    ufCallDecls.clear();
    currentTupleDecl = NULL;
    // known UFs are preserved for reference in next
    // macros are accumulated over Visitor lifetime
}

std::map<std::string, std::string>* VisitorChangeUFsForOmega::getUFMacros() {
    return &macros;
}

std::set<std::string> VisitorChangeUFsForOmega::getUFCallDecls() {
    return ufCallDecls;
}


std::map<int,std::string>& VisitorChangeUFsForOmega::getTupleAssignments(){
    return tupleAssignments;
}



void VisitorChangeUFsForOmega::preVisitSparseConstraints(
    SparseConstraints* sc) {
    if (sc->getNumConjuncts() != 1) {
        throw assert_exception(
            "Must have exactly one conjunction for Omega conversion");
    }
}
void VisitorChangeUFsForOmega::preVisitConjunction(Conjunction* c){
    bool requireChange = false;
    std::string prefix = "__x";
    std::list<Exp*> newConstraints;
    TupleDecl decl = c->getTupleDecl();
    for(unsigned int i = 0; i <decl.getSize(); i++){
        if(decl.elemIsConst(i)){
	    requireChange = true;
	    decl.setTupleElem(i,prefix+std::to_string(i));
	    // Create constraint for replacement
	    TupleVarTerm* tupleTerm = new TupleVarTerm(i);
	    Term* constTerm = new Term(-decl.elemConstVal(i));
	    Exp * eqConstraint = new Exp();
	    eqConstraint->setEquality();
            eqConstraint->addTerm(tupleTerm);
            eqConstraint->addTerm(constTerm);
	    newConstraints.push_back(eqConstraint);
	}
    }

    // Change the conjunction at this point to the
    // new conjunction.
    if (requireChange){
        Conjunction* conj = new Conjunction(decl);
        conj->setInArity(c->inarity());
	for(auto it = newConstraints.begin();
		it!=newConstraints.end(); it++){
	    conj->addEquality(*it);
	}
        for (auto it = c->equalities().begin();
		it!=c->equalities().end(); it++){
	    conj->addEquality((*it)->clone());
	}

        for (auto it = c->inequalities().begin();
		it!=c->inequalities().end(); it++){
	    conj->addInequality((*it)->clone());
	}
	if(c->isUnsat()){
            conj->setUnsat();
	}
        // Copy contents of new conjunction to current
	// conjunction being visited.
	*c = *conj;
	delete conj;
    }
    currentTupleDecl = decl;
    // Initilize all tuple variable to zero
    for (unsigned int i = 0; i < decl.size(); i++ ){
       tupleAssignments[i] = "0";
    }
}
void VisitorChangeUFsForOmega::preVisitExp(iegenlib::Exp * e){
    // The goal of this code section is to generate tuple
    // variable intiializations. The code is currently buggy
    // and will be bypassed for now.
    /*
    // Make sure the expression does not have UF
    // and is equality.
    if(!e->hasIndexedUFCall() && e->isEquality()){
        std::cerr << "Analysing: " << e->toString() << "\n";
	// Check if expression has only one tuple variable.
	TupleVarTerm * term = NULL;
	bool isValid = true;
	std::list<Term*> terms = e->getTermList();
	// Count number of tuple variable terms in expression,
	// multiple tuple term in an expression is not considered
	// as a candidate for tuple initialization.
	int tv_count = std::count_if(terms.begin(), terms.end(),
	    [](Term* t){return dynamic_cast<TupleVarTerm*>(t)!=NULL;});
	if (tv_count != 1){
	    return;
	}
	for(auto it = terms.begin();!term && it != terms.end(); it++){
	   if((*it)->isUFCall()){
	       isValid = false;
	       break;
	   }
	   term = dynamic_cast<TupleVarTerm*>(*it);
	}
	// If this expression is valid for assignment
	// we go ahead.
	if(isValid && term){
	    std::cerr << "Is Valid: " << term->toString() << "\n" ;
	    if(terms.size()==1){
	        tupleAssignments[term->tvloc()] = "0";
	        return;
	    }
	    term->setCoefficient(1);
	    Exp * solveFor = e->solveForFactor(term);
	    std::string solvedForString = solveFor->toString();
	    tupleAssignments[term->tvloc()] =  solvedForString;
	    delete solveFor;
	}
    }*/
}


void VisitorChangeUFsForOmega::postVisitUFCallTerm(UFCallTerm* callTerm) {
    if (currentTupleDecl == NULL) {
        throw assert_exception(
            "No TupleDecl collected -- is this Visitor (incorrectly) being run "
            "on something other than a Set/Relation?");
    }

    // determine which tuple variables are needed in the call (how large of a
    // prefix)
    int max_tvloc = -1;
    for (unsigned int i = 0; i < callTerm->numArgs(); ++i) {
        // loop through all terms, processing as needed
        for (const auto& term : callTerm->getParamExp(i)->getTermList()) {
            if (term->isUFCall()) {
                throw assert_exception("Nested UF calls are not yet supported");
            } else if (term->type() == "TupleVarTerm") {
                TupleVarTerm* termAsTupleVar = static_cast<TupleVarTerm*>(term);
                max_tvloc = std::max(termAsTupleVar->tvloc(), max_tvloc);
            } else if (term->type() == "TupleExpTerm") {
                throw assert_exception("TupleExpTerm unsupported");
            }
        }
    }
    // ensure presence of at least one tuple var (UF calls cannot be
    // constant-only)
    if (max_tvloc == -1) {
        throw assert_exception(
            "Cannot make UF calls with only constant arguments");
    }
    // Create a ufMacro uf Array access          
    std::string ufMacro = callTerm->name();
    std::string ufArrayAccess = callTerm->name(); 
    ufMacro+= "(";
    bool isFirst = true;
    for (int i = 0; i < callTerm->numArgs(); i++){
           
	if(isFirst){
            ufMacro+= "t"+ std::to_string(i);
            isFirst = false;
	}else{
	    ufMacro += ",t"+std::to_string(i);
	}
        ufArrayAccess+="[t"+std::to_string(i)+"]";
    }
    ufMacro+=")";
    // Map ufs as macros to actual array aaccess.
    macros[ufMacro] = ufArrayAccess;
    // save original coefficient, then temporarily modify for printing
    int originalCoefficient = callTerm->coefficient();
    callTerm->setCoefficient(1);
    std::string originalCall = callTerm->toString();

    // rewrite argument list as a prefix of input tuple
    callTerm->resetNumArgs(max_tvloc + 1);
    for (int i = 0; i < callTerm->numArgs(); ++i) {
        Exp* newParamExp = new Exp();
        TupleVarTerm* tupleVarParam = new TupleVarTerm(i);
        newParamExp->addTerm(tupleVarParam);
        callTerm->setParamExp(i, newParamExp);
    }

    std::string replacementName;
    auto it = knownUFs.find(originalCall);
    // check if this particular UF invocation has already been encountered
    if (it != knownUFs.end()) {
        // use the function name from the already-existing definition
        replacementName = it->second;
        callTerm->setName(replacementName);
    } else {
        // assign a new name to this function and add it to our list
        replacementName =
            callTerm->name() + "_" + std::to_string(nextFuncReplacementNumber);
        nextFuncReplacementNumber++;
        knownUFs.emplace(originalCall, replacementName);

        callTerm->setName(replacementName);
        // this is a new UF, so add a macro definition for it
        macros.emplace(callTerm->toString(), originalCall);
    }
    // add UF call to the list of declarations
    ufCallDecls.emplace(callTerm->name() + "(" + std::to_string(max_tvloc + 1) +
                        ")");

    // restore coefficient, which was changed temporarily for printing
    callTerm->setCoefficient(originalCoefficient);
}

}  // namespace iegenlib
