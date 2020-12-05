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

#include "codegen.h"
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

void Computation::toDot(std::fstream& dotFile, string fileName){

  std::vector<string> data_spaces;    //Maintains the list of dataspaces already created
  std::vector<string> loop_variable;  //Maintains list of loop variables
  std::cout<<"Reached toDot()"<<"\n"; 
  dotFile.open(fileName,std::ios::out); // Open the file to write the dot code

  // Start of the dotFile
  dotFile << "digraph dataFlowGraph_1{ \n";

  //Adding nodes for each statement
  for(int i=0; i<getNumStmts(); i++){
        dotFile << '\t' << "subgraph cluster_S" << i <<" { \n"
		       << "\t\t" << "style = bold; \n"
			   << "\t\t" << "color = grey; \n"
               << "\t\t" << ""
			   << "\t\t" <<"label = \" Domain: " << getStmt(i)->getIterationSpace()->prettyPrintString() <<"\"; \n"
			   << "\t\t" << "S" << i <<"[label= \" " << getStmt(i)->getStmtSourceCode() <<"\"][shape=Mrecord][style=filled][color=lightgrey] ; \n"
               << "\t\t" << "}";
  }

  //Adding the participating dataspaces for each statement and mapping out the read and write access.  
  for(int i=0; i<getNumStmts(); i++) {
       
       //Iterates over the read-DataSpaces 
       for(int data_read_index=0; data_read_index< getStmt(i)->getNumReads(); data_read_index++){

           string readDataSpace = getStmt(i)->getReadDataSpace(data_read_index);
            // Check to make sure the data space is not created if it already exists
            if(!(std::count(data_spaces.begin(), data_spaces.end(), readDataSpace))){
                // Creates data space
                dotFile << "\t\t" << "subgraph cluster_dataspace" << readDataSpace << "{ \n"
                        << "\t\t\t" << "style = filled; \n"
                        << "\t\t\t" << "color = lightgrey; \n"
                        << "\t\t\t" << "label=\" \"; \n"
                        << "\t\t\t" << readDataSpace << "[label=\"" << readDataSpace << "[] \"] [shape=box][style=filled][color=lightgrey];\n"
                        << "\t\t\t" << "}\n";
                                   
                data_spaces.push_back(readDataSpace);
            }

            size_t start_pos = getStmt(i)->getReadRelation(data_read_index)->getString().rfind("[");
            size_t end_pos = getStmt(i)->getReadRelation(data_read_index)->getString().rfind("]");

             dotFile <<"\t\t" << readDataSpace 
                     <<  "->" 
                     << "S" << i 
                     << "[label=\"["<< getStmt(i)->getReadRelation(data_read_index)->getString().substr(start_pos+1,end_pos-start_pos-1)<<"]\"]"
                     << "\n";
        }

        //Iterates over the read-DataSpaces
        for(int data_write_index=0; data_write_index< getStmt(i)->getNumWrites(); data_write_index++){

            string writeDataSpace = getStmt(i)->getWriteDataSpace(data_write_index);
            // Check to make sure the data space is not created if it already exists
            if(!(std::count(data_spaces.begin(), data_spaces.end(), writeDataSpace))) {
                
                 dotFile << "\t\t" << "subgraph cluster_dataspace" << writeDataSpace << "{ \n"
                         << "\t\t\t" << "style = filled; \n"
                         << "\t\t\t" << "color = lightgrey; \n"
                         << "\t\t\t" << "label= \" \"; \n"
                         << "\t\t\t" << writeDataSpace << "[label=\"" << writeDataSpace << "[] \"] [shape=box][style=filled][color=lightgrey];\n"
                         << "\t\t\t" << "}\n";
                                     
                         data_spaces.push_back(writeDataSpace);
               }

            size_t start_pos = getStmt(i)->getWriteRelation(data_write_index)->getString().find("[");
            size_t end_pos = getStmt(i)->getWriteRelation(data_write_index)->getString().find("]");

            dotFile <<"\t\t" << "S" 
                    << i 
                    << "->" 
                    << writeDataSpace
                    << "[label=\"["<< getStmt(i)->getWriteRelation(data_write_index)->getString().substr(start_pos+1,end_pos-start_pos-1)<<"]\"]"
                    << "\n";
            
         }
                         
    }

    dotFile << "}\n";
    dotFile.close();


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

std::string Computation::codeGen() {
    try {
        std::vector<omega::Relation> transforms;
        std::vector<omega::Relation> iterSpaces;
        omega::CodeGen cg(transforms, iterSpaces);
        omega::CG_result* cgr = cg.buildAST();
        if (cgr != NULL) {
            std::string s = cgr->printString();
            std::cout << s << std::endl;
            delete cgr;
        } else
            std::cout << "/* empty */" << std::endl;
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}
}  // namespace iegenlib
