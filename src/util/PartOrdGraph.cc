/* PartOrdGraph.cc */

/*
   Implementation Notes:

   The partial ordering is implemented with an adjacency matrix.  We do not
   maintain the adjacency matrix as a sparse matrix, because we are assuming 
   on about 10 elements in the partial ordering, which at most would mean 
   about 100 entries in the adjacency matrix.
   
   Invariant: full transitive closure is maintained on partial ordering.
   The partial ordering aspect is maintained and checked for each insertion.
   This means that upon insertion we can check that the transitive closure
   going the other direction has not happened (although it is ok for the
   <= to go in both directions, that just indicates equality), and upon
   insertion of a new edge, transitive closure is performed.
   
   Relationships to maintain:
        if a<b  and b<c     then a<c
        if a<b  and b<=c    then a<c
        if a<=b and b<c     then a<c 

   From iscc (see constraints.iscc)
        {[i,j] : i<j and i<=j} ==> { [i, j] : i<j }

        {[i,j] : i<j and i<=j and j<=i} ==> FALSE

        {[i,j] : i<j and i>j} ==> FALSE

        {[i,j] : i<=j and j<=i} ==> { [i, j] : i=j }
        
        {[i,j] : i<=j and j=i}  ==> { [i, j] : i=j }


   Michelle Strout, 11/4/15
   Copyright (c) 2015, University of Arizona
*/

#include "PartOrdGraph.h"
#include <iostream>
#include <sstream>
#include <assert.h>

PartOrdGraph::PartOrdGraph(unsigned int N) {
    this->mN = N;
    this->mAdjacencyMatrix = new CompareEnum[N*N];
    // Initialize all 
    for (int i=0; i<N; i++) {
        for (int j=0; j<N; j++) {
            mAdjacencyMatrix[getIndex(i,j)] = NO_ORD;
            if (i==j) {
                mAdjacencyMatrix[getIndex(i,j)] = EQUAL;
            }
        }
    }
}

//! Copy constructor.  Performs a deep copy.
PartOrdGraph::PartOrdGraph(const PartOrdGraph& other) {
    *this = other;
}

//! Copy assignment.
PartOrdGraph& PartOrdGraph::operator=(const PartOrdGraph& other) {
    mN = other.mN;
    this->mAdjacencyMatrix = new CompareEnum[mN*mN];
    for (int i=0; i<mN; i++) {
        for (int j=0; j<mN; j++) {
            mAdjacencyMatrix[getIndex(i,j)]
                = other.mAdjacencyMatrix[getIndex(i,j)];
        }
    }
    return *this;
}

//! Destructor will delete the adjacency matrix.
PartOrdGraph::~PartOrdGraph() {
    delete mAdjacencyMatrix;
}

// Maintaining the above constraints when assigning an ordering to
// a pair.  When wanting to update "from" to "to" will return what
// value should become.
//      NO_ORD can be overwritten by anything
//      Anything being updated with NO_ORD will maintain value
//      NONSTRICT can be updated to anything (but NO_ORD)
//      STRICT will stay STRICT with NONSTRICT, but will fail on EQUAL.
//      EQUAL will stay EQUAL with NONSTRICT, but will fail on STRICT.
//      Anything can be updated to itself.
CompareEnum PartOrdGraph::update(CompareEnum from, CompareEnum to) {
    if (from==NO_ORD)                           { return to; }
    else if (to==NO_ORD)                        { return from; }
    else if (from==NONSTRICT)                   { return to; }
    else if (from==STRICT && to==NONSTRICT)     { return STRICT; }
    else if (from==EQUAL && to==NONSTRICT)      { return EQUAL; }
    else if (from==to)                          { return to; }
    else {
        std::cerr << "Illegal update: from = " << from << ", to = " << to 
                 << std::endl;
        assert(0); 
    }
}

// The ordering constraints belong to their own partial ordering.
// When they meet in a transitive closure to two pair,
// then we want consistent results.  Meet is commutative.
//      noOrd meet X  is noOrd
//      < meet <= is <
//      < meet =  is <
//      <= meet = is <=
//      X meet X  is X
CompareEnum PartOrdGraph::meet(CompareEnum op1, CompareEnum op2) {
    if (op1==NO_ORD)                            { return NO_ORD; }
    else if (op2==NO_ORD)                       { return NO_ORD; }
    else if (op1==EQUAL)                        { return op2; }
    else if (op2==EQUAL)                        { return op1; }
    else if (op1==STRICT || op2==STRICT)        { return STRICT; }
    else if (op1==op2)                          { return op1; }
    else {
        assert(0);      // should have dealt with all cases
    }
}



void PartOrdGraph::updatePair(unsigned int a, unsigned int b, CompareEnum to) {
    // Check vertices are within range of partial ordering.
    assert((a<mN) && (b<mN));
    
    // Fail if the opposite direction is already in partial ordering
    // or if this an an attempt for a self-loop unless we are doing EQUAL.
    if (to==STRICT) { assert(isNoOrder(b,a)); }
    if (to!=EQUAL)  { assert(a!=b); }

    // create the new ordering
    mAdjacencyMatrix[ getIndex(a,b) ] 
        = update( mAdjacencyMatrix[ getIndex(a,b) ],    // from
                  to );                                 // to

    // perform transitive closure to maintain data structure invariant
    transitiveClosure();
}

void PartOrdGraph::strict(unsigned int a, unsigned int b) {
    updatePair(a,b,STRICT);
}

void PartOrdGraph::nonStrict(unsigned int a, unsigned int b) {
    updatePair(a,b,NONSTRICT);
}

void PartOrdGraph::equal(unsigned int a, unsigned int b) {
    updatePair(a,b,EQUAL);
}

// Most straight-forward implementation.  Not efficient.
// Implements relationships described in file header.
void PartOrdGraph::transitiveClosure() {
    
    // longest possible path is mN
    for (unsigned int p=0; p<mN; p++) {
        // iterate over all rows i
        for (unsigned int i=0; i<mN; i++) {
            // iterate over all columns k
            for (unsigned int k=0; k<mN; k++) {
                // iterate over all j
                for (unsigned int j=0; j<mN; j++) {
                    mAdjacencyMatrix[ getIndex(i,k) ]
                        = update( mAdjacencyMatrix[ getIndex(i,k) ],
                                  meet( mAdjacencyMatrix[ getIndex(i,j) ],
                                        mAdjacencyMatrix[ getIndex(j,k) ]) );                    
                }
            }
        }
    }
    
    // Check for equalities and denote them.               
     // iterate over all rows i
    for (unsigned int i=0; i<mN; i++) {
        // iterate over all columns k
        for (unsigned int k=0; k<i; k++) {
            if (isNonStrict(i,k) && isNonStrict(k,i)) {
                mAdjacencyMatrix[ getIndex(i,k) ] = EQUAL;        
                mAdjacencyMatrix[ getIndex(k,i) ] = EQUAL;   
            }
        }
    }     
}

bool PartOrdGraph::isStrict(unsigned int a, unsigned int b) {
    assert((a<mN) && (b<mN));
    return (mAdjacencyMatrix[ getIndex(a,b) ] == STRICT);
}

bool PartOrdGraph::isNonStrict(unsigned int a, unsigned int b) {
    assert((a<mN) && (b<mN));
    return (mAdjacencyMatrix[ getIndex(a,b) ] == NONSTRICT);
}

bool PartOrdGraph::isEqual(unsigned int a, unsigned int b) {
    assert((a<mN) && (b<mN));
    return (mAdjacencyMatrix[ getIndex(a,b) ] == EQUAL);
}

bool PartOrdGraph::isNoOrder(unsigned int a, unsigned int b) {
    assert((a<mN) && (b<mN));
    return (mAdjacencyMatrix[ getIndex(a,b) ] == NO_ORD);
}


// Doing indexing arithmetic in a helper function.
unsigned int PartOrdGraph::getIndex(unsigned int a, unsigned int b) {
    return a*mN + b;
}

std::string PartOrdGraph::toString() {
    std::stringstream ss;
    ss << "PartOrdGraph:\n\tmN = " << this->mN << std::endl;
    // column headers
    ss << "\t";
    for (int j=0; j<this->mN; j++) {
        ss << "\t" << j;
    }
    ss << std::endl;
    // rows in adjacency matrix
    for (int i=0; i<this->mN; i++) {
        ss << "\t" << i;
        for (int j=0; j<this->mN; j++) {
            ss << "\t";
            if (isStrict(i,j)) { ss << "<"; }
            if (isNonStrict(i,j)) { ss << "<="; }
            if (isEqual(i,j)) { ss << "="; }
        }
        ss << std::endl;
    }
    ss << std::endl;

    return ss.str();
}

