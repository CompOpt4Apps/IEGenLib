/* 
   PartOrdGraph.h

   A graph representing a partial ordering.  The nodes are all integers (use an
   external data structure that contains a PartOrdGraph to map more interesting
   objects to the integers).  The ordering between each vertex in the graph
   can be strict or non-strict.  Cycles are not permitted.

   Example usage:
    PartOrdGraph g(N);          // N is the number of vertices in PO
    g.strict(a,b);              // a < b, asserts if causes a cycle
    g.nonstrict(a,b);           // a <= b, asserts if causes a cycle

    lt = g.isStrict(a,b);       // returns true if a<b
    lte = g.isNonStrict(a,b);   // returns true if a<=b
    
    str = g.toString();         // returns a string representation of data structure

   Michelle Strout, 11/4/15
   Copyright (c) 2015, University of Arizona
*/

#include <string>
#include <cassert>

#ifndef __PARTORDGRAPH__
#define __PARTORDGRAPH__

typedef enum {
    NO_ORD=3,
    STRICT=1,
    NONSTRICT=2,
    EQUAL=0
} CompareEnum;


class PartOrdGraph {
    public:
        PartOrdGraph(unsigned int curN, unsigned int maxN);
        //! Copy constructor.  Performs a deep copy.
        PartOrdGraph(const PartOrdGraph& other);
        //! Copy assignment.
        PartOrdGraph& operator=(const PartOrdGraph& other);
        //! Destructor will delete the adjacency matrix.
        ~PartOrdGraph();

        void strict(unsigned int, unsigned int);
        void nonStrict(unsigned int, unsigned int);
        void equal(unsigned int, unsigned int);

        bool isStrict(unsigned int, unsigned int);
        bool isNonStrict(unsigned int, unsigned int);
        bool isEqual(unsigned int, unsigned int);
        bool isNoOrder(unsigned int, unsigned int);

        int numItems() { return mCurN; }
        int numMaxItems() { return mMaxN; }

        void updateNumItems(int i) { 
          assert( (mCurN+i) <= mMaxN );
          //if( (mCurN+i) > mMaxN ){
          //  throw assert_exception( "PartOrdGraph::updateNumItems: ERROR"
          //    " exceeding parOrdGraph's Max capicity");
          //}
          mCurN += i;
        }

        std::string toString(); 

    private:
// MAHDI FIXME
//        unsigned int mN;
        unsigned int mCurN;
        unsigned int mMaxN;

        CompareEnum * mAdjacencyMatrix;

        // helper routines to access the matrix, stored in 1D array
        unsigned int getIndex(unsigned int,unsigned int);
    
        // Most of logic for each compare type is shared.    
        void updatePair(unsigned int a, unsigned int b, CompareEnum to);
    
        // maintains logic for assigning new relationship between a pair
        CompareEnum update(CompareEnum from, CompareEnum to);
        
        // maintains transtive closure logic
        CompareEnum meet(CompareEnum op1, CompareEnum op2);

        
        // routine for performing transitive closure on ordering
        void transitiveClosure();
};

#endif
