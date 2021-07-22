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
#define DEFAULT_COLOR "grey"
#define PARAM_COLOR "purple"
#define RETURN_COLOR "red"
#define PARAM_RETURN_COLOR "lime"

class Edge;
typedef std::shared_ptr<Edge> EdgePtr;
class Node;
typedef std::shared_ptr<Node> NodePtr;

class Edge {
  public:
    Edge() = default;
    Edge(bool _isWrite, NodePtr _stmtNode, NodePtr _dataNode)
        : isWrite(_isWrite), stmtNode(_stmtNode), dataNode(_dataNode) {}
    ~Edge() = default;

    void generateLabel(Relation* dataRelation);

    void generateDotString(std::ostringstream &ss);

    bool isConst();

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

    void reset() { written = false; }

  private:
    // is the edge a write (stmt->dataSpace)
    // or a read (dataSpace->stmt)
    bool isWrite;

    NodePtr stmtNode, dataNode;
    
    std::string label, accessStr;
    std::string color = DEFAULT_COLOR;

    bool written = false;
};

class Node {
  public:
    Node() = default;
    ~Node() = default;

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
    // name
    void setName(std::string str) { name = str; }
    std::string getName() { return name; }
    // label
    void setLabel(std::string str) { label = str; }
    void addLabel(std::string str) { label.append(str); }
    std::string getLabel() { return label; }
    // shape
    void setShape(std::string str) { shape = str; }
    std::string getShape() { return shape; }
    // color
    void setColor(std::string str) { color = str; }
    std::string getColor() { return color; }

    void reset() { written = false; }

  private:
    std::vector<EdgePtr> inEdges, outEdges;

    std::string name, label, shape;
    std::string color = DEFAULT_COLOR;

    bool written = false;
};

class Subgraph {
  public:
    Subgraph() = default;
    Subgraph(int _level) : level(_level) {}
    ~Subgraph() = default;

    void generateDotString(std::ostringstream &ss);

    void addStmt(NodePtr node) { stmts.insert(node); }
    std::set<NodePtr> getStmts() { return stmts; }
    void addSubgraph(Subgraph graph) { subgraphs.push_back(graph); }
    
    void reduceStmts(int toLevel);
    void reduceDataSpaces(int toLevel);

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

    void create(Computation* comp);
    void addDebugStmts(std::vector<std::pair<int, std::string>> debugStmts);
    void reduceStmts();
    void reduceStmts(int toLevel);
    void reduceDataSpaces();
    void reduceDataSpaces(int toLevel);
    void fusePCRelations();

    std::string toDotString();

  private:
    std::vector<std::vector<std::pair<int,Set*>>> split
	(int level, std::vector<std::pair<int,Set*>>& activeStmt);
    Subgraph generateSubgraph(std::vector<std::pair<NodePtr ,Set*>>& activeStmt, int level);

    void resetWritten();

    void generateDotWrites(Subgraph &subgraph, std::ostringstream &ss);
    void generateDotReads(std::ostringstream &ss);

    static std::string getDotColor(std::string color1, std::string color2);

    std::map<int, NodePtr> stmtNodes;
    std::map<std::string, NodePtr> dataNodes;
    Subgraph subgraph;
};

}

#endif
