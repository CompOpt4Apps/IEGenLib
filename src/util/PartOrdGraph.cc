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

PartOrdGraph::PartOrdGraph(unsigned int maxN) {
    this->mCurN = 0;
    this->mMaxN = maxN;
    unsat=false;
    this->mAdjacencyMatrix = new CompareEnum[maxN*maxN];
    // Initialize all 
    for (unsigned int i=0; i<maxN; i++) {
        for (unsigned int j=0; j<maxN; j++) {
            mAdjacencyMatrix[getIndex(i,j)] = NO_ORD;
            if (i==j) {
                mAdjacencyMatrix[getIndex(i,j)] = EQUAL;
            }
        }
    }
}

//! Copy constructor.  Performs a deep copy.
PartOrdGraph::PartOrdGraph(const PartOrdGraph& other) {

    mCurN = other.mCurN;
    mMaxN = other.mMaxN;
    unsat = other.unsat;
    this->mAdjacencyMatrix = new CompareEnum[mMaxN*mMaxN];
    for (unsigned int i=0; i<mMaxN; i++) {
        for (unsigned int j=0; j<mMaxN; j++) {
            mAdjacencyMatrix[getIndex(i,j)]
                = other.mAdjacencyMatrix[getIndex(i,j)];
        }
    }
}

//! Copy assignment.
PartOrdGraph& PartOrdGraph::operator=(const PartOrdGraph& other) {

    PartOrdGraph temp(other);
    temp.swap (*this); // Non-throwing swap
    return *this;
}

//! helper function for implementing copy-and-swap
void PartOrdGraph::swap(PartOrdGraph& second) throw() {
    std::swap(mCurN, second.mCurN);
    std::swap(mMaxN, second.mMaxN);
    std::swap(unsat, second.unsat);
    std::swap(mAdjacencyMatrix, second.mAdjacencyMatrix);
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
        return Illegal;
        // FIXME Mahdi: Should I consider this assertion as a case of unsat?!
        //std::cerr << "Illegal update: from = " << from << ", to = " << to 
        //         << std::endl;
        //assert(0); 
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
        return Illegal;
        //assert(0);      // should have dealt with all cases
    }
}



void PartOrdGraph::updatePair(unsigned int a, unsigned int b, CompareEnum to) {
    // FIXME Mahdi: turn these asserts into "throw assert_exception"
    // Check vertices are within range of allocated memory for partial ordering.
    assert((a<mMaxN) && (b<mMaxN));

    if( a >= mCurN ){ // If any of a or b is bigger than current number of
      mCurN = a+1;    // terms that PartOrdGraphis keeping track of, it 
    }                 // means that TermPartOrdGraph has inserted some new
    if( b >= mCurN ){ // terms (at least one). Therefore, we need to update 
      mCurN = b+1;    // the number of current terms, namely mCurN.
    }

    // Fail if the opposite direction is already in partial ordering
    // or if this an an attempt for a self-loop unless we are doing EQUAL.
    if (to==STRICT) { 
      if( !isNoOrder(b,a) ){unsat=true; return;}
    }
    if (to!=EQUAL)  { assert(a!=b); }

    // create the new ordering
    mAdjacencyMatrix[ getIndex(a,b) ] 
        = update( mAdjacencyMatrix[ getIndex(a,b) ],    // from
                  to );                                 // to

    if( mAdjacencyMatrix[ getIndex(a,b) ] == Illegal ){unsat=true; return;}

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

// Implements relationships described in file header.
// This implementation is an adaptation from Floyd Warshall
// F-W calculates all pairs shortest paths in O(n^3)
// transitive closure is an easier problem than this, the same algorithm works
void PartOrdGraph::transitiveClosure() {
    
    // iterate over all nodes k
    for (unsigned int k=0; k<mCurN; k++) {
        // iterate over all rows k
        for (unsigned int i=0; i<mCurN; i++) {
            // iterate over all columns j
            for (unsigned int j=0; j<mCurN; j++) {
                mAdjacencyMatrix[ getIndex(i,j) ]
                    = update( mAdjacencyMatrix[ getIndex(i,j) ],
                              meet( mAdjacencyMatrix[ getIndex(i,k) ],
                                    mAdjacencyMatrix[ getIndex(k,j) ]) );   
                if( mAdjacencyMatrix[ getIndex(i,j) ] == Illegal )
                {unsat=true; return;}      
                if (isNonStrict(i,j) && isNonStrict(j,i)) {
                    mAdjacencyMatrix[ getIndex(i,j) ] = EQUAL;        
                    mAdjacencyMatrix[ getIndex(j,i) ] = EQUAL;   
                }
            }
        }
    }
}

bool PartOrdGraph::isStrict(unsigned int a, unsigned int b) {
    return (mAdjacencyMatrix[ getIndex(a,b) ] == STRICT);
}

bool PartOrdGraph::isNonStrict(unsigned int a, unsigned int b) {
    return (mAdjacencyMatrix[ getIndex(a,b) ] == NONSTRICT);
}

bool PartOrdGraph::isEqual(unsigned int a, unsigned int b) {
    return (mAdjacencyMatrix[ getIndex(a,b) ] == EQUAL);
}

bool PartOrdGraph::isNoOrder(unsigned int a, unsigned int b) {
    return (mAdjacencyMatrix[ getIndex(a,b) ] == NO_ORD);
}


// Doing indexing arithmetic in a helper function.
unsigned int PartOrdGraph::getIndex(unsigned int a, unsigned int b) {
    return a*mMaxN + b;
}

std::string PartOrdGraph::toString() {
    std::stringstream ss;
    ss << "PartOrdGraph:\n\tmCurN = "<<this->mCurN<< 
                         "\tmMaxN = "<<this->mMaxN<<std::endl;
    // column headers
    ss << "\t";
    for (unsigned int j=0; j<this->mCurN; j++) {
        ss << "\t" << j;
    }
    ss << std::endl;
    // rows in adjacency matrix
    for (unsigned int i=0; i<this->mCurN; i++) {
        ss << "\t" << i;
        for (unsigned int j=0; j<this->mCurN; j++) {
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

