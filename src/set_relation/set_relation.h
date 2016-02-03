/*!
 * \file set_relation.h
 *
 * \brief Declarations for the set and relation classes and classes
 *        they contain except for the Exp and Term classes.
 *
 * The Set and Relation classes represent integer tuple sets
 * and relations with sets of conjunctions that contain
 * sets of affine inequality and equality constraints. The constraints
 * can include uninterpreted function symbol terms.
 *
 * \date Started: 3/28/12
 *
 * \authors Michelle Strout
 *
 * Copyright (c) 2012, Colorado State University <br>
 * Copyright (c) 2015, University of Arizona <br>
 * All rights reserved. <br>
 * See ../../COPYING for details. <br>
 */

#ifndef SET_RELATION_H_
#define SET_RELATION_H_

#include <util/util.h>
#include "expression.h"
#include "UFCallMapAndBounds.h"
#include "SubMap.h"
#include "UFCallMap.h"
class Visitor;

#include <set>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <map>

#include <isl/set.h>   // ISL Sets
#include <isl/map.h>   // ISL Relations

namespace iegenlib{

class Set;
class Relation;

namespace parser{
extern Set* parse_set(std::string set_string);
extern Relation* parse_relation(std::string relation_string);
}

/*!
 * This function runs the Relation string through ISL using isl_map
 * and returns the resulting string.  More normalization possible.
 */
//std::string getRelationStringFromISL(std::string rstr);

/*!
 * This function runs the Relation string through ISL using isl_basic_mao
 * and returns the resulting string.  Less normalization possible.
 */
//std::string getRelationStringFromBasicISL(std::string rstr);


/*!
 * \class Conjunction
 * \brief Class containing sets of all the equalities and inequalities
 *
 * This class holds two sets: one for the equalities and one for the
 * inequalities.
 */
class Conjunction {
public:

    Conjunction(int arity);
    Conjunction(TupleDecl tdecl);
    Conjunction(int arity, int inarity);
    Conjunction(const Conjunction& other);
    Conjunction& operator=(const Conjunction& other);
    void reset();
    virtual ~Conjunction();
    Conjunction* clone() const;

    //! Comparison operator -- lexicographic order
    bool operator<(const Conjunction& other) const;

/*
	//! Given inarity parameter is adopted.
	//! If inarity parameter is outside of feasible range for the existing
	//! existing TupleDecl then throws exception.
    void setInArity(int inarity);
*/
    
    //! Given tuple declaration parameter is adopted.
    //! If there are some constants that don't agree then throws exception.
    //! If replacing a constant with a variable ignores the substitution
    //! in that conjunction.
    void setTupleDecl( TupleDecl tuple_decl );
    //! Returns a copy of its tuple declaration.
    TupleDecl getTupleDecl(  ) const;
    //! Below concatenates the two tuple declarations into one.
    //! If there are some constants that don't agree then throws exception.
    //! If replacing a constant with a variable ignores the substitution
    //! in that conjunction.
    void setTupleDecl( TupleDecl tuple_decl_in, TupleDecl tuple_decl_out );

    /*! addEquality -- add the given expression, interpreted as an
    ** equality (Exp = 0), to our set of equalities.
    ** \param equality (adopted)
    */
    void addEquality(Exp* equality);

    /*! addInequality -- add the given expression, interpreted as an
    ** inequality (Exp >= 0), to our set of inequalities.
    ** \param inequality (adopted)
    */
    void addInequality(Exp* inequality);

    const std::list<Exp*> &equalities() const { return mEqualities; }

    const std::list<Exp*> &inequalities() const { return mInequalities; }

    /*! substituteTupleDecl -- substitute TupleVarTerms in for any
    **  VarTerms in the expressions whose names match the corresponding
    **  tuple element declaration.
    */
    void substituteTupleDecl();

    /*! copyConstraintsFrom -- copy all the equalities and inequalities
    **  from source, and add them to our own constraints.
    */
    void copyConstraintsFrom(const Conjunction *source);

    /*! Substitute each factor (i.e. the non-coefficient
    ** part of a term) with the expression mapped to that factor 
    ** in all our equalities and inequalities.
    ** \param searchTermToSubExp (none of the Term* or Exp* are adopted)
    */
    void substituteInConstraints(SubMap& searchTermToSubExp);

    //! Convert to a human-readable string.
    virtual std::string toString() const;

    //! Convert to a human-readable string, pretty printed.
    virtual std::string prettyPrintString() const;

    //! Convert to a DOT string.
    //! Pass in the parent node id and the next node id.
    //! The next node id will be set upon exit from this routine.
    //! If no parent id is given then will not draw edge from parent to self.
    virtual std::string toDotString(int & next_id) const;
    virtual std::string toDotString(int parent_id, int & next_id) const;

    //! Get an iterator over the symbolic constants, or non-tuple vars
    // FIXME: rename this getParamIterator due to IEGRTWO-79
    StringIterator* getSymbolIterator() const;

    //! Get an iterator over the tuple variables, in order.
    StringIterator* getTupleIterator() const;

    //! Get our arity.
    int arity() const { return mTupleDecl.size(); }
    //! Get/Set inarity, for use with relations
    int inarity() const { return mInArity; }
    void setinarity(int in) { mInArity = in; }
    
    //! Returns true if the conjunction has at least one equality or inequality
    //! constraints.  If it contains none then this Conjunction is just
    //! representing TRUE.
    bool hasConstraints() const;

    /*! Search among our equality constraints for one that 
    **  defines tupleLocToFind
    **  as a function of only tuple variables in the location range
    **  [startTupleRange, endTupleRange].  Return a new copy of that expression.
    */
    Exp *findFunction(int tupleLocToFind, int startTupleRange, 
                      int endTupleRange) const;

    //! Same as findFunction except that the equality the function is
    //! derived from is removed.
    Exp *findAndRemoveFunction(int tupleLocToFind, int startTupleRange,
                               int endTupleRange);

    //! Compose with another conjunction, given innerArity (which
    //! is this's in arity and rhs's out arity).
    //! \param rhs (not adopted)
    //! \param innerArity
    Conjunction *Compose(const Conjunction *rhs, int innerArity) const;

    //! Apply this (interpreted as a Relation) to rhs, which is interpreted
    //! as a set.
    //! r = { x -> y : x = G(y) && C }
    //! s = { z : D }
    //! r(s) = { y : D[z/G(y)] && C[x/G(y)] }
    //! \param rhs (not adopted)
    Conjunction *Apply(const Conjunction *rhs) const;

    //! Compute Inverse of this conjunction.  Interpreted as a Relation.
    Conjunction *Inverse() const;

    /*! Intersect this conjunction with the given other one
    **    (i.e., this conjunction rhs).  Returns a new Conjunction,
    **    which the caller is responsible for deallocating.
    ** \param rhs (not adopted)
    */
    Conjunction* Intersect(const Conjunction* rhs) const;

    /*! Treating this Conjunction like a domain or range.  Creates
    ** a new set where passed in tuple expression is
    ** bound assuming this domain, or range.
    ** User must deallocate returned Conjunction.
    ** 
    ** \param tuple_exp Expression tuple to bound.  Could just have one elem.
    ** 
    ** \return Conjunction will contain all bounds on expressions 
    **         in tuple expression.  Will have no tuple variables.
    */
    Conjunction* boundTupleExp(const TupleExpTerm& tuple_exp) const;

    //! Return true if the constraints in the conjunction are satisfiable.
    bool satisfiable() const;

    /*! Pushes the constants in the tuple declaration into equality
    **  constraints instead.
    */
    void pushConstToConstraints();

    /*! Pushes constants in constraints into the tuple declaration.
    */
    void pushConstConstraintsToTupleDecl();

    /*! Find any TupleVarTerms in this Conjunction (and subexpressions)
    **  and remap the locations according to the oldToNewLocs vector,
    **  where oldToNewLocs[i] = j means that old location i becomes new
    **  location j (i.e. __tvi -> __tvj).  Throws an exception if an
    **  old location is out of range for the given oldToNewLocs.
    */
    void remapTupleVars(const std::vector<int>& oldToNewLocs);

    /*! Cleans up constraints in the conjunction.
    **  - Removes equality and inequality constraints that are equal to zero
    **  from list of Expressions.
    **  - Also removes duplicate constraints.  
    **  
    **  (FIXME: with normalize will either of these be necessary?)
    **
    */
    void cleanUp();
    
    /*!
    ** (step 0) Group together all equality expressions that 
    ** are parts of the same UFCallTerm, IOW i=f(k)[0] and 
    ** j=f(k)[1] should become (i,j) = f(k).
    */
    void groupIndexedUFCalls();

    /*! Create a conjunction that represents an affine
    **  superset of the "this" Conjunction.  Will be replacing
    **  uf calls with temporary variables recursively bottom up and 
    **  will be bounding parameter expressions and expressions equivalent
    **  to UF call return values based on the domain and range
    **  declarations for the UF call.
    **  The returned conjunction will need to be deleted by caller.
    */
    //Set* createAffineSuperSet(UFCallMapAndBounds & ufcallmap);
    void ufCallsToTempVars(UFCallMapAndBounds & ufcallmap);
    
    /*! Returns a normalized Set that represents this Conjunct.
    **  It has to be a set because bounds on parameters and UF calls
    **  can introduce disjunctions.
    **  See SparseConstraints::normalize() for algorithm overview.
    */
    Set* normalize() const;
    Set* normalizeR() const;  // BK:  was trying this out ... may not stay

    // Want to use these in Relation::isFunction and other so must be public.
    bool isFunction(int inArity) const;
    bool isFunctionInverse(int inArity) const;

    //! Visitor design pattern, see Visitor.h for usage
    void acceptVisitor(Visitor *v);

private:

    /// Set of equality constraints.
    std::list<Exp*> mEqualities;

    /// Set of inequality constraints.
    std::list<Exp*> mInequalities;

    /// Tuple declaration for this conjunction
    TupleDecl mTupleDecl;

    /// To track how many tuple variables counted in the arity are input
    int mInArity;

};

/*!
 * \class SparseConstraints
 *
 * \brief Base class that contains the conjunctions and a pointer to an environment.
 *
 * The purpose of this class, at the moment, is mostly as a base class and
 * to be extended by set and relation, both of which will have their own special
 * behaviors dealing with the arity.
 */
class SparseConstraints {
public:
    SparseConstraints();
    SparseConstraints(const SparseConstraints& other);
    virtual SparseConstraints& operator=(const SparseConstraints& other);

    void reset();
    virtual ~SparseConstraints();

    //! Less than operator.
    virtual bool operator<( const SparseConstraints& other) const;

    //! For all conjunctions, sets them to the given tuple declaration.
    //! If there are some constants that don't agree then throws exception.
    //! If replacing a constant with a variable ignores the substitution
    //! in that conjunction.
    virtual void setTupleDecl( TupleDecl tuple_decl );
    //! Returns a copy of the first conjunction's tuple declaration.
    TupleDecl getTupleDecl(  ) const;
    //! For all conjunctions, sets them to the given tuple declarations.
    //! If there are some constants that don't agree then throws exception.
    //! If replacing a constant with a variable ignores the substitution
    //! in that conjunction.
    void setTupleDecl( TupleDecl tuple_decl_in, TupleDecl tuple_decl_out );

    //! addConjunction
    //! \param adoptedconjuction (adopted)
    virtual void addConjunction(Conjunction *adoptedConjunction);
    
    //! Get an iterator to the first conjunction we contain.
    std::list<Conjunction*>::const_iterator conjunctionBegin() const {
    	return mConjunctions.begin();
    }

    //! Get an iterator pointing past the last conjunction we contain.
    std::list<Conjunction*>::const_iterator conjunctionEnd() const {
    	return mConjunctions.end();
    }

    //! Get our total arity, IOW number of tuple elements.
    //! Should be overridden in subclasses.
    virtual int arity() const { return 0; }

    //! Get an iterator over the tuple variables, in order.
    //! Caller in charge of deleting returned StringIterator.
    StringIterator* getTupleIterator() const;

    //! Convert to a human-readable string.
    //! Still need arity split here because works for Sets and Relations.
    virtual std::string toString() const { return toString(0); }
    std::string toString(int aritySplit) const;

    //! Convert to human-readable format (substitute in tuple vars).
    virtual std::string prettyPrintString() const
        { return prettyPrintString(0); }
    std::string prettyPrintString(int aritySplit) const;

    //! Convert to ISL format (substitute in tuple vars and declare symbolics).
    virtual std::string toISLString() const
        { return toISLString(0); }
    std::string toISLString(int aritySplit) const;

    //! Create a graph for visualization with graphviz
    virtual std::string toDotString() const;

    /*! Substitute each factor (i.e. the non-coefficient
    ** part of a term) with the expression mapped to that factor 
    ** in all our equalities and inequalities.
    ** None of the Term's in the map can be constant or an exception will
    ** be thrown.
    ** After this substitution the Set or Relation may not be normalized,
    ** but it is cleaned up.  What does that mean?
    ** \param searchTermToSubExp (none of the Term* or Exp* are adopted)
    */
    void 
    substituteInConstraints(SubMap& searchTermToSubExp);

    //! Remove duplicate constraints and trivial constraints
    void cleanUp();
    
    /*! Barbara's algorithm: bottom up convert all UF calls to 
    **  affine constraints on the argument expressions and expressions
    **  equivalent to the output.  Send these constraints to ISL
    **  for normalization with row reduction form for equalities and hopefully
    **  a convergent form for inequalities.  Then substitute the UF calls
    **  back in for the temporary variables.  
    **  See normalize_test.cc for examples.
    */
    virtual void normalize();

    /*! Find any TupleVarTerms in this expression (and subexpressions)
    **  and remap the locations according to the oldToNewLocs vector,
    **  where oldToNewLocs[i] = j means that old location i becomes new
    **  location j (i.e. __tvi -> __tvj).  Throws an exception if an
    **  old location is out of the domain for the given oldToNewLocs.
    **  The new location will be -1 for old locations that are not being
    **  remapped.  For example some might be constants in the TupleDecl.
    */
    void remapTupleVars(const std::vector<int>& oldToNewLocs);

    //! Visitor design pattern, see Visitor.h for usage
    void acceptVisitor(Visitor *v);

    // Is tuple variable tupleID argument to an UFS?
    bool isUFCallParam(int tupleID);

    // The function traverses all conjunctions to find UFcalls.
    // For every distinct UGCall, it creates a equ. symbolic constant (string)
    // and stores the (UFC, Sym) pair in a UFCallMap. The function returns
    // a pointer to final UFCallMap that the user is responsible for deleting.
    UFCallMap* mapUFCtoSym();

// FIXME: what methods should we have to iterate over conjunctions so
// this can go back to protected?
//protected:
    std::list<Conjunction*> mConjunctions;


  protected:
    // FIXME: can't we incorporate these into visitor?
    void addUFConstraintsHelper(std::string uf1str, 
                                std::string opstr, std::string uf2str);

    void addConstraintsDueToMonotonicityHelper();
};

/*!
 * \class Set
 *
 * \brief A SparseConstraints class that represents a Set
 *
 * This class has one arity related to it.
 *
 * Representation example: {[x]:x < 100}
 */
class Set: public SparseConstraints {
public:
    //! Parses the string to construct Set, assuming omega or ISL syntax.
    Set(std::string str);

    //! Creates a set with the specified arity.  It starts with no constraints
    //! so all tuples of that arity belong in it.
    Set(int arity);

    //! Creates a set with the specified tuple declaration.  
    //! It starts with no constraints so all tuples of that arity belong in it.
    Set(TupleDecl tdecl);

    //! Copy constructor
    Set(const Set& other);
    
    ~Set();

    Set& operator=(const Set& other);

    bool operator==( const Set& other) const;
    //! Less than operator.
    bool operator<( const Set& other) const;
    
    std::string toDotString() const;

    //! For all conjunctions, sets them to the given tuple declaration.
    //! If there are some constants that don't agree then throws exception.
    //! If replacing a constant with a variable ignores the substitution
    //! in that conjunction.
    //! Also modifies arity to match.
    void setTupleDecl( TupleDecl tuple_decl );

    //! Get our total arity, IOW number of tuple elements.
    int arity() const { return mArity; }

    /*! Union this set with the given other one
    **    (i.e., this Union rhs).  Returns a new Set,
    **    which the caller is responsible for deallocating.
    ** \param rhs (not adopted)
    */
    Set *Union(const Set* rhs) const;

    /*! Intersect this set with the given other one
    **    (i.e., this Intersect rhs).  Returns a new Set,
    **    which the caller is responsible for deallocating.
    ** \param rhs (not adopted)
    */
    Set *Intersect(const Set* rhs) const;
      
    /*! Treating this Set like a domain or range.  Creates
    ** a new set where passed in tuple expression is
    ** bound assuming this domain, or range.
    ** User must deallocate returned Set.
    ** 
    ** \param tuple_exp Expression tuple to bound.  Could just have one elem.
    **
    ** \return Set will contain all bounds on expressions in tuple expression.
    */
    Set* boundTupleExp(const TupleExpTerm& tuple_exp) const; 
    
    /*! Will create constraints uf1str(e) opstr uf2str(e) for all
    **  actual parameters that occur for those UFs. 
    ** See SparseConstraints::addUFConstraintsHelper for more docs.
    **
    ** \param uf1str name of first uninterpreted function
    ** \param opstr  operator that describes relationship between UFs
    ** \param uf2str name of second uninterpreted function.
    **
    ** \return Set will contain new constraints and will be owned by caller
    */
    Set* addUFConstraints(std::string uf1str, 
                          std::string opstr, std::string uf2str) const;
    
    /*! For UFs declared as having a Monotonicity value (see 
    **  MonotonicType in UninterFunc.h) constraints will be
    **  added to parameter expressions as needed.
    **  For example, if we find that f(e1)<f(e2) and f is monotonically
    **  non-decreasing, then we will add the constraint that e1<e2.
    **
    ** \return Set will contain new constraints and will be owned by caller
    */
    Set* addConstraintsDueToMonotonicity() const;

    
    //void normalize();
    
    //! Visitor design pattern, see Visitor.h for usage
    void acceptVisitor(Visitor *v);    

    //  Projects out tuple varrable No. tvar
    Set* projectOut(int tvar);

private:
    int mArity;
};

/*!
 * \class Relation
 *
 * \brief A SparseConstraints class that represents a Relation
 *
 * This class has two arities related to it to indicate the in arity
 * and the out arity.
 *
 * Representation example: {[x]->[y]:x < 100 and y > 0}
 */
class Relation: public SparseConstraints {
public:
    Relation(std::string str);
    Relation(int inArity, int outArity);
    Relation(const Relation& other);
    Relation& operator=(const Relation& other);
    Relation& operator=(const Set& other);
    
    //! Equals operator
    bool operator==( const Relation& other) const;
    
    //! Less than operator.
    bool operator<( const Relation& other) const;
    
    ~Relation();

    //! Convert to a human-readable string.
    std::string toString() const;

    //! Convert to a human-readable string (substitute in tuple vars).
    std::string prettyPrintString() const;

    //! Get/Set our in/out arity.
    int inArity() const { return mInArity; }
    int outArity() const { return mOutArity; }
    void SetinArity(int in) { mInArity = in; }
    void SetoutArity(int out) { mOutArity = out; }

    //! For all conjunctions, sets them to the given tuple declaration.
    //! If there are some constants that don't agree then throws exception.
    //! If replacing a constant with a variable ignores the substitution
    //! in that conjunction.
    //! Also modifies arity to match.
    //! FIXME: MMS 10/21/15, why do we need this?
    void setTupleDecl( TupleDecl tuple_decl );

    //! Get our total arity (in plus out).
    int arity() const { return mInArity + mOutArity; }

    //! Create a string for use with the GraphViz tool dot so we
    //! can visualize the Relation data structure.
    std::string toDotString() const;

    /*! Compose this relation with the given other one
    **    (i.e., this Compose rhs).  Returns a new Relation,
    **    which the caller is responsible for deallocating.
    ** \param rhs (not adopted)
    */
    Relation *Compose(const Relation* rhs) const;

    /*! Apply this relation to the given set. Returns a new Set,
    **  which the caller is responsible for deallocating.
    */
    Set *Apply(const Set* rhs) const;

    /*! Union this relation with the given other one
    **    (i.e., the Union rhs).  Returns a new Relation,
    **    which the caller is responsible for deallocating.
    ** \param rhs (not adopted)
    */
    Relation *Union(const Relation* rhs) const;

    /*! Intersect this relation with the given other one
    **    (i.e., the Intersect rhs).  Returns a new Relation,
    **    which the caller is responsible for deallocating.
    ** \param rhs (not adopted)
    */
    Relation *Intersect(const Relation* rhs) const;

    /*! Create the inverse of this relation. Returns a new Relation,
    **    which the caller is responsible for deallocating.
    */
    Relation *Inverse() const;

    /*! Determine whether all of the outputs can be determined as
    **  functions of the inputs and/or vice versa.
    */
    bool isFunction() const;
    bool isFunctionInverse() const;

    /*! Return the expression that describes the value of the tupleLoc
    *   specified as a function of the tuple locations in the start
    *   through the stop locations.
    */
    Exp* findFunction(int tupleLocToFind,
                      int startTupleRange, int endTupleRange);

    //! addConjunction that checks the Conjunction and Relation arities match
    //! \param adoptedconjuction (adopted)
    void addConjunction(Conjunction *adoptedConjunction);
    
    /*! Will create constraints uf1str(e) opstr uf2str(e) for all
    **  actual parameters that occur for those UFs. 
    ** See SparseConstraints::addUFConstraintsHelper for more docs.
    **
    ** \param uf1str name of first uninterpreted function
    ** \param opstr  operator that describes relationship between UFs
    ** \param uf2str name of second uninterpreted function.
    **
    ** \return Relation will contain new constraints and is owned by caller
    */
    Relation* addUFConstraints(std::string uf1str, 
                               std::string opstr, std::string uf2str) const;
    
    /*! For UFs declared as having a Monotonicity value (see 
    **  MonotonicType in UninterFunc.h) constraints will be
    **  added to parameter expressions as needed.
    **  For example, if we find that f(e1)<f(e2) and f is monotonically
    **  non-decreasing, then we will add the constraint that e1<e2.
    **
    ** \return Relation will contain new constraints and is owned by caller
    */
    Relation* addConstraintsDueToMonotonicity() const;    

    // Iterate over all conjunctions and normalize each conjunction.
    // Then call cleanup to resort things?
    // BK: testing this out as a possible solution
    //     Not ready for primetime yet
    void normalize();

    //! Visitor design pattern, see Visitor.h for usage
    void acceptVisitor(Visitor *v);

    // Projects out tuple varrable No. tvar
    Relation* projectOut(int tvar);

private:

    int mInArity;
    int mOutArity;
};

}//end namespace iegenlib

#endif /* SET_RELATION_H_ */
