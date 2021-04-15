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

#include "set_relation/set_relation.h"

//! Base string for use in name prefixing
#define NAME_PREFIX_BASE "_iegen_"

namespace iegenlib {

/* Computation */

Computation::Computation() {}

Computation::~Computation() {
    clear();
}

Computation::Computation(const Computation& other) { *this = other; }

Computation& Computation::operator=(const Computation& other) {
    this->dataSpaces = other.dataSpaces;
    this->stmts = other.stmts;
}

bool Computation::operator==(const Computation& other) const {
    return (this->dataSpaces == other.dataSpaces && this->stmts == other.stmts);
}

Computation* Computation::getDataPrefixedCopy() {
    std::string namePrefix = NAME_PREFIX_BASE + std::to_string(numRenames++);
    Computation* prefixedCopy = new Computation();

    // prefix all data in the Computation and insert it to the new one
    for (auto& stmt : this->stmts) {
        prefixedCopy->addStmt(stmt->getDataPrefixedCopy(namePrefix));
    }
    for (auto& space : this->dataSpaces) {
        prefixedCopy->addDataSpace(namePrefix + space);
    }
    for (auto& param : this->parameters) {
        prefixedCopy->addParameter(namePrefix + param.first, param.second);
    }

    return prefixedCopy;
}

void Computation::addStmt(Stmt* stmt) {
    stmts.push_back(stmt);
}

Stmt* Computation::getStmt(unsigned int index) { return stmts.at(index); }

void Computation::addDataSpace(std::string dataSpaceName) {
    dataSpaces.emplace(dataSpaceName);
}

std::unordered_set<std::string> Computation::getDataSpaces() const {
    return dataSpaces;
}

void Computation::addParameter(std::string paramName, std::string paramType) {
    parameters.push_back({paramName, paramType});
}

std::string Computation::getParameterName(unsigned int index) const {
    return parameters.at(index).first;
}

std::string Computation::getParameterType(unsigned int index) const {
    return parameters.at(index).second;
}

unsigned int Computation::getNumParams() const {
    return parameters.size();
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

int Computation::appendComputation(Computation* other,
                                   std::vector<std::string> arguments,
                                   unsigned int level) {
    // create a working copy of the other Computation, with unique data space
    // names; this copy is discarded after use
    Computation* toAppend = other->getDataPrefixedCopy();

    const unsigned int numArgs = arguments.size();

    // store last statement's execution schedule information
    Relation* precedingExecSchedule = stmts.back()->getExecutionSchedule();
    int precedingInArity = precedingExecSchedule->inArity();
    int precedingOutArity = precedingExecSchedule->outArity();
    if (precedingExecSchedule->getNumConjuncts() != 1) {
        throw assert_exception(
            "Execution schedule should have exactly 1 Conjunction.");
    }

    // ensure valid append depth
    if ((level % 2) != 0) {
        throw assert_exception(
            "Append depth should be an even number -- odd numbers indicate "
            "loop iterator names.");
    }
    // this is a strict < because level is 0-indexed whereas arity is a count
    if (!(level < precedingOutArity)) {
        throw assert_exception(
            "Cannot append at depth " + std::to_string(level) +
            " (0-indexed), preceding schedule only has length of " +
            std::to_string(precedingOutArity));
    }

    // calculate indexes/offsets for tuple modifications
    TupleDecl precedingTuple = precedingExecSchedule->getTupleDecl();
    // index within the tuple corresponding to the specified level, adjusted for
    // input size that comes before it
    const int adjustedLevelIndex = level + precedingInArity;
    // Value to offset schedule tuples by at specified level.
    int offsetValue = precedingTuple.elemConstVal(adjustedLevelIndex) + 1;
    // Keep track of the latest execution schedule position used.
    // This initial value is chosen for the case where no statements are
    // appended, so the latest position is simply the previous one.
    int latest_value = offsetValue - 1;

    // ensure that arguments match parameter list length
    if (numArgs != toAppend->getNumParams()) {
        throw assert_exception(
            "Incorrect number of parameters specified for appendComputation -- "
            "expected " +
            std::to_string(toAppend->getNumParams()) + ", got " +
            std::to_string(numArgs));
    }
    // ensure that arguments are valid data spaces in appender
    std::unordered_set<std::string> parentDataSpaces = this->getDataSpaces();
    for (const std::string& arg : arguments) {
        if (parentDataSpaces.find(arg) == parentDataSpaces.end()) {
            throw assert_exception(
                "Argument " + arg +
                " is not a data space that exists in the Computation");
        }
    }

    // Insert declarations+assignment of (would-have-been if not for inlining)
    // parameter values, at the beginning of the appendee.
    // Assignment of a parameter i will be like:
    // [type of i] [name of i] = [name of argument i from passed-in list];
    // Insertion is done by prepending statements one at a time in reverse
    // order.
    for (int i = ((signed int)numArgs) - 1; i >= 0; --i) {
        Stmt* paramDeclStmt = new Stmt();

        paramDeclStmt->setStmtSourceCode(toAppend->getParameterType(i) + " " +
                                         toAppend->getParameterName(i) + " = " +
                                         arguments[i] + ";");
        paramDeclStmt->setIterationSpace("{[0]}");
        paramDeclStmt->setExecutionSchedule("{[0]->[" + std::to_string(i) +
                                            "]}");

        paramDeclStmt->addRead(arguments[i], "{[0]->[0]}");
        this->addDataSpace(toAppend->getParameterName(i));
        paramDeclStmt->addWrite(toAppend->getParameterName(i), "{[0]->[0]}");

        toAppend->stmts.insert(toAppend->stmts.begin(), paramDeclStmt);
    }

    // keep track of all iterators that exist at the level we're using, others
    // will be discarded
    std::vector<std::string> savedIterators;
    for (int i = precedingInArity; i < adjustedLevelIndex; ++i) {
        if (!precedingTuple.elemIsConst(i)) {
            savedIterators.push_back(precedingTuple.elemToString(i));
        }
    }

    // adjust execution schedule for each statement, including parameter
    // declarations
    unsigned int remainingParamDeclStmts = numArgs;
    bool processingOriginalStmts = false;
    for (auto currentStmt = toAppend->stmtsBegin();
         currentStmt != toAppend->stmtsEnd(); ++currentStmt) {
        // once we've finished processing prepended parameter declaration
        // statements, increase the offset for remaining (original) statements
        // by the number of prepended statements
        if (!processingOriginalStmts) {
            if (remainingParamDeclStmts == 0) {
                offsetValue += numArgs;
                processingOriginalStmts = true;
            } else {
                remainingParamDeclStmts--;
            }
        }

        // original execution schedule for statement to be appended
        Relation* appendExecSchedule = (*currentStmt)->getExecutionSchedule();
        TupleDecl appendTuple = appendExecSchedule->getTupleDecl();
        int appendInArity = appendExecSchedule->inArity();

        // construct new execution schedule tuple
        int newInArity = savedIterators.size() + appendInArity;
        int newOutArity = level + appendExecSchedule->outArity();
        TupleDecl newTuple = TupleDecl(newInArity + newOutArity);
        unsigned int currentTuplePos = 0;
        bool haveInsertedIterator = false;
        bool skippedAZero = false;
        // insert iterators from surrounding context
        for (const std::string& iterator : savedIterators) {
            newTuple.setTupleElem(currentTuplePos++, iterator);
            haveInsertedIterator = true;
        }
        // insert iterators from schedule of appended statement
        // skip '0' iterator placeholder, if present
        if (appendTuple.elemIsConst(0) && appendTuple.elemConstVal(0) == 0) {
            skippedAZero = true;
            if (appendInArity != 1) {
                throw assert_exception(
                    "Found a 0 iterator placeholder, so there shouldn't be "
                    "other iterators present (but found a total of " +
                    std::to_string(appendInArity) + " instead of 1).");
            }
        } else {
            for (int iteratorPos = 0; iteratorPos < appendInArity;
                 ++iteratorPos) {
                newTuple.copyTupleElem(appendTuple, iteratorPos,
                                       currentTuplePos++);
                haveInsertedIterator = true;
            }
        }
        // if there are no iterators, insert a constant 0 instead
        if (!haveInsertedIterator) {
            newTuple.setTupleElem(currentTuplePos++, 0);
            // if a zero was skipped earlier, it is now re-inserted, which is
            // equivalent to having never been skipped
            skippedAZero = false;
        }
        // insert base tuple elements up to specified level
        for (int it = precedingInArity; it < adjustedLevelIndex; ++it) {
            newTuple.copyTupleElem(precedingTuple, it, currentTuplePos++);
        }
        // insert specified level value, with offset, and save it
        latest_value = appendTuple.elemConstVal(appendInArity) + offsetValue;
        newTuple.setTupleElem(currentTuplePos++, latest_value);
        // insert remaining append tuple values
        for (int it = appendInArity + 1; it < appendTuple.size(); ++it) {
            newTuple.copyTupleElem(appendTuple, it, currentTuplePos++);
        }
        // if we've skipped a 0-iterator, shrink the tuple appropriately
        if (skippedAZero) {
            TupleDecl tmpTuple = TupleDecl(currentTuplePos);
            for (unsigned int i = 0; i < currentTuplePos; ++i) {
                tmpTuple.copyTupleElem(newTuple, i, i);
            }
            newTuple = tmpTuple;
            // the 0 iterator has been removed, reducing input arity
            newInArity--;
        }

        // insert a new execution schedule Relation using (only) the new tuple
        (*currentStmt)
            ->setExecutionSchedule("{" + newTuple.toString(true, newInArity) +
                                   "}");

        // copy the modified statement into this Computation
        this->addStmt(new Stmt(*(*currentStmt)));
    }

    delete toAppend;

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

std::string Computation::codeGen(Set* knownConstraints) {
    std::ostringstream generatedCode;

    // convert sets/relations to Omega format for use in codegen, and
    // collect statement macro definitions
    VisitorChangeUFsForOmega* vOmegaReplacer = new VisitorChangeUFsForOmega();
    std::vector<omega::Relation> transforms;
    std::vector<omega::Relation> iterSpaces;
    std::ostringstream stmtMacroUndefs;
    std::ostringstream stmtMacroDefs;
    int stmtCount = 0;
    for (const auto& stmt : stmts) {


	// new Codegen would require an application
	// be performed first before the set is sent
	// to omega. This is a temporary solution to
	// circumvent Omega's schedulling bug.
        Set * iterSpace = stmt->getExecutionSchedule()->
		Apply(stmt->getIterationSpace());
	iterSpace->acceptVisitor(vOmegaReplacer);
	std::string tupleString =
            iterSpace->getTupleDecl().toString();
        // Stmt Macro:
        stmtMacroUndefs << "#undef s" << stmtCount << "\n";
        stmtMacroDefs << "#define s" << stmtCount << "(" << tupleString
                      << ")   " << stmt->getStmtSourceCode() << " \n";
        stmtCount++;

        std::string omegaIterString =
            iterSpace->toOmegaString(vOmegaReplacer->getUFCallDecls());
        omega::Relation* omegaIterSpace =
            omega::parser::ParseRelation(omegaIterString);

        iterSpaces.push_back(omega::copy(*omegaIterSpace));
	// Use identity transformation instead.
        transforms.push_back(omega::Identity(iterSpace->arity()));
        delete omegaIterSpace;
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
    omega::Relation* omegaKnown =
        omega::parser::ParseRelation(omegaKnownString);

    delete vOmegaReplacer;
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

    // undefine macros, which are now extraneous
    generatedCode << stmtMacroUndefs.str() << UFMacroUndefs.str();

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

Stmt* Stmt::getDataPrefixedCopy(std::string prefix) const {
    Stmt* prefixedCopy = new Stmt(*this);

    // modify reads and writes, keeping track of original names for further replacing in other fields
    std::unordered_set<std::string> dataSpaceNames;
    for (auto& read : prefixedCopy->dataReads) {
        dataSpaceNames.emplace(read.first);
        read.first = prefix + read.first;
    }
    for (auto& write : prefixedCopy->dataWrites) {
        dataSpaceNames.emplace(write.first);
        write.first = prefix + write.first;
    }

    // replace data space names in statement source code, iteration space, execution schedule
    std::string srcCode = prefixedCopy->stmtSourceCode;
    std::string iterSpaceStr = prefixedCopy->iterationSpace->prettyPrintString();
    std::string execScheduleStr = prefixedCopy->executionSchedule->prettyPrintString();
    for (const string& originalName : dataSpaceNames) {
        srcCode = replaceInString(srcCode, originalName, prefix + originalName);
        iterSpaceStr = replaceInString(iterSpaceStr, originalName, prefix + originalName);
        execScheduleStr = replaceInString(execScheduleStr, originalName, prefix + originalName);
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

void VisitorChangeUFsForOmega::preVisitSparseConstraints(
    SparseConstraints* sc) {
    if (sc->getNumConjuncts() != 1) {
        throw assert_exception(
            "Must have exactly one conjunction for Omega conversion");
    }
    currentTupleDecl = sc->getTupleDecl();
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
