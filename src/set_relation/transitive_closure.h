/*!
* \file transitive_closure.h
*
* \brief Transitive Closure Support in presence of UF.
*
* This file contains algorithms for transitive closure
*  in the presence of UFs.
*
* \date Started: 4/15/21
*
* \authors Tobi Popoola
*
* Copyright (c) 2012, 2013 Colorado State University <br>
* Copyright (c) 2015-2016, University of Arizona <br>
* Copyright (c) 2021       Boise State University <br>
* All rights reserved. <br>
* See ../../COPYING for details. <br>
*/
#ifndef TRANSITIVE_CLOSURE_H
#define TRANSITIVE_CLOSURE_H
#include <vector>
#include <string>
#include <list>
#include <ostream>
#include <memory>
#include "expression.h"
namespace iegenlib{

/**!
 * \enum EdgeType
 * 
 * \brief Describes the edge between vertices in transitive closure Di Grpah
 *        The order of elements in the enum is important as it reflects
 *        precedence.
 *
 */
enum class EdgeType {GREATER_THAN=0 , GREATER_OR_EQUAL_TO, EQUAL,NONE };
class Vertex;

// TODO: Change the type specificatiction for 
// vertices in the graph for better data control
typedef std::unique_ptr<Vertex> VertexPtr;

/*!
 *  \struct Vertex
 *
 *  \brief A Vertex in the graph.
 *       
 *
 */
class Vertex{
    std::list<Term*> terms;
public:
    ~Vertex();
    
    //! Function adds term to Vertex.    
    //! \param t (adopted)    
    void addTerm(Term * t);
   
    //! Destructor.
    void reset();
   
    //! Copy assignment. Deep Copy
    Vertex& operator= (const Vertex &v1);

    std::string toString() const;
    bool operator== (const Vertex &v1);
    
    //! Function gets constant number present
    // in the vertex, it returns a null pointer
    // if there is none.
    Term* getConstantNumber() const;
    
    //! Function returns list of terms, these
    //  terms are owned by the object.
    std::list<Term*> &getTermList() {return terms;}
   
    //! Function deletes term from vertex.
    void deleteTerm(Term *t);
};


/*!
 *  \class DiGraph
 *
 *  \brief A Directed Graph representing 
 *         constraint expressions as vertex in a 
 *         directed graph.
 *       
 *
 */
class DiGraph{
private:
    std::vector<std::vector<EdgeType>> adj;
    std::vector<Vertex> vertices;
    //! Expands adjMatrix by 1
    void adjustAdjMatrix( int N);
   /*!
    * Function looks for monotonic vertices
    * and appropriates monotonicity to enclosing
    * vertices.
    * rowptr(i+1) >= col(i,j) > rowptr(i)
    * col(i+1,j) > col(i,j)
   */
    void findAddMonotonicity () ;

    /*! Function runs the transitive closure algorithm: Floyd Warshall on 
     * the graph. The graph changes in place.
    */
    void transitiveClosure();
    /*!
     * Simplifies the graph to remove unnecessary, >= constraints
     * 
     * */
    void simplifyGreaterOrEqual();
    

      
    //! This outlines the combination of two edges.
    EdgeType edgeOp (const EdgeType e1, const EdgeType e2);

    //! Converts an edge to an actual string.
    std::string edgeToString (const EdgeType e)const;
    //! This function performs a lookup of nodes in 
    //the graph and returns single term nodes that 
    //have a direct equality relationship with this term
    // \param t term being aliased for and is not adopted.
    std::vector<Term*> getAliasTerms(Term& t);
public:
    
    DiGraph();
    
    ~DiGraph();
    
    /* Performs closure on the graph.
     * */
    void Closure(); 
    /*! Adds an edge to the graph.
     * pointers in u and v gets adopted.
     * */
    void addEdge(Vertex u, Vertex v, EdgeType e); 
    
    
    /*!
     * Dump debug information of the graph.
     * *
     * */
    void dumpGraph(std::ostream& os) const;
    
    /**
     * !
     * Dump graph as a dot format to string.
     */
    std::string toDotString() const;


    /**
     * !
     * Function returns edges in the graph as expressions.
     * Example: Vertex u =  0, Vertex v = row(n), Edge: >
     *          Expression =  -row(n) - 1 >=  0
     * Expressions are owned by the caller, caller must deallocate.          
     */
    std::vector<Exp*> getExpressions();   
};
}// namespace iegen


#endif


