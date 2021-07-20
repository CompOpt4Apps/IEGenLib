#include "CompGraph.h"

#include "util.h"
#include "Computation.h"

namespace iegenlib {

void Edge::setAccessSpace(Relation* dataRelation) {
    std::string relStr = dataRelation->getString();
    int pos1 = relStr.rfind('[');
    int pos2 = relStr.rfind(']');
    accessSpace = relStr.substr(pos1 + 1, pos2 - pos1 + 1);
}

std::string Edge::toDotString() {
    std::string color = DEFAULT_COLOR;
    std::ostringstream ss;
    ss << "\t\t";
    if (isWrite) {
        ss << "S" << stmtIdx << "->\"" << dataSpace << "\"";
//        if (isReturnValue(dataSpace)) { color = RETURN_COLOR; }
    } else {
        ss << "\"" << dataSpace << "\"->S" << stmtIdx;
//        if (isParameter(dataSpace)) P color = PARAM_COLOR; }
    }
    ss << "[" << generateDotLabel(accessSpace)
       << "][color=" << color << "]\n";
    return ss.str();
}

void Node::removeInEdge(EdgePtr ptr) {
    auto it = std::find(inEdges.begin(), inEdges.end(), ptr);
    if (it != inEdges.end()) { inEdges.erase(it); }
}

void Node::removeOutEdge(EdgePtr ptr) {
    auto it = std::find(outEdges.begin(), outEdges.end(), ptr);
    if (it != outEdges.end()) { outEdges.erase(it); }
}

void CompGraph::create(Computation* comp) {
    stmtNodes.clear();
    dataNodes.clear();
    for (int i = 0; i < comp->getNumStmts(); i++) {
        Stmt* stmt = comp->getStmt(i);
        // TODO: Color
        for (int j = 0; j < stmt->getNumReads(); j++) {
            std::string read = stmt->getReadDataSpace(j);
            Relation* readRel = stmt->getReadRelation(j); 
            EdgePtr ptr = std::make_shared<Edge>(false, i, read,
                                                 readRel, DEFAULT_COLOR);
            stmtNodes[i].addInEdge(ptr);
            dataNodes[read].addOutEdge(ptr);
        }
        for (int j = 0; j < stmt->getNumWrites(); j++) {
            std::string write = stmt->getWriteDataSpace(j);
            Relation* writeRel = stmt->getWriteRelation(j);
            EdgePtr ptr = std::make_shared<Edge>(true, i, write,
                                                 writeRel, DEFAULT_COLOR);
            stmtNodes[i].addOutEdge(ptr);
            dataNodes[write].addInEdge(ptr);
        }
    }
}

void CompGraph::fusePCRelations() {
    bool didFuse = false;
    for (auto it = stmtNodes.begin(); it != stmtNodes.end(); it++) {
        // Make sure the statement writes to only one dataSpace
        Node writeNode = it->second;
        if (writeNode.numOutEdges() != 1) { continue; }

        // This is the edge between writeNode and dataNode
        EdgePtr write = writeNode.getOutEdge(0);
        auto dataIt = dataNodes.find(write->dataSpace);
        if (dataIt == dataNodes.end()) { continue; }
        // Make sure the data space is written to and read from only one statement,
        // respectively
        Node dataNode = dataIt->second;
        if (dataNode.numInEdges() != 1 || dataNode.numOutEdges() != 1) { continue; }

        // This is the edge between dataNode and readNode
        EdgePtr read = dataNode.getOutEdge(0);

        // Make sure we are doing a scalar write (access space is 0)
        if (write->accessSpace.compare(read->accessSpace) != 0) { continue; }
        if (write->accessSpace != "0" || read->accessSpace != "0") { continue; }

        // Get the read statement's node
        Node readNode = stmtNodes.at(read->stmtIdx);
        // Remove its connection to dataNode
        readNode.removeInEdge(read);
        // Connect it to and update all of writeNode's reads
        for (EdgePtr ptr : readNode.getInEdges()) {
            ptr->stmtIdx = read->stmtIdx;
            readNode.addInEdge(ptr);
        }

        didFuse = true;

        // Remove dataNode
        dataNodes.erase(dataIt);
        // Remove writeNode
        it = stmtNodes.erase(it);
        if (it == stmtNodes.end()) { break; }
    }

    if (didFuse) { fusePCRelations(); }
}

std::string CompGraph::toDotString(std::vector<std::pair<int, Set*>> &iterSpace) {
    std::ostringstream ss;
    ss << "digraph dataFlowGraph_1{ \n";
    generateDotStmts(iterSpace, 0, ss);
    generateDotReadWrites(ss);
    ss << "}";
    return ss.str();
}

std::string CompGraph::toDotString(std::vector<std::pair<int, Set*>> &iterSpace, int stmtIdx) {

}

//! param  activeStmt is assumed to be sorted lexicographically
std::vector<std::vector<std::pair<int,Set*> > > CompGraph::split
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

void CompGraph::generateDotStmts(std::vector<std::pair<int, Set*>> &activeStmts, int level,
	       std::ostringstream& ss){
    if(activeStmts.size() == 1){
	std::string stmIter = activeStmts[0].second->prettyPrintString();

        std::string label;
        // label = getStmt(activeStmts[0].first)->getStmtSourceCode();
        label = "S" + std::to_string(activeStmts[0].first);

        ss << "S" << activeStmts[0].first
           << "[" << generateDotLabel({stmIter, "\\n ", label})
           << "][shape=Mrecord][style=bold]"
           << "[color=" << DEFAULT_COLOR << "];\n";
        return;
    }
    std::vector<std::vector< std::pair <int, Set*> > > bins=
	    split(level,activeStmts);
    if(bins.size() > 1 && level > 0){
        // Replace all '$' because dot throws a fit
        Set* projectedIS = new Set(replaceInString(activeStmts[0].second->getString(), "$", ""));
            //Perform projections for each column
        for (int j = activeStmts[0].second->arity() - 1; j >= level; j--) {
            Set* tmp = projectedIS;
            projectedIS = projectedIS->projectOut(j);
            delete tmp;
        }

        std::string domainIter = projectedIS->prettyPrintString();

        ss << "subgraph cluster" << level << " {\n"
           << "style = filled;\n"
           << "color = \"\";\n"
           << generateDotLabel({"Domain :", domainIter})
           << "\n";

        delete projectedIS;
    }

    for(auto active : bins){
        generateDotStmts(active, level+1, ss);
    }
    if(bins.size() > 1 && level > 0){
        ss << "}\n"; 
    }
}

void CompGraph::generateDotReadWrites(std::ostringstream &ss) {
    for (auto pair : dataNodes) {
        ss << "\"" << pair.first << "\"["
           << generateDotLabel(pair.first)
           << "][shape=box][style=bold]"
           << "[color=" << /*getDataSpaceDotColor(pair.first)*/DEFAULT_COLOR << "];\n";

        Node node = pair.second;
        for (EdgePtr ptr : node.getInEdges())  { ss << ptr->toDotString(); }
        for (EdgePtr ptr : node.getOutEdges())  { ss << ptr->toDotString(); }
    }
}

}
