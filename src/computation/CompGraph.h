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

class Edge {
    friend class CompGraph;

  public:
    Edge() = default;
    Edge(bool _isWrite, int _stmtIdx, std::string _dataSpace, Relation* dataRel, std::string _color)
        : isWrite(_isWrite), stmtIdx(_stmtIdx), dataSpace(_dataSpace),
          color(_color) { setAccessSpace(dataRel); }
    ~Edge() = default;

  private:
    void setAccessSpace(Relation* dataRelation);

    std::string toDotString();

    // is the edge a write (stmt->dataSpace)
    // or a read (dataSpace->stmt)
    bool isWrite;

    int stmtIdx;
    std::string dataSpace, accessSpace;
    std::string color = DEFAULT_COLOR;
};

typedef std::shared_ptr<Edge> EdgePtr;

class Node {
    friend class CompGraph;

  public:
    Node() = default;
    ~Node() = default;

  private:
    void addInEdge(EdgePtr ptr) { inEdges.push_back(ptr); }
    void removeInEdge(EdgePtr ptr);
    EdgePtr getInEdge(int idx) { return idx < inEdges.size() ? inEdges[idx] : nullptr; }
    std::vector<EdgePtr> getInEdges() { return inEdges; }
    int numInEdges() { return inEdges.size(); }

    void addOutEdge(EdgePtr ptr) { outEdges.push_back(ptr); }
    void removeOutEdge(EdgePtr ptr);
    EdgePtr getOutEdge(int idx) { return idx < outEdges.size() ? outEdges[idx] : nullptr; }
    std::vector<EdgePtr> getOutEdges() { return outEdges; }
    int numOutEdges() { return outEdges.size(); }

    std::string color = DEFAULT_COLOR;
    std::vector<EdgePtr> inEdges, outEdges;
};

class Computation;

class CompGraph {
  public:
    CompGraph() = default;
    ~CompGraph() = default;

    void create(Computation* comp);

    void fusePCRelations();

    std::string toDotString(std::vector<std::pair<int, Set*>> &iterSpace);
    std::string toDotString(std::vector<std::pair<int, Set*>> &iterSpace, int stmtIdx);
 
  private:
    std::vector<std::vector<std::pair<int,Set*>>> split
	(int level, std::vector<std::pair<int,Set*>>& activeStmt);
    void generateDotStmts(std::vector<std::pair<int,Set*>>& activeStmt, int level, std::ostringstream &ss);
    void generateDotReadWrites(std::ostringstream &ss);

    std::map<int, Node> stmtNodes;
    std::map<std::string, Node> dataNodes;
};

}

#endif
