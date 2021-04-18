#include "transitive_closure.h"
#include <algorithm>
#include <assert.h>
#include <iostream>
#include <typeinfo>
namespace iegenlib{


/******************************************************************************/

/******************************************************************************/
#pragma mark -
/****************** Vertex Code *******************************/


std::string  Vertex::toString()  const {
    std::string result;
    bool firstTerm = true;
    // FIXME does the line just below go in other
    // toString() or prettyPrintString methods?
    if(terms.size() == 0) return "0";
    for (std::list<Term*>::const_iterator i=terms.begin();
            i != terms.end(); ++i) {
        bool absValue = false;
        if (not firstTerm) {
            if ((*i)->coefficient() < 0) {
                result += " - ";
            } else {
                result += " + ";
            }
            absValue = true;
        }
        result += (*i)->toString(absValue);
        firstTerm = false;
    }
    return result;
}

bool  Vertex::operator== (const Vertex &v1){
    if(terms.size()!=v1.terms.size()){
        return false;
    }
    for(auto t:v1.terms){
        auto it = std::find_if(terms.begin(),terms.end(),
			[&t](Term* a){
			    return typeid(t) == typeid(a) && *a == *t;
			});
	if(it== terms.end())
            return false;
    }
    return true;
}


Term* Vertex::getConstantNumber() const{
    auto it = std::find_if(terms.begin(),terms.end(),[](Term* a){
		    return a->isConst();});
    if(it != terms.end())
        return *it;
    return nullptr;
}



//! Function deletes term from vertex.
void Vertex::deleteTerm(Term *t){
    auto it = std::find(terms.begin(),terms.end(),t);
    if(it!=terms.end()){
        terms.erase(it);
    }    
}

//! Function adds term to Vertex.
//! \param t (adopted)    
void Vertex::addTerm(Term * t){
    if (t == nullptr){
        throw assert_exception("Term cannot be null");
    }
    terms.push_back(t);
        
}
   
//! Destructor.
void Vertex::reset(){
    for(auto t : terms){
        delete t;
    }
    terms.clear();
}


/******************************************************************************/

/******************************************************************************/
#pragma mark -
/****************** DiGraph  Code *******************************/


DiGraph::DiGraph()
{
}


DiGraph::~DiGraph()
{
    for(auto v : vertices){
        v.reset();
    }
    vertices.clear();
    //TODO: Check this out, might not necessarily be needed.
    for(int i = 0; i < adj.size(); i++){
        adj[i].clear();
    }
    adj.clear();
}


//! This outlines the combination of two edges.
EdgeType DiGraph::edgeOp (const EdgeType e1, const EdgeType e2){
    if(e1 < e2) return e1;
    else return e2;
}

void  DiGraph::addEdge(Vertex u, 
		Vertex v, EdgeType e){
    auto itU = std::find(vertices.begin(),vertices.end(),u);
    // Get the Positions to place the edges
    // in the adjacency matrix.
    int uPos =  0;
    int vPos =  0;
    if (itU == vertices.end()){
        uPos = vertices.size();
	vertices.push_back(u);

	//adjust adjacency matrix.
	adjustAdjMatrix(vertices.size());
    }else{
        uPos = itU - vertices.begin();
    }

    auto itV = std::find(vertices.begin(),vertices.end(),v);
      
    if (itV == vertices.end()){
        vPos = vertices.size();
	vertices.push_back(v);

	//adjust adjacency matrix.
	adjustAdjMatrix(vertices.size());
    }else{
        vPos = itV - vertices.begin();
    } 

    adj[uPos][vPos] = e; 
}

void DiGraph::adjustAdjMatrix(int N){
    int currentSize = adj.size();
    for(int i = 0; i < N; i++){
        if(i < currentSize)
	    adj[i].resize(N,EdgeType::NONE);
	else if(i >= currentSize){
	    adj.push_back(std::vector<EdgeType>(N,EdgeType::NONE)); 
	}
    }
}
void DiGraph::transitiveClosure(){
   // Using floyd warshall algorithm 
   for(int k = 0; k < adj.size(); k++){
       for(int i = 0 ; i < adj.size(); i++){
           for(int j = 0; j < adj[i].size(); j++){
	      if(adj[i][k] != EdgeType::NONE && adj[k][j] !=EdgeType::NONE){
	          // replace current relation ship with strongest
		  // constraint
		  adj[i][j] = edgeOp(adj[i][j],edgeOp(adj[i][k],adj[k][j]));
	      } 
	   }
       }
   }
}


void DiGraph::mergeVertices( int u, int v) {
    //TODO: Choose which vertice to delete, after the merge
    //discuss with Dr Cathie. 
    assert(0 && "merging vertices not currently supported");   
}

void DiGraph::simplifyGreaterOrEqual(){
    //There is a chance that simplifying constraint
    //will lead to vertexs that have the same node
    //content. When this happens such vertexs will
    //be merged in the adjacency matrix.
    std::vector<int> possibleMerge;
    
    for(int i = 0 ; i < adj.size(); i++){
        for(int j = 0; j < adj[i].size(); j++){
	    if(adj[i][j] == EdgeType::GREATER_OR_EQUAL_TO ){
	       Term* t = vertices[j].getConstantNumber();
	       if(!t) continue;
	       
	       // Remove +1 from destination and replace
	       // constaint with EdgeType::GREATER_THAN
	       adj[i][j] = EdgeType::GREATER_THAN;
               int newCoefficient = t->coefficient() - 1;
	       // remove the term if the subtraction results
	       // to a zero. 
	       if (newCoefficient == 0){
	           vertices[j].deleteTerm(t);
	       }
	       possibleMerge.push_back(j);
	    }  
       }
   }
   
   // Apply merging. 
   for(auto pos :possibleMerge){
       int length = vertices.size();
       for(int i = 0 ; i < length; i++){
           if( i != pos && vertices[i] == vertices[pos]){
	       mergeVertices(i,pos);
	       --length; 
	   }
       } 
   }

}
void DiGraph::dumpGraph() const{
    std::cerr << "\nVertices:\n";
    int i = 0;
    for(auto v : vertices){
       std::cerr << "[" << i++ << "]->" << v.toString() << "\n";
    }
   
    for(int i = 0 ; i < adj.size(); i++){
       std::cerr<< "\n";
       for(int j = 0; j < adj[i].size(); j++){
           std::cerr << (adj[i][j] == EdgeType::GREATER_THAN?">":
			   adj[i][j] == EdgeType::GREATER_OR_EQUAL_TO?">=":
			   adj[i][j] == EdgeType::EQUAL ? "=": "#") << " ";
       }
    }
    std::cerr << "\n";
}

/**
  * !
  * Function returns edges in the graph as expressions.
  * Example: Vertex u =  0, Vertex v = row(n), Edge: >
  *          Expression =  -row(n) - 1 >=  0
  * Expressions are owned by the caller, caller must deallocate.          
  */
std::vector<Exp*> DiGraph::getExpressions() const {
    std::vector<Exp*> res;
    for (int i = 0 ; i < adj.size(); ++i){
        for(int j = 0; j < adj[i].size();  ++j){
	    if(adj[i][j] == EdgeType::NONE)
                continue;
            Exp * e = new Exp;
	    std::list<Term*> lhsTerms = vertices[i].getTermList();
	    std::list<Term*> rhsTerms = vertices[j].getTermList();
            for(auto t: rhsTerms){
	        Term* tClone = t->clone();
		// rhs getTermList() are multiplied by -1 to move 
		// to lhs to be iegenlib compliant.
		tClone->setCoefficient(tClone->coefficient() * -1);
                e->addTerm(tClone);
	    }
	       
            for(auto t: lhsTerms){
	        Term* tClone = t->clone();
		// rhs getTermList() are multiplied by -1 to move 
		// to lhs to be iegenlib compliant.
                e->addTerm(tClone);
	    }    
	    
	    switch (adj[i][j]){
	        case EdgeType::GREATER_OR_EQUAL_TO:
		    // set inequality >= 0
		    e->setInequality();
		    break;
		case EdgeType::EQUAL:
		    e->setEquality();
                    break;
		case EdgeType::GREATER_THAN:
		    // add a constant -1 to expression to make it >=0
		    e->setInequality();
		    Term* t = new Term(-1);
		    e->addTerm(t);
		    break;
	    }
            res.push_back(e);
	}	
    }
    return res;
}	

}
