/*!
 * \file set_relation.cc
 *
 * \brief Implementations of the Set and Relation classes.
 *
 * \date Started: 3/28/12
 * # $Revision:: 805                $: last committed revision
 * # $Date:: 2013-09-09 03:27:10 -0#$: date of last committed revision
 * # $Author:: mstrout              $: author of last committed revision
 *
 * \authors Michelle Strout and Joe Strout
 *
 * Copyright (c) 2012, Colorado State University <br>
 * All rights reserved. <br>
 * See ../../COPYING for details. <br>
 */

#include "set_relation.h"

namespace iegenlib{

/************************ ISL helper routines ****************************/


// runs the Relation through ISL and returns the resulting string
string getRelationStringFromISL(string rstr) {

    // Sending r thru ISL and returning resulting string
    
    // Get an isl context
    isl_ctx *ctx;
    ctx = isl_ctx_alloc();
    
    // Get an isl printer and associate to an isl context
    isl_printer * ip = NULL;
    ip = isl_printer_to_str(ctx);
    
    // load Relation r into ISL map
    isl_map* imap = NULL;
    imap = isl_map_read_from_str(ctx, rstr.c_str());
    
    // get string back from ISL map
    char * cstr;
    isl_printer_set_output_format(ip , ISL_FORMAT_ISL);
    isl_printer_print_map(ip ,imap);
    cstr=isl_printer_get_str(ip);
    std::string stringFromISL = cstr;
    
    // clean-up
    isl_printer_flush(ip);
    isl_printer_free(ip);
    free(cstr);
    isl_map_free(imap);
    imap= NULL;
    isl_ctx_free(ctx);  

	return stringFromISL;
}

// runs the Relation through ISL using isl_basic_map 
// and returns the resulting string
std::string getRelationStringFromBasicISL(std::string rstr) {

    // Sending r thru ISL and returning resulting string
    
    // Get an isl context
    isl_ctx *ctx;
    ctx = isl_ctx_alloc();
    
    // Get an isl printer and associate to an isl context
    isl_printer * ip = NULL;
    ip = isl_printer_to_str(ctx);
    
    // load Relation r into ISL map
    isl_basic_map* imap = NULL;
    imap = isl_basic_map_read_from_str(ctx, rstr.c_str());
    
    // get string back from ISL map
    char * cstr;
    isl_printer_set_output_format(ip , ISL_FORMAT_ISL);
    isl_printer_print_basic_map(ip ,imap);
    cstr=isl_printer_get_str(ip);
    std::string stringFromISL = cstr;
    
    // clean-up
    isl_printer_flush(ip);
    isl_printer_free(ip);
    free(cstr);
    isl_basic_map_free(imap);
    imap= NULL;
    isl_ctx_free(ctx);  

	return stringFromISL;
}

#pragma mark -
/****************************** Conjunction *********************************/

Conjunction::Conjunction(int arity) : mTupleDecl(arity), mInArity(0)
{
}

Conjunction::Conjunction(TupleDecl tdecl) : mTupleDecl(tdecl), mInArity(0) {
}


Conjunction::Conjunction(int arity, int inarity)
    : mTupleDecl(arity), mInArity(inarity){
}

Conjunction::Conjunction(const Conjunction& other) {
    *this = other;
}

Conjunction& Conjunction::operator=(const Conjunction& other) {
    reset();

    for (std::list<Exp*>::const_iterator i=other.mEqualities.begin();
                i != other.mEqualities.end(); i++) {
        mEqualities.push_back((*i)->clone());
    }

    for (std::list<Exp*>::const_iterator i=other.mInequalities.begin();
                i != other.mInequalities.end(); i++) {
        mInequalities.push_back((*i)->clone());
    }

    mTupleDecl = other.mTupleDecl;
    mInArity = other.mInArity;
    return *this;
}

void Conjunction::reset() {
    for (std::list<Exp*>::iterator i=mEqualities.begin();
                i != mEqualities.end(); i++) {
        delete (*i);
    }
    mEqualities.clear();

    for (std::list<Exp*>::iterator i=mInequalities.begin();
                i != mInequalities.end(); i++) {
        delete (*i);
    }
    mInequalities.clear();
}

Conjunction::~Conjunction() {
    reset();
}

//! Create a copy of this Conjuntion (of the same subclass)
Conjunction* Conjunction::clone() const {
    return new Conjunction(*this);
}

/*! Less than operator.
**      Compare two Conjunction in the following order:
**          1. by tuple declaration: using TupleDecl::operator<(TupleDecl)
**          2. by number of equalities: number of terms in mEqualities
**          3. by number of inequalities: number of terms in mInequalities
**          4. by sorted equalities list values: using Exp::operator<(Exp)
**          5. by sorted inequalities list values: using Exp::operator<(Exp)
**      This operator assumes that the list of equalities (mEqualities) and
**          inequalities (mInequalities) is sorted for both Conjunction being
**          compared.
** @param other, object to be compared
*/
bool Conjunction::operator<( const Conjunction& other) const {
    // 1. compare tuple declarations
    if (mTupleDecl < other.mTupleDecl) { return true; }
    if (other.mTupleDecl < mTupleDecl) { return false; }

    // 2. compare equality sizes
    if (mEqualities.size() < other.mEqualities.size()) { return true; }
    if (other.mEqualities.size() < mEqualities.size()) { return false; }

    // 3. compare inequality sizes
    if (mInequalities.size() < other.mInequalities.size()) { return true; }
    if (other.mInequalities.size() < mInequalities.size()) { return false; }

    // 4. compare sorted equalities lists
    std::list<Exp*>::const_iterator thisIter;
    std::list<Exp*>::const_iterator otherIter;
    otherIter = other.mEqualities.begin();
    thisIter = mEqualities.begin();
    while (thisIter != mEqualities.end()) {
        //compare values
        if (**thisIter < **otherIter) { return true; }
        if (**otherIter < **thisIter) { return false; }
        //our mEqualities[i]'s are equal, continue
        ++thisIter;
        ++otherIter;
    }

    // 5. compare sorted inequalities lists
    otherIter = other.mInequalities.begin();
    for (thisIter = mInequalities.begin();
            thisIter != mInequalities.end(); ++thisIter) {
        //compare values
        if (**thisIter < **otherIter) { return true; }
        if (**otherIter < **thisIter) { return false; }
        //our mInequalities[i]'s are equal, continue
        ++otherIter;
    }

    // All our elements are equal, so we are not less than other
    return false;

}

/*
//! Given inarity parameter is adopted.
//! If inarity parameter is outside of feasible range for the existing
//! existing TupleDecl then throws exception.
void Conjunction::setInArity(int inarity) {
 	if (inarity < 0 || inarity > (mTupleDecl.size()-1) ) {
        throw assert_exception("Conjunction::setInArity: impossible arity match");
	}
	mInArity = inarity;
}
*/

//! If there are some constants that don't agree then throws exception.
//! If replacing a constant with a variable ignores the substitution
//! for that tuple variable in that conjunction to keep constant.
void Conjunction::setTupleDecl( TupleDecl tuple_decl ) {
    // Check that the given tuple_decl is at least as large as the
    // current mTupleDecl.
    /* FIXME: apparently do use this somewhere.
    if (tuple_decl->size() < mTupleDecl.size()) {
        throw assert_exception("Conjunction::setTupleDecl: given tuple_decl "
            "has fewer entries than current tuple declaration");
    }
    */

    // Iterate through tuple declaration we already have to
    // see if the changes are compatible.  Change entries in given
    // tuple_decl if we already have a constant.
    for (unsigned int i=0; (i<mTupleDecl.size()) && (i<tuple_decl.size()); i++) 
    {
        if (mTupleDecl.elemIsConst(i) && tuple_decl.elemIsConst(i)
                && mTupleDecl.elemToString(i)!=tuple_decl.elemToString(i)) {
            throw assert_exception("Conjunction::setTupleDecl: constants "
                "don't match.");
        }
        if (mTupleDecl.elemIsConst(i) && !tuple_decl.elemIsConst(i)) {
            tuple_decl.copyTupleElem(mTupleDecl, i, i);
        }
    }
    
    // copy possibly modified tuple declaration over our current tuple decl
    mTupleDecl = tuple_decl;
}

//! Creates a copy of its tuple declaration and returns it.
TupleDecl Conjunction::getTupleDecl(  ) const {
    return mTupleDecl;
}

//! Concatenates the two tuple declarations into one.
//! Consumes the passed in tuple_decl.
void Conjunction::setTupleDecl( TupleDecl tuple_decl_in, 
        TupleDecl tuple_decl_out ) {

    TupleDecl concatTuple = tuple_decl_in.concat(tuple_decl_out );
    setTupleDecl(concatTuple);
}


/*! addEquality -- add the given expression, interpreted as an
** equality (Exp = 0), to our list of equalities.
** Maintains a sorted order on the constraints.
**
** FIXME: the check for duplicate equalities is done in O(n) time here,
** but with a different strategy could be reduced to O(log n) time.
**
*/
void Conjunction::addEquality(Exp* equality) {
    equality->normalizeForEquality();
    if(equality->equalsZero()){
        delete equality;
        return;
    }

    for (std::list<Exp*>::iterator i=mEqualities.begin();
                i != mEqualities.end(); i++) {
        Exp* e = *i;

        if(*equality == *e){
            delete equality;
            return;
        }
        if (*equality < *e) {
            // We've found a term that belongs after the new one,
            // so insert the new term here, and we're done.
            mEqualities.insert(i, equality);
            return;
        }
    }

    mEqualities.push_back(equality);
}

/*! addInequality -- add the given expression, interpreted as an
** inequality (Exp >= 0), to our list of inequalities.
**
** FIXME: the check for duplicate inequalities is done in O(n) time here,
** but with a different strategy could be reduced to O(log n) time.
*/
void Conjunction::addInequality(Exp* inequality) {
    if(inequality->equalsZero()){
        delete inequality;
        return;
    }
    for (std::list<Exp*>::iterator i=mInequalities.begin();
                i != mInequalities.end(); i++) {
        Exp* e = *i;

        if(*inequality == *e){
            delete inequality;
            return;
        }
        if (*inequality < *e) {
            // We've found a term that belongs after the new one,
            // so insert the new exp here, and we're done.
            mInequalities.insert(i, inequality);
            return;
        }
    }
    mInequalities.push_back(inequality);
}

void Conjunction::substituteTupleDecl() {
    std::map<std::string, int> nameToLocationMap;
    SubMap var2TupleVar;

    for (unsigned int i=0; i<mTupleDecl.size(); i++) {
        // if it's a constant, no substitution is needed
        if (mTupleDecl.elemIsConst(i)) continue;

        // prepare the variable to search for
        std::string var = mTupleDecl.elemVarString(i);
        std::map<std::string, int>::iterator it = nameToLocationMap.find(var);
        if (it != nameToLocationMap.end()) {
            // This variable is one we've seen before; we don't
            // need to substitute, but we do need to add a new
            // equality constraint.
            Exp *equality = new Exp;
            equality->addTerm(new TupleVarTerm(it->second));
            equality->addTerm(new TupleVarTerm(-1, i));
            addEquality(equality);
        } else {
            // This is not a variable name we've seen before;
            // store it in our map for future reference, and
            // substitute into our constraints.
            nameToLocationMap[var] = i;
            Exp* tupleVarExp = new Exp();
            tupleVarExp->addTerm(new TupleVarTerm(i));
            var2TupleVar.insertPair(new VarTerm(var), tupleVarExp);
        }
    }

    // perform the substitution on all our constraints
    for (std::list<Exp*>::iterator expIter=mEqualities.begin();
                expIter != mEqualities.end(); expIter++) {
        (*expIter)->substitute(var2TupleVar);
    }
    for (std::list<Exp*>::iterator expIter=mInequalities.begin();
                expIter != mInequalities.end(); expIter++) {
        (*expIter)->substitute(var2TupleVar);
    }

    // The order of tuple locations may not match the previous alphabetical
    // order of the symbols we've replaced.  So, we need to renormalize now.
    this->cleanUp();

}

/*! copyConstraintsFrom -- copy all the equalities and inequalities
**  from source, and add them to our own constraints.
*/
void Conjunction::copyConstraintsFrom(const Conjunction *source) {
    for (std::list<Exp*>::const_iterator expIter=source->mEqualities.begin();
                expIter != source->mEqualities.end(); expIter++) {
        mEqualities.push_back((*expIter)->clone());
    }
    for (std::list<Exp*>::const_iterator expIter=source->mInequalities.begin();
                expIter != source->mInequalities.end(); expIter++) {
        mInequalities.push_back((*expIter)->clone());
    }
}

/*! Substitute each factor (i.e. the non-coefficient
** part of a term) with the expression mapped to that factor 
** in all our equalities and inequalities.
** Calls cleanup at end to resort constraints.
** \param searchTermToSubExp (none of the Term* or Exp* are adopted)
*/
void Conjunction::substituteInConstraints(SubMap& searchTermToSubExp) {

    // straight-forward substitution into equalities
    std::list<Exp*>::iterator expIter=mEqualities.begin();
    while (expIter != mEqualities.end()) {
        (*expIter)->substitute(searchTermToSubExp);
        expIter++;
    }
    expIter=mInequalities.begin();
    while (expIter != mInequalities.end()) {
        (*expIter)->substitute(searchTermToSubExp);
        expIter++;
    }
    cleanUp();
}


/*! Convert to a human-readable string.
**
*/
std::string Conjunction::toString() const {
    std::stringstream ss;
    ss << "{ " << mTupleDecl.toString(true,mInArity);

    bool first = true;
    for (std::list<Exp*>::const_iterator i=mEqualities.begin();
                i != mEqualities.end(); i++) {
        if (not first) { ss << " && "; }
        else { ss << " : ";  first = false; }
        ss << (*i)->toString() << " = 0";
    }

    for (std::list<Exp*>::const_iterator i=mInequalities.begin();
                i != mInequalities.end(); i++) {
        if (not first) { ss << " && "; }
        else { ss << " : ";  first = false; }
        ss << (*i)->toString() << " >= 0";
    }

    ss << " }";
    return ss.str();
}

/*! Convert to a human-readable string (sub in for tuple vars).
*/
std::string Conjunction::prettyPrintString() const {
    std::stringstream ss;
    ss << "{ " << mTupleDecl.toString(true,mInArity);
    bool first = true;
    Conjunction *dup = this->clone();

/* MMS, not sure why we are doing this code
    for (unsigned int i=0; i<dup->mTupleDecl.size(); i++) {
        Exp *varExp = new Exp();
        varExp->addTerm(mTupleDecl.elemCreateTerm(i,i));
        TupleVarTerm *tv = new TupleVarTerm(i);

        // perform the substitution on all our constraints
        for (std::list<Exp*>::const_iterator expIter=dup->mEqualities.begin();
                expIter != dup->mEqualities.end(); expIter++) {
            (*expIter)->substitute(varExp->clone(), *tv);
        }
        for (std::list<Exp*>::const_iterator expIter=dup->mInequalities.begin();
                expIter != dup->mInequalities.end(); expIter++) {
            (*expIter)->substitute(varExp->clone(), *tv);
        }
        delete tv;
        delete varExp;
    }
*/

    dup->cleanUp();
    for (std::list<Exp*>::const_iterator i = dup->mEqualities.begin();
                i != dup->mEqualities.end(); i++) {
        if (not first) { ss << " && "; }
        else { ss << " : ";  first = false; }
        ss << (*i)->prettyPrintString(mTupleDecl) << " = 0";
    }

    for (std::list<Exp*>::const_iterator i = dup->mInequalities.begin();
                i != dup->mInequalities.end(); i++) {
        if (not first) { ss << " && "; }
        else { ss << " : ";  first = false; }
        ss << (*i)->prettyPrintString(mTupleDecl) << " >= 0";
    }

    ss << " }";
    delete dup;
    return ss.str();
}

/* Convert to a string for DOT output
**
** \param next_id       id for this Conjunction node
**
** Reuses short segments of code from toString()
**
*/
std::string Conjunction::toDotString(int & next_id) const
{
    std::stringstream result;
    int self_id = next_id++;

    // Print out self node
    result << self_id << " [label = \"Conjunction\\n";
    result << mTupleDecl.toString(true,mInArity);
    result << "\"];\n";

    // Print out the Equalities
    if(mEqualities.begin() != mEqualities.end()){
        int eq_node = next_id++;
        result << self_id << "->" << eq_node << ";\n";
        result << eq_node << " [label = \"Equalities\\n... = 0\"];\n";
        for (std::list<Exp*>::const_iterator i=mEqualities.begin();
                i != mEqualities.end(); i++) {
            // recursively call on expressions
            result << (*i)->toDotString(eq_node,next_id);
        }
    }

    // Print out the Inequalities
    if(mInequalities.begin() != mInequalities.end()){
        int ineq_node = next_id++;
        result << self_id << "->" << ineq_node << ";\n";
        result << ineq_node << " [label = \"Inequalities\\n... >= 0\"];\n";
        for (std::list<Exp*>::const_iterator i=mInequalities.begin();
                i != mInequalities.end(); i++) {
            // recursively call on expressions
            result << (*i)->toDotString(ineq_node,next_id);
        }
    }

    return result.str();
}

/* Convert to a string for DOT output
**
** \param parent_id     id for parent node
** \param next_id       id for this Conjunction node
**
** Reuses short segments of code from toString()
**
*/
std::string Conjunction::toDotString(int parent_id, int & next_id) const
{
    std::stringstream result;
    int self_id = next_id;

    // Connect parent to self
    result << parent_id << " -> " << self_id << ";\n";

    // Print out self node
    result << this->toDotString(next_id);

    return result.str();
}

/*!
    The generated SymboIterator escapes this method and needs to be
    deleted by the caller.

    FIXME: this is an example where having a visitor design pattern
    would be helpful.  Having to create this getSymbolIterator
    at Conjunction and Exp level.
*/
StringIterator* Conjunction::getSymbolIterator() const {
    std::set<std::string> symbolSet;

    // Collect symbols from equalities
    for (std::list<Exp*>::const_iterator i=mEqualities.begin();
            i != mEqualities.end(); i++) {
        // recursively call on expressions
        StringIterator* subSymIter = (*i)->getSymbolIterator();
        while (subSymIter->hasNext()) {
            symbolSet.insert( subSymIter->next() );
        }
        delete subSymIter;
    }

    // Collect symbols from inequalities
    for (std::list<Exp*>::const_iterator i=mInequalities.begin();
            i != mInequalities.end(); i++) {
        // recursively call on expressions
        StringIterator* subSymIter = (*i)->getSymbolIterator();
        while (subSymIter->hasNext()) {
            symbolSet.insert( subSymIter->next() );
        }
        delete subSymIter;
    }

    return new StringIterator( symbolSet );
}

//! Get an iterator over the tuple variables, in order.
StringIterator* Conjunction::getTupleIterator() const {
    std::list<std::string> tupleList;
    for (int i=0; i<arity(); i++) {
        if (mTupleDecl.elemIsConst(i)) {
            throw assert_exception("Conjunction::getTupleIterator: expecting "
                               "all tuple vars to be non const");
        }
        tupleList.push_back(mTupleDecl.elemVarString(i));
    }
    return new StringIterator( tupleList );
}


bool Conjunction::isFunction(int inArity) const {
    for (int i=inArity; i<arity(); i++) {
        // otherwise, we must be able to find a function for
        // it in terms of the input tuple decls
        Exp *funcExp = findFunction(i, 0, inArity-1);
        if (not funcExp) return false;
        delete funcExp;
    }
    return true;
}

bool Conjunction::isFunctionInverse(int inArity) const {
    for (int i=0; i<inArity; i++) {
        // otherwise, we must be able to find a function for
        // it in terms of the output tuple decls
        Exp *funcExp = findFunction(i, inArity, arity()-1);
        if (not funcExp) return false;
        delete funcExp;
    }
    return true;
}
//! Compose with another conjunction, given innerArity (which
//! is this's in arity and rhs's out arity).
//
// Both the point where we are setting up equality constraints and
// when we do substitute into constraints can cause constraints such
// as 1=0 to occur.  Instead of dealing with them at each point they
// can be introduced, we decided to have the SparseConstraints level
// cleanup remove a conjunction when such constraints exist within
// the function.
Conjunction*
Conjunction::Compose(const Conjunction *rhs, int innerArity) const {
    // Compute the input and output arities for LHS and RHS conjunctions.
    int lhsInArity = innerArity;
    int lhsOutArity = arity() - lhsInArity;
    int rhsOutArity = innerArity;
    int rhsInArity = rhs->arity() - rhsOutArity;
    int finalInArity = rhsInArity;
    int finalOutArity = lhsOutArity;

    // figure out whether the relations are functions or inverses of functions
    bool lhsIsFunction = isFunction(lhsInArity);
    bool rhsIsFunction = rhs->isFunction(rhsInArity);
    bool lhsIsFunctionInverse = isFunctionInverse(lhsInArity);
    bool rhsIsFunctionInverse = rhs->isFunctionInverse(rhsInArity);
    
    // Check that we can do compose.
    if (!( (lhsIsFunction && rhsIsFunction) || 
           (lhsIsFunctionInverse && rhsIsFunctionInverse) ) ) {
        // If none of the above apply, we can't compose.
        // Should never happen, since we require all relations to
        // be either a function or a function inverse.
        std::stringstream ss;
        ss << "Conjunction::Compose: both relations are "
                            "required to be a function or a function "
                            "inverse: lhsIsFunction:" << lhsIsFunction <<
                            " lhsIsFunctionInverse:" << lhsIsFunctionInverse <<
                            " rhsIsFunction:" << rhsIsFunction <<
                            " rhsIsFunctionInverse:" <<rhsIsFunctionInverse;
        throw assert_exception(ss.str());
    }           

    // Set up the resulting conjunction, using input tuple variables
    // from the RHS, and output tuple variables from the LHS, and putting
    // the inner arity variables off the end (we truncate those at the end).
    // e.g. {[a,b,c] -> [e,f]} compose {[i]->[a,b,c]}
    // result will be {[i] -> [e,f, a,b,c]}
    Conjunction *result = new Conjunction( 
        finalInArity +  finalOutArity + innerArity);
    result->mInArity = finalInArity;
    
    // get copies of lhs and rhs to line up their tuple vars with result
    // e.g. if lhs = {[a,b,c]->[e,f]} then lhs_copy = {[ , e, f, a, b, c]}
    Conjunction* lhs_copy = new Conjunction(*this);
    lhs_copy->pushConstToConstraints();
    std::vector<int> shiftLHSVars(lhs_copy->arity());
    for (int i=0; i<lhsInArity; i++) {
        shiftLHSVars[i] = i + finalInArity + finalOutArity;
    }
    for (int i=lhsInArity, j=0; i<lhs_copy->arity(); i++, j++) {
        shiftLHSVars[i] = j + finalInArity;
    }
    lhs_copy->remapTupleVars(shiftLHSVars);
    // e.g. if rhs = {[i]->[a,b,c]} then rhs_copy = {[ i, , , a, b, c]}
    Conjunction* rhs_copy = new Conjunction(*rhs);
    rhs_copy->pushConstToConstraints();
    std::vector<int> shiftRHSVars(rhs->arity());
    for (int i=0; i<rhsInArity; i++) {
        shiftRHSVars[i] = i;
    }
    for (int i=rhsInArity, j=0; i<rhs->arity(); i++, j++) {
        shiftRHSVars[i] = j + finalInArity + finalOutArity;
    }
    rhs_copy->remapTupleVars(shiftRHSVars);

    // copy the constraints from both into result
    result->copyConstraintsFrom(rhs_copy);
    result->copyConstraintsFrom(lhs_copy);    

    // Keep track of variable names so we don't have collisions
    // in the result tuple declarations.  
    // First get used var names from result input tuple.
    std::set<std::string> takenNames;
    for (int i=0; i<finalInArity; i++) {
        result->mTupleDecl.copyTupleElem(rhs_copy->mTupleDecl, i, i);
        takenNames.insert(result->mTupleDecl.elemVarString(i));
    }
    for (int i=0; i<finalOutArity; i++) {
        // Ensure the output tuple vars in result have different
        // names that the input tuple vars in result.
        result->mTupleDecl.setTupleElem(finalInArity+i,
            createUniqueName(lhs_copy->mTupleDecl.elemVarString(finalInArity+i),
                             takenNames) );
    }
    // Not going to name the inner arity tuple variables off the end
    // so they will have default names.  They are going away anyway
    // do don't need names.    
    // done with lhs_copy and rhs_copy
    delete lhs_copy;
    delete rhs_copy;

    // Compose Case 1: both relations are functions
    // lhs = { v -> z : z = F1(v) && C1 }
    // rhs = { x -> y : y = F2(x) && C2 }
    // lhs compose rhs = { x -> z : z=F1(F2(x)) && C1[v/F2(x)] && C2[y/F2(x)] }
    // Recall in result everything has been shifted so have x z v=y
    // F1 substitution is already there so just do F2 substitution 
    // to replace all v and y variables.
    if (lhsIsFunction and rhsIsFunction) {
        // Keep track of required substitutions.
       SubMap submapF2;

        // For each output tuple variable on the RHS, y_j, we'll
        // find it as an expression of the RHS input tuple variables, x.
        for (int j=0; j<innerArity; j++) {
            int rhsIdx = finalInArity + finalOutArity + j;
            Term* t = result->mTupleDecl.elemCreateTerm(rhsIdx, rhsIdx);
            // y_j = F2_j(x)
            Exp* F2_j = result->findFunction(rhsIdx, 0, rhsInArity-1);
            submapF2.insertPair(t, F2_j);
        }
        
        result->substituteInConstraints(submapF2);

    // Compose Case 2: both relations are function inverses
    // lhs = { v -> z : v = G1(z) && C1 }
    // rhs = { x -> y : x = G2(y) && C2 }
    // lhs compose rhs = { x -> z : x=G2(G1(z)) && C1[v/G1(z)] && C2[y/G1(z)] }
    // Recall in result everything has been shifted so have x z v=y
    // G2 substitution is already there so just do G1 substitution 
    // to replace all v and y variables.
    } else if (lhsIsFunctionInverse and rhsIsFunctionInverse) {
        // Keep track of required substitutions.
        SubMap submapG1;

        // For each input tuple variable on the LHS, v_j, we'll
        // find it as an expression of the LHS output tuple variables, z.
        for (int j=0; j<innerArity; j++) {
            int lhsIdx = finalInArity + finalOutArity + j;
            Term* t = result->mTupleDecl.elemCreateTerm(lhsIdx, lhsIdx);
            // v_j = G1_j(z)
            Exp* G1_j = result->findFunction(lhsIdx, 
                finalInArity, finalInArity+finalOutArity-1);
                
            submapG1.insertPair(t, G1_j);
        }
        
        result->substituteInConstraints(submapG1);
    }

    // Remove the inner tuple vars for relation from end of result set.
    // e.g. x z v=y should become x -> z
    // Create a new truncated tuple declaration
    TupleDecl final_tdecl(finalInArity+finalOutArity);
    for (int i=0; i<finalInArity+finalOutArity; i++) {
        final_tdecl.copyTupleElem(result->mTupleDecl, i, i);
    }
    result->setTupleDecl(final_tdecl);
    // All done!
    result->pushConstConstraintsToTupleDecl();
    return result;
}

/*! Pushes the constants in the tuple declaration into equality
**  constraints instead.
*/
void Conjunction::pushConstToConstraints() {
    // Loop through own tuple decl in search of constants.
    for (int i=0; i<arity(); i++) {
        if (mTupleDecl.elemIsConst(i)) {
            int const_val = mTupleDecl.elemConstVal(i);
            std::stringstream ss;
            ss << "c_" << const_val;
            mTupleDecl.setTupleElem(i, ss.str());
            Exp* e = new Exp();
            e->addTerm( mTupleDecl.elemCreateTerm(i,i) );
            e->addTerm( new Term(-1*const_val) );
            addEquality( e );
        }
    }
}

/*! Pushes constants in constraints into the tuple declaration.
*/
void Conjunction::pushConstConstraintsToTupleDecl() {
    std::map<unsigned int,int> tupleIndexToConst;
    std::set<Exp> equalitiesToRemove;

    // loop through own equality constraints to find
    // when tuple vars are equal to constants.
    for (std::list<Exp*>::const_iterator iter=mEqualities.begin();
                iter != mEqualities.end(); iter++ ) {
        Exp* e = *iter;
        
        // For each tuple variable solve for that tuple var in expression.
        // If result is a constant then keep that mapping of tuple var
        // index to constant and keep track of Exp as one we want
        // to erase from constraints.
        for (int i=0; i<arity(); i++) {
                    
            if (! mTupleDecl.elemIsConst(i)) {
                // solve for the tuple variable in expression
                Term* t = mTupleDecl.elemCreateTerm(i,i);
                Exp* solution = e->solveForFactor(t);
                // if the solution is a constant
                if (solution && solution->equalsZero()) {
                    tupleIndexToConst[i] = 0;
                    equalitiesToRemove.insert(*e);
                } 
                else if (solution && (t=solution->getTerm()) && t->isConst() ) {
                    tupleIndexToConst[i] = t->coefficient();
                    equalitiesToRemove.insert(*e);
                }
                if (solution) { delete solution; }
            }
        }
    }
        
    // Now go set the tuple variables to constants where necessary
    std::map<unsigned int,int>::iterator mapIter;
    for (mapIter = tupleIndexToConst.begin();
            mapIter != tupleIndexToConst.end(); mapIter++) {
        mTupleDecl.setTupleElem(mapIter->first, mapIter->second );
    }
    
    // Finally remove all those constant equality expressions.
    for (std::list<Exp*>::iterator iter=mEqualities.begin();
                iter != mEqualities.end(); ) {
        Exp* e = *iter;
        if (equalitiesToRemove.find(*e)!=equalitiesToRemove.end()) {
            delete (*iter);
            iter = mEqualities.erase(iter);
        } else {
            iter++;
        }
    }
}

//! Apply this (interpreted as a Relation) to rhs, which is interpreted
//! as a set.
// Implementation of Apply that uses the en masse substitution.
// FIXME: what if we have name collisions in the tuple declarations?
Conjunction *Conjunction::Apply(const Conjunction *rhs) const {

    // Check that "this" relation can solve for the input tuple elements
    // as constants or functions of the output tuple elements.
    // e.g. for {[i,j,k]->[t,v]} finding i,j,k as functions of t,v
    bool lhsIsFunctionInverse = isFunctionInverse(rhs->arity());
    if (!lhsIsFunctionInverse) {
        // If none of the above apply, we can't apply the relation to the set.
        // This happens if the relation is not a function inverse.
        throw assert_exception("Conjunction::Apply: Relation not invertible");
    }

    // Compute the input and output arities for LHS conjunction and result set.
    int lhsInArity = rhs->arity();
    int lhsOutArity = arity() - lhsInArity;
    int finalResultArity = lhsOutArity;

    // Set up the resulting set.  At first it will start out
    // as a copy of the lhs relation with the input and output
    // variables shifted.
    // e.g. if lhs={[i,j,k]->[t,v]} then result={[t,v]->[i,j,k]}
    Conjunction* result = new Conjunction(*this);
    result->pushConstToConstraints();
    result->mInArity = lhsOutArity;

    // Create map for remapping tuple variables.
    std::vector<int> shiftResultVars(result->arity());
    for (int i=0; i<lhsInArity; i++) {
        // moving input tuple vars over to the right
        // e.g. [i,j,k, , ] to [ , , i, j, k]
        shiftResultVars[i] = i+finalResultArity;
    }
    for (int i=lhsInArity; i<result->arity(); i++) {
        // moving output tuple vars over to the left
        // e.g. [ , , , t, v] to [t, v, , , ]
        shiftResultVars[i] = i-lhsInArity;
    }
    result->remapTupleVars(shiftResultVars);
    
    // Need to do shift the tuple variables in set
    // and in relation so that they all with tuple variables 
    // in the result.
    // e.g. shift [a,b,c] to [ , , a, b, c] so they line up with
    // i,j,k in result tuple declaration [t,v,i,j,k]
    // Shift tuple variables in the 
    Conjunction *rhs_copy = new Conjunction(*rhs);
    rhs_copy->pushConstToConstraints();
    std::vector<int> shiftRHSVars(rhs->arity());
    for (int i=0; i<rhs->arity(); i++) {
        shiftRHSVars[i] = i + finalResultArity;
    }
    rhs_copy->remapTupleVars(shiftRHSVars);

    // Copy all constraints from rhs set into
    // our result, where we will then do the needed substitutions.
    result->copyConstraintsFrom(rhs_copy);
    delete rhs_copy;

    // Apply
    // r = { x -> y : x = G(y) && C }
    // s = { z : D }
    // r(s) = { y : D[z/G(y)] && C[x/G(y)] }
    // For each input tuple variable on the LHS, we'll find it as an
    // expression of the LHS output tuple variables and then substitute this
    // in for the other constraints in the relation and the set.
    SubMap submap;
    for (int i=finalResultArity; i<result->arity(); i++) {
        // x_i = G_i(y)
        // e.g. recall input tuple are now to right of output
        // {[i,j,k] -> [t,v]} is [t,v, i,j,k] in result set
        Exp* G_i = result->findFunction(i, 0, finalResultArity-1);
        // Now create term that this function will be replacing.
        // D[z_i/G_i(y)] and C[x_i/G_i(y)]
        // substituting function in constraints for set and result
        Term* t = result->mTupleDecl.elemCreateTerm(i,i);
        submap.insertPair(t,G_i); 
    }

    // perform the substitution on all z_i and x_i vars at once
    result->substituteInConstraints(submap); 
    
    // Remove the input tuple vars for relation from end of result set.
    // e.g. [t,v,i,j,k] should become [t,v]
    // Create a new truncated tuple declaration
    TupleDecl final_tdecl(finalResultArity);
    for (int i=0; i<finalResultArity; i++) {
        final_tdecl.copyTupleElem(result->mTupleDecl, i, i);
    }
    Conjunction* final_result = new Conjunction(final_tdecl);
    final_result->copyConstraintsFrom(result);
    delete result;

    // All done!
    final_result->pushConstConstraintsToTupleDecl();
    return final_result;
}

//! Returns true if the conjunction has at least one equality or inequality
//! constraints.  If it contains none then this Conjunction is just
//! representing TRUE.
bool Conjunction::hasConstraints() const {
    if (mEqualities.size() + mInequalities.size() > 0 ) { return true; }
    else { return false; }
}



/*! Search among our equality constraints for one that defines tupleLocToFind
**  as a function of only tuple variables in the location range
**  [startTupleRange, endTupleRange].  Return a new copy of that expression.
*/
Exp *Conjunction::findFunction(int tupleLocToFind, int startTupleRange,
                               int endTupleRange) const {

    // Handle the trivial case of a constant tuple element.
    if (mTupleDecl.elemIsConst(tupleLocToFind)) {
        Exp *result = new Exp();
        int constVal = mTupleDecl.elemConstVal(tupleLocToFind);
        result->addTerm(new Term(constVal));
        return result;
    }

    // Otherwise, search all our equalities for one that can
    // be solved for this tuple element.
    for (std::list<Exp*>::const_iterator i=mEqualities.begin();
                i != mEqualities.end(); i++) {
        TupleVarTerm *factor = new TupleVarTerm(tupleLocToFind);
        Exp *solution = (*i)->solveForFactor(factor);
        if (not solution) {
          continue;
        }

        // OK, we found a solution, but we must verify that it
        // does not depend on any tuple variables outside the given range.
        bool acceptable = true;
        for (int j=0; j < arity(); j++) {
            if (j >= startTupleRange and j <= endTupleRange) continue;
            TupleVarTerm term(j);
            if (solution->dependsOn(term)) {
                // This depends on a forbidden tuple variable, so it's no good.
                acceptable = false;
                break;
            }
        } // check next tuple variable dependency
        if (acceptable) {
            return solution;    // Hooray!
        } else {
            delete solution;
        }
    } // try next equality

    // No function could be found.
    return NULL;
}

/*! Search among our equality constraints for one that defines tupleLocToFind
**  as a function of only tuple variables in the location range
**  [startTupleRange, endTupleRange].  Return a new copy of that expression.
**  Additionally remove the equality constraint that the function was
**  derived from.
*/
Exp *Conjunction::findAndRemoveFunction(int tupleLocToFind,
                                int startTupleRange, int endTupleRange) {

    // FIXME: this in large part is a copy of findFunction
    // can we do some refactoring?

    // Handle the trivial case of a constant tuple element.
    if (mTupleDecl.elemIsConst(tupleLocToFind)) {
        Exp *result = new Exp();
        int constVal = mTupleDecl.elemConstVal(tupleLocToFind);
        result->addTerm(new Term(constVal));
        return result;
    }

    // Otherwise, search all our equalities for one that can
    // be solved for this tuple element.
    for (std::list<Exp*>::iterator i=mEqualities.begin();
                i != mEqualities.end(); i++) {
        TupleVarTerm *factor = new TupleVarTerm(tupleLocToFind);
        Exp *solution = (*i)->solveForFactor(factor);
        if (not solution) {
          continue;
        }

        // OK, we found a solution, but we must verify that it
        // does not depend on any tuple variables outside the given range.
        bool acceptable = true;
        for (int j=0; j < arity(); j++) {
            if (j >= startTupleRange and j <= endTupleRange) continue;
            TupleVarTerm term(j);
            if (solution->dependsOn(term)) {
                // This depends on a forbidden tuple variable, so it's no good.
                acceptable = false;
                break;
            }
        } // check next tuple variable dependency
        if (acceptable) {
            //remove the equality constraint and then return the solution
            delete (*i);
            mEqualities.erase(i);
            return solution;    // Hooray!
        } else {
            delete solution;
        }
    } // try next equality

    // No function could be found.
    return NULL;
}


/*! Find any TupleVarTerms in this expression (and subexpressions)
**  and remap the locations according to the oldToNewLocs vector,
**  where oldToNewLocs[i] = j means that old location i becomes new
**  location j (i.e. __tvi -> __tvj).  As a special case, a value
**  of -1 means that old location goes away entirely.
*/
void Conjunction::remapTupleVars(const std::vector<int>& oldToNewLocs) {
    // Remap tuple variables in our equalities.
    for (std::list<Exp*>::iterator i=mEqualities.begin();
                i != mEqualities.end(); i++) {
        (*i)->remapTupleVars(oldToNewLocs);
        (*i)->normalizeForEquality();
    }

    // Remap tuple variables in our inequalities.
    for (std::list<Exp*>::iterator i=mInequalities.begin();
                i != mInequalities.end(); i++) {
        (*i)->remapTupleVars(oldToNewLocs);
    }

    // Remap our tuple declarations to match.
    // (There may be gaps -- we'll declare those as constant zeros.)
    int maxLoc = -1;
    for (unsigned int i=0; i<oldToNewLocs.size(); i++) {
        if (oldToNewLocs[i] > maxLoc) { maxLoc = oldToNewLocs[i]; }
    }

    TupleDecl newDecl(maxLoc+1);    // create a new tuple declaration
    for (unsigned int i=0; i<oldToNewLocs.size(); i++) {
        if (oldToNewLocs[i] >= 0) {
            newDecl.copyTupleElem(mTupleDecl,i,oldToNewLocs[i]);
        }
    }
    mTupleDecl = newDecl;
}

/*! Create the inverse of this conjunction. Returns a new Conjunction,
**    which the caller is responsible for deallocating.
*/
Conjunction* Conjunction::Inverse() const{

    std::vector<int> oldToNewLocs;

    unsigned int outArity = mTupleDecl.size() - mInArity;

    // Result conjunction should have its inArity be our outArity
    Conjunction *result = new Conjunction(*this);
    result->mInArity = outArity;

    // Create permutation for input and output tuples
    // so that the output tuple variables become the input tuple
    // variables and the input tuple variables become the outputs.
    for(unsigned int j = outArity; j < mTupleDecl.size(); j++){
        oldToNewLocs.push_back(j);
    }
    for(unsigned int j = 0; j < outArity; j++){
        oldToNewLocs.push_back(j);
    }
    result->remapTupleVars(oldToNewLocs);

    return result;
}


/*! Intersect this conjunction with the given other one
**    (i.e., this conjunction rhs).  Returns a new Conjunction,
**    which the caller is responsible for deallocating.
** \param rhs (not adopted)
*/
Conjunction* Conjunction::Intersect(const Conjunction* rhs) const {

    // copy in all of the constraints from ourselves
    Conjunction* retval = new Conjunction(*this);

    // then get equalities and equalities from rhs
    for (std::list<Exp*>::const_iterator i=rhs->mEqualities.begin();
            i != rhs->mEqualities.end(); i++) {
        retval->addEquality((*i)->clone());
    }
    for (std::list<Exp*>::const_iterator i=rhs->mInequalities.begin();
            i != rhs->mInequalities.end(); i++ ) {
        retval->addInequality((*i)->clone());
    }

    return retval;
}

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
Conjunction* Conjunction::boundTupleExp(const TupleExpTerm& tuple_exp) const {
    // Check that arities match.
    if (tuple_exp.size()!=(unsigned)arity()) {
        throw assert_exception("Conjunction::boundTupleExp tuple_exp arity "
                               "does not match that of Conjunction");
    }

    // Create a zero arity conjunction.
    Conjunction* retval = new Conjunction(0);
    
    // copy all constraints from a clone of ourselves that has had
    // constant values pushed to the constraints
    Conjunction* dup = new Conjunction(*this);
    dup->pushConstToConstraints();
    retval->copyConstraintsFrom(dup);
    
    // then create bounds by substituting expressions in tuple_exp
    // into the expressions in copy of self in retval.
    // Has to be done in order because 
    SubMap submap;
    for (unsigned int i=0; i<tuple_exp.size(); i++) {
        Term * t = dup->mTupleDecl.elemCreateTerm(i, i);
        Exp * elemExp = tuple_exp.cloneExp(i);
        submap.insertPair( t, elemExp );
    }
    retval->substituteInConstraints( submap );

    delete dup;
    return retval;
}


//! Return true if the constraints in the conjunction are satisfiable.
// FIXME: need to check for affine inequalities and equalities that are
// not satisfiable.  For now just checking for constant equalities.
bool Conjunction::satisfiable() const {

    for (std::list<Exp*>::const_iterator i=mEqualities.begin();
            i != mEqualities.end(); i++) {
        if ( (*i)->isConst() ) {
            return false;
        }
    }
    return true;
}


/*! Removes equality and inequality Exps that are equal to zero
**  from list of Expressions.
*/

void Conjunction::cleanUp() {
    // Remove zero equalities, normalize, and remove nested inverse funcs
    for (std::list<Exp*>::iterator i=mEqualities.begin();
            i != mEqualities.end(); ) {

        // make first term positive
        (*i)->normalizeForEquality();

        // get rid of nested inverse funcs like f( f_inv( i ) )
        Exp* temp = (*i);
        (*i) = temp->collapseNestedInvertibleFunctions();
        delete temp;

        // remove zero expressions
        if((*i)->equalsZero()){
            std::list<Exp*>::iterator old = i;
            i++;
            delete *old;
            mEqualities.erase(old);
        } else { i++; }
    }

    // copy the current list of equalities and then add then
    // so they are added in order and duplicates are eliminated
    std::list<Exp*> equalityListCopy = mEqualities;
    mEqualities.clear();
    for (std::list<Exp*>::iterator i=equalityListCopy.begin();
            i != equalityListCopy.end(); i++) {
        this->addEquality(*i);
    }

    // Remove zero inequalities
    for (std::list<Exp*>::iterator i=mInequalities.begin();
            i != mInequalities.end(); ) {

         // get rid of nested inverse funcs like f( f_inv( i ) )
        Exp* temp = (*i);
        (*i) = temp->collapseNestedInvertibleFunctions();
        delete temp;

        if((*i)->equalsZero()){
            std::list<Exp*>::iterator old = i;
            i++;
            delete *old;
            mInequalities.erase(old);
        } else { i++; }
    }

    // copy the current list of inequalities and then add then
    // so they are added in order and duplicates are eliminated
    std::list<Exp*> inequalityListCopy = mInequalities;
    mInequalities.clear();
    for (std::list<Exp*>::iterator i=inequalityListCopy.begin();
            i != inequalityListCopy.end(); i++) {
        this->addInequality(*i);
    }

}

/*!
** (step 0) Group together all equality expressions that 
** are parts of the same UFCallTerm, IOW i=f(k)[0] and 
** j=f(k)[1] should become (i,j) = f(k).
*/
void Conjunction::groupIndexedUFCalls() {
    // loop over all equalities in conjunction and gather up
    // all indexed UFCall return expressions (i.e. what they are equal to)
    std::map<UFCallTerm,std::map<int,Exp*> > ufcallAndIndex2Exp;
    for (std::list<Exp*>::iterator i=mEqualities.begin();
                i != mEqualities.end(); ) {
        Exp* e = (*i);

        // FIXME: assuming only one indexed UFCall, have
        // hasIndexedUFCall throw an assertion if that is
        // not the case.
        if (e->hasIndexedUFCall()) {
            UFCallTerm* uf_call_ptr = e->cloneIndexedUFCallTerm();
            // ensure that clone has a coefficient of 1
            // FIXME: what if it is something else?
            if (uf_call_ptr->coefficient()==-1) {
                uf_call_ptr->multiplyBy(-1);
            }
            Exp* ret_exp = e->solveForFactor(uf_call_ptr->clone());
            // FIXME: put in code that checks we haven't seen this
            // tuple index before
            // Store off the equality expression.
            UFCallTerm* nonindexed = uf_call_ptr->nonIndexedClone();
            ufcallAndIndex2Exp[*nonindexed][uf_call_ptr->tupleIndex()] 
                = ret_exp;
            delete nonindexed;
            delete uf_call_ptr;
            // remove original equality for now
            delete (*i);
            i=mEqualities.erase(i);
        } else {
            i++;  // must increment iterator if not remove expression
        }
    }
    
    // Loop through return expressions we found and see if have all
    // for a particular UFCall.  If do then replace them all with
    // a single equality.  If don't, then have to put them all back in.
    std::map<UFCallTerm,std::map<int,Exp*> >::const_iterator ufcallIter;
    for (ufcallIter=ufcallAndIndex2Exp.begin();
            ufcallIter!=ufcallAndIndex2Exp.end(); ufcallIter++) {
        bool foundAllIndices = true;
        // Get each ufcall
        UFCallTerm ufcall = ufcallIter->first;
        std::map<int,Exp*> index2retexp = ufcallIter->second;
        // Check that have a return expression for each index
        // and build TupleExpTerm.
        TupleExpTerm ret_exp_tuple(ufcall.size());
        for (unsigned int i=0; i<ufcall.size(); i++) {
            if (index2retexp.find(i)==index2retexp.end()) {
                foundAllIndices = false;
                break;
            } else {
                // Give TupleExpTerm ownership of saved expression.
                ret_exp_tuple.setExpElem(i,index2retexp[i]);
            }
        }
        // If we found all indices create a new equality and add it.
        if (foundAllIndices) {
            Exp* new_equality = new Exp();
            TupleExpTerm* tuple_term = new TupleExpTerm(ret_exp_tuple);
            tuple_term->multiplyBy(-1);
            new_equality->addTerm( tuple_term );
            new_equality->addTerm( new UFCallTerm( ufcall ) );
            addEquality( new_equality );
        }
        // otherwise we have to put all the equalities back in
        else {
            std::map<int,Exp*>::iterator indexIter;
            for (indexIter=index2retexp.begin(); 
                    indexIter!=index2retexp.end(); indexIter++) {
                Exp* ret_exp = indexIter->second;
                ret_exp->multiplyBy(-1);
                // Set ufcall term back to having the associated index.
                ufcall.setTupleIndex(indexIter->first);
                ret_exp->addTerm( new UFCallTerm(ufcall) );
                addEquality( ret_exp );
            }
        }
    }
    
}

/*! Create a conjunction that represents an affine
**  superset of the "this" Conjunction.  Will be replacing
**  uf calls with temporary variables recursively bottom up and 
**  will be bounding parameter expressions and expressions equivalent
**  to UF call return values based on the domain and range
**  declarations for the UF call.
**  The returned conjunction will need to be deleted by caller.
*/
void Conjunction::ufCallsToTempVars(UFCallMapAndBounds & ufcallmap) {
    Conjunction modified(getTupleDecl());
    
    // Iterate over the equality expressions.
    for (std::list<Exp*>::iterator i=mEqualities.begin();
                i != mEqualities.end(); i++) {
        Exp* e = (*i);
//std::cout << "Conjunction::ufCallsToTempVars: e     = " << e->toString() << std::endl;
        Exp* modified_e = e->ufCallsToTempVars(ufcallmap);
//std::cout << "Conjunction::ufCallsToTempVars: mod_e = " << e->toString() << std::endl;
        // Check if the modified expression is just one tuple exp
        // term.  If so break it into separate equalities.
        TupleExpTerm* tup_exp;
        if ((tup_exp=dynamic_cast<TupleExpTerm*>(modified_e->getTerm()))) {
            for (unsigned int i=0; i<tup_exp->size(); i++) {
                modified.addEquality(tup_exp->cloneExp(i));
            }
            delete modified_e;
        } else {
            modified.addEquality(modified_e);
        }
    }
 
     // Iterate over the inequality expressions.
    for (std::list<Exp*>::iterator i=mInequalities.begin();
                i != mInequalities.end(); i++) {
        Exp* e = (*i);
        Exp* modified_e = e->ufCallsToTempVars(ufcallmap);
        modified.addInequality(modified_e);
    }
    
    *this = modified;
}

/*! Replace "this" Conjunct with normalized set of equalities
**  and inequalities.  See SparseConstraints::normalize() for 
**  algorithm overview.
*/
Set* Conjunction::normalize() const {

    /////////////////////
    // (step 0) Group together all equality expressions that 
    // are parts of the same UFCallTerm, IOW i=f(k)[0] and 
    // j=f(k)[1] should become (i,j) = f(k).
    // FIXME: what about inequalities?
//std::cout << "Conjunction::normalize: original       = " << toString() << std::endl;
    Conjunction* selfcopy = new Conjunction(*this);
	TupleDecl origTupleDecl = selfcopy->getTupleDecl(); // for (step 3)
//std::cout << "Conjunction::normalize: selfcopy-bfour = " << selfcopy->toString() << std::endl;
    selfcopy->groupIndexedUFCalls();
//std::cout << "Conjunction::normalize: selfcopy-after = " << selfcopy->toString() << std::endl;
 
    /////////////////////
    // (step 1)
    // Replace all uninterpreted function calls with variables
    // and create a new affine conjunction that is a superset of the
    // current conjunction.  UFCallMapAndBounds object will
    // maintain the mapping of temporary variables to UF calls.
    UFCallMapAndBounds ufcallmap(getTupleDecl());
    selfcopy->ufCallsToTempVars( ufcallmap );
//std::cout << "Conjunction::normalize: selfcopy = " << selfcopy->toString() << std::endl;    
    // To test step 1, we return Set of constraints that have been
    // gathered in UFCallMapAndBounds.
    Set* constraints = ufcallmap.cloneConstraints();
//std::cout << "Conjunction::normalize: constraints = " << constraints->toString() << std::endl;    
//std::cout << "Conjunction::normalize: ufcallmap = " << ufcallmap.toString() << std::endl;
    // update the tuple declaration in selfcopy to match what is
    // in the constraints
    selfcopy->setTupleDecl( constraints->getTupleDecl() );
    
    // Also need to include all the constraints in selfcopy were UF calls
    // were replaced by temporaries.
    Set* selfcopyset = new Set(selfcopy->getTupleDecl());
    selfcopyset->addConjunction(selfcopy);
//std::cout << "Conjunction::normalize: selfcopyset = " << selfcopyset->toString() << std::endl;
    Set* retval1 = constraints->Intersect(selfcopyset);
//std::cout << "Conjunction::normalize: after intersect = , retval = " << retval1->toString() << std::endl;
    //return retval1;
    
    //////////////////////
    // (step 2) 
    // Send the result of step 1 to ISL and back.
    
    // (a) get string from result of step 1
    std::string fromStep1 = retval1->toISLString();
//std::cout << "Conjunction::normalize: fromStep1 = " << fromStep1 << std::endl;
 
    // (b) send through ISL
    //std::string fromISL = getRelationStringFromISL(fromStep1);
//std::cout << "Conjunction::normalize: fromISl = " << fromISL << std::endl;
    
    // (a & b) could do this as one statement (below), but broken apart (above) for testing
    string fromISL = getRelationStringFromISL(retval1->toISLString());

//std::cout << "Conjunction::normalize: fromISl = " << fromISL << std::endl;

    // (c) convert back to a Set
    //Set* retval2 = iegenlib::parser::parse_set(fromISL);
    Set* retval2 = new Set(fromISL);
//std::cout << "Conjunction::normalize: retval2 = " << retval2->toString() << std::endl;

    delete retval1;
    
    // Then in (step 3) we will need to use substitute to replace extra
    // tuple vars with UF calls by making queries to ufcallmap.  Will need
    // to call setTupleDecl on retval to remove those extra tuple vars that
    // acted as temporaries to replace uf calls.  Then return retval Set.
    
    /////////////////////
    // (step 3)
    //
    // (a-1) create map of affine information that we can glean from retval2 ... from ISL.
    
    SubMap affineSubstMap;
    
    // FIXME: just assuming one conjunction right now.
    if (retval2->mConjunctions.size()!=1) {
        throw assert_exception("Conjunction::normalize: "
            "currently only handle one Conjunction fromISL");
    }
    Conjunction* conj = retval2->mConjunctions.front();
    
    // Determine starting index of extra tuple vars
    int numTempVars = ufcallmap.numTempVars();
    int expandedArity = constraints->arity();
    int indexStart = expandedArity - numTempVars;

	// walk backward through additional temp vars 
    for (int i = expandedArity-1; i >= indexStart; i--) {
    	// (1) find an expression for temp var i in terms of vars 0 - (i-1)
    	TupleVarTerm* tvTerm = new TupleVarTerm(i);
        Exp * tvExp = conj->findFunction(i,0,i-1);
    	
    	if (not tvExp) {
    		/*
            throw assert_exception("Conjunction::normalize: "
                "no expression for " + tvTerm->toString() + 
                " in Conjunction fromISL");
        	*/
        	continue;
        }
        
//std::cout << "Conjunction::normalize: __tv" << i << " = " << tvExp->toString() << std::endl;

        // (2) substitute this equivalence (tvTerm = e) into affineSubstMap
        
      
        affineSubstMap.startIter();
        while (affineSubstMap.hasNext()) {
            Term* mapTerm = affineSubstMap.next();
            Exp* mapExp = affineSubstMap.subExp(mapTerm);
            if (not mapExp) {
            	throw assert_exception("Conjunction::normalize: "
               	 "no mapExp for mapTerm in affineSubstMap");
        	}
        	// Barbara, you don't get to use this old substitute.
        	// Sub be setting up a substitute map.  But wait you
        	// already have one.  I am confused.  Going to replace this
        	// code with something that will work, but we need
        	// to do a code review so I understand what is going on.
        	//mapExp->substitute(tvExp->clone(),*tvTerm);
        	SubMap tmpMap;
        	tmpMap.insertPair(tvTerm->clone(), tvExp->clone());
        	mapExp->substitute(tmpMap);
        }

        // (3) add this equivalence to affineSubstMap
        affineSubstMap.insertPair(tvTerm,tvExp); // takes ownership of tvTerm and tvExp
    }
    
    // walk backward through original tuple vars
    TupleDecl retval2TupleDecl = conj->getTupleDecl(); 
    for (unsigned int i = indexStart-1; i > 0; i--) {
    	// check: if ISL tuple var is different from original tuple var
    	if (retval2TupleDecl.elemToString(i) == origTupleDecl.elemToString(i)) {
    	   continue;
    	}
    	
    	// if different, find expression for var i in terms of vars 0 - (i-1)
   		TupleVarTerm* tvTerm = new TupleVarTerm(i);
        Exp * tvExp = conj->findFunction(i,0,i-1);
    	
    	if (tvExp == NULL) {
            throw assert_exception("Conjunction::normalize: "
                "no expression for " + tvTerm->toString() + 
                " in Conjunction fromISL");
        }
//std::cout << "Conjunction::normalize: __tv" << i << " = " << tvExp->toString() << std::endl;
        
        // (2) substitute this equivalence (tvTerm = e) into affineSubstMap
        
        affineSubstMap.startIter();
        while (affineSubstMap.hasNext()) {
            Term* mapTerm = affineSubstMap.next();
            Exp* mapExp = affineSubstMap.subExp(mapTerm);
            if (not mapExp) {
            	throw assert_exception("Conjunction::normalize: "
               	 "no mapExp for mapTerm in affineSubstMap");
        	}
        	//mapExp->substitute(tvExp->clone(),*tvTerm);
        	SubMap tmpMap;
        	tmpMap.insertPair(tvTerm->clone(), tvExp->clone());
        	mapExp->substitute(tmpMap);
        }
        
        // (3) add this equivalence to affineSubstMap
        affineSubstMap.insertPair(tvTerm,tvExp); // takes ownership of tvTerm and tvExp
    }
    	

//std::cout << "Conjunction::normalize: affineSubstMap = " << affineSubstMap.toString() << std::endl;
                
        
    	    
    // (a-2) build up a map of non-affine information
    //       -- from the affine SubMap, and 
    //       -- from the ufcallmap (holding ufcalls/tempvars that
    //           we substituted back in (step 1)
    //
    SubMap nonAffineSubstMap;

    
    for (int i = indexStart; i < expandedArity; i++) {
    	TupleVarTerm* tvTerm = new TupleVarTerm(i);
    	UFCallTerm* origUFCall = ufcallmap.cloneUFCall(i);
		Exp* tvExp = new Exp();
		tvExp->addTerm(origUFCall);
		 
		// substitute affine results into original UFCall
		tvExp->substitute(affineSubstMap);
		
		// substitute non-affine results into original UFCall
		tvExp->substitute(nonAffineSubstMap);

		nonAffineSubstMap.insertPair(tvTerm, tvExp);
	}
	
	// (b) substituteInConstraints using non-affine SubMap

//std::cout << "Conjunction::normalize: nonAffineSubstMap = " << nonAffineSubstMap.toString() << std::endl;
//std::cout << "Conjunction::normalize: before subtitution = " << retval2->toString() << std::endl;	    	
    retval2->substituteInConstraints(nonAffineSubstMap);
//std::cout << "Conjunction::normalize: after  subtitution = " << retval2->toString() << std::endl;
//std::cout << "Conjunction::normalize: after  subtitution = " << retval2->prettyPrintString() << std::endl;
    
    // (c) remove extra tuple vars (since they have been "substituted"
    
    retval2->setTupleDecl(origTupleDecl);
    
//std::cout << "Conjunction::normalize: after tuple shrink = " << retval2->prettyPrintString() << std::endl;
    
    
    
    // Later Set::normalize will have to
    // union all of the returned Sets.

    
    delete constraints;
    delete selfcopy;
    return retval2;  
}

Set* Conjunction::normalizeR() const
{
    /////////////////////
    // (step 0) Group together all equality expressions that 
    // are parts of the same UFCallTerm, IOW i=f(k)[0] and 
    // j=f(k)[1] should become (i,j) = f(k).
    // FIXME: what about inequalities?
//std::cout << "Conjunction::normalizeR: original       = " << toString() << std::endl;
    Conjunction* selfcopy = new Conjunction(*this);
	TupleDecl origTupleDecl = selfcopy->getTupleDecl(); // for (step 3)
//std::cout << "Conjunction::normalizeR: selfcopy-bfour = " << selfcopy->toString() << std::endl;
    selfcopy->groupIndexedUFCalls();
//std::cout << "Conjunction::normalizeR: selfcopy-after = " << selfcopy->toString() << std::endl;
 
    /////////////////////
    // (step 1)
    // Replace all uninterpreted function calls with variables
    // and create a new affine conjunction that is a superset of the
    // current conjunction.  UFCallMapAndBounds object will
    // maintain the mapping of temporary variables to UF calls.
    UFCallMapAndBounds ufcallmap(getTupleDecl());
    selfcopy->ufCallsToTempVars( ufcallmap );
//std::cout << "Conjunction::normalize: selfcopy = " << selfcopy->toString() << std::endl;    
    // To test step 1, we return Set of constraints that have been
    // gathered in UFCallMapAndBounds.
    Set* constraints = ufcallmap.cloneConstraints();
//std::cout << "Conjunction::normalize: constraints = " << constraints->toString() << std::endl;    
//std::cout << "Conjunction::normalize: ufcallmap = " << ufcallmap.toString() << std::endl;
    // update the tuple declaration in selfcopy to match what is
    // in the constraints
    selfcopy->setTupleDecl( constraints->getTupleDecl() );
    
    // Also need to include all the constraints in selfcopy were UF calls
    // were replaced by temporaries.
    Set* selfcopyset = new Set(selfcopy->getTupleDecl());
    selfcopyset->addConjunction(selfcopy);
//std::cout << "Conjunction::normalize: selfcopyset = " << selfcopyset->toString() << std::endl;
    Set* retval1 = constraints->Intersect(selfcopyset);
//std::cout << "Conjunction::normalize: after intersect = , retval = " << retval1->toString() << std::endl;
    //return retval1;
    
    //////////////////////
    // (step 2) 
    // Send the result of step 1 to ISL and back.
    
    // (a) get string from result of step 1
    //std::string fromStep1 = retval->toISLString();
//std::cout << "Conjunction::normalize: fromStep1 = " << fromStep1 << std::endl;
 
    // (b) send through ISL
    //std::string fromISL = getRelationStringFromISL(fromStep1);
//std::cout << "Conjunction::normalize: fromISl = " << fromISL << std::endl;
    
    // (a & b) could do this as one statement (below), but broken apart (above) for testing
    string fromISL = getRelationStringFromISL(retval1->toISLString());
    
    //string fromISL = "[N, P] -> { [tstep, i, tstep, t, i, 0, t] : i >= 0 and t <= 0 and i <= -1 + N and t >= 1 - P }";

//std::cout << "Conjunction::normalize: fromISl = " << fromISL << std::endl;

    // (c) convert back to a Set
    Set* retval2 = iegenlib::parser::parse_set(fromISL);
    //Relation* retval2 = new Relation(fromISL);
//std::cout << "Conjunction::normalize: retval2 = " << retval2->toString() << std::endl;

    delete retval1;
    
    // Then in (step 3) we will need to use substitute to replace extra
    // tuple vars with UF calls by making queries to ufcallmap.  Will need
    // to call setTupleDecl on retval to remove those extra tuple vars that
    // acted as temporaries to replace uf calls.  Then return retval Set.
    
    /////////////////////
    // (step 3)
    // (a) create map of extra tuple vars to UF calls to send to substituteInConstrains
    
    SubMap substMap;
    
    // Determine starting index of extra tuple vars
    int numTempVars = ufcallmap.numTempVars();
    int expandedArity = constraints->arity();
    int indexStart = expandedArity - numTempVars;
    
    for (int i = indexStart; i < expandedArity; i++) {
    	TupleVarTerm* tvTerm = new TupleVarTerm(i);
    	UFCallTerm* origUFCall = ufcallmap.cloneUFCall(i);
		Exp* tvExp = new Exp();
		tvExp->addTerm(origUFCall);
		
		substMap.insertPair(tvTerm, tvExp);
	}
	
	// (b) substituteInConstraints

//std::cout << "Conjunction::normalize: substMap = " << substMap.toString() << std::endl;
//std::cout << "Conjunction::normalize: before subtitution = " << retval2->toString() << std::endl;	    	
    retval2->substituteInConstraints(substMap);
//std::cout << "Conjunction::normalize: after  subtitution = " << retval2->toString() << std::endl;
//std::cout << "Conjunction::normalize: after  subtitution = " << retval2->prettyPrintString() << std::endl;
    
    // (c) remove extra tuple vars (since they have been "substituted"
    
    retval2->setTupleDecl(origTupleDecl);
    
//std::cout << "Conjunction::normalize: after tuple shrink = " << retval2->prettyPrintString() << std::endl;
    
    
    
    // Later Set::normalize will have to
    // union all of the returned Sets.


    
    delete constraints;
    delete selfcopy;
    return retval2;  

}


/******************************************************************************/
#pragma mark -

/**************************** SparseConstraints *******************************/

SparseConstraints::SparseConstraints(){
}

SparseConstraints::SparseConstraints(const SparseConstraints& other) {
    *this = other;
}

SparseConstraints&
SparseConstraints::operator=(const SparseConstraints& other) {
    reset();
    for (std::list<Conjunction*>::const_iterator i=other.mConjunctions.begin();
                i != other.mConjunctions.end(); i++) {
        this->addConjunction(new Conjunction(**i));
    }
    this->cleanUp();
    return *this;
}

/*! Less than operator.
**      Compare two SparseConstraints in the following order:
**          1. by number of conjunctions: number of Conjunctions in mConjunctions
**          2. by sorted conjunctions list values: using Conjunction::operator<(Exp)
**      This operator assumes that the list of conjunctions (mConjunctions) and
**          is sorted for both SparseConstraints being compared.
** @param other, object to be compared
*/
bool SparseConstraints::operator<(const SparseConstraints& other) const {
    // 1. compare equality sizes
    if (mConjunctions.size() < other.mConjunctions.size()) { return true; }
    if (other.mConjunctions.size() < mConjunctions.size()) { return false; }

    // 2. compare sorted conjunctions list values
    std::list<Conjunction*>::const_iterator thisIter;
    std::list<Conjunction*>::const_iterator otherIter;
    otherIter = other.mConjunctions.begin();
    thisIter = mConjunctions.begin();
    
    while ( thisIter != mConjunctions.end() ) {
        //compare values
        if (**thisIter < **otherIter) { return true; }
        if (**otherIter < **thisIter) { return false; }
        //our mEqualities[i]'s are equal, continue
        ++thisIter;
        ++otherIter;
    }

    // All our elements are equal, so we are not less than other
    return false;
}

void SparseConstraints::reset() {
    for (std::list<Conjunction*>::iterator i=mConjunctions.begin();
                i != mConjunctions.end(); i++) {
        delete (*i);
    }
    mConjunctions.clear();
}

SparseConstraints::~SparseConstraints() {
    reset();
}

//! For all conjunctions, sets them to the given tuple declaration.
//! If there are some constants that don't agree then throws exception.
//! If replacing a constant with a variable ignores the substitution
//! in that conjunction.
void SparseConstraints::setTupleDecl( TupleDecl tuple_decl ) {
    for (std::list<Conjunction*>::iterator i=mConjunctions.begin();
            i != mConjunctions.end(); i++) {
        Conjunction* c = *i;
        c->setTupleDecl( tuple_decl );
    }
}

//! Returns a copy of the first conjunction's tuple declaration.
TupleDecl SparseConstraints::getTupleDecl(  ) const {
    return mConjunctions.front()->getTupleDecl();
}

//! For all conjunctions, sets them to the given tuple declarations.
//! If there are some constants that don't agree then throws exception.
//! If replacing a constant with a variable ignores the substitution
//! in that conjunction.
void SparseConstraints::setTupleDecl( TupleDecl tuple_decl_in, 
                                      TupleDecl tuple_decl_out ) {

    for (std::list<Conjunction*>::iterator i=mConjunctions.begin();
            i != mConjunctions.end(); i++) {
        Conjunction* c = *i;
        c->setTupleDecl( tuple_decl_in, tuple_decl_out);
    }
}


//! addConjunction
//! \param adoptedconjuction (adopted)
void SparseConstraints::addConjunction(Conjunction *adoptedConjunction) {
    // If the Set/Relation was created with only an arity or TupleDecl
    // then there will be a single empty conjunction to indicate TRUE.
    // No Conjunction indicates FALSE.  Remove the empty Conjunction.
    if (mConjunctions.size()==1 && ! mConjunctions.front()->hasConstraints()) {
        delete mConjunctions.front();
        mConjunctions.clear();
    }

    // Look through all conjunctions to determine where this one should go.
    // Maintain a sorted order.
    for (std::list<Conjunction*>::iterator i=mConjunctions.begin();
            i != mConjunctions.end(); i++) {
        Conjunction* c = *i;

        if (!(*adoptedConjunction < *c) && !(*c < *adoptedConjunction)) {
            //This conjunction already exists, no need to add it
            delete adoptedConjunction;
            return;
        }

        if (*adoptedConjunction < *c) {
            // We've found a term that belongs after the new one,
            // so insert the new term here, and we're done.
            mConjunctions.insert(i, adoptedConjunction);
            return;
        }
    }
    mConjunctions.push_back(adoptedConjunction);
}



std::string SparseConstraints::toString(int aritySplit) const {
    std::stringstream ss;

    // Print out all the conjunctions
    bool first = true;
    for (std::list<Conjunction*>::const_iterator i=mConjunctions.begin();
                i != mConjunctions.end(); i++) {
        if (not first) { ss << " union "; }
        ss << (*i)->toString();
        first = false;
    }

    // If there are no conjunctions then indicate we have an empty set
    // by printing out generic arity tuple declarations and FALSE as 
    // a constraint.
    if (mConjunctions.size()==0) {
        ss << "{ " 
           << TupleDecl::sDefaultTupleDecl(arity()).toString(true,aritySplit)
           << " : FALSE }";
    }

    return ss.str();
}

std::string SparseConstraints::prettyPrintString(int aritySplit) const {
    std::stringstream ss;

    bool first = true;
    for (std::list<Conjunction*>::const_iterator i=mConjunctions.begin();
                i != mConjunctions.end(); i++) {
        if (not first) { ss << " union "; }
        ss << (*i)->prettyPrintString();
        first = false;
    }

    // If there are no conjunctions then indicate we have an empty set
    // by printing out generic arity tuple declarations and
    // FALSE as a constraint.
    // FIXME: refactor with identical code in SparseConstraints::toString
    if (mConjunctions.size()==0) {
        TupleDecl genericTupleDecl(arity());
        for (int i=0; i<arity(); i++) {
            std::stringstream varname;
            varname << "tv" << i;
            genericTupleDecl.setTupleElem(i,varname.str());
        }
        ss << "{ " << genericTupleDecl.toString(true,aritySplit)
           << " : FALSE }";
    }

    return ss.str();
}

std::string SparseConstraints::toISLString(int aritySplit) const {

    // collect all symbolic/parameter variable names
    // and print the declaration for the symbolics
    std::stringstream ss;
    StringIterator * symIter;
    bool foundSymbols = false;
    for (std::list<Conjunction*>::const_iterator i=mConjunctions.begin();
            i != mConjunctions.end(); i++) {
        symIter = (*i)->getSymbolIterator();
        while (symIter->hasNext()) {
            // print out start of symbol declaration at first symbol
            if (foundSymbols == false) {
                ss << "[ ";
                foundSymbols = true;
                // print the symbol declaration itself
                ss << symIter->next();

            // later symbols will have a comma and then var name
            } else {
                ss << ", " << symIter->next();
            }
        }
        delete symIter;
    }
    // finish off declaration if there were symbols
    if (foundSymbols) {
        ss << " ] -> ";
    }

    // do a typical prettyPrint
    ss << prettyPrintString(aritySplit);

    return ss.str();
}


std::string SparseConstraints::toDotString() const{
    std::stringstream result;
    int self_id = 0;
    int next_id = 1;
    for (std::list<Conjunction*>::const_iterator i=mConjunctions.begin();
            i != mConjunctions.end(); i++) {
        result << (*i)->toDotString(self_id, next_id);
        next_id++;
    }
    return result.str();
}

//! Get an iterator over the tuple variables, in order.
// Just grabs the tuple variable names in the first conjunction.
StringIterator* SparseConstraints::getTupleIterator() const {
    if (mConjunctions.size() < 1) {
        throw assert_exception("SparseConstraints::getTupleIterator: "
                               "no conjunctions");
    }
    return mConjunctions.front()->getTupleIterator();
}

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
SparseConstraints::substituteInConstraints(SubMap& searchTermToSubExp) {
        
    searchTermToSubExp.startIter();
    while ( searchTermToSubExp.hasNext() ) {
        Term* search = searchTermToSubExp.next();
        if (search->isConst()) {
            throw assert_exception("SparseConstraints::substituteInConstraints:"
                " at least one Term in map is a constant, not allowed");
        }
    }

    // Each conjunction should do it's own substitution.
    for (std::list<Conjunction*>::const_iterator i=mConjunctions.begin();
                i != mConjunctions.end(); i++) {
        (*i)->substituteInConstraints(searchTermToSubExp);
    }
}



bool _compareConjunctions( Conjunction* first, Conjunction* second ) {
    return *first < *second;
}

//! Remove duplicate constraints and trivial constraints
// FIXME: need a case where affine constraints within the conjunction
// make the conjunction unsatisfiable.  Should deal with this in release 2
// and use ISL.
void SparseConstraints::cleanUp(){
    // removes duplicate constraints, FIXME: does it also resort constraints?
    for (std::list<Conjunction*>::const_iterator i=mConjunctions.begin();
                i != mConjunctions.end(); i++) {
        (*i)->cleanUp();
    }
    // remove conjunctions that are not satisfiable
    for (std::list<Conjunction*>::iterator i=mConjunctions.begin();
                i != mConjunctions.end(); ) {
        if (not (*i)->satisfiable()) {
            delete (*i);
            mConjunctions.erase(i++);
        } else {
            i++;
        }
    }

    //sort the list of conjunctions
    mConjunctions.sort(_compareConjunctions);

}

// Iterate over all conjunctions and normalize each conjunction.
// Then call cleanup to resort things?
void SparseConstraints::normalize() {
    //Set* normalized_set = new Set(arity());

    // FIXME: just assuming one conjunction right now.
    if (mConjunctions.size()!=1) {
        throw assert_exception("SparseConstraints::normalize: "
            "currently only handle one Conjunction Sets/Relations");
    }
    Conjunction* conj = mConjunctions.front();
    Set* result = conj->normalize();
    *this =  *result;
    delete result;
/*  FIXME: not ready to do this yet.  Need to get all parts
    of normalization working.   
    // normalize each conjunction and union together returned sets
    for (std::list<Conjunction*>::iterator i=mConjunctions.begin();
                i != mConjunctions.end(); i++) {
std::cout << "normalized_set = " << normalized_set->toString() << std::endl;
        Set* result_set = (*i)->normalize();
std::cout << "result_set = " << result_set->prettyPrintString() << std::endl;
        Set* union_result = normalized_set->Union(result_set);
        delete result_set;
        delete normalized_set;
        normalized_set = union_result;
    }
*/
    // replace self with this normalized set
    // What if we are a relation?
    //*this = *normalized_set;
    
    // FIXME: might need to re-sort Conjunctions here and determine
    // if any are equivalent.
}

/*! Find any TupleVarTerms in this expression (and subexpressions)
**  and remap the locations according to the oldToNewLocs vector,
**  where oldToNewLocs[i] = j means that old location i becomes new
**  location j (i.e. __tvi -> __tvj).  Throws an exception if an
**  old location is out of range for the given oldToNewLocs.
**  The new location will be -1 for old locations that are not being
**  remapped.  For example some might be constants in the TupleDecl.
*/
void SparseConstraints::remapTupleVars(const std::vector<int>& oldToNewLocs) {
    for (std::list<Conjunction*>::iterator iter=mConjunctions.begin();
            iter != mConjunctions.end(); iter++) {
        (*iter)->remapTupleVars(oldToNewLocs);
    }
}

/******************************************************************************/
#pragma mark -

/********************************** Set ***************************************/
Set::Set(std::string str) {
    Set* s = parser::parse_set(str); // parse string
    *this = *s; // copy created Set
    delete s;
}

//! Creates a set with the specified arity.  It starts with no constraints
//! so all tuples of that arity belong in it.
Set::Set(int arity) : SparseConstraints(), mArity(arity) {
    addConjunction(new Conjunction(TupleDecl::sDefaultTupleDecl(arity)));
}

//! Creates a set with the specified tuple declaration.  
//! It starts with no constraints so all tuples of that arity belong in it.
Set::Set(TupleDecl tdecl) : SparseConstraints(), mArity(tdecl.size()) {
    addConjunction(new Conjunction(tdecl));
}

Set::Set(const Set& other) {
    *this = other;
}
Set& Set::operator=(const Set& other) {
    SparseConstraints::operator=(other);
    mArity = other.mArity;
    return *this;
}
/*! Equal operator.
** 
** @param other, object to be compared
*/
bool Set::operator==( const Set& other) const {
    if ((*this) < other){ return false; }
    if ( other < (*this)){ return false; }
    return true;
}

/*! Less than operator.
**      Compare two Set in the following order:
**          1. by arity: mArity
**          2. by Conjunctions: using SparseConstraints::operator<(Exp)
** @param other, object to be compared
*/
bool Set::operator<( const Set& other) const {
    if ((*this).mArity < other.mArity){ return true; }
    if ((*this).mArity > other.mArity){ return false; }
    return (static_cast<SparseConstraints>(*this) < static_cast<SparseConstraints>(other));
}

Set::~Set() {
}

std::string Set::toDotString() const{
    std::stringstream result;
    result << "digraph SparseConstraints {\n";
    result << 0 << " [label = \"Set\\nmArity=" << mArity << "\"];\n";
    result << SparseConstraints::toDotString();
    result << "}\n";
    return result.str();
}

//! For all conjunctions, sets them to the given tuple declaration.
//! If there are some constants that don't agree then throws exception.
//! If replacing a constant with a variable ignores the substitution
//! in that conjunction.
//! Also modifies arity to match.
void Set::setTupleDecl( TupleDecl tuple_decl ) {
    mArity = tuple_decl.size();
    SparseConstraints::setTupleDecl(tuple_decl);
}

/*! Union this set with the given other one
**    (i.e., this Union rhs).  Returns a new Set,
**    which the caller is responsible for deallocating.
** \param rhs (not adopted)
*/
Set* Set::Union(const Set* rhs) const{
    // Check that the arities are compatible.
    if (rhs->mArity != mArity) {
        throw assert_exception("Set::Union: mismatched arities");
    }

    Set *result = new Set(mArity);
    
    // If both sets have no conjunctions then they are both empty sets
    // and so this set needs to be empty as well.  Removing default
    // conjunction before adding in conjunctions of other sets.
    if (result->mConjunctions.size()==1 
            && ! result->mConjunctions.front()->hasConstraints()) {
        delete result->mConjunctions.front();
        result->mConjunctions.clear();
    }
    
    // Add in conjunctions from lhs/this set.
    for (std::list<Conjunction*>::const_iterator i=mConjunctions.begin();
        i != mConjunctions.end(); i++) {
        result->addConjunction(new Conjunction(**i));
    }

    // Add in conjunctions from other set.
    for (std::list<Conjunction*>::const_iterator i=rhs->mConjunctions.begin();
        i != rhs->mConjunctions.end(); i++) {
        result->addConjunction(new Conjunction(**i));
    }

    return result;
}

/*! Intersect this set with the given other one
**    (i.e., this Intersect rhs).  Returns a new Set,
**    which the caller is responsible for deallocating.
** \param rhs (not adopted)
*/
Set* Set::Intersect(const Set* rhs) const{
    // Check that the arities are compatible.
    if (rhs->mArity != mArity) {
        throw assert_exception("Set::Intersect: mismatched arities");
    }

    Set *result = new Set(mArity);

    // Have to do cross product intersection between conjunctions in both sets.
    for (std::list<Conjunction*>::const_iterator i=mConjunctions.begin();
            i != mConjunctions.end(); i++) {

        for (std::list<Conjunction*>::const_iterator
                j=rhs->mConjunctions.begin();
                j != rhs->mConjunctions.end(); j++) {

            result->addConjunction((*i)->Intersect(*j));
        }
    }

    return result;
}

/*! Treating this Set like a domain or range.  Creates
** a new set where passed in tuple expression is
** bound assuming this domain, or range.
** User must deallocate returned Set.
** 
** \param tuple_exp Expression tuple to bound.  Could just have one elem.
**
** \return Set will contain all bounds on expressions in tuple expression.
*/
Set* Set::boundTupleExp(const TupleExpTerm& tuple_exp) const {
    Set *result = new Set(0);

    // Have to pull bounds from all conjunctions.
    for (std::list<Conjunction*>::const_iterator i=mConjunctions.begin();
            i != mConjunctions.end(); i++) {
        result->addConjunction((*i)->boundTupleExp(tuple_exp));
    }

    return result;
}


/******************************************************************************/
#pragma mark -

/******************************** Relation ************************************/
Relation::Relation(std::string str) {
    Relation* r = parser::parse_relation(str); // parse relation
    *this = *r; // copy created Relation
    delete r;
}

Relation::Relation(int inArity, int outArity)
  : SparseConstraints(), mInArity(inArity), mOutArity(outArity) {
}

Relation::Relation(const Relation& other) {
    *this = other;
}

Relation& Relation::operator=(const Relation& other) {
    mInArity = other.mInArity;
    mOutArity = other.mOutArity;
    SparseConstraints::operator=(other);
    return *this;
}

// copies conjunctions from the Set, but keeps *this's mInArity
// Checks for feasibility of mInArity in relation to other's mTupleDecl.size()
Relation& Relation::operator=(const Set& other) {

	if (mInArity >= other.arity()) {
        throw assert_exception("Relation::operator=(Set): impossible arity match");
	}
	mOutArity = other.arity() - mInArity;
	
    reset();
    for (std::list<Conjunction*>::const_iterator i=other.mConjunctions.begin();
                i != other.mConjunctions.end(); i++) {
        // copy Set's conjunction with Relation's inArity     
        Conjunction* c = new Conjunction((*i)->arity(), mInArity);
        c->setTupleDecl((*i)->getTupleDecl());
        c->copyConstraintsFrom(*i);
        // add it to Relation
        this->addConjunction(c);
    }

    this->cleanUp();
    return *this;
}

/*! Equals operator.
**      if self < other return false
**      if other < self return false
**      return true
**   
** @param other, object to be compared
*/   
bool Relation::operator==( const Relation& other) const {

    if( (*this) < other){
        return false;
    }
    if( other < (*this)){
        return false;
    }
    return true;
}

/*! Less than operator.
**      Compare two Relation in the following order:
**          1. by in arity: mInArity
**          2. by out arity: mOutArity
**          2. by Conjunctions: using SparseConstraints::operator<(Exp)
** @param other, object to be compared
*/
bool Relation::operator<( const Relation& other) const {
    //In arity
    if ((*this).mInArity < other.mInArity){ return true; }
    if ((*this).mInArity > other.mInArity){ return false; }
    //Out arity
    if ((*this).mOutArity < other.mOutArity){ return true; }
    if ((*this).mOutArity > other.mOutArity){ return false; }
    //Conjunctions
    return (static_cast<SparseConstraints>(*this) < 
            static_cast<SparseConstraints>(other));
}

Relation::~Relation() {
}

std::string Relation::toString() const {
    return SparseConstraints::toString(mInArity);
}

std::string Relation::prettyPrintString() const {
    return SparseConstraints::prettyPrintString(mInArity);
}

std::string Relation::toDotString() const{
    std::stringstream result;
    result << "digraph SparseConstraints {\n";
    result << 0 << " [label = \"Relation\\nmInArity=" << mInArity;
    result << "\\nmOutArity=" << mOutArity << "\"];\n";
    result << SparseConstraints::toDotString();
    result << "}\n";
    return result.str();
}

//! For all conjunctions, sets them to the given tuple declaration.
//! If there are some constants that don't agree then throws exception.
//! If replacing a constant with a variable ignores the substitution
//! in that conjunction.
//! Also modifies arity to match.
void Relation::setTupleDecl( TupleDecl tuple_decl ) {
    // Only modify the output arity.
    mOutArity = tuple_decl.size() - mInArity;
    SparseConstraints::setTupleDecl(tuple_decl);
}

/*! Compose this relation with the given other one
**    (i.e., this Compose rhs).  Returns a new Relation,
**    which the caller is responsible for deallocating.
*/
Relation *Relation::Compose(const Relation *rhs) const {
    // Check that the arities are compatible.
    if (rhs->mOutArity != mInArity) {
        throw assert_exception("Relation::Compose: mismatched arities");
    }
    // For convenience, remember this inner arity that matches
    // the RHS out arity and the LHS in arity.  This is used
    // below while composing the conjunctions.
    int innerArity = rhs->mOutArity;

    Relation *result = new Relation(rhs->mInArity, mOutArity);

    // Find all combinations of LHS and RHS conjunctions.
    for (std::list<Conjunction*>::const_iterator lhsIter=mConjunctions.begin();
                lhsIter != mConjunctions.end(); lhsIter++) {
        for (std::list<Conjunction*>::const_iterator
                rhsIter=rhs->mConjunctions.begin();
                rhsIter != rhs->mConjunctions.end(); rhsIter++) {
            Conjunction *combo = (*lhsIter)->Compose(*rhsIter, innerArity);
            if (combo) { result->addConjunction(combo); }
        } // next RHS conjunction
    } // next LHS conjunction

    result->cleanUp();
    return result;
}

/*  Apply this relation to the given set. Returns a new Set,
    which the caller is responsible for deallocating.

    Apply is almost exactly like Compose except that now the
    rhs is a set.

*/
Set *Relation::Apply(const Set* rhs) const {
    // Check that the arities are compatible.
    if (rhs->arity() != mInArity) {
       throw assert_exception("Relation::Apply: mismatched arities");
    }

    Set *result = new Set(mOutArity);

    // Find all combinations of LHS relation and RHS set conjunctions.
    for (std::list<Conjunction*>::const_iterator lhsIter=mConjunctions.begin();
                lhsIter != mConjunctions.end(); lhsIter++) {
        for (std::list<Conjunction*>::const_iterator
                rhsIter=rhs->conjunctionBegin();
                rhsIter != rhs->conjunctionEnd(); rhsIter++) {
            Conjunction *combo = (*lhsIter)->Apply(*rhsIter);
            if (combo) { result->addConjunction(combo); }
        } // next RHS conjunction
    } // next LHS conjunction

    result->cleanUp();
    return result;
}


/*! Union this relation with the given other one
**    (i.e., this Union rhs).  Returns a new Relation,
**    which the caller is responsible for deallocating.
** \param rhs (not adopted)
*/
Relation* Relation::Union(const Relation* rhs) const{
    // Check that the arities are compatible.
    if (rhs->mInArity != mInArity || rhs->mOutArity != mOutArity) {
        throw assert_exception("Relation::Union: mismatched arities");
    }

    Relation *result = new Relation(mInArity, mOutArity);

    for (std::list<Conjunction*>::const_iterator i=mConjunctions.begin();
        i != mConjunctions.end(); i++) {
        result->addConjunction(new Conjunction(**i));
    }

    for (std::list<Conjunction*>::const_iterator i=rhs->mConjunctions.begin();
        i != rhs->mConjunctions.end(); i++) {
        result->addConjunction(new Conjunction(**i));
    }
    //result->cleanUp();  FIXME: might want later when cleanup can merge
    //constraints that have adjacent constraints
    return result;
}

/*! Intersect this relation with the given other one
**    (i.e., this Intersect rhs).  Returns a new Relation,
**    which the caller is responsible for deallocating.
** \param rhs (not adopted)
*/
Relation* Relation::Intersect(const Relation* rhs) const{
    // Check that the arities are compatible.
    if (rhs->mInArity != mInArity || rhs->mOutArity != mOutArity) {
        throw assert_exception("Relation::Union: mismatched arities");
    }

    Relation *result = new Relation(mInArity, mOutArity);

    // Have to do cross product intersection between conjunctions in both sets.
    for (std::list<Conjunction*>::const_iterator i=mConjunctions.begin();
            i != mConjunctions.end(); i++) {

        for (std::list<Conjunction*>::const_iterator
                j=rhs->mConjunctions.begin();
                j != rhs->mConjunctions.end(); j++) {

            result->addConjunction((*i)->Intersect(*j));
        }
    }

    //result->cleanUp();  FIXME: might want later when cleanup can merge
    //constraints that have adjacent constraints
    return result;
}


/*! Inverse this relation. Returns a new Relation,
**    which the caller is responsible for deallocating.
*/
Relation* Relation::Inverse() const{
    Relation *result = new Relation(mOutArity, mInArity);

    // Compute the inverse of each Conjunction.
    for (std::list<Conjunction*>::const_iterator i=mConjunctions.begin();
        i != mConjunctions.end(); i++) {
        result->addConjunction((*i)->Inverse());
    }

    result->cleanUp();

    return result;
}

/*! Determine whether all of the outputs can be determined as
**  functions of the inputs.  Need to check for each conjunction.
*/
bool Relation::isFunction() const {
    bool result = true;
    
    // Each conjunction must be a function
    for (std::list<Conjunction*>::const_iterator i=mConjunctions.begin();
        i != mConjunctions.end(); i++) {
        result = result && (*i)->isFunction(inArity());
    }
    
    return result;
}

/*! Determine whether all of the inputs can be determined as
**  functions of the outputs.  Need to check for each conjunction.
*/
bool Relation::isFunctionInverse() const {
    bool result = true;
    
    // Each conjunction must be a function
    for (std::list<Conjunction*>::const_iterator i=mConjunctions.begin();
        i != mConjunctions.end(); i++) {
        result = result && (*i)->isFunctionInverse(inArity());
    }
    
    return result;
}


/*! Return the expression that describes the value of the tupleLoc
*   specified as a function of the tuple locations in the start
*   through the stop locations.
*/
Exp* Relation::findFunction(int tupleLocToFind,
                            int startTupleRange, int endTupleRange)
{
    // This should only be called if there is a single Conjunction.
    if(mConjunctions.size() != 1 ){
        throw assert_exception("more than one Conjunction in Relation");
    }

    return mConjunctions.front()->findFunction(tupleLocToFind,
            startTupleRange, endTupleRange);
}

void Relation::addConjunction(Conjunction *adoptedConjunction) {
    if (adoptedConjunction->inarity() != mInArity) {
        throw assert_exception("Relation::addConjunction: mismatched arities");
    }
    SparseConstraints::addConjunction(adoptedConjunction);
}

// Iterate over all conjunctions and normalize each conjunction.
// Then call cleanup to resort things?
void Relation::normalize() {

    // FIXME: ?? essentially the same as SparseConstraints::normalize(), but 
    // forces call to Relation::operator=(Set) at the statement:   *this = *result;

    // FIXME: just assuming one conjunction right now.
    if (mConjunctions.size()!=1) {
        throw assert_exception("Relation::normalize: "
            "currently only handle one Conjunction Sets/Relations");
    }

    Conjunction* conj = mConjunctions.front();
    Set* result = conj->normalize();
    //Set* result = conj->normalizeR();
    *this =  *result;
    delete result;
    
    // FIXME: will need more ... See SparseConstraints::normalize()
}

/******************************************************************************/

}//end namespace iegenlib
