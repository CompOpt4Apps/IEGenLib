#include "CompGraph.h"

#include "util.h"
#include "Computation.h"

//#define DEBUG

namespace iegenlib {

struct NodeType {
    std::string shape, style, color;

    std::string generate() const {
        std::ostringstream ss;
        if (shape != "") { ss << "[shape=" << shape << "]"; }
        if (style != "") { ss << "[style=" << style << "]"; }
        if (color != "") { ss << "[color=" << color << "]"; }
        return ss.str();
    }
};

const std::pair<const char*, NodeType> nodeTypes[] = {
    std::make_pair("Debug Statements are Enabled", NodeType {"box", "", "white"}),
    std::make_pair("Statement", NodeType {"Mrecord", "bold", DEFAULT_COLOR}),
    std::make_pair("Data Space", NodeType {"box", "bold", DEFAULT_COLOR}),
    std::make_pair("Read-only Parameter", NodeType {"box", "bold", PARAM_COLOR}),
    std::make_pair("Parameter", NodeType {"box", "filled", PARAM_COLOR}),
    std::make_pair("Returned Data Space", NodeType {"box", "bold", RETURN_COLOR}),
    std::make_pair("Read-only Returned Paramter", NodeType {"box", "bold", PARAM_RETURN_COLOR}),
    std::make_pair("Returned Parameter", NodeType {"box", "filled", PARAM_RETURN_COLOR}),
    std::make_pair("Source Statement", NodeType {"Mrecord", "bold", SOURCE_COLOR}),
    std::make_pair("Hidden Statement/Data Space", NodeType {"point", "", ""})
};

std::string generateNode(std::string name, std::string label, NodeTypes type) {
    std::ostringstream ss;
    ss << name << "[" << generateDotLabel(label) << "]"
       << nodeTypes[type].second.generate() << ";\n";
    return ss.str();
}

std::string generateLegendNode(NodeTypes type) {
    std::string label = nodeTypes[type].first;
    const NodeType &nodeType = nodeTypes[type].second;
    std::ostringstream ss;
    if (nodeType.shape == "point") {
        ss << "subgraph clusterLegend" << type << " {\n"
           << "style=bold;\n" << generateDotLabel(label) << ";\n";
    }
    ss << "Legend" << type << "[" << generateDotLabel(label) << "]"
       << nodeType.generate() << ";\n";
    if (nodeType.shape == "point") { ss << "}\n"; }
    return ss.str();
}

std::string createLegend(const std::set<NodeTypes> &nodes) {
#ifdef DEBUG
    std::cerr << "Creating Legend" << std::endl;
#endif

    std::ostringstream ss, arrows;
    ss << "subgraph clusterLegend {\n"
       << "style = bold;\n"
       << "color = black;\n"
       << "label = \"Legend\";\n";

    for (NodeTypes type : nodes) {
        ss << generateLegendNode(type);
        if (type != *nodes.begin()) { arrows << "->"; }
        arrows << "Legend" << type;
    }
    
//    ss << arrows.str() << "[style=invis]\n";

    ss << "}\n";

#ifdef DEBUG
    std::cerr << "Legend Created" << std::endl;
#endif

    return ss.str();
}


/*Edge copy() {
    Edge edge;
    edge.write = write;
    edge.label = label;
    edge.accessStr = accessStr;
    edge.color = color;
    return edge;
}*/

void Edge::generateLabel(Relation* dataRelation) {
    std::string relStr = dataRelation->getString();
    int pos1 = relStr.rfind('[');
    int pos2 = relStr.rfind(']');
    accessStr = relStr.substr(pos1, pos2 - pos1 + 1);
    label = accessStr;
}

void Edge::generateDotString(std::ostringstream &ss) {
    if (written) { return; }
    ss << "\t\t";
    if (write) { ss << stmtNode->getName() << "->" << dataNode->getName(); }
    else { ss << dataNode->getName() << "->" << stmtNode->getName(); }
    ss << "[" << generateDotLabel(label)
       << "][style=bold][color=" << color << "]\n";
    written = true;
}

bool Edge::isConst() {
    for (auto &ch : accessStr) {
        if (isalpha(ch)) { return false; }
    }
    return true;
}

/*Node copy() {
    Node node;
    node.name = name;
    node.label = label;
    node.shape = shape;
    node.color = color;
}*/

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
    ss << generateNode(name, label, type);
    written = true;
}

void Subgraph::reduceStmts(int toLevel) {
    if (toLevel == -1 || level <= toLevel) {
        for (NodePtr stmtNode : stmts) {
            stmtNode->setType(NodeTypes::hidden);
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
                ptr->getDataNode()->setType(NodeTypes::hidden);
            }
        }
        for (Subgraph& subgraph : subgraphs) {
            subgraph.reduceDataSpaces(toLevel);
        }
    }
}

void Subgraph::generateDotString(std::ostringstream &ss) {
    int cnt = 0;
    generateDotString(ss, cnt);
}

void Subgraph::generateDotString(std::ostringstream &ss, int &cnt) {
    if (level > 0) {
//        ss << "subgraph cluster" << subgraph.level << " {\n"
        ss << "subgraph cluster" << cnt << " {\n"
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
    cnt++;
    for (Subgraph &sg : subgraphs) {
        sg.generateDotString(ss, cnt);
    }

    if (level > 0) { ss << "}\n"; }
}

bool Subgraph::removeStmt(NodePtr stmt) {
    for (auto it = stmts.begin(); it != stmts.end(); it++) {
        if (*it == stmt) { stmts.erase(it); return true; }
    }
    for (Subgraph& sg : subgraphs) {
        if (sg.removeStmt(stmt)) { return true; }
    }
    return false;
}


/*CompGraph CompGraph::copy() {
    CompGraph compGraph;
    compGraph.subgraph = copySubgraph(compGraph, subgraph);
}

Subgraph CompGraph::copySubgraph(CompGraph &toGraph, Subgraph &fromSG) {
    Subgraph copySG;
 
    for (StmtPtr stmt : fromSG.stmts) {

    }   

    for (Subgraph &sg : fromSG.subgraphs) {
        copySG.addSubgraph(copySubgraph(toGraph, sg));
    }
    return copySG;
}*/

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
        stmtNode->setType(NodeTypes::stmt);
        stmtNode->setName("S" + std::to_string(i));
        stmtNodes[i] = stmtNode;
    }
    for (auto& pair : dataNodes) {
        pair.second->setType(NodeTypes::data); 
        pair.second->setName(pair.first);
        pair.second->setLabel(pair.first);
    }

    // Generate parameter/return colors
    for (std::string param : comp->getParameters()) {
        auto it = dataNodes.find(comp->trimDataSpaceName(param));
        if (it == dataNodes.end()) { continue; }
        it->second->setType(it->second->numInEdges() > 0 ? NodeTypes::param
                                                         : NodeTypes::readParam);
        for (EdgePtr ptr : it->second->getOutEdges()) {
            ptr->setColor(getDotColor(ptr->getColor(), PARAM_COLOR));
        }
    }
    for (std::string ret : comp->getReturnValues()) {
        auto it = dataNodes.find(comp->trimDataSpaceName(ret));
        if (it == dataNodes.end()) { continue; }
        if (it->second->getType() == NodeTypes::data) {
            it->second->setType(NodeTypes::returnVal);
        } else if (it->second->getType() == NodeTypes::readParam) {
            it->second->setType(NodeTypes::readReturnedParam);
        } else {
            it->second->setType(NodeTypes::returnedParam);
        }
        for (EdgePtr ptr : it->second->getInEdges()) {
            ptr->setColor(getDotColor(ptr->getColor(), RETURN_COLOR));
        }
    }

    std::vector<std::pair<int, Set*>> compIterSpaces = comp->getIterSpaces();
    std::vector<std::pair<NodePtr, Set*>> iterSpaces;
    // Generate statement labels and remove any unused iteration spaces
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
        // Perform projections for each column
        // TODO: Don't need to iterate
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
    ss << "digraph dataFlowGraph_1{\nrankdir=LR;\n"
       << createLegend(legend);
    subgraph.generateDotString(ss);
    generateDotReads(ss);
    ss << "}";
#ifdef DEBUG
    std::cerr << "Dot String Created" << std::endl;
#endif
    return ss.str();
}

std::string CompGraph::toDotString(int stmtIdx, bool reads, bool writes) {
    auto it = stmtNodes.find(stmtIdx);
    if (it == stmtNodes.end()) { return ""; }
    NodePtr stmt = it->second;

    resetWritten();

    auto myLegend = legend;
    myLegend.insert(NodeTypes::source);
    std::ostringstream ss;
    ss << "digraph dataFlowGraph_1{\nrankdir=LR;\n"
       << createLegend(myLegend);

    NodeTypes type = stmt->getType();
    stmt->setType(NodeTypes::source);
    stmt->generateDotString(ss);
    stmt->setType(type);

    bool arr[2] = {reads, writes};
    for (int i = 0; i < 2; i++) {
        if (!arr[i]) { continue; }
        std::queue<NodePtr> toVisit;
        for (EdgePtr ptr : (i == 0 ? stmt->getInEdges() : stmt->getOutEdges())) {
            std::string color = ptr->getColor();
            ptr->setColor(SOURCE_COLOR);
            ptr->generateDotString(ss);
            ptr->setColor(color);
            toVisit.push(ptr->getDataNode());
        }
        while (!toVisit.empty()) {
            NodePtr node = toVisit.front();
            toVisit.pop();
            if (node->isWritten()) { continue; }
            node->generateDotString(ss);
            for (EdgePtr ptr : (i == 0 ? node->getInEdges() : node->getOutEdges())) {
                ptr->generateDotString(ss);
                toVisit.push(ptr->isWrite() == (i == 0) ? ptr->getStmtNode() :
                                                          ptr->getDataNode());
            }
        }
    }
    ss << "}";

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
    legend.insert(NodeTypes::debug);
}

void CompGraph::reduceStmts() {
    for (auto &pair : stmtNodes) {
        bool allConst = true;
        for (EdgePtr ptr : pair.second->getInEdges()) {
            if (!ptr->isConst()) { allConst = false; break; }
        }
        if (!allConst) { continue; }
        for (EdgePtr ptr : pair.second->getOutEdges()) {
            if (!ptr->isConst()) { allConst = false; break; }
        }
        if (allConst) {
            pair.second->setType(NodeTypes::hidden);
        }
    }
    legend.insert(NodeTypes::hidden);
}

void CompGraph::reduceStmts(int toLevel) {
    subgraph.reduceStmts(toLevel);
}

void CompGraph::reduceDataSpaces() {
    for (auto &pair : dataNodes) {
        bool allConst = true;
        for (EdgePtr ptr : pair.second->getInEdges()) {
            if (!ptr->isConst()) { allConst = false; break; }
        }
        if (!allConst) { continue; }
        for (EdgePtr ptr : pair.second->getOutEdges()) {
            if (!ptr->isConst()) { allConst = false; break; }
        }
        if (allConst) {
            pair.second->setType(NodeTypes::hidden);
        }
    }
    legend.insert(NodeTypes::hidden);
}

void CompGraph::reduceDataSpaces(int toLevel) {
    subgraph.reduceDataSpaces(toLevel);
    legend.insert(NodeTypes::hidden);
}

void CompGraph::fusePCRelations() {
    auto it = stmtNodes.begin();
    while (it != stmtNodes.end()) {
        NodePtr writeNode = it->second;
        // Increment it in case we need to go to the next iteration
        ++it;
        // Make sure the statement writes to only one dataSpace
        if (writeNode->numOutEdges() != 1) { continue; }

        // This is the edge between writeNode and dataNode
        EdgePtr write = writeNode->getOutEdge(0);
        NodePtr dataNode = write->getDataNode();
        // Make sure the data space is written to and read from only one statement,
        // respectively
        if (dataNode->numInEdges() != 1 || dataNode->numOutEdges() != 1) { continue; }

        // This is the edge between dataNode and readNode
        EdgePtr read = dataNode->getOutEdge(0);

        // Make sure we are doing a scalar write (access space is constant)
        if (!write->isConst() || !read->isConst()) { continue; }

        // Get the read statement's node
        NodePtr readNode = read->getStmtNode();
        // Remove its connection to dataNode
        readNode->removeInEdge(read);
        // Connect it to and update all of writeNode's reads
        for (EdgePtr ptr : writeNode->getInEdges()) {
            ptr->setStmtNode(readNode);
            readNode->addInEdge(ptr);
        }

        // Remove dataNode
        for (auto it2 = dataNodes.begin(); it2 != dataNodes.end(); it2++) {
            if (it2->second == dataNode) { dataNodes.erase(it2); break; }
        }
        // Remove writeNode
        // Don't forget to decrement it
        stmtNodes.erase(--it);
        subgraph.removeStmt(writeNode);
        // Start over
        it = stmtNodes.begin();
    }
}


std::string CompGraph::getDotColor(std::string color1, std::string color2) {
    if (color1 == DEFAULT_COLOR) { return color2; }
    if (color2 == DEFAULT_COLOR) { return color1; }
    return PARAM_RETURN_COLOR;
}

}
