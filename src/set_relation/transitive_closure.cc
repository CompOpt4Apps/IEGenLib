#include "transitive_closure.h"
#include <algorithm>
#include <assert.h>
#include <iostream>
#include <typeinfo>
#include <sstream>
#include <map>
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


//! Copy assignment. Deep Copy
Vertex& Vertex::operator= (const Vertex &v1){
    reset();
    for(auto t : v1.terms){
        this->terms.push_back(t->clone());
    }
}

//! Destructor
Vertex::~Vertex(){
  //  reset();
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
   // for(auto v : vertices){
   //     v.reset();
   // }
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

void  DiGraph::addEdge(Vertex& u, 
		Vertex& v, EdgeType e){
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


void DiGraph::mergeVertices(std::vector<int>& aliasMap, int u, int v) {
    //assert(0 && "merging vertices not currently supported");   
    // Remove edges between u and v
    adj[u][v] = EdgeType::NONE;
    adj[v][u] = EdgeType::NONE;

    // Put all the contents of v in u.
    for(int i = 0; i < adj[v].size() ; i++){
	// edgeOp will pick a stronger constraint
	// if a constraint already exist in u when
	// merging.
	
	// Merge all outgoing edges in v with u.    
        adj[u][i] = edgeOp(adj[u][i],adj[v][i]);

	// Merge all incoming edges in v with u.
	adj[i][u] = edgeOp(adj[i][u],adj[i][v]);
    }

    // Remove column v from the adj matrix.

    // Shift each column after index v backward 
    // to delete column v for each row.
    for(int i =  0 ; i < adj.size() ; i++){
        for(int j = v; j < adj[i].size()-1; j++){
	    adj[i][j] = adj[i][j+1];

	}
	adj[i].resize(adj[i].size() -1);
    }
     
    // update aliasmap to ensure that oldvertices
    // are mapped to a new position after merge.
    //
    // The way it works is that vertexes just after
    // the vertex removed gets their indices decremented
    // by 1 in the alias map.
    for(int i = v+1; i < aliasMap.size(); i++){
        --aliasMap[i];
    }
    // Any check on deleted vertex becomes aliased 
    // to the merged vertex u; 
    aliasMap[v] = u;

    // Remove row v from the matrix.
    adj.erase(adj.begin() + v);
    // Clear content of removed vertex.
    vertices[v].reset();
    // Remove v from vertices
    vertices.erase(vertices.begin() +v);
}

void DiGraph::simplifyGreaterOrEqual(){
    //There is a chance that simplifying constraint
    //will lead to vertexs that have the same node
    //content. When this happens such vertexs will
    //be merged in the adjacency matrix.
    std::vector<int> possibleMerge;
    int size = adj.size();
    for(int i = 0 ; i < size; i++){
        for(int j = 0; j < size; j++){
	    if(adj[i][j] == EdgeType::GREATER_OR_EQUAL_TO ){
	       Term* t = vertices[j].getConstantNumber();
	       if(!t) continue;
	       
	       
	       // Deep copy current vertex. This copy will
	       // be added to the graph rather than modifying
	       // j's vertex
	       Vertex newVertex = vertices[j]; 
	       t = newVertex.getConstantNumber(); 
               int newCoefficient = t->coefficient() - 1;
	       // remove the term if the subtraction results
	       // to a zero. 
	       if (newCoefficient == 0){
	           newVertex.deleteTerm(t);
	       }
	       // Add a new greater than edge to the graph from vertex at i
	       // to the newVertex.
               addEdge(vertices[i],newVertex,EdgeType::GREATER_THAN);

               // Remove the current constraint since 
	       // another node has been created to 
	       // specify the constraint.
	       adj[i][j] = EdgeType::NONE;

	       
	    }  
       }
    }
   /* 
    // Apply merging. 
    // Vertices aliasing, this data structure helps
    // to store already merged vertices to where they
    // point to after merging.
    
    std::vector<int> aliasMap (vertices.size());
    // Intialize indices in aliasMap to map back to 
    // its index value.
    for(int i  = 0 ; i < aliasMap.size() ;i++){
        aliasMap[i] = i;
    }

    for(auto vertex :possibleMerge){
        int length = vertices.size();
        for(int i = 0 ; i < length; i++){
	    // Check if there is an alias for the current vertex.
            int aliasVertex = aliasMap[vertex];
	     
	    // if a current vertex is the same as the vertex for possible 
	    // merge, go ahead to merge. 
	    if( i != aliasVertex && vertices[i] == vertices[aliasVertex]){
		mergeVertices(aliasMap,i,aliasVertex);
	        break;
	    }
        } 
    }
*/
}
void DiGraph::dumpGraph(std::ostream& os) const{
    os << "\nVertices:\n";
    int i = 0;
    for(auto v : vertices){
       os << "[" << i++ << "]->" << v.toString() << "\n";
    }
    // TODO: Make enum to string function  
    for(int i = 0 ; i < adj.size(); i++){
       os<< "\n";
       for(int j = 0; j < adj[i].size(); j++){
           os <<edgeToString(adj[i][j])<< " ";
       }
    }
    os << "\n";
}

//! Converts an edge to an actual string.
std::string DiGraph::edgeToString (const EdgeType e) const{
   return (e == EdgeType::GREATER_THAN?">":
	   e == EdgeType::GREATER_OR_EQUAL_TO?">=":
	   e == EdgeType::EQUAL ? "=": "#"); 
}

/**
 * !
 * Dump graph as a dot format to string.
*/
std::string DiGraph::toDotString() const{
    std::stringstream ss;
    ss << "digraph constraintGraph { \n";
    // Outline all edges.
    for(int i = 0; i < adj.size() ; i++){
        for(int j = 0; j < adj[i].size(); j++){
            if(adj[i][j] != EdgeType::NONE){
               ss << i << "->" << j << "[label=\""<<
		       edgeToString(adj[i][j])<<"\"] ;\n" ;   
	    }
	}
    }
    // label all vertices
    for(int i =0; i < vertices.size(); i++){
        ss << i << "[label=\""<< vertices[i].toString() <<"\"];\n";
    }
    ss << "}\n";
    return ss.str();
}

/**
  * !
  * Function returns edges in the graph as expressions.
  * Example: Vertex u =  0, Vertex v = row(n), Edge: >
  *          Expression =  -row(n) - 1 >=  0
  * Expressions are owned by the caller, caller must deallocate.          
  */
std::vector<Exp*> DiGraph::getExpressions() {
    std::vector<Exp*> res;
    for (int i = 0 ; i < adj.size(); ++i){
        for(int j = 0; j < adj[i].size();  ++j){
	    if(adj[i][j] == EdgeType::NONE)
                continue;
            Exp * e = new Exp;
	    std::list<Term*>& lhsTerms = vertices[i].getTermList();
	    std::list<Term*>& rhsTerms = vertices[j].getTermList();
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

/*!
 * Function looks for monotonic vertices
 * and appropriates monotonicity to enclosing
 * vertices.
 * rowptr(i) <= col(i,j) < rowptr(i+1)
 * col(i,j) < col(i+1,j)
*/
/*
void DiGraph::findAddMonotonicity (){
    for( int i = 0 ;  i < vertices.size(); i ++ ){
        if (vertices[i].getTermList().size() == 1 && 
			vertices[i].getTermList()[0].isUFCall()){
	    for (int j = i+1; j < vertices.size(); j++){
	        if(vertices[j].getTermList().size() == 1 && 
			vertices[j].getTermList()[0].isUFCall()){
		    UFCallTerm* u1 = vertices[i].getTermList()[0];
		    UFCallTerm* u2 = vertices[j].getTermList()[0];
		    if(u1->name()!=u2->name() || u1->numArgs() != u2->numArgs())
	                continue;
                    bool isMonotonic = true;
		    for(int q = 0 ;  q < u1->numArgs() ; q++ ){
		        // Relationship in adjacency matrix must 
			// be consitent with each param expression 
			if(adj[i][j] == EdgeType::GREATER_THAN && 
			   u1->getParamExp(q) < u2->getParamExp(q)){
			    isMonotonic = false;
			    break;
			}else 
			if(adj[j][i] == EdgeType::GREATER_THAN && 
			   u2->getParamExp(q) < u1->getParamExp(q)){
			    isMonotonic = false;
			    break;
			}
                        
		    }



		}
	    }
	}	
    }    
}
*/
}
