#include "CompGraph.h"
#include "Computation.h"
#include "util.h"

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

	// Corresponds to NodeTypes
	const std::pair<const char*, NodeType> nodeTypes[] = {
		std::make_pair("Debug Statements are Enabled", NodeType {"box", "", "white"}),
		std::make_pair("Statement", NodeType {"Mrecord", "bold", DEFAULT_COLOR}),
		std::make_pair("Data Space", NodeType {"box", "bold", DEFAULT_COLOR}),
		std::make_pair("Unread Data Space", NodeType {"box", "filled", UNREAD_COLOR}),
		std::make_pair("Read-only Parameter", NodeType {"box", "bold", PARAM_COLOR}),
		std::make_pair("Parameter", NodeType {"box", "filled", PARAM_COLOR}),
		std::make_pair("Returned Data Space", NodeType {"box", "bold", RETURN_COLOR}),
		std::make_pair("Read-only Returned Paramter", NodeType {"box", "bold", PARAM_RETURN_COLOR}),
		std::make_pair("Returned Parameter", NodeType {"box", "filled", PARAM_RETURN_COLOR}),
		std::make_pair("Source Statement", NodeType {"Mrecord", "bold", SOURCE_COLOR}),
		std::make_pair("Source Data Space", NodeType {"box", "bold", SOURCE_COLOR}),
		std::make_pair("Hidden Statement/Data Space", NodeType {"point", "", ""})
	};

	std::string generateLegendNode(NodeTypes type) {
		std::string label = nodeTypes[type].first;
		const NodeType& nodeType = nodeTypes[type].second;
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

	std::string createLegend(const std::set<NodeTypes>& nodes) {
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

	std::string getRelationLabel(Relation* rel) {
		std::string relStr = rel->getString();
		int pos1 = relStr.rfind("[");
		int pos2 = relStr.rfind("]");

		if (pos1 == std::string::npos || pos2 == std::string::npos) {
			return "";
		}
		return relStr.substr(pos1, pos2 - pos1 + 1);
	}

	// StmtNode
	void StmtNode::removeInEdge(const std::weak_ptr<DataNode>& dataNode) {
		auto it = in.find(dataNode);
		if (it != in.end()) { in.erase(it); }
	}

	void StmtNode::removeOutEdge(const std::weak_ptr<DataNode>& dataNode) {
		auto it = out.find(dataNode);
		if (it != out.end()) { out.erase(it); }
	}

	void StmtNode::toDotString(std::ostringstream& os) const {
		std::ostringstream label;
		label << setStr << "\\nS" << id; 
        if (!eatenStmts.empty()) {
            label << "\\nEaten: ";
		    for (auto it = eatenStmts.begin(); it != eatenStmts.end(); it++) {
			    if (it != eatenStmts.begin()) { label << ", "; }
			    label << "S" << *it;
		    }
        }
		label << "\\n" << debugStr;
		// Node
		os << "S" << id
			<< "[" << generateDotLabel(label.str()) << "]"
			<< nodeTypes[type].second.generate() << ";\n";
	}

	void StmtNode::outEdgeDotString(std::ostringstream& os) const {
		// Out edges
		for (auto& pair : out) {
			edgeDotString(os, pair.first.lock().get());
		}

	}

	void StmtNode::inEdgeDotString(std::ostringstream& os) const {
		// In edges
		for (auto& weakPtr : in) {
			auto dataNode = weakPtr.lock();
			if (dataNode) {
				dataNode->edgeDotString(os, this);
			}
		}
	}

	void StmtNode::edgeDotString(std::ostringstream& os,
		const DataNode* toNode) const {
		auto it = std::find_if(out.begin(), out.end(),
			[&toNode](const std::pair<std::weak_ptr<DataNode>, std::string>& p) {
				return p.first.lock().get() == toNode;
			});
		if (it != out.end()) {
			auto dataNode = it->first.lock();
			os << "S" << id << "->" << dataNode->id
				<< "[" << generateDotLabel(it->second)
				<< "][style=bold][color=";
			// Special color for writing to active out data space
			switch (dataNode->type) {
				case NodeTypes::activeOut:
				case NodeTypes::activeOutParam:
				case NodeTypes::readActiveOutParam:
					os << RETURN_COLOR; break;
				default:
					os << DEFAULT_COLOR; break;
			};
			os << "]\n";
		}
	}

	// DataNode
	void DataNode::removeInEdge(const std::weak_ptr<StmtNode>& stmtNode) {
		auto it = in.find(stmtNode);
		if (it != in.end()) { in.erase(it); }
	}

	void DataNode::removeOutEdge(const std::weak_ptr<StmtNode>& stmtNode) {
		auto it = out.find(stmtNode);
		if (it != out.end()) { out.erase(it); }
	}

	void DataNode::toDotString(std::ostringstream& os) const {
		// Node
		os << id << "[" << generateDotLabel(id) << "]"
			<< nodeTypes[type].second.generate() << ";\n";
	}

	void DataNode::outEdgeDotString(std::ostringstream& os) const {
		// Out edges
		for (auto& pair : out) {
			edgeDotString(os, pair.first.lock().get());
		}
	}

	void DataNode::inEdgeDotString(std::ostringstream& os) const {
		// In edges
		for (auto& weakPtr : in) {
			auto stmtNode = weakPtr.lock();
			if (stmtNode) {
				stmtNode->edgeDotString(os, this);
			}
		}
	}

	void DataNode::edgeDotString(std::ostringstream& os,
		const StmtNode* toNode) const {
		auto it = std::find_if(out.begin(), out.end(),
			[&toNode](const std::pair<std::weak_ptr<StmtNode>, std::string>& p) {
				return p.first.lock().get() == toNode;
			});
		if (it != out.end()) {
			auto stmtNode = it->first.lock();
			os << id << "->S" << stmtNode->id
				<< "[" << generateDotLabel(it->second)
				<< "][style=bold][color=";
			// Special color for reading from parameter
			switch (type) {
				case NodeTypes::param:
				case NodeTypes::readParam:
				case NodeTypes::activeOutParam:
				case NodeTypes::readActiveOutParam:
					os << PARAM_COLOR; break;
				default:
					os << DEFAULT_COLOR; break;
			};
			os << "]\n";
		}
	}

	// CompGraph

	// Functions for creating/manipulating the graph
	void CompGraph::create(Computation* comp) {
#ifdef DEBUG 
		std::cerr << "Creating Graph" << std::endl;
#endif
		stmtNodes.clear();
		dataNodes.clear();
		// Populate nodes
		for (int i = 0; i < comp->getNumStmts(); i++) {
			Stmt* stmt = comp->getStmt(i);
			auto stmtNode = std::make_shared<StmtNode>();
			stmtNode->id = i;
			for (int j = 0; j < stmt->getNumReads(); j++) {
				std::string read =
					comp->trimDataSpaceName(stmt->getReadDataSpace(j));
				auto& dataNode = dataNodes[read];
				if (!dataNode) {
					dataNode = std::make_shared<DataNode>();
					dataNode->id = read;
				}
				dataNode->out[stmtNode] =
					getRelationLabel(stmt->getReadRelation(j));
				stmtNode->in.insert(dataNode);
			}
			for (int j = 0; j < stmt->getNumWrites(); j++) {
				std::string write =
					comp->trimDataSpaceName(stmt->getWriteDataSpace(j));
				auto& dataNode = dataNodes[write];
				if (!dataNode) {
					dataNode = std::make_shared<DataNode>();
					dataNode->id = write;
				}
				stmtNode->out[dataNode] =
					getRelationLabel(stmt->getWriteRelation(j));
				dataNode->in.insert(stmtNode);
			}
			stmtNodes[i] = stmtNode;
		}

		// Mark parameters
		for (std::string param : comp->getParameters()) {
			auto it = dataNodes.find(comp->trimDataSpaceName(param));
			if (it != dataNodes.end()) {
				it->second->type = it->second->in.size() > 0 ? NodeTypes::param
					: NodeTypes::readParam;
			}
		}
		// Mark returns
		for (std::string ret : comp->getActiveOutValues()) {
			auto it = dataNodes.find(comp->trimDataSpaceName(ret));
			if (it != dataNodes.end()) {
				switch (it->second->type) {
					case NodeTypes::param:
						it->second->type = NodeTypes::activeOutParam;
						break;
					case NodeTypes::readParam:
						it->second->type = NodeTypes::readActiveOutParam;
						break;
					default:
						it->second->type = NodeTypes::activeOut;
						break;
				}
			}
		}
		// Mark unread data spaces except for returns
		for (auto pair : dataNodes) {
			if (pair.second->out.size() == 0) {
				switch (pair.second->type) {
					case NodeTypes::activeOut:
					case NodeTypes::activeOutParam:
					case NodeTypes::readActiveOutParam:
						break;
					default:
						pair.second->type = NodeTypes::unread;
						break;
				}
			}
		}

		std::vector<std::pair<int, Set*>> compIterSpaces = comp->getIterSpaces();
		std::vector<std::pair<std::shared_ptr<StmtNode>, Set*>> iterSpaces;
		// Generate statement labels and remove any unused iteration spaces
		for (auto& pair : compIterSpaces) {
			auto it = stmtNodes.find(pair.first);
			if (it != stmtNodes.end()) {
				it->second->setStr = pair.second->prettyPrintString();
				// + "\\nS" + std::to_string(it->first);
				iterSpaces.push_back(std::make_pair(it->second, pair.second));
			}
		}
		// Generate subgraphs
		generateSubgraphs(iterSpaces);
		for (auto& pair : compIterSpaces) {
			delete pair.second;
		}

#ifdef DEBUG
		std::cerr << "Graph Created" << std::endl;
#endif
	}

	void CompGraph::generateSubgraphs(
		std::vector<std::pair<std::shared_ptr<StmtNode>, Set*>>& activeStmts) {
		subgraphCnt = 0;
		subgraphs = generateSubgraph(activeStmts, 0, subgraphCnt);
	}

	SubgraphNode CompGraph::generateSubgraph(
		std::vector<std::pair<std::shared_ptr<StmtNode>, Set*>>& activeStmts,
		int level, int& sgCnt) {
#ifdef DEBUG
		std::cerr << "Creating Subgraphs" << std::endl;
#endif
		SubgraphNode sgNode;

		if (activeStmts.size() == 0) { sgNode.val = -1; return sgNode; } 		else { sgNode.val = sgCnt++; }

		Set* set = activeStmts[0].second;
		if (level > 0) {
			// Replace all '$' because dot throws a fit
			Set* projectedIS = new Set(
				replaceInString(set->getString(), "$", ""));
			// Perform projections for each column
			// TODO: Don't need to iterate
			for (int j = set->arity() - 1; j >= level; j--) {
				Set* tmp = projectedIS;
				projectedIS = projectedIS->projectOut(j);
				delete tmp;
			}

			sgNode.label = std::string("Domain: ")
				+ projectedIS->prettyPrintString();

			delete projectedIS;
		}

		// This is the final subgraph
		if (level >= set->arity() - 1) {
			for (auto& pair : activeStmts) {
				pair.first->subgraph = sgNode.val;
				for (auto& pair : pair.first->out) {
					pair.first.lock()->subgraph = sgNode.val;
				}
			}
		// Differentiate between statements in this subgraph
		// and those in further nested subgraphs
		} else {
			std::map<int,
				std::vector<std::pair<std::shared_ptr<StmtNode>, Set*>>> stmtGroups;
			for (auto& pair : activeStmts) {
				TupleDecl tupleDecl = pair.second->getTupleDecl();
				// No more iterators
				if (tupleDecl.elemIsConst(level + 1)) {
					pair.first->subgraph = sgNode.val;
					for (auto& pair : pair.first->out) {
						pair.first.lock()->subgraph = sgNode.val;
					}
				// More iterators
				} else {
					stmtGroups[tupleDecl.elemConstVal(level)].push_back(pair);
				}
			}
			for (auto& pair : stmtGroups) {
				SubgraphNode child = generateSubgraph(
					pair.second, level + 2, sgCnt);
				if (child.val != -1) { sgNode.children.push_back(child); }
			}
		}
#ifdef DEBUG
		std::cerr << "Subgraphs Created" << std::endl;
#endif
		return sgNode;
	}

	void CompGraph::addDebugStmts(
		std::vector<std::pair<int, std::string>> debugStmts) {
		// Add debug statements
		for (auto& pair : debugStmts) {
			auto it = stmtNodes.find(pair.first);
			if (it != stmtNodes.end()) {
				it->second->debugStr = pair.second;
			}
		}
		legend.insert(NodeTypes::debug);
	}

	void CompGraph::reduceNodes(bool stmts, bool dataSpaces, int toLevel) {
		if (!stmts && !dataSpaces) { return; }

		// Get subgraphs to be changed
		std::set<int> validSGs;
		std::queue<SubgraphNode*> sgQueue;
		sgQueue.push(&subgraphs); sgQueue.push(NULL);
		int level = 0;
		while (level <= toLevel && !sgQueue.empty()) {
			SubgraphNode* sgNode = sgQueue.front();
			sgQueue.pop();
			if (!sgNode) { ++level; sgQueue.push(NULL); } 			else {
				validSGs.insert(sgNode->val);
				for (auto& child : sgNode->children) {
					sgQueue.push(&child);
				}
			}
		}

		// Check to write statements
		if (stmts) {
			for (auto& pair : stmtNodes) {
				if (validSGs.find(pair.second->subgraph) != validSGs.end()
                    && pair.second->type == NodeTypes::stmt) {
					pair.second->type = NodeTypes::hidden;
				}
			}
		}

		// Check to write data spaces
		if (dataSpaces) {
			for (auto& pair : dataNodes) {
				if (validSGs.find(pair.second->subgraph) != validSGs.end()
                    && pair.second->type == NodeTypes::data) {
					pair.second->type = NodeTypes::hidden;
				}
			}
		}

		legend.insert(NodeTypes::hidden);
	}

	void CompGraph::removeNodes(int toLevel) {
		// Get subgraphs to be changed
		std::set<int> validSGs;
		std::queue<SubgraphNode*> sgQueue;
		sgQueue.push(&subgraphs); sgQueue.push(NULL);
		int level = 0;
		while (level <= toLevel && !sgQueue.empty()) {
			SubgraphNode* sgNode = sgQueue.front();
			sgQueue.pop();
			if (!sgNode) { ++level; sgQueue.push(NULL); } 			else {
				validSGs.insert(sgNode->val);
				for (auto& child : sgNode->children) {
					sgQueue.push(&child);
				}
			}
		}

		// Remove statements
		auto sIt = stmtNodes.begin();
		while (sIt != stmtNodes.end()) {
			auto stmtNode = (sIt++)->second;
			if (validSGs.find(stmtNode->subgraph) != validSGs.end()) {
				removeNode(stmtNode->id);
			}
		}

		// Remove any remaining data spaces
		auto dIt = dataNodes.begin();
		while (dIt != dataNodes.end()) {
			if (validSGs.find(dIt->second->subgraph) != validSGs.end()) {
				dIt = dataNodes.erase(dIt);
			} else { ++dIt; }
		}
	}

	void CompGraph::reducePCRelations() {
		/* What we are doing:
		*  Let S1, S2 be statements and D be a data space.
		*  A Producer-Consumer (PC) relationship occurs iff:
		*  1) S1 only writes to D
		*  2) D is only written to by S1
		*  3) D is only read from by S2
		*
		*  For our purposes, we only remove a PC relationship if
		*  1) S1 and D are not in a subgraph
		*
		*  In this case, dataflow is a straight line and S1, D are not needed.
		*  Thus we call CompGraph::removeNode(S1); See removeNode() for details.
		*  In short, it removes S1, D and connects S1's reads to S2.
		*/

		auto sIt = stmtNodes.begin();
		while (sIt != stmtNodes.end()) {
			// Get s1
			auto s1 = sIt->second;

			// Increment sIt in case we need to go to the next iteration early
			++sIt;

			// Make sure s1 writes only to d
			if (s1->out.size() != 1) { continue; }

			// Get d
			auto d = s1->out.begin()->first.lock();

			// Make sure d is only writte to by s1 and read from by s2
			if (d->in.size() != 1 || d->out.size() != 1) { continue; }

			// Make sure s1, d are not in subgraphs
			if (s1->subgraph != 0 || d->subgraph != 0) { continue; }

			// Get s2
			auto s2 = d->out.begin()->first.lock();

			s2->eatenStmts.push_back(s1->id);
			s2->eatenStmts.insert(s2->eatenStmts.end(), s1->eatenStmts.begin(), s1->eatenStmts.end());

			// Remove s1
			removeNode(s1->id);

			// Start over (previously checked nodes may now be in PC relations)
			sIt = stmtNodes.begin();
		}
	}

	void CompGraph::removeNode(int stmtId) {
		auto sIt = stmtNodes.find(stmtId);
		if (sIt == stmtNodes.end()) { return; }
		auto stmtNode = sIt->second;

		/* What we are doing:
		*  Let "stmt" be stmtNode defined above
		*  Let "reads" be all the data spaces read by stmt
		*  Let "writes" be all the data spaces written to by stmt
		*  Let "readStmts" be all the statements which read from writes
		*
		*  General data flow is: reads -> stmt -> writes -> readStmts
		*
		*  1) We are removing stmt and writes from the graph
		*  2) We are connecting every element of reads
		*     to every element of readStmts
		*
		*  New data flow is: reads -> readStmts
		*/

		bool badNode = false;

		// Statements that read from stmtNode's writes
		std::vector<std::shared_ptr<StmtNode>> readStmts;

		// Go through writes
		auto sOutIt = stmtNode->out.begin();
		while (sOutIt != stmtNode->out.end()) {
			auto dataNode = sOutIt->first.lock();
			// dataSpace has another write statement
			// and is too complicated to delete for now
			if (dataNode->in.size() > 1) {
				// TODO: can we assume a data spaces only has one write statement?
				std::cerr << "Data node has multiple write statements, cannot delete" << std::endl;
				badNode = true;
				++sOutIt;
			} else {
				// Go through readStmts
				while (!dataNode->out.empty()) {
					auto readStmt = dataNode->out.begin()->first.lock();
					// Disconnect swrites and readStmts
					dataNode->out.erase(dataNode->out.begin());
					readStmt->removeInEdge(dataNode);
					// Save readStmt
					readStmts.push_back(readStmt);
				}
				// Disconnect stmt and writes
				sOutIt = stmtNode->out.erase(sOutIt);
				dataNode->removeInEdge(stmtNode);
				// Remove writes
				auto dIt = dataNodes.find(dataNode->id);
				if (dIt != dataNodes.end()) { dataNodes.erase(dIt); }
			}
		}

		// Go through reads
		auto sInIt = stmtNode->in.begin();
		while (sInIt != stmtNode->in.end()) {
			auto readDataNode = sInIt->lock();
			// Connect reads to readStmts
			std::string relStr = readDataNode->out[stmtNode];
			for (auto& readStmt : readStmts) {
				readDataNode->out[readStmt] = relStr;
				readStmt->in.insert(readDataNode);
			}
			// Disconnect stmt from reads if there was no bad node
			if (badNode) { ++sInIt; } 			else {
				sInIt = stmtNode->in.erase(sInIt);
				readDataNode->removeOutEdge(stmtNode);
			}
		}

		// If node deletion was successful, remove the statement
		if (!badNode) { stmtNodes.erase(sIt); }
	}

	void CompGraph::removeDeadNodes(std::vector<int>& stmtIds,
		std::vector<std::string>& dataSpaces) {
		stmtIds.clear();
		dataSpaces.clear();
		for (auto& pair : dataNodes) {
			if (pair.second->type == NodeTypes::unread) {
				std::queue<std::shared_ptr<DataNode>> dataQueue;
				dataQueue.push(pair.second);
				while (!dataQueue.empty()) {
					auto dataNode = dataQueue.front();
					dataQueue.pop();

					// dataNode must be a leaf node
					if (dataNode->out.size() != 0) { continue; }

					// Save data node
					dataSpaces.push_back(dataNode->id);

					auto dInIt = dataNode->in.begin();
					while (dInIt != dataNode->in.end()) {
						auto stmtNode = dInIt->lock();

						// Disconnect stmtNode and dataNode
						dInIt = dataNode->in.erase(dInIt);
						stmtNode->removeOutEdge(dataNode);

						// Making sure that we dont remove 
						// statements that have multiple writes.
						if (stmtNode->out.size() > 1) { continue; }

						// Store the statement id
						stmtIds.push_back(stmtNode->id);

						// Save and remove all the data nodes that
						// statement node reads from
						auto sInIt = stmtNode->in.begin();
						while (sInIt != stmtNode->in.end()) {
							auto readDataNode = sInIt->lock();
							// Disconnect readDataSpace and stmtNode
							sInIt = stmtNode->in.erase(sInIt);
							readDataNode->removeOutEdge(stmtNode);
							// Add readDataNode to the queue
							dataQueue.push(readDataNode);
						}

						// Remove stmtNode
						auto sIt = stmtNodes.find(stmtNode->id);
						if (sIt != stmtNodes.end()) { stmtNodes.erase(sIt); }
					}

					// Remove dataNode
					auto dIt = dataNodes.find(dataNode->id);
					if (dIt != dataNodes.end()) { dataNodes.erase(dIt); }
				}
			}
		}
#ifdef DEBUG
		std::cerr << "Total Data Nodes to be Removed: " << dataSpaces.size()
			<< "\n Total Stmt Nodes to be removed: " << stmtIds.size()
			<< "\n";
#endif
	}

	// Functions for writing to the dot format
	void CompGraph::resetWritten() {
		for (auto& pair : stmtNodes) {
			pair.second->written = 0;
		}
		for (auto& pair : dataNodes) {
			pair.second->written = 0;
		}
	}

	std::string CompGraph::toDotString() {
#ifdef DEBUG
		std::cerr << "Creating Dot String" << std::endl;
#endif

		resetWritten();
		std::ostringstream edges;
		std::vector<std::ostringstream> sgos;
		sgos.resize(subgraphCnt);
		for (auto& pair : stmtNodes) {
			auto stmtNode = pair.second;
			if (stmtNode->written == 0 &&
				stmtNode->subgraph >= 0 && stmtNode->subgraph < subgraphCnt) {
				stmtNode->toDotString(sgos[stmtNode->subgraph]);
				stmtNode->outEdgeDotString(edges);
				stmtNode->written = 1;
            }
		}
		for (auto& pair : dataNodes) {
			auto dataNode = pair.second;
			if (dataNode->written == 0 &&
				dataNode->subgraph >= 0 && dataNode->subgraph < subgraphCnt) {
				dataNode->toDotString(sgos[dataNode->subgraph]);
				dataNode->outEdgeDotString(edges);
				dataNode->written = 1;
			}
		}

		std::ostringstream os;
		os << "digraph dataFlowGraph_1{\nrankdir=LR;\n"
			<< createLegend(legend);
		subgraphDotString(os, sgos);
		os << edges.str() << "}";

#ifdef DEBUG
		std::cerr << "Dot String Created" << std::endl;
#endif
		return os.str();
	}

	std::string CompGraph::toDotString(int stmtIdx, bool reads, bool writes) {
		auto it = stmtNodes.find(stmtIdx);
		if (it == stmtNodes.end()) { return ""; }
		return partialDotString(it->second, NULL, reads, writes);
	}

	std::string CompGraph::toDotString(std::string dataIdx, bool reads, bool writes) {
		auto it = dataNodes.find(dataIdx);
		if (it == dataNodes.end()) { return ""; }
		return partialDotString(NULL, it->second, reads, writes);
	}

    std::string CompGraph::partialDotString(std::shared_ptr<StmtNode> stmtNode,
        std::shared_ptr<DataNode> dataNode, bool reads, bool writes) {
#ifdef DEBUG
		std::cerr << "Creating Dot String" << std::endl;
#endif
		if (!stmtNode && !dataNode) { return ""; }

		resetWritten();
		std::ostringstream edges;
		std::vector<std::ostringstream> sgos;
		sgos.resize(subgraphCnt);

		std::queue<std::shared_ptr<StmtNode>> stmtWQueue, stmtRQueue;
		std::queue<std::shared_ptr<DataNode>> dataWQueue, dataRQueue;

		// Add source node type to legend
		auto myLegend = legend;

		const uint8_t R = 1, W = 2, B = 3;

		// Give the source node a special node type
		NodeTypes type = stmtNode ? stmtNode->type : dataNode->type;
		if (stmtNode) {
			// Draw source node with special node type
			stmtNode->type = NodeTypes::srcStmt;
			myLegend.insert(NodeTypes::srcStmt);
			if (reads) { stmtRQueue.push(stmtNode); }
			if (writes) { stmtWQueue.push(stmtNode); }
		} else {
			// Draw source node with special node type
			dataNode->type = NodeTypes::srcData;
			myLegend.insert(NodeTypes::srcData);
			if (reads) { dataRQueue.push(dataNode); }
			if (writes) { dataWQueue.push(dataNode); }
		}

		// Draw reads/writes
		while (!stmtRQueue.empty() || !stmtWQueue.empty()
			|| !dataRQueue.empty() || !dataWQueue.empty()) {
			if (!stmtRQueue.empty()) {
				auto stmtNode = stmtRQueue.front();
				stmtRQueue.pop();
				if (stmtNode->written == 0) {
					if (stmtNode->subgraph >= 0 && stmtNode->subgraph < subgraphCnt) {
						stmtNode->toDotString(sgos[stmtNode->subgraph]);
						stmtNode->inEdgeDotString(edges);
						stmtNode->written = R;
					}
				} else if (stmtNode->written == W) {
					stmtNode->inEdgeDotString(edges);
					stmtNode->written = B;
				}
				
				for (auto& dataNode : stmtNode->in) {
					dataRQueue.push(dataNode.lock());
				}
			}
			if (!stmtWQueue.empty()) {
				auto stmtNode = stmtWQueue.front();
				stmtWQueue.pop();
				if (stmtNode->written == 0) {
					if (stmtNode->subgraph >= 0 && stmtNode->subgraph < subgraphCnt) {
						stmtNode->toDotString(sgos[stmtNode->subgraph]);
						stmtNode->outEdgeDotString(edges);
						stmtNode->written = W;
					}
				} else if (stmtNode->written == R) {
					stmtNode->outEdgeDotString(edges);
					stmtNode->written = B;
				}
				for (auto& pair : stmtNode->out) {
					dataWQueue.push(pair.first.lock());
				}
			}
			if (!dataRQueue.empty()) {
				auto dataNode = dataRQueue.front();
				dataRQueue.pop();
				if (dataNode->written == 0) {
					if (dataNode->subgraph >= 0 && dataNode->subgraph < subgraphCnt) {
						dataNode->toDotString(sgos[dataNode->subgraph]);
						dataNode->inEdgeDotString(edges);
						dataNode->written = R;
					}
				} else if (dataNode->written == W) {
					dataNode->inEdgeDotString(edges);
					dataNode->written = B;
				}
				for (auto& stmtNode : dataNode->in) {
					stmtRQueue.push(stmtNode.lock());
				}
			}
			if (!dataWQueue.empty()) {
				auto dataNode = dataWQueue.front();
				dataWQueue.pop();
				if (dataNode->written == 0) {
					if (dataNode->subgraph >= 0 && dataNode->subgraph < subgraphCnt) {
						dataNode->toDotString(sgos[dataNode->subgraph]);
						dataNode->outEdgeDotString(edges);
						dataNode->written = W;
					}
				} else if (dataNode->written == R) {
					dataNode->outEdgeDotString(edges);
					dataNode->written = B;
				}
				for (auto& pair : dataNode->out) {
					stmtWQueue.push(pair.first.lock());
				}
			}
		}

		// Reset source node type
		if (stmtNode) { stmtNode->type = type; }
		else { dataNode->type = type; }

		std::ostringstream os;
		os << "digraph dataFlowGraph_1{\nrankdir=LR;\n"
			<< createLegend(myLegend);
		subgraphDotString(os, sgos);
		os << edges.str() << "}";

#ifdef DEBUG
		std::cerr << "Dot String Created" << std::endl;
#endif
		return os.str();
	}

	void CompGraph::subgraphDotString(std::ostringstream& os,
		std::vector<std::ostringstream>& sgos) {
		if (sgos.size() < subgraphCnt) { return; }

		std::vector<bool> notEmpty(subgraphCnt, false);
		checkEmptySubgraphs(notEmpty, sgos, subgraphs);
		std::vector<SubgraphNode*> order;
		getSubgraphOrder(order, subgraphs);
		// Write non-subgraph statements
		os << sgos[0].str();
		// Skip first and last order elements
		// as these are for the non-subgraph statements
		for (auto it = order.begin(); it != order.end(); it++) {
			// Skip the first subgraph
			if (it == order.begin() || it == order.end() - 1) { continue; }
			// Start the subgraph
			if (*it && notEmpty[(*it)->val]) {
				os << "subgraph cluster" << (*it)->val << " {\n"
					<< "style = filled;\n"
					<< "color = \"\";\n"
					<< generateDotLabel((*it)->label)
					<< "\n";
				os << sgos[(*it)->val].str();
			// Skip this subgraph and its children
			} else if (*it) {
				int j = 1;
				while (j != 0 && ++it != order.end()) {
					j += *it ? 1 : -1;
				}
				if (it == order.end()) { break; }
			} else { os << "}\n"; }
		}
	}

	void CompGraph::checkEmptySubgraphs(std::vector<bool>& result,
		std::vector<std::ostringstream>& sgos,
		const SubgraphNode& sgNode) {
		if (sgos.size() < subgraphCnt || result.size() < subgraphCnt) {
            std::cerr << "checkEmptySubgraphs() input vectors too small" << std::endl;
			return;
		}

		if (sgNode.val >= 0 && sgNode.val < subgraphCnt) {
            std::streampos pos = sgos[sgNode.val].tellp();
            sgos[sgNode.val].seekp(0, std::ios_base::end);
            result[sgNode.val] = sgos[sgNode.val].tellp() != 0;
            sgos[sgNode.val].seekp(pos);
			for (const auto& child : sgNode.children) {
				checkEmptySubgraphs(result, sgos, child);
				if (result[child.val]) { result[sgNode.val] = true; }
			}
		}
	}

	void CompGraph::getSubgraphOrder(std::vector<SubgraphNode*>& order,
		SubgraphNode& sgNode) {
		order.push_back(&sgNode);
		for (auto& child : sgNode.children) {
			getSubgraphOrder(order, child);
		}
		order.push_back(NULL);
	}
}




