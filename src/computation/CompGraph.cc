#include "CompGraph.h"

#include "util.h"
#include "Computation.h"

//#define DEBUG

namespace iegenlib {

void Edge::generateLabel(Relation* dataRelation) {
    std::string relStr = dataRelation->getString();
    int pos1 = relStr.rfind('[');
    int pos2 = relStr.rfind(']');
    label = relStr.substr(pos1, pos2 - pos1 + 1);
}

void Edge::generateDotString(std::ostringstream &ss) {
    if (written) { return; }
    ss << "\t\t";
    if (isWrite) { ss << stmtNode->getName() << "->" << dataNode->getName(); }
    else { ss << dataNode->getName() << "->" << stmtNode->getName(); }
    ss << "[" << generateDotLabel(label)
       << "][color=" << color << "]\n";
    written = true;
}

void Node::removeInEdge(EdgePtr ptr) {
    auto it = std::find(inEdges.begin(), inEdges.end(), ptr);
    if (it != inEdges.end()) { inEdges.erase(it); }
}

void Node::removeOutEdge(EdgePtr ptr) {
    auto it = std::find(outEdges.begin(), outEdges.end(), ptr);
    if (it != outEdges.end()) { outEdges.erase(it); }
}

void Node::generateDotString(std::ostringstream &ss) {
    if (written) { return; }
    ss << name
       << "[" << generateDotLabel(label)
       << "][shape=" << shape << "][style=bold]"
       << "[color=" << color << "];\n";
    written = true;
}

void Subgraph::reduceStmts(int toLevel) {
    if (toLevel == -1 || level <= toLevel) {
        for (NodePtr stmtNode : stmts) {
            stmtNode->setShape("point");
        }
        for (Subgraph& subgraph : subgraphs) {
            subgraph.reduceStmts(toLevel);
        }
    }
}

void Subgraph::reduceDataSpaces(int toLevel) {
    if (toLevel == -1 || level <= toLevel) {
        for (NodePtr stmtNode : stmts) {
            for (EdgePtr ptr : stmtNode->getOutEdges()) {
                ptr->getDataNode()->setShape("point");
            }
        }
        for (Subgraph& subgraph : subgraphs) {
            subgraph.reduceDataSpaces(toLevel);
        }
    }
}

static int subgraphCnt;
void Subgraph::generateDotString(std::ostringstream &ss) {
    if (level == 0) { subgraphCnt = 0; }

    if (level > 0) {
//        ss << "subgraph cluster" << subgraph.level << " {\n"
        ss << "subgraph cluster" << subgraphCnt++ << " {\n"
           << "style = filled;\n"
           << "color = \"\";\n"
           << generateDotLabel(label)
           << "\n";
    }

    // Generate statements
    for (NodePtr stmtNode : stmts) {
        stmtNode->generateDotString(ss);
        for (EdgePtr ptr : stmtNode->getOutEdges()) {
            ptr->getDataNode()->generateDotString(ss);
            ptr->generateDotString(ss);
        }
    }

    // Generate subgraphs
    for (Subgraph &s : subgraphs) {
        s.generateDotString(ss);
    }

    if (level > 0) { ss << "}\n"; }
}

void CompGraph::create(Computation* comp) {
#ifdef DEBUG 
    std::cerr << "Creating Graph" << std::endl;
#endif
    stmtNodes.clear();
    dataNodes.clear();
    // Populate nodes
    for (int i = 0; i < comp->getNumStmts(); i++) {
        Stmt* stmt = comp->getStmt(i);
        NodePtr stmtNode = std::make_shared<Node>();
        for (int j = 0; j < stmt->getNumReads(); j++) {
            std::string read = comp->trimDataSpaceName(stmt->getReadDataSpace(j));
            auto it = dataNodes.find(read);
            if (it == dataNodes.end()) {
                it = dataNodes.insert(it, {read, std::make_shared<Node>()});
            }
            EdgePtr ptr = std::make_shared<Edge>(false, stmtNode, it->second);
            ptr->generateLabel(stmt->getReadRelation(j));
            stmtNode->addInEdge(ptr);
            it->second->addOutEdge(ptr);
        }
        for (int j = 0; j < stmt->getNumWrites(); j++) {
            std::string write = comp->trimDataSpaceName(stmt->getWriteDataSpace(j));
            auto it = dataNodes.find(write);
            if (it == dataNodes.end()) {
                it = dataNodes.insert(it, {write, std::make_shared<Node>()});
            }
            EdgePtr ptr = std::make_shared<Edge>(true, stmtNode, it->second);
            ptr->generateLabel(stmt->getWriteRelation(j));
            stmtNode->addOutEdge(ptr);
            it->second->addInEdge(ptr);
        }
        stmtNode->setShape("Mrecord");
        stmtNode->setName("S" + std::to_string(i));
        stmtNodes[i] = stmtNode;
    }
    for (auto& pair : dataNodes) { 
        pair.second->setShape("box");
        pair.second->setName(pair.first);
        pair.second->setLabel(pair.first);
    }

    // Generate parameter/return colors
    for (std::string param : comp->getParameters()) {
        auto it = dataNodes.find(comp->trimDataSpaceName(param));
        if (it == dataNodes.end()) { continue; }
        it->second->setColor(getDotColor(it->second->getColor(), PARAM_COLOR));
        for (EdgePtr ptr : it->second->getOutEdges()) {
            ptr->setColor(getDotColor(ptr->getColor(), PARAM_COLOR));
        }
    }
    for (std::string ret : comp->getReturnValues()) {
        auto it = dataNodes.find(comp->trimDataSpaceName(ret));
        if (it == dataNodes.end()) { continue; }
        it->second->setColor(getDotColor(it->second->getColor(), RETURN_COLOR));
        for (EdgePtr ptr : it->second->getInEdges()) {
            ptr->setColor(getDotColor(ptr->getColor(), RETURN_COLOR));
        }
    }

    std::vector<std::pair<int, Set*>> compIterSpaces = comp->getIterSpaces();
    std::vector<std::pair<NodePtr, Set*>> iterSpaces;
    // Generate statement lavels and remove any unused iteration spaces
    for (auto pair = compIterSpaces.begin(); pair != compIterSpaces.end(); pair++) {
        auto it = stmtNodes.find(pair->first);
        if (it != stmtNodes.end()) {
            it->second->setLabel(pair->second->prettyPrintString());
            it->second->addLabel("\\nS");
            it->second->addLabel(std::to_string(pair->first));
            iterSpaces.push_back({it->second, pair->second});
        }
    }
    // Generate subgraphs
    subgraph = generateSubgraph(iterSpaces, 0);
    for (auto& pair : iterSpaces) {
        delete pair.second;
    }
#ifdef DEBUG
    std::cerr << "Graph Created" << std::endl;
#endif
}

Subgraph CompGraph::generateSubgraph(std::vector<std::pair<NodePtr, Set*>> &activeStmts, int level) {
#ifdef DEBUG
    std::cerr << "Creating Subgraphs" << std::endl;
#endif
    Subgraph subgraph(level);

    if (activeStmts.size() == 0) { return subgraph; }

 // std::vector<std::vector<std::pair<int, Set*>>> bins = split(level, activeStmts);

    Set* set = activeStmts[0].second;
    if (level > 0) {
        // Replace all '$' because dot throws a fit
        Set* projectedIS = new Set(replaceInString(set->getString(), "$", ""));
        //Perform projections for each column
        for (int j = set->arity() - 1; j >= level; j--) {
            Set* tmp = projectedIS;
            projectedIS = projectedIS->projectOut(j);
            delete tmp;
        }

        subgraph.setLabel("Domain: ");
        subgraph.addLabel(projectedIS->prettyPrintString());

        delete projectedIS;
    }

    // This is the final subgraph
    if (level >= set->arity() - 1) {
        for (auto &pair : activeStmts) {
            subgraph.addStmt(pair.first);
        }
    // Differentiate between statements in this subgraph
    // and those in further nested subgraphs
    } else {
        std::map<int, std::vector<std::pair<NodePtr, Set*>>> stmtGroups;
        for (auto &pair : activeStmts) {
            TupleDecl tupleDecl = pair.second->getTupleDecl();
            // No more iterators
            if (tupleDecl.elemIsConst(level + 1)) {
                subgraph.addStmt(pair.first);
            // More iterators
            } else {                
                stmtGroups[tupleDecl.elemConstVal(level)].push_back(pair);
            }
        }
        for(auto &pair : stmtGroups) {
//            if (pair.second.size() > 1) {
                subgraph.addSubgraph(generateSubgraph(pair.second, level+2));
//            } else if (pair.second.size() == 1){
//                subgraph.addStmt(pair.second.begin()->first);
//            }
        }
    }
#ifdef DEBUG
    std::cerr << "Subgraphs Created" << std::endl;
#endif
   return subgraph;
}

std::string CompGraph::toDotString() {
#ifdef DEBUG
    std::cerr << "Creating Dot String" << std::endl;
#endif
    resetWritten();

    std::ostringstream ss;
    ss << "digraph dataFlowGraph_1{ \n";
    subgraph.generateDotString(ss);
    generateDotReads(ss);
    ss << "}";
#ifdef DEBUG
    std::cerr << "Dot String Created" << std::endl;
#endif
    return ss.str();
}

void CompGraph::generateDotReads(std::ostringstream &ss) {
    for (auto& pair : stmtNodes) {
        for (EdgePtr ptr : pair.second->getInEdges()) {
            ptr->getDataNode()->generateDotString(ss);
            ptr->generateDotString(ss);
        }
    }
}

void CompGraph::resetWritten() {
    for (auto& pair : stmtNodes) {
        pair.second->reset();
        for (EdgePtr ptr : pair.second->getInEdges()) {
            ptr->reset();
        }
        for (EdgePtr ptr : pair.second->getOutEdges()) {
            ptr->reset();
        }
    }
    for (auto& pair : dataNodes) {
        pair.second->reset();
    }
}

void CompGraph::addDebugStmts(std::vector<std::pair<int, std::string>> debugStmts) {
    // Add debug statements
    for (auto& pair : debugStmts) {
        auto it = stmtNodes.find(pair.first);
        if (it == stmtNodes.end()) { continue; }
        it->second->addLabel("\\n");
        it->second->addLabel(pair.second);
    }   
}

void CompGraph::reduceStmts(int toLevel) {
    subgraph.reduceStmts(toLevel);
}

void CompGraph::reduceDataSpaces(int toLevel) {
    subgraph.reduceDataSpaces(toLevel);
}

void CompGraph::fusePCRelations() {
/*    bool didFuse = false;
    for (auto it = stmtNodes.begin(); it != stmtNodes.end(); it++) {
        // Make sure the statement writes to only one dataSpace
        Node& writeNode = it->second;
        if (writeNode.numOutEdges() != 1) { continue; }

        // This is the edge between writeNode and dataNode
        EdgePtr write = writeNode.getOutEdge(0);
        auto dataIt = dataNodes.find(write->dataSpace);
        if (dataIt == dataNodes.end()) { continue; }
        // Make sure the data space is written to and read from only one statement,
        // respectively
        Node& dataNode = dataIt->second;
        if (dataNode.numInEdges() != 1 || dataNode.numOutEdges() != 1) { continue; }

        // This is the edge between dataNode and readNode
        EdgePtr read = dataNode.getOutEdge(0);

        // Make sure we are doing a scalar write (access space is 0)
        if (write->label != read->label) { continue; }
        if (write->label != "0" || read->label != "0") { continue; }

        // Get the read statement's node
        Node& readNode = stmtNodes.at(read->stmtIdx);
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

    if (didFuse) { fusePCRelations(); }*/
}

/*//! param  activeStmt is assumed to be sorted lexicographically
std::vector<std::vector<std::pair<int,Set*>>> CompGraph::split
	(int level, std::vector<std::pair<int,Set*> >& activeStmt){
    std::map<std::string,std::vector<std::pair<int,Set*>>> grouping;
    
    for(std::pair<int,Set*> s : activeStmt) {
       if (s.second->getTupleDecl().elemIsConst(level)) {
           grouping[std::to_string(
 			  s.second->getTupleDecl().elemConstVal(level))].push_back(s);
       } else {
 	   // This will be expanded further to use constraints;
           grouping["t"].push_back(s);
       }  
    }
    std::vector<std::vector<std::pair<int,Set*>>> res;
    for( auto k : grouping){
        //Next iteration of the algorithm will be
        //focused on this section. 
        res.push_back(k.second);
    }
    return res;
}*/

std::string CompGraph::getDotColor(std::string color1, std::string color2) {
    if (color1 == DEFAULT_COLOR) { return color2; }
    if (color2 == DEFAULT_COLOR) { return color1; }
    return PARAM_RETURN_COLOR;
}

}
