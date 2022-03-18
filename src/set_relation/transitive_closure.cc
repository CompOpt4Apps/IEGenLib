#include "transitive_closure.h"
#include <algorithm>
#include <assert.h>
#include <iostream>
#include <typeinfo>
#include <sstream>
#include <map>

// TODO: Switch internal memory to use heap memory
// for vertices.
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


std::vector<Term*> DiGraph::getAliasTerms(Term& t){
   Vertex tVert;
   tVert.addTerm(&t);
   std::vector<Term*> res;
   for(int i = 0; i < vertices.size() ; i++){
      Vertex& vertex = vertices[i];
      if (vertex == tVert){
         // Find vertices with equality relationships
	 // we are currently only concerned with 
	 // single terms 
	 for( int j = 0; j < adj[i].size(); j++){
            if(i!=j && adj[i][j] == EdgeType::EQUAL){
	       Vertex& jVertex = vertices[j];
	       if(jVertex.getTermList().size() == 1){
	           res.push_back(jVertex.getTermList().front());   
	       } 
	    }
	 }
      }
   }
   return res;
}


void DiGraph::Closure(){
    simplifyGreaterOrEqual();
    transitiveClosure();
    findAddMonotonicity ();
}


void DiGraph::simplifyGreaterOrEqual(){
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
	       }else{
	           t->setCoefficient(newCoefficient);
	       }
               // Remove the current constraint since 
	       // another node has been created to 
	       // specify the constraint.
	       adj[i][j] = EdgeType::NONE;
	       
	       // Add a new greater than edge to the graph from vertex at i
	       // to the newVertex.
               addEdge(vertices[i],newVertex,EdgeType::GREATER_THAN);
	    }  
       }
    }
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
 * rowptr(i+1) >= col(i,j) > rowptr(i)
 * col(i+1,j) > col(i,j)
*/
void DiGraph::findAddMonotonicity (){
    int vertexSize = vertices.size();
    for( int i = 0 ;  i < vertexSize; i ++ ){
        if (vertices[i].getTermList().size() == 1 && 
            vertices[i].getTermList().front()->isUFCall()){
	    UFCallTerm* currentUF = (UFCallTerm*) vertices[i].
			   getTermList().front();
 
	    std::vector<std::pair<int,EdgeType>> upperBounds;
	    std::vector<std::pair<int,EdgeType>> lowerBounds;

	    for(int j = 0 ; j < vertexSize; j++){
	        if (vertices[j].getTermList().size() == 1 &&
		    vertices[j].getTermList().front()->isUFCall()) {
		    
	            if ( adj[j][i] == EdgeType::GREATER_OR_EQUAL_TO||
	                  adj[j][i] == EdgeType::GREATER_THAN){
		        // This is some UF Call that upper bounds
		        // current vertex
		        upperBounds.push_back(std::make_pair(j,adj[j][i]));
		    }
                    if ( adj[i][j] == EdgeType::GREATER_OR_EQUAL_TO||
	                  adj[i][j] == EdgeType::GREATER_THAN){
		        // This is some UF Call that lower bounds
		        // current vertex.
		        lowerBounds.push_back(std::make_pair(j,adj[i][j]));
		    }
		}
	    }

	    //Iterate through upper and lowe bounds for 
	    //current UF.

	    for(auto& upperVertex : upperBounds){
	        for (auto& lowerVertex: lowerBounds) {
	            UFCallTerm* uV = (UFCallTerm*) vertices[upperVertex.first].
			   getTermList().front();
		    UFCallTerm* lV = (UFCallTerm*) vertices[lowerVertex.first].
			   getTermList().front();
                    // Right now only restrict to upper and lower bounds 
		    // with the same uf & the same number of arguments
		    // the upper and lower bound must be 
		    if(lV->name()!=uV->name() || lV->numArgs() != uV ->numArgs())
	                continue;

		    bool isMonotonic = true;

		    // pair of upper bound and lower bound 
		    // parameter expressions for each argument
                    std::vector<std::pair<Exp*,Exp*>> paramExpPairs; 
		    for(int q = 0 ;  q < uV->numArgs() ; q++ ){
                        //TODO: Refactor this code once we can 
			// compare two expressions.
			//
			// We need to ensure every index acess
			// to the UF must be consistent
			// with it's monotnicity. But we currently'
			// do not have a way to prove that an expression
			// is greater than another.
			Exp* expL = lV->getParamExp(q);
			Exp* expU = uV->getParamExp(q);
			// This contradicts the upper bound and lower
			// bound relationship thereby it is not monotnic
			//if(*expL > *expU){
			//    isMonotonic = false;
			//    break;
			//}
			// Create a new expression 
			// NewExp = upperBoundExp - lowerBoundExpression
			// This will give a new expression that can 
			// then be added to the UF bounded by 
			// this monotonicity.
			//Exp* expTemp = expU->clone(); 
			//Exp* expTempLBound = expL->clone();
			//expTempLBound->multiplyBy(-1);
			//expTemp->addExp(expTempLBound);
			//resExp.push_back(expTemp);
			paramExpPairs.push_back({expL ,expU});
		        	
		    }

		    if(isMonotonic){
		        UFCallTerm* currentClone =(UFCallTerm*) currentUF->clone();
		        EdgeType strongerConst = edgeOp(upperVertex.second,
			    lowerVertex.second);	
                        // Create new relationship with 
			// current vertex
                       
			// Iterate through each parameters 
			// in current UF. See which tuple argument,
			// has a term present in list of expressions 
			// from upper and lower bounds.
			for( int q = 0; q < currentUF->numArgs(); q++){
			    Exp* exp = currentClone->getParamExp(q);
			    for(auto term: exp->getTermList()){
			       // At this point there are new relationships
			       // and a term T might be equal to some other 
			       // terms so we need to get alias terms 
			       auto aliasTerms= getAliasTerms(*term);  
				auto it = std::find_if(paramExpPairs.begin(),
				   paramExpPairs.end(),
			           [term,&aliasTerms](
			           std::pair<Exp*,Exp*>& e){ 
				       
				       bool res = false;
				       for(auto t : aliasTerms){
				           res|= e.first->dependsOn(*t) || 
					         e.second->dependsOn(*t);
				       }
				       res |= e.first->dependsOn(*term)
				       || e.second->dependsOn(*term);
				       return res;
				   });
			       if (it!=paramExpPairs.end()){
                                   
			           Exp* expTempUBound = (*it).second->clone(); 
			           Exp* expTempLBound = (*it).first->clone();
			           expTempLBound->multiplyBy(-1);
			           expTempUBound->addExp(expTempLBound);
			           exp->addExp(expTempUBound);
			       }
			    }
			}
                        Vertex newVertex;
                        newVertex.addTerm(currentClone);
                        addEdge(vertices[i],newVertex,strongerConst);
                        

		    }
		}
	    }
	}	
    }    
}
}
