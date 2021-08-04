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

//! String appended to dataspaces to avoid name conflicts when SSA renaming
#define DATA_RENAME_STR "__w__"

//! Constraint used to test if constraints evaluate to false/true for phi nodes
#define CONSTR_TEST "test__"

//! String appended to arrays to crerate data spaces for constant accesses
#define ARR_ACCESS_STR "__at"

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

    return prefixedCopy;
}

void Computation::resetNumRenames() {
    Computation::numRenames = 0;
}

std::string Computation::getPrefixedDataSpaceName(const std::string& originalName, const std::string& prefix) {
    return DATA_SPACE_DELIMITER + prefix + originalName.substr(1);
}

void Computation::enforceArraySSA(Stmt* stmt) {
    // We don't want to do any array SSA enforcing for array accesses
    if (stmt->isArrayAccess()) {
        std::cerr << "Skipped Array SSA {\n"
                  << stmt->prettyPrintString() << "}" << std::endl;
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
        name << "$" << trimDataSpaceName(dataSpace);
        tuple << "[";
        for (auto it = accessVals.begin(); it != accessVals.end(); it++) {
            name << ARR_ACCESS_STR << *it;
            access << "[" << *it << "]";
            if (it != accessVals.begin()) { tuple << ", "; }
            tuple << *it;
        }
        name << "$";
        tuple << "]";
//        std::cerr << "Array Access: " << access.str() << std::endl;
//        std::cerr << "Access Tuple: " << tuple.str() << std::endl;
        std::string newDataSpace = name.str();
//        std::cerr << "New Name: " << newDataSpace << std::endl;
        // This array has not been accessed in this way before
        if (!isDataSpace(newDataSpace)) {
            // Add the rename as a data space
            addDataSpace(newDataSpace, getDataSpaceType(dataSpace));
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
                arrAccessStmt->setArrayAccess();
                addStmt(arrAccessStmt);
                std::cerr << "Added Array Access Statement {\n"
                          << arrAccessStmt->prettyPrintString() << "}" << std::endl;
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

void Computation::enforceSSA(Stmt* stmt) {
    std::vector<std::pair<std::string, std::string>> writes;
    for (int i = 0; i < stmt->getNumWrites(); i++) {
        std::string write = stmt->getWriteDataSpace(i);
        if (write == "" || write[0] != '$') { continue; }
        if (isWrittenTo(write) == -1) { continue; }
        if (isConstArray(write)) { continue; }

        std::string newWrite = getDataSpaceRename(write);
        writes.push_back({write, newWrite});

        addDataSpace(newWrite, "string");

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

void Computation::locatePhiNodes(Stmt* stmt) {
    // Phi nodes will always generate another phi node
    // which is just a duplicate of itself - skip phi nodes
    if (stmt->isPhiNode()) { return; }
    for (int i = 0; i < stmt->getNumReads(); i++) {
        locatePhiNode(stmt->getReadDataSpace(i), stmt);
    }
}

void Computation::locatePhiNode(std::string dataSpace, Stmt* srcStmt) {
//    std::cerr << "Locating Phi Node" << std::endl;
    std::pair<int, std::string> first {-1, ""}, guaranteed {-1, ""};
    for (int i = getNumStmts() - 1; i >= 0; i--) {
        Stmt* stmt = getStmt(i);
        for (int j = 0; j < stmt->getNumWrites(); j++) {
            std::string write = stmt->getWriteDataSpace(j);
            if (areEquivalentRenames(dataSpace, write)) {
                if (isGuaranteedExecute(stmt, srcStmt)) {
                    guaranteed = {i, write};
                    addPhiNode(first, guaranteed, srcStmt);
//                    std::cerr << "Phi Node Located" << std::endl;
                    return;
                } else if (first.first == -1) { first = {i, write}; }
            }
        }
    }
//    std::cerr << "No guaranteed write identified" << std::endl;
}

void Computation::addPhiNode(std::pair<int, std::string> &first, std::pair<int, std::string> &guaranteed,
        Stmt* srcStmt) {
    if (first.first == -1) { return; }
//    std::cerr << "Adding Phi Node" << std::endl;

    // Removing every tuple element will cause an error
    // add an extra tuple element at the end to ensure there is one
    // tuple element remaining
    TupleDecl pad(1);
    pad.setTupleElem(0, 0);

    // Test constraint to check for true/false constraints in firstIS
    // If it is eliminated, 1+ other constraint(s) = false
    // If it is the last constraint, all other constraints = true
    // Otherwise, ignore
    Conjunction* testConstr = new Conjunction(1);
    Exp* testExp = new Exp();
    testExp->addTerm(new VarTerm(CONSTR_TEST));
    testExp->setEquality();
    testConstr->addEquality(testExp);

    // Project out everything in the source read's iteration space
    Set* sourceIS = trimISDataSpaces(srcStmt->getIterationSpace());
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
    Set* firstIS = trimISDataSpaces(firstStmt->getIterationSpace(), trimmedNames);
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
//    std::cerr << "Source Read Constraints: ";
    for (std::string str : sourceConstr) {
        if (str != sourceConstr.at(0)) { std::cerr << " && "; }
//        std::cerr << str;
    }
//    std::cerr << std::endl;

    // Get all conditions for the first write that aren't
    // used by the source read
    std::vector<std::string> constrs;
    for (std::string constr : getSetConstraints(firstIS)) {
        if (std::find(sourceConstr.begin(), sourceConstr.end(), constr) ==
                sourceConstr.end()) {
            constrs.push_back(constr);
        }
    }
//    std::cerr << "First Write Constraints: ";
    for (std::string str : constrs) {
        if (str != constrs.at(0)) { std::cerr << " && "; }
//        std::cerr << str;
    }
//    std::cerr << std::endl;

    // Stmt source code
    std::ostringstream code;
    // Stmt reads/writes
    std::vector<std::pair<std::string, std::string>> reads, writes; 
    // We always write to this
    writes.push_back({first.second, "{[0]->[0]}"});
    // No constraints so one of the constraints = false
    // skip to guaranteed write
    if (constrs.size() == 0) {
        reads.push_back({guaranteed.second, "{[0]->[0]}"});
        code << first.second << " = " << guaranteed.second << ";";
    // The only constraint is the test constraint, so all constraints = true
    // skip to first write
    } else if (constrs.size() == 1) {
        reads.push_back({first.second, "{[0]->[0]}"});
        code << first.second << " = " << first.second << ";";
    // Some constraints are left, generate ternary expression
    } else {
        reads.push_back({first.second, "{[0]->[0]}"});
        reads.push_back({guaranteed.second, "{[0]->[0]}"});

        // Remove the test string
        std::string testStr = " == 0";
        testStr = CONSTR_TEST + testStr;
        constrs.erase(std::find(constrs.begin(), constrs.end(), testStr));
        // Write all our constraints
        for (int i = 0; i < constrs.size(); i++) {
            if (i != 0) { code << " && "; }
            code << constrs.at(i);
        }
        // Re-add "$" to data spaces
        std::string codeStr = code.str();
        for (std::string str : trimmedNames) {
            std::string newStr = "$" + str + "$";
            std::string old = codeStr;
            codeStr = iegenlib::replaceInString(codeStr, str, newStr);
            if (codeStr != old) {
                // TODO: correct access relation
                // Add the data space as a read
                reads.push_back({newStr, "{[0]->[0]}"});
            }
        }
        // Compose full string
        code.str("");
        code << first.second << " = " << codeStr << " ? "
             << first.second << " : " << guaranteed.second << ";";
    }
    // TODO: better way to identify phi nodes
    // Extra semicolon to identify phi nodes
    code << ";";

    // Create phi node
    Stmt* phiStmt = new Stmt(code.str(), sourceIS->prettyPrintString(),
                             srcStmt->getExecutionSchedule()->prettyPrintString(),
                             reads, writes);
    phiStmt->setPhiNode();
//    std::cerr << "New Phi Node {\n" << phiStmt->prettyPrintString() << "}" << std::endl;

    // Add the statement
    addStmt(phiStmt);

    // Clean up
    delete sourceIS;
    delete firstIS;

    std::cerr << "Phi Node Added {\n" << phiStmt->prettyPrintString() << "}" << std::endl;
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
    Set* s = new Set(iegenlib::replaceInString(set->getString(), "$", ""));
    return s;
}

Set* Computation::trimISDataSpaces(Set* set, std::set<std::string> &trimmedNames) {
    std::string setStr = set->getString();
    int pos = setStr.find('$');
    while (pos != std::string::npos) {
        std::string begin = setStr.substr(0, pos);
        std::string end = setStr.substr(pos + 1);
        pos = end.find('$');
        if (pos == std::string::npos) {
            setStr = begin + end;
            break;
        }
        trimmedNames.insert(end.substr(0, pos));
        setStr = begin + end.substr(0, pos) + end.substr(pos + 1);
        pos = setStr.find('$');
    }
    return new Set(setStr);
}

void Computation::addStmt(Stmt* stmt) {
    enforceArraySSA(stmt);
    locatePhiNodes(stmt);
    enforceSSA(stmt);
    stmts.push_back(stmt);
    transformationLists.push_back({});
}

Stmt* Computation::getStmt(unsigned int index) const { return stmts.at(index); }

unsigned int Computation::getNumStmts() const { return stmts.size(); }

void Computation::addDataSpace(std::string dataSpaceName, std::string dataSpaceType) {
    assertValidDataSpaceName(dataSpaceName);
    dataSpaces[dataSpaceName] = dataSpaceType;
}

std::map<std::string, std::string> Computation::getDataSpaces() const {
    return dataSpaces;
}

std::string Computation::getDataSpaceType(std::string dataSpaceName) const{
    if(isDataSpace(dataSpaceName)){
        return dataSpaces.at(dataSpaceName);
    }
    return "";
}

bool Computation::isDataSpace(std::string name) const {
    return dataSpaces.find(name) != dataSpaces.end();    
}

void Computation::addParameter(std::string paramName, std::string paramType) {
    assertValidDataSpaceName(paramName);
    parameters.push_back(paramName);
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

std::vector<std::string> Computation::getActiveOutValues() const {
    std::vector<std::string> names = getReturnValues();
    for (std::string name : parameters) {
        if (getDataSpaceType(name).find('&') != std::string::npos) {
            names.push_back(name);
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
    std::map<std::string, std::string> dataSpacesActuallyAccessed;
    for (const auto& stmt : stmts) {
        // check completeness of each statement
        if (!stmt->isComplete()) {
            return false;
        }

        // collect all data space accesses
        for (unsigned int i = 0; i < stmt->getNumReads(); ++i) {
            dataSpacesActuallyAccessed[stmt->getReadDataSpace(i)] = "";
            //dataSpacesActuallyAccessed.emplace_back(stmt->getReadDataSpace(i), "");
        }
        for (unsigned int i = 0; i < stmt->getNumWrites(); ++i) {
            dataSpacesActuallyAccessed[stmt->getWriteDataSpace(i)] = "";
            // dataSpacesActuallyAccessed.emplace_back(stmt->getWriteDataSpace(i), "");
        }
    }

    // check that list of data spaces matches those accessed in statements
    if (dataSpaces != dataSpacesActuallyAccessed) {
        return false;
    }
    
    return true;

}

std::string Computation::codeGenMemoryManagementString() {
  
    std::string outputString = ""; 
    std::map<std::string, std::string> dataSpaces = this->getDataSpaces();
    std::map<std::string, std::string>::iterator it = dataSpaces.begin();

    while (it != dataSpaces.end()){
        std::string name = it->first;

        std::string type = it->second;

        std::string combinedString = type + " " + name + ";\n";
        outputString += combinedString;
        it++;
    }
    
    return outputString;
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
            toAppend->addDataSpace(newParam,"string");
        }

        Stmt* paramDeclStmt = new Stmt();

        paramDeclStmt->setStmtSourceCode(newParam + " = " + arguments[i] + ";");
        paramDeclStmt->setIterationSpace("{[0]}");
        paramDeclStmt->setExecutionSchedule("{[0]->[" + std::to_string(idx++) +
                                            "]}");
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
        if (toAppend->getParameterType(i).find("&") != std::string::npos ||
            toAppend->getParameterType(i).find("*") != std::string::npos) {
            Stmt* paramReassignStmt = new Stmt(
                arguments[i] + " = " + param + ";",
                "{[0]}",
                "{[0]->["+std::to_string(numStmts++)+"]}",
                {{param, "{[0]->[0]}"}},
                {{arguments[i], "{[0]->[0]}"}}
            );
            toAppend->addStmt(paramReassignStmt);
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

/*std::vector<int> Computation::compressPCNodes() {
    // Because this process uses a graph structure,
    // statements are often refered to as nodes
    int numStmts = getNumStmts();

    bool visited[numStmts];
    for (int i  = 0; i < numStmts; i++) { visited[i] = false; } 

    std::queue<int> toVisit;
    for (int i = 0; i < numStmts; i++) {
        if (getStmt(i)->numOutDependencies() == 0) { toVisit.push(i); }
    }
    std::vector<int> results;

    while (!toVisit.empty()) {
        int i = toVisit.front();
        toVisit.pop();
        if (getStmt(i)->numOutDependencies() == 1) { std::cerr << "Bad: " << i << std::endl; }
        if (visited[i]) { continue; }

        Stmt* stmt = getStmt(i);

        // Backtrack and compress all in-dependencies
        std::queue<std::pair<int, int>> depQueue;
        for (int dep : stmt->getInDependencies()) {
            depQueue.push(std::make_pair(dep, i));
        }
        stmt->clearInDependencies();
        while (!depQueue.empty()) {
            auto pair = depQueue.front();
            depQueue.pop();
            Stmt* depStmt = getStmt(pair.first);
            // If the dependency is not P-C, update the out-dependencies
            // to point to stmt
            if (depStmt->numOutDependencies() != 1) {
                depStmt->removeOutDependency(pair.second);
                // If depStmt already points to stmt, depStmt may now
                // only have 1 out dependency, making it P-C
                depStmt->addOutDependency(i);
            }
            // If it only has one write (is P-C), ignore it and keep going back
            if (depStmt->numOutDependencies() == 1) {
                for (int dep : depStmt->getInDependencies()) {
                    depQueue.push(std::make_pair(dep, pair.first));
                }
            // By this point, depStmt is guaranteed not P-C.
            } else {
                // Recognize it as an in-dependency
                stmt->addInDependency(pair.first);
                toVisit.push(pair.first);       
            }
        }

        visited[i] = true;
        results.push_back(i);
    }

    results.clear();
    for (int i = 0; i < numStmts; i++) {
        if (getStmt(i)->numOutDependencies() != 1) { results.push_back(i); }
    }
    return results; 
}*/

std::string Computation::toDotString() {
    //TODO: Deal with disjunction of conjunctions later.

    CompGraph graph = CompGraph();
    graph.create(this);
//    graph.fusePCRelations();
//    graph.reduceNormalNodes();
//    graph.addDebugStmts(getStmtDebugStrings());
//    return graph.toDotString(546, true, true);
    return graph.toDotString();
}

//! Function returns a dot string representing nesting
//  and loop carrie dependency. Internally it uses
//  a lite version of polyhedral scanning to generate
//  subgraphs in the dot file.
//
std::string Computation::toDotStringOld(){
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
    
    int maxLevel = newIS[0].second->arity();
//    std::cerr << "Max Level: " << maxLevel << std::endl;
    std::vector<std::vector<Set*> > projectedIS(maxLevel);
    
    for(int i = 0 ; i < maxLevel; i++ ){
        projectedIS[i] = std::vector<Set*>(newIS.size());
    }
    //TODO: Move deletiing of active statement's set pointersto
    //      toDotScan. This is because overlapping 
    //      statements might result to new disjoint
    //      active iteration spaces of the same statement.
    //      and these disjoint active statements cannot
    //      be seen at this point. Or use unique pointers
    //      to help delete when sets get out of scope. 
    //      Easier way. :) 
    for(int i = 0; i < stmts.size(); i++){
        if (maxLevel > 0)
            // Replace all '$' because iegenlib throw a fit TODO: why does iegenlib throw a fit? (it didn't previously)
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


//! param  activeStmt is assumed to be sorted lexicographically
std::vector<std::vector<std::pair<int,Set*> > > Computation::split
	(int level, std::vector<std::pair<int,Set*> >& activeStmt){
    std::map<std::string,std::vector<std::pair<int,Set*> > > grouping;
    
    for(std::pair<int,Set*> s : activeStmt){
       if(s.second->getTupleDecl().elemIsConst(level)){
           grouping[std::to_string(
 			  s.second->getTupleDecl().elemConstVal(level))].push_back(s); 
 	     	  
       } else {
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

std::vector<std::pair<int, Set*>> Computation::getIterSpaces() {
    padExecutionSchedules();

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
    //adding header file with declerations
    generatedCode << "#include codegen.h" << std::endl;
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
                      << getDataSpaceType(stmt->getWriteDataSpace(0)) << " "  
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
    auto iterator = dataSpaces.find(searchString);
    //auto iterator = std::find(dataSpaces.begin(), dataSpaces.end(), searchString);
    if(iterator != dataSpaces.end()){
        dataSpaces.erase(iterator);
    }

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

bool Computation::isConstArray(std::string dataSpaceName) {
    auto it = arrays.find(dataSpaceName);
    return it != arrays.end() && it->second;
}


std::string Computation::trimDataSpaceName(std::string dataSpaceName) {
    return dataSpaceName == "" || dataSpaceName[0] != '$' ? dataSpaceName :
           dataSpaceName.substr(1, dataSpaceName.length() - 2);
}

std::string Computation::getOriginalDataSpaceName(std::string dataSpaceName) {
    if (dataSpaceName == "") { return ""; }
    dataSpaceName = trimDataSpaceName(dataSpaceName);
    return "$" + dataSpaceName.substr(0, dataSpaceName.rfind(DATA_RENAME_STR)) + "$";
}

bool Computation::areEquivalentRenames(std::string a, std::string b) {
    return getOriginalDataSpaceName(a).compare(getOriginalDataSpaceName(b)) == 0;
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
    if (idx >= dataReads.size()) { return; }
    dataReads[idx].first = dataSpace;
    dataReads[idx].second = std::unique_ptr<Relation>(new Relation(relationStr));
}

unsigned int Stmt::getNumReads() const { return dataReads.size(); }

std::string Stmt::getReadDataSpace(unsigned int index) const {
    return dataReads.at(index).first;
}

Relation* Stmt::getReadRelation(unsigned int index) const {
    return dataReads.at(index).second.get();
}

Relation* Stmt::getReadRelation(std::string name) const {
    auto pos = std::find_if(dataReads.begin(), dataReads.end(),
                            [&name](const std::pair<std::string, std::unique_ptr<Relation>> &p) {
                                return name.compare(p.first) == 0;
                            });
    if (pos == dataReads.end()) {
        std::cerr << "Read dataspace not found: " << name << std::endl;
        return nullptr;
    }
    return pos->second.get();
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
    if (idx >= dataWrites.size()) { return; }
    dataWrites[idx].first = dataSpace;
    dataWrites[idx].second = std::unique_ptr<Relation>(new Relation(relationStr));
}

unsigned int Stmt::getNumWrites() const { return dataWrites.size(); }

std::string Stmt::getWriteDataSpace(unsigned int index) const {
    return dataWrites.at(index).first;
}

Relation* Stmt::getWriteRelation(unsigned int index) const {
    return dataWrites.at(index).second.get();
}

Relation* Stmt::getWriteRelation(std::string name) const {
    auto pos = std::find_if(dataWrites.begin(), dataWrites.end(),
                            [&name](const std::pair<std::string, std::unique_ptr<Relation>> &p) {
                                return name.compare(p.first) == 0;
                            });
    if (pos == dataWrites.end()) {
        std::cerr << "Write dataspace not found: " << name << std::endl;
        return nullptr;
    }
    return pos->second.get();
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
