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
    friend class CompGraph;

  public:
    Edge() = default;
    Edge(bool _isWrite, NodePtr _stmtNode, NodePtr _dataNode)
        : isWrite(_isWrite), stmtNode(_stmtNode), dataNode(_dataNode) {}
    ~Edge() = default;

    void generateDotString(std::ostringstream &ss);

    NodePtr getStmtNode() { return stmtNode; }
    NodePtr getDataNode() { return dataNode; }

  private:
    void generateLabel(Relation* dataRelation);

    // is the edge a write (stmt->dataSpace)
    // or a read (dataSpace->stmt)
    bool isWrite;

    NodePtr stmtNode, dataNode;
    
    std::string label;
    std::string color = DEFAULT_COLOR;

    bool written = false;
};

class Node {
    friend class CompGraph;

  public:
    Node() = default;
    ~Node() = default;

    void generateDotString(std::ostringstream &ss);

    std::string getName() { return name; }

    std::vector<EdgePtr> getInEdges() { return inEdges; }
    EdgePtr getInEdge(int idx) { return idx < inEdges.size() ? inEdges[idx] : nullptr; }
    int numInEdges() { return inEdges.size(); }
  
    std::vector<EdgePtr> getOutEdges() { return outEdges; }
    EdgePtr getOutEdge(int idx) { return idx < outEdges.size() ? outEdges[idx] : nullptr; }
    int numOutEdges() { return outEdges.size(); }

  private:
    void addInEdge(EdgePtr ptr) { inEdges.push_back(ptr); }
    void removeInEdge(EdgePtr ptr);

    void addOutEdge(EdgePtr ptr) { outEdges.push_back(ptr); }
    void removeOutEdge(EdgePtr ptr);

    std::vector<EdgePtr> inEdges, outEdges;

    std::string name, label, shape;
    std::string color = DEFAULT_COLOR;

    bool written = false;
};

class Subgraph {
    friend class CompGraph;

  public:
    Subgraph() = default;;
    ~Subgraph() = default;

    void generateDotString(std::ostringstream &ss);

  private:
    void addStmt(NodePtr node) { stmts.insert(node); }
    void addSubgraph(Subgraph graph) { subgraphs.push_back(graph); }

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
