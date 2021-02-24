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
 * \authors Anna Rift
 *
 * Copyright (c) 2020, University of Arizona <br>
 * Copyright (c) 2020, Boise State University <br>
 * All rights reserved. <br>
 * See ../../COPYING for details. <br>
 */

#include "Computation.h"

#include <code_gen/parser/parser.h>
#include <codegen.h>

#include <set>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "set_relation/set_relation.h"

namespace iegenlib {

/* Computation */

Computation::Computation(const Computation& other) { *this = other; }

Computation& Computation::operator=(const Computation& other) {
    this->dataSpaces = other.dataSpaces;
    this->stmts = other.stmts;
}

bool Computation::operator==(const Computation& other) const {
    return (this->dataSpaces == other.dataSpaces && this->stmts == other.stmts);
}

Stmt* Computation::addStmt(const Stmt& stmt) {
    stmts.push_back(Stmt(stmt));
    return getStmt(stmts.size() - 1);
}

Stmt* Computation::getStmt(unsigned int index) { return &stmts.at(index); }

void Computation::addDataSpace(std::string dataSpaceName) {
    dataSpaces.emplace(dataSpaceName);
}

std::unordered_set<std::string> Computation::getDataSpaces() const {
    return dataSpaces;
}

unsigned int Computation::getNumStmts() const { return stmts.size(); }

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
        stmtsOutput << stmtName << ": " << it.getStmtSourceCode() << "\n";
        // iter spaces
        iterSpacesOutput << stmtName << ": "
                         << it.getIterationSpace()->prettyPrintString() << "\n";
        // exec schedules
        execSchedulesOutput << stmtName << ": "
                            << it.getExecutionSchedule()->prettyPrintString()
                            << "\n";
        // data reads
        unsigned int numReads = it.getNumReads();
        dataReadsOutput << stmtName << ":";
        if (numReads == 0) {
            dataReadsOutput << " none";
        } else {
            dataReadsOutput << "{\n";
            for (unsigned int i = 0; i < numReads; ++i) {
                dataReadsOutput << "    " << it.getReadDataSpace(i) << ": "
                                << it.getReadRelation(i)->prettyPrintString()
                                << "\n";
            }
            dataReadsOutput << "}";
        }
        dataReadsOutput << "\n";
        // data writes
        unsigned int numWrites = it.getNumWrites();
        dataWritesOutput << stmtName << ":";
        if (numWrites == 0) {
            dataWritesOutput << " none";
        } else {
            dataWritesOutput << "{\n";
            for (unsigned int i = 0; i < numWrites; ++i) {
                dataWritesOutput << "    " << it.getReadDataSpace(i) << ": "
                                 << it.getReadRelation(i)->prettyPrintString()
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
        if (!stmt.isComplete()) {
            return false;
        }

        // collect all data space accesses
        for (unsigned int i = 0; i < stmt.getNumReads(); ++i) {
            dataSpacesActuallyAccessed.emplace(stmt.getReadDataSpace(i));
        }
        for (unsigned int i = 0; i < stmt.getNumWrites(); ++i) {
            dataSpacesActuallyAccessed.emplace(stmt.getWriteDataSpace(i));
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
    stmts.clear();
}

int Computation::appendComputation(Computation* other) {
    // store last statement's execution schedule information
    Relation* precedingExecSchedule = stmts.back().getExecutionSchedule();
    if (precedingExecSchedule->getNumConjuncts() != 1) {
        throw assert_exception(
            "Execution schedule should have exactly 1 Conjunction.");
    }
    TupleDecl precedingTuple = precedingExecSchedule->getTupleDecl();
    int offsetValue =
            precedingTuple.elemConstVal(precedingExecSchedule->inArity())+1;
    int latest_value = offsetValue;
    

    // adjust execution schedule for each statement
    for (unsigned int i = 0; i < other->getNumStmts(); ++i) {
        Stmt* currentStmt = new Stmt(*other->getStmt(i));
        // collect original execution schedule information for statement to be
        // appended
        Relation* appendExecSchedule = currentStmt->getExecutionSchedule();
        // construct new execution schedule tuple
        TupleDecl newTuple = TupleDecl(appendExecSchedule->getTupleDecl());
        int oldValue = newTuple.elemConstVal(appendExecSchedule->inArity());
        latest_value = oldValue + offsetValue;
        newTuple.setTupleElem(appendExecSchedule->inArity(),
                              latest_value);
        // build a new execution schedule relation using the new tuple
        Relation* newExecSchedule = new Relation(
            appendExecSchedule->inArity(), appendExecSchedule->outArity());
        newExecSchedule->setTupleDecl(newTuple);
        Conjunction* newConj =
            new Conjunction(newTuple.size(), appendExecSchedule->inArity());
        newConj->setTupleDecl(newTuple);
        Conjunction* conjToCopy = *appendExecSchedule->conjunctionBegin();
        for (auto eq : conjToCopy->equalities()) {
            newConj->addEquality(eq->clone());
        }
        for (auto ineq : conjToCopy->inequalities()) {
            newConj->addInequality(ineq->clone());
        }
        newExecSchedule->addConjunction(newConj);
        // replace old execution schedule with modified one
        currentStmt->setExecutionSchedule(newExecSchedule->prettyPrintString());
        delete newExecSchedule;

        // add the modified statement to this Computation
        addStmt(*currentStmt);
    }
    return latest_value;
}

void Computation::toDot(std::fstream& dotFile, string fileName) {
    std::vector<string>
        data_spaces;  // Maintains the list of dataspaces already created
    std::vector<string> loop_variable;  // Maintains list of loop variables
    std::cout << "Reached toDot()"
              << "\n";
    dotFile.open(fileName,
                 std::ios::out);  // Open the file to write the dot code

    // Start of the dotFile
    dotFile << "digraph dataFlowGraph_1{ \n";

    // Adding nodes for each statement
    for (int i = 0; i < getNumStmts(); i++) {
        dotFile << '\t' << "subgraph cluster_S" << i << " { \n"
                << "\t\t"
                << "style = bold; \n"
                << "\t\t"
                << "color = grey; \n"
                << "\t\t"
                << ""
                << "\t\t"
                << "label = \" Domain: "
                << getStmt(i)->getIterationSpace()->prettyPrintString()
                << "\"; \n"
                << "\t\t"
                << "S" << i << "[label= \" " << getStmt(i)->getStmtSourceCode()
                << "\"][shape=Mrecord][style=filled][color=lightgrey] ; \n"
                << "\t\t"
                << "}";
    }

    // Adding the participating dataspaces for each statement and mapping out
    // the read and write access.
    for (int i = 0; i < getNumStmts(); i++) {
        // Iterates over the read-DataSpaces
        for (int data_read_index = 0;
             data_read_index < getStmt(i)->getNumReads(); data_read_index++) {
            string readDataSpace =
                getStmt(i)->getReadDataSpace(data_read_index);
            // Check to make sure the data space is not created if it already
            // exists
            if (!(std::count(data_spaces.begin(), data_spaces.end(),
                             readDataSpace))) {
                // Creates data space
                dotFile
                    << "\t\t"
                    << "subgraph cluster_dataspace" << readDataSpace << "{ \n"
                    << "\t\t\t"
                    << "style = filled; \n"
                    << "\t\t\t"
                    << "color = lightgrey; \n"
                    << "\t\t\t"
                    << "label=\" \"; \n"
                    << "\t\t\t" << readDataSpace << "[label=\"" << readDataSpace
                    << "[] \"] [shape=box][style=filled][color=lightgrey];\n"
                    << "\t\t\t"
                    << "}\n";

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

            dotFile << "\t\t" << readDataSpace << "->"
                    << "S" << i << "[label=\"["
                    << getStmt(i)
                           ->getReadRelation(data_read_index)
                           ->getString()
                           .substr(start_pos + 1, end_pos - start_pos - 1)
                    << "]\"]"
                    << "\n";
        }

        // Iterates over the read-DataSpaces
        for (int data_write_index = 0;
             data_write_index < getStmt(i)->getNumWrites();
             data_write_index++) {
            string writeDataSpace =
                getStmt(i)->getWriteDataSpace(data_write_index);
            // Check to make sure the data space is not created if it already
            // exists
            if (!(std::count(data_spaces.begin(), data_spaces.end(),
                             writeDataSpace))) {
                dotFile
                    << "\t\t"
                    << "subgraph cluster_dataspace" << writeDataSpace << "{ \n"
                    << "\t\t\t"
                    << "style = filled; \n"
                    << "\t\t\t"
                    << "color = lightgrey; \n"
                    << "\t\t\t"
                    << "label= \" \"; \n"
                    << "\t\t\t" << writeDataSpace << "[label=\""
                    << writeDataSpace
                    << "[] \"] [shape=box][style=filled][color=lightgrey];\n"
                    << "\t\t\t"
                    << "}\n";

                data_spaces.push_back(writeDataSpace);
            }

            size_t start_pos = getStmt(i)
                                   ->getWriteRelation(data_write_index)
                                   ->getString()
                                   .find("[");
            size_t end_pos = getStmt(i)
                                 ->getWriteRelation(data_write_index)
                                 ->getString()
                                 .find("]");

            dotFile << "\t\t"
                    << "S" << i << "->" << writeDataSpace << "[label=\"["
                    << getStmt(i)
                           ->getWriteRelation(data_write_index)
                           ->getString()
                           .substr(start_pos + 1, end_pos - start_pos - 1)
                    << "]\"]"
                    << "\n";
        }
    }

    dotFile << "}\n";
    dotFile.close();
}

std::string Computation::toOmegaString() {
    std::ostringstream omegaString;
    try {
        VisitorChangeUFsForOmega* vOmegaReplacer =
            new VisitorChangeUFsForOmega();
        int stmtCount = 0;
        for (const auto& stmt : stmts) {
            std::string tupleString =
                stmt.getIterationSpace()->getTupleDecl().toString();

            omegaString << "\n#Statment " << stmtCount << " ("
                        << stmt.getStmtSourceCode() << ") \n";

            // process iterSpace for Omega
            Set* modifiedIterSpace = new Set(*stmt.getIterationSpace());
            modifiedIterSpace->acceptVisitor(vOmegaReplacer);
            std::string omegaIterString = modifiedIterSpace->toOmegaString(
                vOmegaReplacer->getUFCallDecls());
            omegaString << "Domain: " << stmtCount << "\n"
                        << omegaIterString << "\n";
            delete modifiedIterSpace;
            vOmegaReplacer->reset();

            // process transform (exec schedule) for Omega
            Relation* modifiedTransform =
                new Relation(*stmt.getExecutionSchedule());
            modifiedTransform->acceptVisitor(vOmegaReplacer);
            std::string omegaTransformString = modifiedTransform->toOmegaString(
                vOmegaReplacer->getUFCallDecls());

            omegaString << "Schedule: " << stmtCount << "\n"
                        << omegaTransformString << "\n";
            delete modifiedTransform;
            vOmegaReplacer->reset();
        }
        delete vOmegaReplacer;
        omegaString << "\n";

    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    return omegaString.str();
}

std::string Computation::codeGen() {
    std::ostringstream generatedCode;

    try {
        // convert sets/relations to Omega format for use in codegen, and
        // collect required macro substitutions
        std::vector<omega::Relation> transforms;
        std::vector<omega::Relation> iterSpaces;

        VisitorChangeUFsForOmega* vOmegaReplacer =
            new VisitorChangeUFsForOmega();
        int stmtCount = 0;
        for (const auto& stmt : stmts) {
            std::string tupleString =
                stmt.getIterationSpace()->getTupleDecl().toString();
            // Stmt Macro:
            generatedCode << "#undef s" << stmtCount << "(" << tupleString
                          << ") \n"
                          << "#define s" << stmtCount << "(" << tupleString
                          << ")   " << stmt.getStmtSourceCode() << " \n";
            stmtCount++;

            // process iterSpace for Omega
            Set* modifiedIterSpace = new Set(*stmt.getIterationSpace());
            modifiedIterSpace->acceptVisitor(vOmegaReplacer);
            generatedCode << vOmegaReplacer->getMacrosString();
            std::string omegaIterString = modifiedIterSpace->toOmegaString(
                vOmegaReplacer->getUFCallDecls());
            delete modifiedIterSpace;
            vOmegaReplacer->reset();

            // process transform (exec schedule) for Omega
            Relation* modifiedTransform =
                new Relation(*stmt.getExecutionSchedule());
            modifiedTransform->acceptVisitor(vOmegaReplacer);
            generatedCode << vOmegaReplacer->getMacrosString();
            std::string omegaTransformString = modifiedTransform->toOmegaString(
                vOmegaReplacer->getUFCallDecls());
            delete modifiedTransform;
            vOmegaReplacer->reset();

            // create and insert new Omega data structures
            omega::Relation* omegaIterSpace =
                omega::parser::ParseRelation(omegaIterString);
            omega::Relation* omegaTransform =
                omega::parser::ParseRelation(omegaTransformString);
            iterSpaces.push_back(omega::copy(*omegaIterSpace));
            transforms.push_back(omega::copy(*omegaTransform));
            delete omegaIterSpace;
            delete omegaTransform;
        }
        delete vOmegaReplacer;
        generatedCode << "\n";

        // do Omega CodeGen
        omega::CodeGen cg(transforms, iterSpaces);
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

    return generatedCode.str();
}

/* Stmt */

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
        i++;
    }

    return true;
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

void Stmt::addRead(std::string dataSpace, std::string relationStr) {
    dataReads.push_back(
        {dataSpace, std::unique_ptr<Relation>(new Relation(relationStr))});
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
    macros.str(std::string());
    ufCallDecls.clear();
    nextFuncReplacementNumber = 0;
    nextVarReplacementNumber = 0;
}

std::string VisitorChangeUFsForOmega::getMacrosString() { return macros.str(); }

std::set<std::string> VisitorChangeUFsForOmega::getUFCallDecls() {
    return ufCallDecls;
}

void VisitorChangeUFsForOmega::postVisitUFCallTerm(UFCallTerm* callTerm) {
    // set up macro outputs
    std::ostringstream os_replaceFrom;
    std::ostringstream os_replaceTo;

    // set new function name
    std::string replacementName =
        callTerm->name() + "_" + std::to_string(nextFuncReplacementNumber++);
    os_replaceFrom << replacementName;
    os_replaceTo << callTerm->name() << "(";
    callTerm->setName(replacementName);

    // process every parameter
    bool pastFirstParam = false;
    int paramNumber = 0;
    bool haveAddedToOutput;
    bool haveAddedToInput;
    Exp* paramExp;
    // maintain a list of parameters that will remain in the call
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
        for (const auto& term : paramExp->getTermList()) {
            if (term->isConst()) {
                // add the term to the function call, without making an
                // input param for it
                os_replaceTo << (haveAddedToOutput ? "+" : "") << "("
                             << term->toString() << ")";
            } else {
                // add the term to both the input and output function call
                if (!haveAddedToInput && !pastFirstParam) {
                    os_replaceFrom << "(";
                }
                os_replaceFrom
                    << ((pastFirstParam || haveAddedToInput) ? "," : "") << "p"
                    << paramNumber;
                os_replaceTo << (haveAddedToOutput ? "+" : "") << "p"
                             << paramNumber;
                termsToSave.push_back(term->clone());
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

        // add UF call to the list of declarations
        ufCallDecls.emplace(callTerm->name() + "(" +
                            std::to_string(callTerm->numArgs()) + ")");

        i++;
    }

    // complete outputs for this UF call
    if (haveAddedToInput) {
        os_replaceFrom << ")";
    }
    os_replaceTo << ")";
    macros << "#define " << os_replaceFrom.str() << " " << os_replaceTo.str()
           << "\n";
}

}  // namespace iegenlib
