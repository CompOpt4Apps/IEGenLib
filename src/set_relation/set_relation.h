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
 * \authors Michelle Strout, and Mahdi Soltan Mohammadi
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
#include "SubMap.h"
class Visitor;

#include <set>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include "isl_str_manipulation.h"
#include "set_relation/set_relation.h"

#include <isl/set.h>   // ISL Sets
#include <isl/map.h>   // ISL Relations

#include "parser/jsoncons/json.hpp"
using jsoncons::json;

namespace iegenlib{

class Set;
class Relation;
class UFCallMap;
namespace parser{
extern Set* parse_set(std::string set_string);
extern Relation* parse_relation(std::string relation_string);
}



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

    //! Given inarity parameter is adopted.
    //! If inarity parameter is outside of feasible range for the existing
    //! existing TupleDecl then throws exception.
    void setInArity(int inarity);

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

    //TODO: Constants were removed so as to allow 
    //for modification of constraints in a conjunction.
    //Put the constants back and add a functionality to 
    //remove constraint.
    std::list<Exp*> &equalities() { return mEqualities; }

    std::list<Exp*> &inequalities() { return mInequalities; }

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
    //! Get pretty-printed string with modifications for Omega parsing
    virtual std::string prettyPrintStringForOmega(bool isRel) const;

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
    Conjunction* IntersectOnInputTuple(const Conjunction* rhs,
                           std::vector<int> lhsShift,
                           std::vector<int> rhsShift, 
                           int resultInArity) const;

    //! Restrict this (interpreted as a Relation) to rhs, which is interpreted
    //! as a set.
    //! Returns a new conjunction which the caller is responsible for
    //! deallocating.
    //! r = { x -> y : C }
    //! s = { z : D }
    //! r \ s  = { x -> y : D[x/z] && C  }
    //! \param rhs
    //! \return
    Conjunction* Restrict (const Conjunction* rhs) const;



    //! Performs transitive closure in presence of UFs
    //! Returns a new conjunction, which the user is responsible
    //  for deallocating.
    Conjunction* TransitiveClosure();
     

    /*! Treating this Conjunction like a domain or range.  Creates
    ** a new set where passed in tuple expression is
    ** bound assuming this domain, or range.
    ** User must deallocate returned Conjunction.
    **
    ** \param tuple_exp Expression tuple to bound. Could just have one elem.
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
    ** Group together all equality expressions that
    ** are parts of the same UFCallTerm, IOW i=f(k)[0] and
    ** j=f(k)[1] should become (i,j) = f(k).
    */
    void groupIndexedUFCalls();

    // Want to use these in Relation::isFunction and other so must be public.
    bool isFunction(int inArity) const;
    bool isFunctionInverse(int inArity) const;

    //! Visitor design pattern, see Visitor.h for usage
    void acceptVisitor(Visitor *v);

    // returns the number of all terms (unique and replicates)
    // inside the conjuction.
    int termCount();
    
    /*!
     * Returns true if term in conjunction is bounded.
     * A term is bounded if it has an upper and lower 
     * bound.
     */
    bool isConstBounded (TupleVarTerm* t);
    
    
    
    /*!
     * Returns a list of expressions denoting the upper bound 
     * of a tuple variable. The expressions in this 
     * list are owned by caller.
     * \param tVar (not adopted)
     */
    std::list<Exp*> GetUpperBounds(TupleVarTerm& tVar);
    
    /*! Returns a list of expressions on the lower bound
     * of a tuple variable tVar. The expressions returned
     * is owned by the caller.
     * \param tVar (not adopted)
     * */
    std::list<Exp*> GetLowerBounds(TupleVarTerm& tVar);


    void setUnsat(){ unsat = true;}
    bool isUnsat(){return unsat;}

    //
    SetRelationshipType setRelationship(Conjunction* rightSide);

private:

    /// Set of equality constraints.
    std::list<Exp*> mEqualities;

    /// Set of inequality constraints.
    std::list<Exp*> mInequalities;

    /// Tuple declaration for this conjunction
    TupleDecl mTupleDecl;

    /// To track how many tuple variables counted in the arity are input
    int mInArity;

    // When we find out that conjunction is unsatisfiable we set this true.
    bool unsat;
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
    //! Human-readable string with adjustments for Omega parsing
    virtual std::string prettyPrintStringForOmega() const
        { return prettyPrintStringForOmega(0); }
    std::string prettyPrintStringForOmega(int aritySplit,
                                          bool isRel = false) const;

    //! Convert to ISL format (substitute in tuple vars and declare symbolics).
    virtual std::string toISLString() const
        { return toISLString(0); }
    std::string toISLString(int aritySplit) const;

    //! Convert to Omega format (substitute in tuple vars and declare
    //! symbolics properly).
    //! Meant to be used on a modified SparseConstraints which fits
    //! Omega's restrictions -- see VisitorChangeUFsForOmega
    virtual std::string toOmegaString(std::set<std::string> ufDecls) const {
        return toOmegaString(0, ufDecls);
    }
    std::string toOmegaString(int aritySplit, std::set<std::string> ufDecls,
                              bool isRel = false) const;

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


    

    /*! Pushes the constants in the tuple declaration into equality
    **  constraints instead.
    */
    void pushConstToConstraints();


    //! Remove duplicate constraints and trivial constraints
    void cleanUp();


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

    //! Is tuple variable tupleID argument to an UFS?
    bool isUFCallParam(int tupleID);

    /*! This function considers tuple variable i; and counts the number of
    **  constraints in the set where this tuple variable is argument to an UFC.
    **  However, it excludes constraints that are in the domainRangeConsts set.
    **  Since, these constraints are related to domain/range of UFCs in the set.
    */
    int numUFCallConstsMustRemove(int i, std::set<Exp>& domainRangeConsts);

    /*! This function removes any constraints where this tuple variable i
    **  is argument to an UFC.
    **  It also removes such constraints from domainRangeConsts set.
    */
    void removeUFCallConsts(int i);

    //! This function is implementation of a heuristic algorithm to remove
    //  expensive contranits from the set.
    void removeExpensiveConstraints(std::set<int> parallelTvs,
                          int mNumConstsToRemove , std::set<Exp> ignore );

    /*! Sometimes to provide arguments of an UFC like sigma(a1, a2, ...)
    **  we use another UFC that is not indexed like left(f). Here, the
    **  expanded form would look like this:
    **                          sigma(left(f)[0], left(f)[1], ...)
    **  indexUFCs() would create the expanded format for normalization purposes.
    */
    void indexUFCs();

    //! This function returns a set of constraints that are in caller but not in A
    std::set<Exp> constraintsDifference(const SparseConstraints* A) const;

    bool isUnsat(){
      for (std::list<Conjunction*>::const_iterator i = mConjunctions.begin();
           i != mConjunctions.end(); i++)  if((*i)->isUnsat()) return true;
      return false;
    }

    //! Return the number of conjunction
    int getNumConjuncts(){ return mConjunctions.size(); }

    /**! This function calculates the algorithmic complexity of a Set/Relation
    **   that is representing a data dependence. Also, it takes into
    **   account the fact that the set is meant for dependency analysis
    **   for partial parallelism. Basically, it calculates the complexity of
    **   efficient inspector that we need to generate for the dependence.
    **   Therefore, it considers two things:
    **   1) It ignores any tuple variable that we can project out, other
    **   than those that we want to parallelize.
    **   2) It takes into account the useful equalities (e.g i = col(jp),
    **   where we can get values of i from col(jp)). Nonetheless, note that
    **   it does not consider any sort of approximations that
    **   we might be able to do to further optimize the inspector.
    **/
    std::string complexityForPartialParallel(std::set<int> parallelTvs);

    /**! SparseConstraints::getZ3form returns a vector of strings that include
    //    constraints represented in SMT-LIB format that SMT solvers like z3
    //    gets as input. This can be used to check the satisfiability of
    //    an IEGenLib Set/Relation with a SMT solver. The returned list
    //    also includes tuple variable declarations, however they do not include UFSymbol
    //    and global variable declarations. This is because, when checking satisfiability of
    //    a set, we usually also want to define some user defined assertions along side original
    //    constraints. The assertions might have UFSymbols and global variables of their
    //    own that original constraints do not. We can put their UFSymbols and globals into
    //    UFSyms, and VarSyms std::set that SparseConstraints::getZ3form returns by reference,
    //    and then a driver function can declare all the UFSymbols and globals at the beginning of
    //    the input file that it is going to generate and pass to a SMT solver.
    **/
    std::vector<std::string> getZ3form(std::set<std::string> &UFSyms,
                  std::set<std::string> &VarSyms, bool termDef = true);


    /*!
     * Returns a list of expressions denoting the upper bound 
     * of a tuple variable. The expressions in this 
     * list are owned by caller.
     * \param tVar (not adopted)
     */
    std::list<Exp*> GetUpperBounds(TupleVarTerm& tVar);
    
    /*! Returns a list of expressions on the lower bound
     * of a tuple variable tVar. The expressions returned
     * is owned by the caller.
     * \param tVar (not adopted)
     * */
    std::list<Exp*> GetLowerBounds(TupleVarTerm& tVar);



// FIXME: what methods should we have to iterate over conjunctions so
// this can go back to protected?
// protected:
    std::list<Conjunction*> mConjunctions;


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
    //! Parses omega string to IEGenLib Set
    //! \param omegaString String to be parsed 
    //! \param ufMap maps prefixed omega UFs back to 
    //               original UFCallTerm in IEGenLib
    static Set* parseOmegaString(std::string omegaString,
		    std::map<std::string,UFCallTerm*> ufMap);
	
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
    bool operator!=( const Set& other) const { return !(*this == other); };
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
    
    //! Computes the convex hull of this set.
    //! Returns a new set for which the caller is responsible
    //! for deallocating.
    Set * Hull();

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

    /*! Adds constraints due to domain and range of all UFCalls in the Set.
    **  Function returns the new Set with added constraints, leaving caller
    **  unchanged. User owns the returned Set object.
    */
    Set* boundDomainRange();

    //! Send through ISL to achieve a canonical form.
    void normalize(bool bdr=true);

    //! Visitor design pattern, see Visitor.h for usage
    void acceptVisitor(Visitor *v);

    /*! Creates a super affine set from a non-affine set.
    **  To do this:
    **    (1) OPTIONAL: We add constraints due to all UFCalls' domain and range
    **    (2) We replace all UFCalls with symbolic constants in the ufc map.
    **  ufcmap: is going to pupulated with UFCs and their eq. symbolic constants
    **          it can be later used to replace back the UFCs with
    **          reverseAffineSubstitution
    **  boundUFCs: for most purposes we probably want to bound domain and range
    **             of UFCalls in a set before turning it into a super affine set.
    **             However, sometimes it might be desiarable not to do that,
    **             e.g in rule instantiation
    **  The function does not own the ufcmap.
    */
    Set* superAffineSet(UFCallMap* ufcmap, bool boundUFCs = true);

    /*! Creates a sub non-affine set from an affine set.
    **  By replacing symbolic constants that are representative of UFCalls
    **  with their respective UFCalls. The function does not own the ufcmap.
    */
    Set* reverseAffineSubstitution(UFCallMap* ufcmap);

    /*!
    * Projects out specified tuple variable if it is not an argument to any UFCall
    * that also uses other tuple variables. tvar is calculated based on arity
    * starting from 0.
    * Consequently, to project out jp from S:
    *   S = {[i,j,k,ip,jp,kp] : ...}
    *   tvar = 4
    *
    * NOTE: if tvar is an argument to some UFCall that also has other tuple
    * variable arguments, then we cannot project it out and function returns NULL.
    *        Ex: if col(i,k) exists in constraints we cannot project out 'k'
    */
    Set* projectOut(int tvar);

    /*! This function simplifies constraints sets of non-affine sets that
        are targeted for level set parallelism. These sets are representative
        of data access dependency relations. For level set parallelism,
        we need to create an optimized inspector code that checks
        data dependency based these constraints in run time. This function is
        implementation of Simplification Algorithm that simplifies dependency
        relations, so we can generate optimized inspector code from constraint sets.
    */
    Set* simplifyForPartialParallel(std::set<int> parallelTvs);

    // See if this set is deault and uninitialized
    bool isDefault(){
        Set defSet( arity() );
        if( *this == defSet ){
            return true;
        } else {
            return false;
        }
    }
    //! Performs transitive closure in presence of UFs
    //! Returns a new set, which the user is responsible
    //  for deallocating.
    Set* TransitiveClosure();


    //! Gets the domain of an uninterpreted function 
    //! in a set
    //! \param ufName name of the UF
    //! Returns a new set representing the domain of the UF
    //! which the user is responsible
    //! for deallocating.
    Set* GetDomain(std::string ufName);
    
    int getArity(){ return mArity;}

    Set* detectUnsatOrFindEqualities(bool *useRule=NULL);

    string getString(bool generic=false);
    //! permutes the tuple variables into an order that is valid
    //! for codegen.
    //! \returns new Set
    Set* permuteForCodeGen();

    //
    SetRelationshipType setRelationship(Set* rightSide);

    //
    void reOrdTV_OmegaCodeGen(std::set<int> parallelTvs);
    //
    void removeUPs();

private:
    int mArity;
    // give VisitorChangeUFsForOmega access to 
    // mArity
    friend class VisitorChangeUFsForOmega;


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

    //! Not-equals operator
    bool operator!=( const Relation& other) const { return !(*this == other); };

    //! Less than operator.
    bool operator<( const Relation& other) const;

    ~Relation();

    //! Convert to a human-readable string.
    std::string toString() const;

    //! Convert to a human-readable string (substitute in tuple vars).
    std::string prettyPrintString() const;
    //! Pretty print with modifications for Omega parsing
    std::string prettyPrintStringForOmega() const;
    //! Convert to Omega format (substitute in tuple vars and declare
    //! symbolics properly).
    std::string toOmegaString(std::set<std::string>) const;

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
    Relation *IntersectOnInputTuple(const Relation* rhs) const;

    /*!
    ** Restrict a set with a relation. Returns a new
    ** Relation which the caller is responsible for
    ** deallocating
    **  r = { x -> y : C }
    ** s = { z : D }
    **  r \ s  = { x -> y : D[z/x] && C  }
    **
    ** @param rhs
    ** @return
    */

    Relation * Restrict (const Set* rhs) const;

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

    /*! Adds constraints due to domain and range of all UFCalls in the
    **  Relation. Function returns the new Relation with added constraints,
    **  leaving caller unchanged. User owns the returned Relation object.
    */
    Relation* boundDomainRange();

    /*! For UFs declared as having a Monotonicity value (see
    **  MonotonicType in UninterFunc.h) constraints will be
    **  added to parameter expressions as needed.
    **  For example, if we find that f(e1)<f(e2) and f is monotonically
    **  non-decreasing, then we will add the constraint that e1<e2.
    **
    ** \return Relation will contain new constraints and is owned by caller
    */
    Relation* addConstraintsDueToMonotonicity() const;

    //! Send through ISL to achieve a canonical form.
    void normalize(bool bdr=true);

    //! Visitor design pattern, see Visitor.h for usage
    void acceptVisitor(Visitor *v);

    /*! Creates a super affine Relation from a non-affine Relation.
    **  To do this:
    **    (1) OPTIONAL: We add constraints due to all UFCalls' domain and range
    **    (2) We replace all UFCalls with symbolic constants in the ufc map.
    **  ufcmap: is going to pupulated with UFCs and their eq. symbolic constants
    **          it can be later used to replace back the UFCs with
    **          reverseAffineSubstitution
    **  boundUFCs: for most purposes we probably want to bound domain and range
    **             of UFCalls in a set before turning it into a super affine set.
    **             However, sometimes it might be desiarable not to do that,
    **             e.g in rule instantiation
    **  The function does not own the ufcmap.
    */
    Relation* superAffineRelation(UFCallMap* ufcmap, bool boundUFCs = true);
    Relation* superAffineRelation();
    /*! Creates a sub non-affine set from an affine Relation.
    **  By replacing symbolic constants that are representative of UFCalls
    **  with their respective UFCalls.
    **  The function does not own the ufcmap.  Caller must cleanup returned
    **  Relation.
    */
    Relation* reverseAffineSubstitution(UFCallMap* ufcmap);

    //! Same as Set::projectOut
    Relation* projectOut(int tvar);

    /*! This function simplifies constraints sets of non-affine sets that
        are targeted for level set parallelism. These sets are representative
        of data access dependency relations. For level set parallelism,
        we need to create an optimized inspector code that checks
        data dependency based these constraints in run time. This function is
        implementation of Simplification Algorithm that simplifies dependency
        relations, so we can generate optimized inspector code from constraint sets.
    */
    Relation* simplifyForPartialParallel(std::set<int> parallelTvs);

    // See if this set is deault and uninitialized
    bool isDefault(){
        Relation defRel( inArity() , outArity() );
        if( *this == defRel ){
            return true;
        } else {
            return false;
        }
    }

    //
    Relation* detectUnsatOrFindEqualities(bool *useRule=NULL);

    //
    string getString(bool generic=false);

    //
    SetRelationshipType setRelationship(Relation* rightSide);

    //
    SetRelationshipType dataDependenceRelationship(Relation* rightSide, int parallelLoopLevel=0);

    // Returns a list of constraints directly
    // involving output tuple variables and attempts to solve for these
    // variables. Deallocating expressions is required by the caller.
    std::list<Exp*> solveForOutputTuple();

    //! Returns true if expression is part of an inverse family.
    bool hasInverseFamily(Exp* expr);

    //! Returns a list of constraints in the inverse family of
    //! of exp. Inverse family is a concept used in synthesis
    //! as it provides an inverse for an uninvertible function
    //! by using the charactersitics of the mapping it belongs to.
    std::list<Exp*> getInverseFamily(Exp* exp);

    //! Performs transitive closure in presence of UFs
    //! Returns a new relation, which the user is responsible
    //  for deallocating.
    Relation* TransitiveClosure();
    
    //! Gets the domain of an uninterpreted function 
    //! in a set
    //! \param ufName name of the UF
    //! Returns a new set representing the domain of the UF
    //! which the user is responsible
    //! for deallocating.
    Set* GetDomain(std::string ufName);

    //! Creates a set from a relation. Appends output tuple 
    //! to input tuple.
    //! {[x,y]->[z]: <constraints>}
    //! {[x,y,z]: <constriants>}
    Set* ToSet();

    
    //! Computes the convex hull of this relation.
    //! Returns a new relation for which the caller is responsible
    //! for deallocating.
    Relation * Hull();




    //! Parses omega string to IEGenLib Relation
    //! \param omegaString String to be parsed 
    //! \param ufMap maps prefixed omega UFs back to 
    //               original UFCallTerm in IEGenLib
    static Relation* parseOmegaString(std::string omegaString,
		    std::map<std::string,UFCallTerm*> ufMap);
    
    StringIterator* getSymbolIterator() const;

private:
    int mInArity;
    int mOutArity;
};


string passSetStrThruISL(string sstr);
string passUnionSetStrThruISL(string sstr);
string passRelationStrThruISL(string rstr);
string passUnionRelationStrThruISL(string rstr);
Set* passSetThruISL(Set* s);
Relation* passRelationThruISL(Relation* r);
std::pair <std::string,std::string> instantiate(
          UniQuantRule* uqRule, Exp x1, Exp x2,
          UFCallMap *ufcmap, TupleDecl origTupleDecl);
std::set<std::pair <std::string,std::string>> ruleInstantiation
                          (std::set<Exp> instExps, bool *useRule,
                           TupleDecl origTupleDecl, UFCallMap *ufcmap);
Set* islSetProjectOut(Set* s, unsigned pos);

Relation* islRelTransitiveClosure(Relation*r,bool& isExact);

//TODO: Make this a paart of IEGenLib and also write documentation
Set* islSetComplement (Set* s);

Relation* islRelComplement(Relation *r);

}//end namespace iegenlib

#endif /* SET_RELATION_H_ */
