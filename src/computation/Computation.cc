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
#include <regex>
#include <vector>
#include <map>

#include "set_relation/set_relation.h"

//! Base string for use in name prefixing
#define NAME_PREFIX_BASE "_iegen_"

//! String delimiter expected on either side of data space names
//! Must be something that will not appear in normal strings containing data spaces, so that it can be naively found/replaced.
#define DATA_SPACE_DELIMITER '$'

//! String appended to dataspaces to avoid name conflicts when SSA renaming
#define DATA_RENAME_STR "__w__"

//! Constraint used to test if constraints evaluate to false/true for phi nodes
#define CONSTR_TEST "test__"

//! String appended to arrays to create data spaces for constant accesses
#define ARR_ACCESS_STR "__at"


namespace iegenlib {

/* Computation */

unsigned int Computation::numComputationRenames = 0;

Computation::Computation(const std::string& name) {
  setName(name);
}

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
    bool stmtsEqual = true;
    if (this->stmts.size() != other.stmts.size()) {
        stmtsEqual = false;
    } else {
        for (unsigned int i = 0; i < this->stmts.size(); ++i) {
            if (*this->stmts[i] != *other.stmts[i]) {
                stmtsEqual = false;
            }
        }
    }

    bool transformationListsEqual = true;
    if (this->transformationLists.size() != other.transformationLists.size()) {
        transformationListsEqual = false;
    } else {
        for (unsigned int i = 0; i < this->transformationLists.size(); ++i) {
            if (this->transformationLists[i].size() != other.transformationLists[i].size()) {
                transformationListsEqual = false;
            } else {
                for (unsigned int j = 0; j < this->transformationLists[i].size(); ++j) {
                    if (*this->transformationLists[i][j] != *other.transformationLists[i][j]) {
                        transformationListsEqual = false;
                    }
                }
            }
        }
    }

    return (
        this->name == other.name &&
        //this->arrays == other.arrays &&
        stmtsEqual &&
        this->dataSpaces == other.dataSpaces &&
        this->parameters == other.parameters &&
        this->returnValues == other.returnValues &&
        transformationListsEqual
    );
}

std::string Computation::getName() const {
  return name;
}

void Computation::setName(std::string newName) {
  this->name = newName;
}

bool Computation::hasName() const {
    return (!getName().empty());
}

Computation* Computation::getUniquelyNamedClone() const {
    std::string namePrefix = NAME_PREFIX_BASE + std::to_string(numComputationRenames++);
    Computation* prefixedCopy = new Computation();

    // prefix name forerrarity
    if (this->hasName()) {
        prefixedCopy->setName(namePrefix + this->getName());
    }

    // prefix all data in the Computation and insert it to the new one
    for (auto& space : this->dataSpaces) {
        prefixedCopy->addDataSpace(getPrefixedDataSpaceName(space.first, namePrefix), space.second);
    }
    for (auto& param : this->parameters) {
        prefixedCopy->addParameter(getPrefixedDataSpaceName(param, namePrefix), getDataSpaceType(param));
    }
    for (auto& retVal : this->returnValues) {
        // only prefix values that are data space names, avoid trying to prefix
        // literals
        prefixedCopy->addReturnValue(
            (retVal.second ? getPrefixedDataSpaceName(retVal.first, namePrefix) : retVal.first), retVal.second);
    }
    for (auto& stmt : this->stmts) {
        prefixedCopy->addStmt(stmt->getUniquelyNamedClone(namePrefix, this->getDataSpaces()));
    }

    return prefixedCopy;
}

void Computation::resetNumRenamesCounters() {
    Computation::numComputationRenames = 0;
}

std::string Computation::getPrefixedDataSpaceName(const std::string& originalName, const std::string& prefix) {
    return DATA_SPACE_DELIMITER + prefix + originalName.substr(1);
}

void Computation::addStmt(Stmt* stmt) {
    addStmt(stmt, stmts.size());
}

void Computation::addStmt(Stmt* stmt, int stmtIdx) {
    transformationLists.push_back({});
    if (!stmt->isDelimited()) {
        delimitDataSpacesInStmt(stmt);
    }
    // TODO: if stmtIdx < stmts.size(), update later execution schedules
    stmts.insert(stmts.begin() + stmtIdx, stmt);
    //enforceArraySSA(stmt);
    stmtIdx = locatePhiNodes(stmtIdx);
    enforceSSA(stmtIdx);
}

/*
void Computation::enforceArraySSA(Stmt* stmt) {
    // We don't want to do any array SSA enforcing for array accesses
    if (stmt->isArrayAccess()) {
//        std::cerr << "Skipped Array SSA {\n"
//                  << stmt->prettyPrintString() << "}" << std::endl;
        return;
    }
    for (int i = 0; i < stmt->getNumReads(); i++) {
        if (!enforceArraySSA(stmt, i, true)) { return; }
    }
    for (int i = 0; i < stmt->getNumWrites(); i++) {
        if (!enforceArraySSA(stmt, i, false)) { return; }
    }
}

bool Computation::enforceArraySSA(Stmt* stmt, int dataIdx, bool isRead) {
    // Get the data space and scheduling function
    std::string dataSpace = isRead ? stmt->getReadDataSpace(dataIdx) :
                                     stmt->getWriteDataSpace(dataIdx);
    Relation* schedFunc = isRead ? stmt->getReadRelation(dataIdx) :
                                   stmt->getWriteRelation(dataIdx);
//    std::cerr << "Renaming " << dataSpace << " with "
//              << (isRead ? "read" : "write") << " access: "
//              << schedFunc->prettyPrintString() << std::endl;

    // Get all constant elements in the relation tuple
    // If an iterator is found, remove all clear the list
    TupleDecl decl = schedFunc->getTupleDecl();
    std::list<int> accessVals;
    for (int i = schedFunc->inArity(); i < decl.size(); i++) {
        if (!decl.elemIsConst(i)) {
//            std::cerr << "Iterator Access Detected" << std::endl;
            accessVals.clear();
            break;
        } else {
            accessVals.push_back(decl.elemConstVal(i));
        }
    }

    // If accessVals = {0}, check that it's actually an array access
    std::string srcCode = stmt->getStmtSourceCode();
    if (accessVals.size() == 1 && accessVals.front() == 0 &&
        srcCode.find(dataSpace + "[0]") == std::string::npos) { return true; }

    // Update the map of arrays
    auto it = arrays.find(dataSpace);
    if (it == arrays.end()) {
        arrays[dataSpace] = !accessVals.empty();
    } else if (accessVals.empty() == it->second) {
        std::cerr << "Error: " << dataSpace << " has both constant and iterator accesses!"
                  << "\n\tArrays cannot have both constant and iterator accesses." << std::endl;
        return false;
    }

    // Handle constant accesses
    if (!accessVals.empty()) {
        // Get the new data space name and the access string for the array
        std::ostringstream name, access, tuple;
        name << trimDataSpaceName(dataSpace);
        tuple << "[";
        for (auto it = accessVals.begin(); it != accessVals.end(); it++) {
            name << ARR_ACCESS_STR << *it;
            access << "[" << *it << "]";
            if (it != accessVals.begin()) { tuple << ", "; }
            tuple << *it;
        }
        tuple << "]";
//        std::cerr << "Array Access: " << access.str() << std::endl;
//        std::cerr << "Access Tuple: " << tuple.str() << std::endl;
        std::string newDataSpace = Computation::delimitDataSpaceName(name.str());
//        std::cerr << "New Name: " << newDataSpace << std::endl;
        // This array has not been accessed in this way before
        if (!isDataSpace(newDataSpace)) {
            // Add the rename as a data space
            if (isParameter(dataSpace)) {
                addParameter(newDataSpace, getDataSpaceType(dataSpace));
            } else {
                addDataSpace(newDataSpace, getDataSpaceType(dataSpace));
            }
            // Access was read before being written, so we need to initialize it
            if (isRead) {
                // Assign the array access to the new data space
                Stmt* arrAccessStmt = new Stmt(
                    newDataSpace + " = " + dataSpace + access.str() + ";",
                    stmt->getIterationSpace()->prettyPrintString(),
                    stmt->getExecutionSchedule()->prettyPrintString(),
                    {{dataSpace, "{[0]->" + tuple.str() + "}"}},
                    {{newDataSpace, "{[0]->[0]}"}}
                );
                arrAccessStmt->setDelimited();
                arrAccessStmt->setArrayAccess(true);
                addStmt(arrAccessStmt);
//                std::cerr << "Added Array Access Statement {\n"
//                          << arrAccessStmt->prettyPrintString() << "}" << std::endl;
            }
        }
        // Update our current statement
//        std::cerr << "Code: " << stmt->getStmtSourceCode();
        stmt->setStmtSourceCode(iegenlib::replaceInString(
                                    stmt->getStmtSourceCode(),
                                    dataSpace+access.str(), newDataSpace));
//        std::cerr << "\n\t-> " << stmt->getStmtSourceCode() << std::endl;

        // Update other reads/writes - we can no longer identify this array access
        // in the source code
        if (isRead) { stmt->updateRead(dataIdx, newDataSpace, "{[0]->[0]}"); }
        else { stmt->updateWrite(dataIdx, newDataSpace, "{[0]->[0]}"); }
        int ub = isRead ? stmt->getNumWrites() : stmt->getNumReads();
        for (int i = 0; i < ub; i++) {
            std::string dsName = isRead ? stmt->getWriteDataSpace(i) :
                                          stmt->getReadDataSpace(i);
            if (dsName == dataSpace) {
                if (isRead) { stmt->updateWrite(i, newDataSpace, "{[0]->[0]}"); }
                else { stmt->updateRead(i, newDataSpace, "{[0]->[0]}"); }
            }
        }
//        std::cerr << "Array SSA Statement {\n"
//                  << stmt->prettyPrintString() << "}" << std::endl;
    }

    return true;
}
*/

void Computation::enforceSSA(int stmtIdx) {
    if (stmtIdx < 0 || stmtIdx >= getNumStmts()) { return; }

    struct SSAWrite {
        std::string oldWrite, newWrite;
        int firstWrite;
    };

    Stmt* stmt = getStmt(stmtIdx);
    std::vector<SSAWrite> writes;
    for (int i = 0; i < stmt->getNumWrites(); i++) {
        std::string write = stmt->getWriteDataSpace(i);
        // Is a constant access array
        if (!stmt->getConstArrayAccesses(i, false).empty()) { continue; }
        // Is not written to or is written to at or after stmtIdx
        int writeIdx = firstWriteIndex(write);
        if (writeIdx < 0 || writeIdx >= stmtIdx) { continue; }

        std::string newWrite = getDataSpaceRename(write);
        writes.push_back(SSAWrite{ write, newWrite, writeIdx });

        addDataSpace(newWrite, getDataSpaceType(write));

        // Rename in the reads of the new statement
        stmt->replaceRead(write, newWrite);
    }

    for (int i = stmtIdx - 1; i >= 0; i--) {
        Stmt* currStmt = getStmt(i);
        auto it = writes.begin();
        while (it != writes.end()) {
            if (it->firstWrite == i) {
                // Rename in the writes of the current statement
                currStmt->replaceWrite(it->oldWrite, it->newWrite);
                it = writes.erase(it);
            } else {
            // Rename throughout the entire statment
                currStmt->replaceDataSpace(it->oldWrite, it->newWrite);
                ++it;
            }
        }
        if (writes.empty()) { break; }
    }
}

int Computation::locatePhiNodes(int stmtIdx) {
    if (stmtIdx < 0 || stmtIdx >= getNumStmts()) { return stmtIdx; }
    // Phi nodes will always generate another phi node
    // which is just a duplicate of itself - skip phi nodes
    Stmt* stmt = getStmt(stmtIdx);
    if (!stmt->isPhiNode()) {
        for (int i = 0; i < stmt->getNumReads(); i++) {
            if (locatePhiNode(stmtIdx, stmt->getReadDataSpace(i))) {
                stmtIdx++;
            }
        }
    }
    return stmtIdx;
}

bool Computation::locatePhiNode(int stmtIdx, std::string dataSpace) {
    if (stmtIdx < 0 || stmtIdx >= getNumStmts()) { return false; }
//    std::cerr << "Locating Phi Node" << std::endl;
    std::pair<int, std::string> first{ -1, "" }, guaranteed{ -1, "" };
    for (int i = stmtIdx - 1; i >= 0; i--) {
        Stmt* stmt = getStmt(i);
        for (int j = 0; j < stmt->getNumWrites(); j++) {
            std::string write = stmt->getWriteDataSpace(j);
            if (areEquivalentRenames(dataSpace, write)) {
                if (isGuaranteedExecute(stmt, getStmt(stmtIdx))) {
                    // std::cerr << "Phi Node Located" << std::endl;
                    guaranteed = { i, write };
                    return addPhiNode(stmtIdx, first, guaranteed);
                } else if (first.first == -1) { first = { i, write }; }
            }
        }
    }
//    std::cerr << "No guaranteed write identified" << std::endl;
    return false;
}

bool Computation::addPhiNode(int stmtIdx,
    std::pair<int, std::string>& first, std::pair<int, std::string>& guaranteed) {
    if (stmtIdx < 0 || stmtIdx >= getNumStmts()) { return false; }
    if (first.first == -1) { return false; }
//    std::cerr << "Adding Phi Node" << std::endl;

    // Removing every tuple element will cause an error
    // add an extra tuple element at the end to ensure there is one
    // tuple element remaining
    TupleDecl pad(1);
    pad.setTupleElem(0, 0);

    // Test constraint to check for true/false constraints in firstIS
    // If it is eliminated, 1+ other constraints were false
    // If it is the last constraint, all other constraints were true
    // Otherwise, ignore as there are other valid constraints
    Conjunction* testConstr = new Conjunction(1);
    Exp* testExp = new Exp();
    testExp->addTerm(new VarTerm(CONSTR_TEST));
    testExp->setEquality();
    testConstr->addEquality(testExp);

    // Project out everything in the source read's iteration space
    Set* sourceIS = trimISDataSpaces(getStmt(stmtIdx)->getIterationSpace());
    // Pad the tuple
    sourceIS->setTupleDecl(sourceIS->getTupleDecl().concat(pad));
//    std::cerr << "sourceIS: " << sourceIS->prettyPrintString() << std::endl;
    // Ignore the padded element
    int size = sourceIS->getTupleDecl().size() - 1;
    // Remove all iterators, generating a single condition for each corresponding loop
    for (int i = 0; i < size; i++) {
        Set* tmp = sourceIS;
        sourceIS = sourceIS->projectOut(i);
        delete tmp;
    }
//    std::cerr << "\t-> " << sourceIS->prettyPrintString() << std::endl;

    // Stores all data spaces in the first write's constraints
    std::set<std::string> trimmedNames;
    // Project out everything in the first write's iteration space
    Stmt* firstStmt = getStmt(first.first);
    Set* firstIS = trimISDataSpaces(firstStmt->getIterationSpace(),
        trimmedNames);
    // Add the test constraint
    firstIS->addConjunction(testConstr);
    // Pad the tuple
    firstIS->setTupleDecl(firstIS->getTupleDecl().concat(pad));
//    std::cerr << "firstIS: " << firstIS->prettyPrintString() << std::endl;
    // Ignore the padded element
    size = firstIS->getTupleDecl().size() - 1;
    for (int i = 0; i < size; i++) {
        Set* tmp = firstIS;
        firstIS = firstIS->projectOut(i);
        delete tmp;
    }
//    std::cerr << "\t-> " << firstIS->prettyPrintString() << std::endl;

    // Get all conditions for the source read
    std::vector<std::string> sourceConstr = getSetConstraints(sourceIS);
/*    std::cerr << "Source Read Constraints: ";
    for (std::string str : sourceConstr) {
        if (str != sourceConstr.at(0)) { std::cerr << " && "; }
        std::cerr << str;
    }
    std::cerr << std::endl;*/

    // Get all conditions for the first write that aren't
    // used by the source read
    std::vector<std::string> constrs;
    for (std::string constr : getSetConstraints(firstIS)) {
        if (std::find(sourceConstr.begin(), sourceConstr.end(), constr) ==
            sourceConstr.end()) {
            constrs.push_back(constr);
        }
    }
/*    std::cerr << "First Write Constraints: ";
    for (std::string str : constrs) {
        if (str != constrs.at(0)) { std::cerr << " && "; }
        std::cerr << str;
    }
    std::cerr << std::endl;*/

    // Stmt source code
    std::ostringstream code;
    // Stmt reads/writes
    std::vector<std::pair<std::string, std::string>> reads, writes;
    // We always write to this
    writes.push_back({ first.second, "{[0]->[0]}" });
    // No constraints so one of the constraints = false
    // skip to guaranteed write
    if (constrs.size() == 0) {
        reads.push_back({ guaranteed.second, "{[0]->[0]}" });
        code << first.second << " = " << guaranteed.second << ";";
    // The only constraint is the test constraint, so all constraints = true
    // skip to first write
    } else if (constrs.size() == 1) {
        reads.push_back({ first.second, "{[0]->[0]}" });
        code << first.second << " = " << first.second << ";";
    // Some constraints are left, generate ternary expression
    } else {
        reads.push_back({ first.second, "{[0]->[0]}" });
        reads.push_back({ guaranteed.second, "{[0]->[0]}" });

        // Remove the test string
        std::string testStr = " == 0";
        testStr = CONSTR_TEST + testStr;
        constrs.erase(std::find(constrs.begin(), constrs.end(), testStr));
        // Write all our constraints
        for (int i = 0; i < constrs.size(); i++) {
            if (i != 0) { code << " && "; }
            code << constrs.at(i);
        }
        // Re-add delimiter to data spaces
        std::string codeStr = code.str();
        for (std::string str : trimmedNames) {
            std::string newStr = Computation::delimitDataSpaceName(str);
            std::string old = codeStr;
            codeStr = iegenlib::replaceInString(codeStr, str, newStr);
            if (codeStr != old) {
                // TODO: correct access relation
                // Add the data space as a read
                reads.push_back({ newStr, "{[0]->[0]}" });
            }
        }
        // Compose full string
        code.str("");
        code << first.second << " = " << codeStr << " ? "
            << first.second << " : " << guaranteed.second << ";";
    }

    // Create phi node
    Stmt* phiStmt = new Stmt(code.str(), sourceIS->prettyPrintString(),
        getStmt(stmtIdx)->getExecutionSchedule()->prettyPrintString(),
        reads, writes);
    phiStmt->setDelimited();
    phiStmt->setPhiNode(true);
//    std::cerr << "New Phi Node {\n" << phiStmt->prettyPrintString() << "}" << std::endl;
    // Add the statement
    addStmt(phiStmt, stmtIdx);

    // Clean up
    delete sourceIS;
    delete firstIS;

//    std::cerr << "Phi Node Added {\n" << phiStmt->prettyPrintString() << "}" << std::endl;
    return true;
}

bool Computation::isGuaranteedExecute(Stmt* stmt1, Stmt* stmt2) {
    Set* iterSpace1 = stmt1->getIterationSpace();
    Set* iterSpace2 = stmt2->getIterationSpace();

    std::vector<std::string> match = getSetConstraints(iterSpace2);

    // Is guaranteed if all constraints in idx1 are in idx2
    for (std::string str : getSetConstraints(iterSpace1)) {
        if (std::find(match.begin(), match.end(), str) == match.end()) {
            return false;
        }
    }
    return true;
}

std::vector<std::string> Computation::getSetConstraints(Set* set) {
    std::vector<std::string> result;
    for (auto it = set->conjunctionBegin(); it != set->conjunctionEnd(); it++) {
        for (Exp* e : (*it)->inequalities()) {
            result.push_back(e->prettyPrintString((*it)->getTupleDecl()) + " >= 0");
        }
        for (Exp* e : (*it)->equalities()) {
            result.push_back(e->prettyPrintString((*it)->getTupleDecl()) + " == 0");
        }
    }
    return result;
}

Set* Computation::trimISDataSpaces(Set* set) {
    Set* s = new Set(stripDataSpaceDelimiter(set->getString()));
    return s;
}

Set* Computation::trimISDataSpaces(Set* set, std::set<std::string> &trimmedNames) {
    std::string setStr = set->getString();
    int pos = setStr.find(DATA_SPACE_DELIMITER);
    while (pos != std::string::npos) {
        std::string begin = setStr.substr(0, pos);
        std::string end = setStr.substr(pos + 1);
        pos = end.find(DATA_SPACE_DELIMITER);
        if (pos == std::string::npos) {
            setStr = begin + end;
            break;
        }
        trimmedNames.insert(end.substr(0, pos));
        setStr = begin + end.substr(0, pos) + end.substr(pos + 1);
        pos = setStr.find(DATA_SPACE_DELIMITER);
    }
    return new Set(setStr);
}

Stmt* Computation::getStmt(unsigned int index) const { return stmts.at(index); }

unsigned int Computation::getNumStmts() const { return stmts.size(); }

void Computation::addDataSpace(std::string dataSpaceName, std::string dataSpaceType) {
    if (isDataSpace(dataSpaceName)) {
        std::string previousType = getDataSpaceType(dataSpaceName);
        if (dataSpaceType != previousType) {
            throw assert_exception("Attempted to add already-added data space '"
                + dataSpaceName + "' with new type '" + dataSpaceType + "' (was '" + previousType + "').");
        }
    }
    bool alreadyDelimited = nameIsDelimited(dataSpaceName);
    assertValidDataSpaceName(dataSpaceName, alreadyDelimited);
    if (alreadyDelimited) {
        undelimitedDataSpaceNames.emplace(trimDataSpaceName(dataSpaceName));
        dataSpaces[dataSpaceName] = dataSpaceType;
    } else {
        undelimitedDataSpaceNames.emplace(dataSpaceName);
        dataSpaces[delimitDataSpaceName(dataSpaceName)] = dataSpaceType;
    }
}

std::map<std::string, std::string> Computation::getDataSpaces() const {
    return dataSpaces;
}

std::string Computation::getDataSpaceType(std::string dataSpaceName) const{
    if (!nameIsDelimited(dataSpaceName)) {
        dataSpaceName = delimitDataSpaceName(dataSpaceName);
    }
    return dataSpaces.at(dataSpaceName);
}

bool Computation::isDataSpace(std::string name) const {
    return dataSpaces.find(name) != dataSpaces.end()
           || undelimitedDataSpaceNames.find(name) != undelimitedDataSpaceNames.end();
}

void Computation::addParameter(std::string paramName, std::string paramType) {
    bool alreadyDelimited = nameIsDelimited(paramName);
    assertValidDataSpaceName(paramName, alreadyDelimited);
    parameters.push_back(alreadyDelimited ? paramName : delimitDataSpaceName(paramName));
    // parameters are automatically available as data spaces to the Computation
    addDataSpace(paramName, paramType);
}

std::string Computation::getParameterName(unsigned int index) const {
    return parameters.at(index);
}

std::string Computation::getParameterType(unsigned int index) const {
    return getDataSpaceType(getParameterName(index));
}

bool Computation::isParameter(std::string dataSpaceName) const {
    return std::find(parameters.begin(), parameters.end(),
                     dataSpaceName) != parameters.end();
}

unsigned int Computation::getNumParams() const {
    return parameters.size();
}

void Computation::addReturnValue(std::string name) {
    this->addReturnValue(name, this->isDataSpace(name));
}

void Computation::addReturnValue(std::string name, bool isDataSpace) {
    bool alreadyDelimited = nameIsDelimited(name);
    if (isDataSpace) {
        assertValidDataSpaceName(name, alreadyDelimited);
    }
    returnValues.emplace_back((alreadyDelimited || !isDataSpace) ? name : delimitDataSpaceName(name),
                            isDataSpace);
}

std::vector<std::string> Computation::getReturnValues() const {
    std::vector<std::string> names;
    for (const auto& retVal : returnValues) {
        names.push_back(retVal.first);
    }
    return names;
}

std::unordered_set<std::string> Computation::getActiveOutValues() const {
    std::unordered_set<std::string> names;

    for (const auto& returnValue : returnValues) {
        // only include non-constant return values
        if (returnValue.second) {
            names.emplace(returnValue.first);
        }
    }
    for (const std::string &parameterName : parameters) {
        if (getDataSpaceType(parameterName).find('&') != std::string::npos ||
            getDataSpaceType(parameterName).find('*') != std::string::npos) {
            names.emplace(parameterName);
        }
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
                dataWritesOutput << "    " << (*it).getWriteDataSpace(i) << ": "
                                 << (*it).getWriteRelation(i)->prettyPrintString()
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
        dataSpacesOutput << it.first << " " << it.second;
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
    std::unordered_set<std::string> dataSpacesAccessedInStmts;
    for (const auto& stmt : stmts) {
        // check completeness of each statement
        if (!stmt->isComplete()) {
            return false;
        }

        // collect all data space accesses
        for (unsigned int i = 0; i < stmt->getNumReads(); ++i) {
            dataSpacesAccessedInStmts.emplace(stmt->getReadDataSpace(i));
        }
        for (unsigned int i = 0; i < stmt->getNumWrites(); ++i) {
            dataSpacesAccessedInStmts.emplace(stmt->getWriteDataSpace(i));
        }
    }

    // check that list of data spaces matches those accessed in statements
    for (const auto& dataSpace : dataSpacesAccessedInStmts) {
        // only checking inclusion in one direction because we aren't detecting accesses in for loop bounds
        if (!dataSpaces.count(dataSpace)) {
            return false;
        }
    }

    return true;
}

std::string Computation::codeGenMemoryManagementString() {

    std::ostringstream outputString;
    std::map<std::string, std::string> dataSpaces = this->getDataSpaces();
    std::map<std::string, std::string>::iterator it = dataSpaces.begin();

    while (it != dataSpaces.end()){
        std::string name = it->first;

        std::string type = it->second;

        outputString << type << " " << trimDataSpaceName(name) << ";\n";
        it++;
    }

    return outputString.str();
}

void Computation::clear() {
    for (auto& stmt : stmts) {
        delete stmt;
    }
    stmts.clear();
    dataSpaces.clear();
    parameters.clear();
    returnValues.clear();
    transformationLists.clear();
}

AppendComputationResult Computation::appendComputation(
  const Computation* other, std::string surroundingIterDomainStr,
  std::string surroundingExecScheduleStr,
  std::vector<std::string> arguments){
    Set* surroundingIterDomain = new Set(delimitDataSpacesInString(surroundingIterDomainStr));
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
    // delimit arguments that are data space names
    for (unsigned int i = 0; i < numArgs; ++i) {
        if (isDataSpace(arguments[i])) {
            arguments[i] = delimitDataSpaceName(arguments[i]);
        }
    }

    // Insert declarations+assignment of (would-have-been if not for inlining)
    // parameter values, at the beginning of the appendee.
    // Assignment of a parameter i will be like:
    // [type of i] [name of i] = [name of argument i from passed-in list];
    // Insertion is done by prepending statements one at a time in reverse
    // order.

    // Get the number of writes.
    // Need this for the correct execution order
    int numWrites = 0;
    for (int i = 0;  i < ((signed int)numArgs); ++i) {
        std::string param = toAppend->getParameterName(i);
        std::string paramType = toAppend->getParameterType(i);
        if (paramType.find("&") != std::string::npos ||
            paramType.find("*") != std::string::npos) {
            toAppend->replaceDataSpaceName(param, arguments[i]);
        } else {
            Stmt* paramDeclStmt = new Stmt();
            paramDeclStmt->setDelimited();

            paramDeclStmt->setStmtSourceCode(param + " = " + arguments[i] + ";");
            paramDeclStmt->setIterationSpace("{[0]}");
            paramDeclStmt->setExecutionSchedule("{[0]->[" + std::to_string(numWrites) +
                                                "]}");
            // If passed-in argument is a data space, mark it as being read
            // (otherwise it is a literal)
            if (this->isDataSpace(arguments[i])) {
                paramDeclStmt->addRead(arguments[i], "{[0]->[0]}");
            }
            paramDeclStmt->addWrite(param, "{[0]->[0]}");

            // Add the statement
            toAppend->stmts.insert(toAppend->stmts.begin() + numWrites, paramDeclStmt);
            numWrites++;
        }
    }

    // add (already name prefixed) data spaces from appendee to appender
    for (const auto& dataSpace : toAppend->getDataSpaces()) {
        this->addDataSpace(dataSpace.first, dataSpace.second);
    }

    // calculate indexes/offsets for execution tuple modifications
    // value to offset 0th position of appended schedule tuples by
    int offsetValue =
        surroundingExecTuple.elemConstVal(surroundingExecSchedule->arity() - 1);
    // Keep track of the latest execution schedule position used.
    int latestTupleValue = offsetValue;

    // construct and insert an adjusted version of each statement of appendee,
    // including parameter declaration statements
    unsigned int remainingParamDeclStmts = numWrites;
    bool processingOriginalStmts = false;
    for (unsigned int stmtNum = 0; stmtNum < toAppend->getNumStmts();
         ++stmtNum) {
        // once we've finished processing prepended parameter declaration
        // statements, increase the offset for remaining (original) statements
        // by the number of prepended statements
        if (!processingOriginalStmts) {
            if (remainingParamDeclStmts == 0) {
                offsetValue += numWrites;
                processingOriginalStmts = true;
            } else {
                remainingParamDeclStmts--;
            }
        }

        // statement to copy and modify from the appendee
        Stmt* appendeeStmt = toAppend->getStmt(stmtNum);
        // new statement built up from appendee which will be inserted
        Stmt* newStmt = new Stmt();
        newStmt->setDelimited();
        // Copy phiNode and arrayAccess status
        newStmt->setPhiNode(appendeeStmt->isPhiNode());
        newStmt->setArrayAccess(appendeeStmt->isArrayAccess());

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
    for (const auto& retVal : toAppend->getReturnValues()) {
        result.returnValues.emplace_back(trimDataSpaceName(retVal));
    }

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

void Computation::finalize(bool deleteDeadNodes) {
    enforceArraySSA();
    adjustExecutionSchedules();
	padExecutionSchedules();

    if (deleteDeadNodes) {
        std::cerr << "Deleting Dead Nodes" << std::endl;
        deleteDeadStatements();
    }
}

bool Computation::consistentSetArity(const std::vector<Set*>& sets) {
    if (sets.size() == 0) { return true; }
    int arity = sets[0]->arity();
    for (Set* set : sets) { if (set->arity() != arity) return false; }
    return true;
}

void Computation::delimitDataSpacesInStmt(Stmt *stmt) {
    if (stmt->isDelimited()) {
        throw assert_exception("Attempted to delimit a Stmt already marked as delimited");
    }

    stmt->setStmtSourceCode(delimitDataSpacesInString(stmt->getStmtSourceCode()));
    stmt->setIterationSpace(delimitDataSpacesInString(stmt->getIterationSpace()->prettyPrintString()));
    for (unsigned int i = 0; i < stmt->getNumReads(); ++i) {
        stmt->updateRead(i, delimitDataSpaceName(stmt->getReadDataSpace(i)),
                         stmt->getReadRelation(i)->prettyPrintString());
    }
    for (unsigned int i = 0; i < stmt->getNumWrites(); ++i) {
        stmt->updateWrite(i, delimitDataSpaceName(stmt->getWriteDataSpace(i)),
                          stmt->getWriteRelation(i)->prettyPrintString());
    }

    stmt->setDelimited();
}

std::string Computation::delimitDataSpacesInString(std::string originalString) {
    std::ostringstream delimitedString;

    static std::regex potentialVariableRegex("[a-zA-Z_][a-zA-Z_0-9]*");
    static std::string quote = "\"";

    std::vector<std::string> quoteSplitStringSegments;

    std::size_t start = 0;
    std::size_t end = originalString.find(quote);
    std::size_t lastQuotePos = 0;
    while (end != std::string::npos)
    {
        lastQuotePos = end;
        // create a new segment on a non-escaped quote
        if (end == 0 || originalString.at(end - 1) != '\\') {
            quoteSplitStringSegments.emplace_back(originalString.substr(start, end - start));
            start = end + quote.length();
        }
        end = originalString.find(quote, end + quote.length());
    }
    // add remainder of string after last quote
    if (end != originalString.length() - 1) {
        quoteSplitStringSegments.emplace_back(originalString.substr(lastQuotePos));
    }

    for (unsigned int i = 0; i < quoteSplitStringSegments.size(); ++i) {
        // rewrite alternate sections, beginning from the first which is unquoted
        if (i % 2 == 0) {
            std::string& originalSegment = quoteSplitStringSegments[i];
            std::ostringstream rewrittenSegment;
            std::size_t  currentPosInOriginalSegment = 0;
            for (auto match_it = std::sregex_iterator(originalSegment.begin(), originalSegment.end(), potentialVariableRegex);
                 match_it != std::sregex_iterator(); ++match_it) {
                // only replace potential identifiers that actually match existing data spaces
                if (this->undelimitedDataSpaceNames.count((*match_it).str())) {
                    // grab everything between the last match and the beginning of this one, then add the delimited version
                    // of this one
                    rewrittenSegment << originalSegment.substr(currentPosInOriginalSegment,
                                                              (*match_it).position() - currentPosInOriginalSegment)
                                     << delimitDataSpaceName((*match_it).str());
                    currentPosInOriginalSegment = (*match_it).position() + (*match_it).length();
                }
            }
            // add the remainder of the original string
            rewrittenSegment << originalSegment.substr(currentPosInOriginalSegment);

            delimitedString << rewrittenSegment.str();
        } else {
            // quoted sections (string literals) are left as-is
            delimitedString << quote << quoteSplitStringSegments[i];
        }
    }

    return delimitedString.str();
}

std::string Computation::delimitDataSpaceName(std::string dataSpaceName) {
    if (nameIsDelimited(dataSpaceName)) {
        throw assert_exception("Attempted to double-delimit data space '" + dataSpaceName + "'");
    }
  return DATA_SPACE_DELIMITER + dataSpaceName + DATA_SPACE_DELIMITER;
}

std::string Computation::stripDataSpaceDelimiter(std::string delimitedStr) {
    static const std::string delimiterAsStr = std::string(1, DATA_SPACE_DELIMITER);
    return iegenlib::replaceInString(delimitedStr, delimiterAsStr, "");
}

bool Computation::nameIsDelimited(std::string name) {
    bool improperlyDelimited = false;
    if (name.front() == DATA_SPACE_DELIMITER) {
        if (name.back() == DATA_SPACE_DELIMITER) {
            return true;
        } else {
            improperlyDelimited = true;
        }
    } else if (name.back() == DATA_SPACE_DELIMITER) {
        improperlyDelimited = true;
    }

    if (improperlyDelimited) {
        throw assert_exception("Data space name '" + name + "' is improperly delimited.");
    } else {
        return false;
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
        stmt->setExecutionSchedule(
            compositions[sched->outArity() - 1]->Compose(sched));
    }

    for (Relation* rel : compositions) { delete rel; }
}

void Computation::enforceArraySSA() {
    // Map <array name : set<unrolled indexes>>
    std::map<std::string, std::set<std::list<int>>> arrays;

    // Go through each statement and unroll arrays
    for (int i = 0; i < getNumStmts(); i++) {
        Stmt* stmt = getStmt(i);
        // Change reads first so that SSA works correctly when changing writes
        for (int j = 0; j < stmt->getNumReads(); j++) {
            auto idxs = stmt->getConstArrayAccesses(j, true);
            if (!idxs.empty()) {
                // Replace the read with the unrolled version
                std::string read = stmt->getReadDataSpace(j);
                std::string unroll, access, tuple;
                getArrayAccessStrs(unroll, access, tuple, read, idxs);
                stmt->replaceReadSourceCode(access, unroll);
                stmt->updateRead(j, unroll, "{[0]->[0]}");

                // Update array accesses
                arrays[read].insert(idxs);
                // If unroll is a new dataspace, add an unroll statement
                if (!isDataSpace(unroll)) {
                    // Add the rename as a data space
                    addDataSpace(unroll, getDataSpaceType(read));
                    // Assign the array access to the new data space
                    // TODO: correct execution schedule
                    // unroll = access;
                    Stmt* unrollStmt = new Stmt(
                        unroll + " = " + access + ";",
                        stmt->getIterationSpace()->prettyPrintString(),
                        stmt->getExecutionSchedule()->prettyPrintString(),
                        { {read, "{[0]->" + tuple + "}"} },
                        { {unroll, "{[0]->[0]}"} }
                    );
                    unrollStmt->setDelimited();
                    unrollStmt->setArrayAccess(true);
                    int numStmts = getNumStmts();
                    addStmt(unrollStmt, i);
                    i += getNumStmts() - numStmts;
                    // std::cerr << "Added Array Access Statement {\n"
                    // << arrAccessStmt->prettyPrintString() << "}" << std::endl;
                }
            }
        }
        for (int j = 0; j < stmt->getNumWrites(); j++) {
            auto idxs = stmt->getConstArrayAccesses(j, false);
            if (!idxs.empty()) {
                // Replace the write with the unrolled version
                std::string write = stmt->getWriteDataSpace(j);
                std::string unroll, access, tuple;
                getArrayAccessStrs(unroll, access, tuple, write, idxs);
                stmt->replaceWriteSourceCode(access, unroll);
                stmt->updateWrite(j, unroll, "{[0]->[0]}");

                // Update array accesses
                arrays[write].insert(idxs);
                // If unroll is a new dataspace, add it
                if (!isDataSpace(unroll)) {
                    // Add the rename as a data space
                    addDataSpace(unroll, getDataSpaceType(write));
                }

                // Perform SSA
                enforceSSA(i);
            }
        }
    }

    // Array rerolling
	auto activeOut = getActiveOutValues();
	for (auto& pair : arrays) {
        if (!activeOut.count(pair.first)) {
        	continue;
    	}
        for (auto& idxs : pair.second) {
            // Add the rerolling statement
            std::string unroll, access, tuple;
            getArrayAccessStrs(unroll, access, tuple,
                pair.first, idxs);
            // access = unroll;
            Stmt* rerollStmt = new Stmt(
                access + " = " + unroll + ";",
                "{[0]}",
                "{[0]->[" + std::to_string(getNumStmts()) + "]}",
                { {unroll, "{[0]->[0]}"} },
                { {pair.first, "{[0]->" + tuple + "}"} }
            );
            rerollStmt->setDelimited();
            rerollStmt->setArrayAccess(true);
            addStmt(rerollStmt);
        }
    }
}

void Computation::adjustExecutionSchedules() {
    if (getNumStmts() <= 0) { return; }
    int maxArity = 0;
    for (int j = 0; j < getNumStmts(); j++) {
        int tmp = getStmt(j)->getExecutionSchedule()->outArity();
        if (tmp > maxArity) { maxArity = tmp; }
    }
    // Create tuples filled with 0's
    TupleDecl oldTuple(maxArity), lastTuple(maxArity);
    for (int i = 0; i < maxArity; i++) {
        oldTuple.setTupleElem(i, 0);
        lastTuple.setTupleElem(i, 0);
    }
    int idx = 0;
    for (int i = 0; i < getNumStmts(); i++) {
        Stmt* stmt = getStmt(i);
        // Skip dynamically inserted nodes
        if (!stmt->isPhiNode() && !stmt->isArrayAccess()) {
            TupleDecl currTuple = stmt->getExecutionSchedule()->getTupleDecl();
            int inArity = stmt->getExecutionSchedule()->inArity();
            int outArity = stmt->getExecutionSchedule()->outArity();
            int arity = inArity + outArity;
            // Compute currTuple - oldTuple
            int level = 0, diff = 0;
            while (level < outArity && diff == 0) {
                if (currTuple.elemIsConst(level + inArity) 
                    && oldTuple.elemIsConst(level)) {
                    diff = currTuple.elemConstVal(level + inArity)
                        - oldTuple.elemConstVal(level);
                }
                level += 2;
            }
            if (diff != 0) { level -= 2; }
            // Update oldTuple
            for( int j = 0; j < outArity; j++) {
                oldTuple.setTupleElem(j, 
                    currTuple.elemConstVal(j + inArity));
            }
            for (int j = outArity; j < maxArity; j++) {
                oldTuple.setTupleElem(j, 0);
            }
            // Compute lastTuple + (currTuple - oldTuple)
            if (level < outArity && lastTuple.elemIsConst(level)) {
                currTuple.setTupleElem(level + inArity,
                    lastTuple.elemConstVal(level) + diff);
            }
            // Update dynamically inserted statements
            for (idx; idx < i; idx++) {
				getStmt(idx)->setExecutionSchedule(new Relation(
                    "{" + currTuple.toString(true, inArity) + "}"));
                currTuple.setTupleElem(arity - 1,
                    currTuple.elemConstVal(arity - 1) + 1);
            }
            // Update the current statement
			stmt->setExecutionSchedule(new Relation(
               "{" + currTuple.toString(true, inArity) + "}"));
            idx++;
            // Update last tuple
            for( int j = 0; j < outArity; j++) {
                lastTuple.setTupleElem(j, 
                    currTuple.elemConstVal(j + inArity));
            }
            for (int j = outArity; j < maxArity; j++) {
                lastTuple.setTupleElem(j, 0);
            }
        }
    }
     
    // Handle any remaining dynamically added statements
	int stmtNum = lastTuple.elemConstVal(0);
    for (idx; idx < getNumStmts(); idx++) {
		getStmt(idx)->setExecutionSchedule(new Relation(
            "{[0]->[" + std::to_string(++stmtNum) + "]}"));
    }
}

void Computation::deleteDeadStatements(){
    CompGraph graph;
    graph.create(this);
    std::vector<int> deadStmts;
    std::vector<std::string> deadDataSpaces;
    graph.removeDeadNodes(deadStmts,deadDataSpaces);


    for(std::string data: deadDataSpaces){

	auto it = dataSpaces.find(data);
        if(it != dataSpaces.end()){
	    dataSpaces.erase(it);
	}
    }

    // Delete statements.
    std::vector<Stmt*> toDelete;
    for(int stmtId : deadStmts){
        toDelete.push_back(getStmt(stmtId));
    }

    for(Stmt* st: toDelete){
        auto it = std::find(stmts.begin(),stmts.end(),st);
	if (it != stmts.end()){
	    stmts.erase(it);
	}
	delete st;
    }
}

std::string Computation::toDotString(bool reducePCRelations,
		bool toPoint, bool onlyLoopLevels, bool addDebugStmts,
		std::string subgraphSrc, bool subgraphReads,bool subgraphWrites) {
    //TODO: Deal with disjunction of conjunctions later.

    CompGraph graph = CompGraph();
    graph.create(this);
    if (reducePCRelations) {
        graph.reducePCRelations();
    }
    if(toPoint){
        graph.reduceNodes(true, true, 0);
    }
    if(onlyLoopLevels){
        graph.removeNodes(0);
    }

    if(addDebugStmts){
        graph.addDebugStmts(getStmtDebugStrings());
    }

    if(!subgraphSrc.empty()){
        try {
            int stmtIdx = std::stoi(subgraphSrc);
            return graph.toDotString(stmtIdx, subgraphReads, subgraphWrites);
        } catch (const std::invalid_argument& ia) {
            return graph.toDotString(subgraphSrc, subgraphReads, subgraphWrites);
        }
    }

    return stripDataSpaceDelimiter(graph.toDotString());
}

std::vector<std::pair<int, std::string>> Computation::getStmtDebugStrings() {
    std::vector<std::pair<int, std::string>> result;
    for (int i = 0; i < getNumStmts(); i++) {
        std::string debugStr = getStmt(i)->getAllDebugStr();
        if (!debugStr.empty()) { result.push_back({i, debugStr}); }
    }
    return result;
}


void Computation::addTransformation(unsigned int stmtIndex, Relation* rel) {
    transformationLists.at(stmtIndex).emplace_back(rel);
}

std::vector<Set*> Computation::applyTransformations() {
    std::vector<Relation*> transformations = getTransformations();
    std::vector<Set*> transformedSchedules;
    for (int stmtNum = 0; stmtNum < getNumStmts(); ++stmtNum) {
        Set* schedule = transformations[stmtNum]->Apply(
            getStmt(stmtNum)->getIterationSpace());
        transformedSchedules.push_back(schedule);
        delete transformations[stmtNum];
    }
    transformations.clear();
    return transformedSchedules;
}

std::vector<Relation*> Computation::getTransformations() const {
    std::vector<Relation*> transformedSchedules(getNumStmts(), NULL);
    for (int stmtNum = 0; stmtNum < this->getNumStmts(); ++stmtNum) {
        transformedSchedules[stmtNum] = new Relation(
            *getStmt(stmtNum)->getExecutionSchedule());
        for (Relation* transformation : transformationLists.at(stmtNum)) {
            Relation* curr = transformedSchedules[stmtNum];
            if (curr->outArity() != transformation->inArity()) {
                std::cerr << "Mismatched Arities: cannot apply transformation" << std::endl;
                continue;
            }
            transformedSchedules[stmtNum] = transformation->Compose(curr);
            delete curr;
        }
    }
    return transformedSchedules;
}

std::vector<std::pair<int, Set*>> Computation::getIterSpaces() {
    std::vector<Set*> transformedSpaces = applyTransformations();
    if (!consistentSetArity(transformedSpaces)) {
        std::cerr << "Iteration spaces do not have a consistent arity" << std::endl;
        for(Set* set : transformedSpaces){
            delete set;
        }
        return {};
    }

    // newIS is a list of pairs of statement id
    // to their transformed spaces.
    std::vector<std::pair<int,Set*>> newIS;

    for(int i = 0; i < transformedSpaces.size(); i++){
	newIS.push_back(std::make_pair(i,transformedSpaces[i]));
    }

    // Sort by lexicographical order
    std::sort(newIS.begin(),newIS.end(),
		    Computation::activeStatementComparator);

    return newIS;
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
    std::vector<Relation*> transformations = getTransformations();

    //Create a string for each statements iteration space
    for (Relation* rel : transformations) {
        Stmt* stmt = getStmt(stmtCount);

        stmtMacroUndefs << "#undef s" << stmtCount << "\n"
                        << "#undef s_" << stmtCount << "\n";

        // Generate the first macro based on the original iteration space
        Set* iterSpace = stmt->getIterationSpace();
        
	stmtMacroDefs << "#define s_" << stmtCount << "("
                      << iterSpace->getTupleDecl().toString() << ")   "
                      << Computation::stripDataSpaceDelimiter(stmt->getStmtSourceCode())
                      << " \n";

        // Get the new iteration space set
        Set* newIterSpace = rel->Apply(stmt->getIterationSpace());
        
        // This is required to generate correct tuple variable names
        newIterSpace->acceptVisitor(vOmegaReplacer);
//newIterSpace->pushConstToConstraints(); 
        // Generate the second macro based on the new iteration space
        // Generate a mapping between the two iteration spaces using
        // the transformation relation
        stmtMacroDefs << "#define s" << stmtCount << "("
                      << newIterSpace->getTupleDecl().toString()
                      << ")   s_" << stmtCount << "(";
        // Generate mapping from out tuple -> in tuple
        bool first = true;
        TupleDecl decl = rel->getTupleDecl();
        for (int i = 0; i < rel->inArity(); i++) {
            // Handle commas
            if (!first) { stmtMacroDefs << ", "; }

            // Get the expression
            Exp* e = rel->findFunction(i, rel->inArity(), rel->arity());
            // Print the expression
            // If e->equalZero = true, the expression may have no terms
            // and prettyPrintString will print '', not '0'
            stmtMacroDefs << (e && !e->equalsZero() ?
                e->prettyPrintString(decl) : "0");
            if (e) { delete e; }

            first = false;
        }
        stmtMacroDefs << ");\n";
        stmtCount++;

        // new Codegen would require an application
	// be performed first before the set is sent
	// to omega. This is a temporary solution to
	// circumvent Omega's schedulling bug.
        std::string omegaIterString =
            newIterSpace->toOmegaString(vOmegaReplacer->getUFCallDecls());
        iterSpaces.push_back(omegaIterString);

        // Use identity transformation instead.
        //transforms.push_back(omega::Identity(iterSpace->arity()));
	arity.push_back(newIterSpace->arity());

        delete newIterSpace;
    	delete rel;
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

    return stripDataSpaceDelimiter(generatedCode.str());
}

std::string Computation::omegaCodeGenFromString(std::vector<int> relationArity, std::vector<std::string> iterSpacesStr, std::string known){
    std::ostringstream generatedCode;
    std::vector<omega::Relation> iterSpaces;
    std::vector<omega::Relation> transforms;

    for(int i=0; i<iterSpacesStr.size(); i++){
        std::string omegaIterString = iterSpacesStr[i];
        omega::Relation* omegaIterSpace =
            omega::parser::ParseRelation(Computation::stripDataSpaceDelimiter(omegaIterString));

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
        std::cerr<< "err" << e.what() << std::endl;
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
    return stripDataSpaceDelimiter(omegaString.str());
}

bool Computation::assertValidDataSpaceName(const std::string &name, bool alreadyDelimited) {
    std::string errorMsg;
    if (name.empty() || (alreadyDelimited && name.length() < 3)) {
        errorMsg = "Data space names must be nonempty.";
    } else if (alreadyDelimited &&
        (name.at(1) == DATA_SPACE_DELIMITER || name.at(name.length() - 2) == DATA_SPACE_DELIMITER)) {
        errorMsg = "Data space appears to be delimited twice. This is a bug.";
    }

    if (!errorMsg.empty()) {
        throw assert_exception(errorMsg + " Error triggered for data space '" + name + "'.");
    }
    return true;
}

int Computation::firstWriteIndex(std::string dataSpace){
    for(int i = 0; i < getNumStmts(); i++) {
        for(int data_write_index = 0;
                data_write_index < getStmt(i)->getNumWrites();
                data_write_index++) {

            string writeDataSpace = getStmt(i)->getWriteDataSpace(data_write_index);
            if(dataSpace == writeDataSpace){
                return i;
            }
        }
    }
    return -1;
}

void Computation::replaceDataSpaceName(std::string original, std::string newString){
    std::string searchString;
    std::string replacedString;
    if(nameIsDelimited(original)){
        searchString = original;
        replacedString = newString;
    }
    else{
        searchString = Computation::delimitDataSpaceName(original);
        replacedString = Computation::delimitDataSpaceName(newString);
    }
    for(int i = 0; i < getNumStmts(); i++) {
        getStmt(i)->replaceDataSpace(searchString, replacedString);
    }
    //This is being used with appendComputation so we don't need
    //to save a copy
    auto iterator = dataSpaces.find(searchString);
    //auto iterator = std::find(dataSpaces.begin(), dataSpaces.end(), searchString);
    if(iterator != dataSpaces.end()){
        dataSpaces.erase(iterator);
    }
    auto original_iterator = undelimitedDataSpaceNames.find(trimDataSpaceName(original));
    if (original_iterator != undelimitedDataSpaceNames.end()) {
        undelimitedDataSpaceNames.erase(original_iterator);
    }

    //Rename return values as well
    for (auto it = returnValues.begin(); it != returnValues.end(); it++){
        if (it->first == original) { it->first = newString; }
    }
    for (auto it = parameters.begin(); it != parameters.end(); it++) {
        if (*it == original) { *it = newString; }
    }
}

std::string Computation::getDataSpaceRename(std::string dataSpaceName) {
    dataSpaceName = trimDataSpaceName(dataSpaceName);
    return Computation::delimitDataSpaceName(dataSpaceName + DATA_RENAME_STR
        + std::to_string(this->numDataSpaceRenames++));
}

void Computation::getArrayAccessStrs(std::string& unroll, std::string& access,
    std::string& tuple, std::string arrName, const std::list<int>& idxs) {
    std::ostringstream unrollOS, accessOS, tupleOS;
    unrollOS << "$" << trimDataSpaceName(arrName);
    accessOS << arrName;
    tupleOS << "[";
    for (auto it = idxs.begin(); it != idxs.end(); it++) {
        unrollOS << ARR_ACCESS_STR << *it;
        accessOS << "[" << *it << "]";
        if (it != idxs.begin()) { tupleOS << ", "; }
        tupleOS << *it;
    }
    unrollOS << "$";
    tupleOS << "]";

    unroll = unrollOS.str();
    access = accessOS.str();
    tuple = tupleOS.str();
}

/*
bool Computation::isConstArray(std::string dataSpaceName) {
    auto it = arrays.find(dataSpaceName);
    return it != arrays.end() && it->second;
}
*/


std::string Computation::trimDataSpaceName(std::string dataSpaceName) {
    return nameIsDelimited(dataSpaceName) ?
        dataSpaceName.substr(1, dataSpaceName.length() - 2)
        : dataSpaceName;
}

std::string Computation::getOriginalDataSpaceName(std::string dataSpaceName) {
    dataSpaceName = trimDataSpaceName(dataSpaceName);
    return Computation::delimitDataSpaceName(dataSpaceName.substr(0, dataSpaceName.rfind(DATA_RENAME_STR)));
}

bool Computation::areEquivalentRenames(std::string a, std::string b) {
    return getOriginalDataSpaceName(a) == getOriginalDataSpaceName(b);
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
    this->phiNode = other.phiNode;
    this->arrayAccess = other.arrayAccess;
    this->delimited = other.delimited;
    return *this;
}

bool Stmt::operator==(const Stmt& other) const {
    bool accessesEqual = true;
    if (this->getNumReads() != other.getNumReads() || this->getNumWrites() != other.getNumWrites()) {
        accessesEqual = false;
    } else {
        for (unsigned int i = 0; i < this->getNumReads(); ++i) {
            if (this->getReadDataSpace(i) != other.getReadDataSpace(i)
            || *this->getReadRelation(i) != *other.getReadRelation(i)) {
                accessesEqual = false;
            }
        }
        for (unsigned int i = 0; i < this->getNumWrites(); ++i) {
            if (this->getWriteDataSpace(i) != other.getWriteDataSpace(i)
                || *this->getWriteRelation(i) != *other.getWriteRelation(i)) {
                accessesEqual = false;
            }
        }
    }

    return (
        this->phiNode == other.phiNode &&
        this->arrayAccess == other.arrayAccess &&
        this->stmtSourceCode == other.stmtSourceCode &&
        *this->iterationSpace == *other.iterationSpace &&
        *this->executionSchedule == *other.executionSchedule &&
        accessesEqual
    );
}

void Stmt::replaceRead(std::string searchStr, std::string replaceStr) {
    if (searchStr == "") { return; }
    if (searchStr[0] != '$') { searchStr = "$" + searchStr + "$"; }
    if (replaceStr == "" || replaceStr[0] != '$') { replaceStr = "$" + replaceStr + "$"; }

    replaceReadDataSpace(searchStr, replaceStr);
    replaceReadSourceCode(searchStr, replaceStr);
}

void Stmt::replaceReadDataSpace(std::string searchStr, std::string replaceStr) {
    for (auto& read : dataReads) {
        if (read.first == searchStr) {
            read.first = replaceStr;
        }
    }
}

void Stmt::replaceReadSourceCode(std::string searchStr, std::string replaceStr) {
    // TODO: will not work if there's an if statement that
    // reads from searchStr
    // Replace all instances of search str between '=' and ';'
    std::string srcCode = getStmtSourceCode();
    std::ostringstream os;
    size_t idx = 0, pos1, pos2;
    do {
        pos1 = srcCode.find("=", idx); pos2 = srcCode.find(";", idx);
        bool noEq = pos1 == std::string::npos;
        bool noSemi = pos2 == std::string::npos;
        // Avoid overflow (max size_t - 0 + 1)
        if (noEq) { pos1--; }
        if (noSemi) { pos2--; }
        // No '=' or '=' after ';', don't replace
        if (noEq || pos1 > pos2) {
            os << srcCode.substr(idx, pos2 - idx + 1);
            idx = pos2 + 1;
        // Yes '=', replace
        } else {
            // Make sure it's not '==' or '<op>='
            bool notEqEq = pos1 + 1 >= srcCode.size()
                || srcCode[pos1 + 1] != '=';
            bool notSelfEq = pos1 - 1 < 0 || isalnum(srcCode[pos1 - 1])
                || srcCode[pos1 - 1] == ' ' || srcCode[pos1 - 1] == '_';
            if (notEqEq && notSelfEq) {
                os << srcCode.substr(idx, pos1 - idx)
                    << replaceInString(srcCode.substr(pos1, pos2 - pos1 + 1),
                        searchStr, replaceStr);
                idx = pos2 + 1;
            } else {
                os << srcCode.substr(idx, pos1 - idx + 1);
                idx = pos1 + 1;
            }
        }
    } while (idx < srcCode.size());
    setStmtSourceCode(os.str());
}

void Stmt::replaceWrite(std::string searchStr, std::string replaceStr) {
    if (searchStr == "") { return; }
    if (searchStr[0] != '$') { searchStr = "$" + searchStr + "$"; }
    if (replaceStr == "" || replaceStr[0] != '$') { replaceStr = "$" + replaceStr + "$"; }

    replaceWriteDataSpace(searchStr, replaceStr);
    replaceWriteSourceCode(searchStr, replaceStr);
}

void Stmt::replaceWriteDataSpace(std::string searchStr, std::string replaceStr) {
    for (auto& write : dataWrites) {
        if (write.first == searchStr) {
            write.first = replaceStr;
        }
    }
}

void Stmt::replaceWriteSourceCode(std::string searchStr, std::string replaceStr) {
    // TODO: will not work if there's an if statement that
    // reads from searchStr
    // Replace all instances of search str between '=' and ';'
    std::string srcCode = getStmtSourceCode();
    std::ostringstream os;
    size_t idx = 0, pos1, pos2;
    do {
        pos1 = srcCode.find("=", idx); pos2 = srcCode.find(";", idx);
        bool noEq = pos1 == std::string::npos;
        bool noSemi = pos2 == std::string::npos;
        // Avoid overflow (max size_t - 0 + 1)
        if (noEq) { pos1--; }
        if (noSemi) { pos2--; }
        // No '=' or '=' after ';', replace to ';'
        if (noEq || pos1 > pos2) {
            os << replaceInString(srcCode.substr(idx, pos2 - idx + 1),
                searchStr, replaceStr);
            idx = pos2 + 1;
        // Yes '=', replace to '=', skip '=' to ';'
        } else {
            // Make sure it's not '==' or '<op>='
            bool notEqEq = pos1 + 1 >= srcCode.size()
                || srcCode[pos1 + 1] != '=';
            bool notSelfEq = pos1 - 1 < 0 || isalnum(srcCode[pos1 - 1])
                || srcCode[pos1 - 1] == ' ' || srcCode[pos1 - 1] == '_';
            if (notEqEq && notSelfEq) {
                os << replaceInString(srcCode.substr(idx, pos1 - idx),
                    searchStr, replaceStr)
                    << srcCode.substr(pos1, pos2 - pos1 + 1);
                idx = pos2 + 1;
            } else {
                os << replaceInString(srcCode.substr(idx, pos1 - idx),
                    searchStr, replaceStr);
                idx = pos1 + 1;
            }
        }
    } while (idx < srcCode.size());
    setStmtSourceCode(os.str());
}

void Stmt::replaceDataSpace(std::string searchString, std::string replaceString) {
    if (searchString == "") { return; }

    std::string oldSourceCode = getStmtSourceCode();
    std::string newSourceCode;
    newSourceCode = iegenlib::replaceInString(oldSourceCode, searchString, replaceString);
    setStmtSourceCode(newSourceCode);

    for (auto& write : dataWrites) {
        if (write.first == searchString) { write.first = replaceString; }
    }
    for (auto& read : dataReads) {
        if (read.first == searchString) { read.first = replaceString; }
    }
    //Rename data space in iteration space 
    std::string iterStr = iterationSpace->getString();
    iterStr = iegenlib::replaceInString(iterStr, searchString, replaceString);
    iterationSpace.reset(new Set(iterStr));

    //Rename data space in execution schedule
    std::string relStr = executionSchedule->getString();
    relStr = iegenlib::replaceInString(relStr, searchString, replaceString);
    executionSchedule.reset(new Relation(relStr));
}


Stmt* Stmt::getUniquelyNamedClone(const std::string& prefix, const std::map<std::string, std::string>& dataSpaces) const {
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
    for (auto dataSpace : dataSpaces) {
        const string&  originalName = dataSpace.first;
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

void Stmt::updateRead(int idx, std::string dataSpace, std::string relationStr) {
    dataReads.at(idx).first = dataSpace;
    dataReads.at(idx).second = std::unique_ptr<Relation>(new Relation(relationStr));
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

void Stmt::updateWrite(int idx, std::string dataSpace, std::string relationStr) {
    dataWrites.at(idx).first = dataSpace;
    dataWrites.at(idx).second = std::unique_ptr<Relation>(new Relation(relationStr));
}

unsigned int Stmt::getNumWrites() const { return dataWrites.size(); }

std::string Stmt::getWriteDataSpace(unsigned int index) const {
    return dataWrites.at(index).first;
}

Relation* Stmt::getWriteRelation(unsigned int index) const {
    return dataWrites.at(index).second.get();
}

std::list<int> Stmt::getConstArrayAccesses(unsigned int index, bool read) const {
    // Get the data space and scheduling function
    std::string dataSpace = read ? getReadDataSpace(index)
        : getWriteDataSpace(index);
    Relation* schedFunc = read ? getReadRelation(index)
        : getWriteRelation(index);

    // Get all constant elements in the relation tuple
    // If an iterator is found, remove all clear the list
    TupleDecl decl = schedFunc->getTupleDecl();
    std::list<int> accessVals;
    for (int i = schedFunc->inArity(); i < decl.size(); i++) {
        if (!decl.elemIsConst(i)) {
//            std::cerr << "Iterator Access Detected" << std::endl;
            accessVals.clear();
            break;
        } else {
            accessVals.push_back(decl.elemConstVal(i));
        }
    }

    if (accessVals.size() == 1 && accessVals.front() == 0) {
        // If accessVals = {0}, check that it's actually an array access
        std::string srcCode = getStmtSourceCode();
        srcCode.erase(std::remove(srcCode.begin(), srcCode.end(), ' '), srcCode.end());
        if (getStmtSourceCode().find(dataSpace + "[0]") == std::string::npos) {
            accessVals.clear();
        }
    }

    return accessVals;
}

void Stmt::setDebugStr(std::string str){
    debug.push_back(str);
}

std::string Stmt::getDebugStr() const {
    return debug.front();
}

std::string Stmt::getAllDebugStr() const{
    std::ostringstream ss;
    for(int i = 0; i < debug.size(); ++i){
       if (i != 0){ss << "\n";}
       ss << debug[i];
    }
    return ss.str();
}

std::string Stmt::prettyPrintString() const {
    std::ostringstream ss;
    ss << "Statement:\n";
    // stmt source code
    ss << "Source Code: " << getStmtSourceCode() << "\n";
    // iter spaces
    ss << "Iteration Space: " << getIterationSpace()->prettyPrintString() << "\n";
    // exec schedules
    ss << "Execution Schedule: " << getExecutionSchedule()->prettyPrintString() << "\n";
    // data reads
    ss << "Data Reads: ";
    unsigned int numReads = getNumReads();
    if (numReads == 0) {
        ss << " none";
    } else {
        ss << "{\n";
        for (unsigned int i = 0; i < numReads; ++i) {
            ss << "    " << getReadDataSpace(i) << ": "
                         << getReadRelation(i)->prettyPrintString()
                         << "\n";
        }
        ss << "}";
    }
    ss << "\n";
    // data writes
    ss << "Data Writes: ";
    unsigned int numWrites = getNumWrites();
    if (numWrites == 0) {
        ss << " none";
    } else {
        ss << "{\n";
        for (unsigned int i = 0; i < numWrites; ++i) {
            ss << "    " << getWriteDataSpace(i) << ": "
                         << getWriteRelation(i)->prettyPrintString()
                         << "\n";
        }
        ss << "}";
    }
    ss << "\n";
    return ss.str();
}


}  // namespace iegenlib



