#ifndef COMP_GRAPH_H_
#define COMP_GRAPH_H_

#include <climits>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "set_relation/set_relation.h"

namespace iegenlib {
	//! Colors used in dot nodes
	constexpr auto DEFAULT_COLOR = "darkgray";
	constexpr auto PARAM_COLOR = "deepskyblue";
	constexpr auto RETURN_COLOR = "red";
	constexpr auto PARAM_RETURN_COLOR = "orchid";
	// Color of the source node when creating a graph from a single statement
	constexpr auto SOURCE_COLOR = "goldenrod";
	// Color of unread data space
	constexpr auto UNREAD_COLOR = "orangered";

	// These could be turned into bytewise operations for optimization
	enum NodeTypes {
		debug = 0, stmt, data, unread, readParam, param,
		activeOut, readActiveOutParam, activeOutParam,
		srcStmt, srcData, hidden
	};

	// Generates a specific node in the legend
	std::string generateLegendNode(NodeTypes type);
	// Creates a legend from the inputted NodeTypes
	std::string createLegend(const std::set<NodeTypes>& nodes);

    //Generate an edge label from the given relation
    std::string getRelationLabel(Relation* rel);

	struct SubgraphNode {
		int val = -1;
		std::string label = "";
		std::vector<SubgraphNode> children;
	};

	struct StmtNode;
	struct DataNode;

	struct StmtNode {
		int id = -1;
		int subgraph = 0;
		std::string setStr = "", debugStr = "";
		std::list<int> eatenStmts;
		NodeTypes type = NodeTypes::stmt;

		std::set<std::weak_ptr<DataNode>,
			std::owner_less<std::weak_ptr<DataNode>>> in;
		std::map<std::weak_ptr<DataNode>, std::string,
			std::owner_less<std::weak_ptr<DataNode>>> out;

		uint8_t written = 0;

		void removeInEdge(const std::weak_ptr<DataNode>& dataNode);
		void removeOutEdge(const std::weak_ptr<DataNode>& dataNode);
		void toDotString(std::ostringstream& os) const;
		void outEdgeDotString(std::ostringstream& os) const;
		void inEdgeDotString(std::ostringstream& os) const;
		void edgeDotString(std::ostringstream& os,
			const DataNode* toNode) const;
	};

	struct DataNode {
		std::string id = "";
		int subgraph = 0;
		NodeTypes type = NodeTypes::data;

		std::set<std::weak_ptr<StmtNode>,
			std::owner_less<std::weak_ptr<StmtNode>>> in;
		std::map<std::weak_ptr<StmtNode>, std::string,
			std::owner_less<std::weak_ptr<StmtNode>>> out;

		uint8_t written = 0;

		void removeInEdge(const std::weak_ptr<StmtNode>& stmtNode);
		void removeOutEdge(const std::weak_ptr<StmtNode>& stmtNode);
		void toDotString(std::ostringstream& os) const;
		void outEdgeDotString(std::ostringstream& os) const;
		void inEdgeDotString(std::ostringstream& os) const;
		void edgeDotString(std::ostringstream& os,
			const StmtNode* toNode) const;
	};

    class Computation;

	class CompGraph {
	public:
		CompGraph() = default;
		~CompGraph() = default;

		// Constructs the graph from the given Computation object
		void create(Computation* comp);

		// Adds debug statements from the inputted (stmtId : debugString) pairs
		void addDebugStmts(std::vector<std::pair<int, std::string>> debugStmts);

		// Reduces all nodes up through subgraph level = toLevel
		// stmts and dataSpaces specify whether to reduce those nodes
		void reduceNodes(bool stmts, bool dataSpaces, int toLevel = INT_MAX);
		// Removes all nodes up through subgraph level = toLevel
		void removeNodes(int toLevel = INT_MAX);

		// Reduces all producer-consumer node relationships
		void reducePCRelations();

		// Removes nodes marked unread (no reads and not active-out)
		void removeDeadNodes(std::vector<int>& stmtIds,
			std::vector<std::string>& dataSpaces);

		// Converts the graph to the dot file format
		std::string toDotString();
		// Converts the graph to the dot file format
		// Only includes nodes which are connected to stmtIdx through
		//     only reads (reads = true) or only writes (writes = true)
		std::string toDotString(int stmtIdx, bool reads, bool writes);
		std::string toDotString(std::string dataIdx, bool reads, bool writes);

	private:
		// Resets the "written" boolean in all nodes
		void resetWritten();

        std::string partialDotString(std::shared_ptr<StmtNode> stmtNode,
            std::shared_ptr<DataNode> dataNode, bool reads, bool writes);

		// Removes a statement node. Also removes all write data nodes
		void removeNode(int stmtId);

		// Generates subgraphs
		void generateSubgraphs(std::vector<std::pair<
			std::shared_ptr<StmtNode>, Set*>>&activeStmts);
		SubgraphNode generateSubgraph(std::vector<std::pair<
			std::shared_ptr<StmtNode>, Set*>>& activeStmts,
			int level, int& sgCnt);

		// Writes subgraphs to os
		// Gets subgraphs in dot fromat from sgos
		void subgraphDotString(std::ostringstream& os,
			std::vector<std::ostringstream>& sgos);
		// Checks for empty subgraphs (children must be empty as well)
		// Uses subgraph string empty status
		void checkEmptySubgraphs(std::vector<bool>& result,
			std::vector<std::ostringstream>& sgos,
			const SubgraphNode& sgNode);
		// Gets order of subgraph traversal
		// Inserts -1 after leaving a subgraph
		void getSubgraphOrder(std::vector<SubgraphNode*>& order,
			SubgraphNode& sgNode);

		// Head node of the subgraph tree
		SubgraphNode subgraphs;
		int subgraphCnt;

		// All statement and data space nodes
		std::map<int, std::shared_ptr<StmtNode>> stmtNodes;
		std::map<std::string, std::shared_ptr<DataNode>> dataNodes;

		// Nodes to generate in the legend
		std::set<NodeTypes> legend = {
			NodeTypes::stmt, NodeTypes::data, NodeTypes::readParam, NodeTypes::param,
			NodeTypes::activeOut, NodeTypes::unread,
			NodeTypes::readActiveOutParam, NodeTypes::activeOutParam
		};
	};
}

#endif




