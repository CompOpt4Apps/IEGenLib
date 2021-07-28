#ifndef COMP_GRAPH_H_
#define COMP_GRAPH_H_

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>

#include "set_relation/set_relation.h"

namespace iegenlib {

//! Colors used in dot nodes
#define DEFAULT_COLOR "darkgray"
#define PARAM_COLOR "deepskyblue"
#define RETURN_COLOR "red"
#define PARAM_RETURN_COLOR "darkorchid"
// Color of the source node when creating a graph from a single statement
#define SOURCE_COLOR "goldenrod"

enum NodeTypes {
    debug = 0, stmt, data, readParam, param,
    returnVal, readReturnedParam, returnedParam,
    source, hidden
};

std::string generateNode(std::string name, std::string label, NodeTypes type);
std::string generateLegendNode(NodeTypes type);
std::string createLegend(const std::vector<NodeTypes> &nodes);

class Edge;
typedef std::shared_ptr<Edge> EdgePtr;
class Node;
typedef std::shared_ptr<Node> NodePtr;

class Edge {
  public:
    Edge() = default;
    Edge(bool _write, NodePtr _stmtNode, NodePtr _dataNode)
        : write(_write), stmtNode(_stmtNode), dataNode(_dataNode) {}
    ~Edge() = default;

//    Edge copy();

    void generateLabel(Relation* dataRelation);

    void generateDotString(std::ostringstream &ss);

    bool isConst();
    bool isWrite() { return write; }

    // Getter/Setters
    // stmtNode
    void setStmtNode(NodePtr ptr) { stmtNode = ptr; }
    NodePtr getStmtNode() { return stmtNode; }
    // dataNode
    void setDataNode(NodePtr ptr) { dataNode = ptr; }
    NodePtr getDataNode() { return dataNode; }
    // label
    void setLabel(std::string str) { label = str; }
    void addLabel(std::string str) { label.append(str); }
    std::string getLabel() { return label; }
    // color
    void setColor(std::string str) { color = str; }
    std::string getColor() { return color; }

    bool isWritten() { return written; }
    void reset() { written = false; }

  private:
    // is the edge a write (stmt->dataSpace)
    // or a read (dataSpace->stmt)
    bool write;

    NodePtr stmtNode, dataNode;
    
    std::string label, accessStr;
    std::string color = DEFAULT_COLOR;

    bool written = false;
};

class Node {
  public:
    Node() = default;
    ~Node() = default;

//    Node copy();

    void generateDotString(std::ostringstream &ss);

    std::vector<EdgePtr> getInEdges() { return inEdges; }
    EdgePtr getInEdge(int idx) { return idx < inEdges.size() ? inEdges[idx] : nullptr; }
    int numInEdges() { return inEdges.size(); }
    void addInEdge(EdgePtr ptr) { inEdges.push_back(ptr); }
    void removeInEdge(EdgePtr ptr);
 
    std::vector<EdgePtr> getOutEdges() { return outEdges; }
    EdgePtr getOutEdge(int idx) { return idx < outEdges.size() ? outEdges[idx] : nullptr; }
    int numOutEdges() { return outEdges.size(); }
    void addOutEdge(EdgePtr ptr) { outEdges.push_back(ptr); }
    void removeOutEdge(EdgePtr ptr);

    // Getter/Setters
    void setType(NodeTypes newType) { type = newType; }
    NodeTypes getType() { return type; }
    // name
    void setName(std::string str) { name = str; }
    std::string getName() { return name; }
    // label
    void setLabel(std::string str) { label = str; }
    void addLabel(std::string str) { label.append(str); }
    std::string getLabel() { return label; }
/*    // shape
    void setShape(std::string str) { shape = str; }
    std::string getShape() { return shape; }
    // color
    void setColor(std::string str) { color = str; }
    std::string getColor() { return color; }
    // filled
    void setFilled(bool b) { filled = b; }
    bool isFilled() { return filled; }*/

    bool isWritten() { return written; }
    void reset() { written = false; }

  private:
    std::vector<EdgePtr> inEdges, outEdges;

    std::string name, label;
    NodeTypes type;

    bool written = false;
};

class Subgraph {
  public:
    Subgraph() = default;
    Subgraph(int _level) : level(_level) {}
    ~Subgraph() = default;

    void generateDotString(std::ostringstream &ss);
    void generateDotString(std::ostringstream &ss, int &cnt);

    void addStmt(NodePtr node) { stmts.insert(node); }
    std::set<NodePtr> getStmts() { return stmts; }
    void addSubgraph(Subgraph graph) { subgraphs.push_back(graph); }
    
    void reduceStmts(int toLevel);
    void reduceDataSpaces(int toLevel);

    bool removeStmt(NodePtr stmt);

    // Getter/Setters
    // level
    int getLevel() { return level; }
    // label
    void setLabel(std::string str) { label = str; }
    void addLabel(std::string str) { label.append(str); }
    std::string getLabel() { return label; }

  private:
    int level;
    std::string label;
    std::set<NodePtr> stmts;
    std::vector<Subgraph> subgraphs;
};

class Computation;

class CompGraph {
  public:
    CompGraph() = default;
    ~CompGraph() = default;

//    CompGraph copy();

    void create(Computation* comp);
    void addDebugStmts(std::vector<std::pair<int, std::string>> debugStmts);

    void reduceStmts();
    void reduceStmts(int toLevel);
    void reduceDataSpaces();
    void reduceDataSpaces(int toLevel);
    void reduceNormalNodes();

    void fusePCRelations();

    std::string toDotString();
    std::string toDotString(int stmtIdx, bool reads, bool writes);

  private:
//    Subgraph copySubgraph(CompGrpah &toGraph, subgraph &fromSG);

    Subgraph generateSubgraph(std::vector<std::pair<NodePtr ,Set*>>& activeStmt, int level);

    void resetWritten();

    void generateDotWrites(Subgraph &subgraph, std::ostringstream &ss);
    void generateDotReads(std::ostringstream &ss);

    std::map<int, NodePtr> stmtNodes;
    std::map<std::string, NodePtr> dataNodes;
    Subgraph subgraph;

    // Nodes to generate in the legend
    std::set<NodeTypes> legend = {
        NodeTypes::stmt, NodeTypes::data, NodeTypes::readParam, NodeTypes::param,
        NodeTypes::returnVal, NodeTypes::readReturnedParam, NodeTypes::returnedParam
    };

    static std::string getDotColor(std::string color1, std::string color2);
};

}

#endif
