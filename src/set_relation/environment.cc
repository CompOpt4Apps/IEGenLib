/*!
 * \file environment.cc
 *
 * \brief Implementations of the Environment class.
 *
 * \date Started: 5/1/12
 * # $Revision:: 739                $: last committed revision
 * # $Date:: 2013-08-06 02:21:28 -0#$: date of last committed revision
 * # $Author:: mstrout              $: author of last committed revision
 *
 * \authors Michelle Strout and Joe Strout
 *
 * Copyright (c) 2012, Colorado State University <br>
 * All rights reserved. <br>
 * See ../../COPYING for details. <br>
 */

#include "environment.h"
#include "set_relation.h"

namespace iegenlib{

Environment currentEnv;

//! Resets the current environment to empty.
void setCurrEnv() {
    currentEnv.reset();
}

//! Resets the current environment to empty and then accepts new UninterpFunc
//! declaration into new environment.
void setCurrEnv(std::string funcName, Set* domain, Set* range,
                bool bijective, MonotonicType monoType) {
    currentEnv.reset();
    appendCurrEnv(funcName, domain, range, bijective, monoType);
}

/*
//! Resets the current environment to empty and then parses the string
//! to create a new environment.
//! Release 1 must be of the form "f() = inverse f'()"
//! where f and f' can be any identifier.
void setCurrEnv(std::string str) {
    // parse environment
    Environment* env = parser::parse_env(str); 
    // copy the contents of the map over to our map
    currentEnv = *env;
    //delete the Environment
    delete env;
}
*/


void appendCurrEnv(std::string funcName, Set* domain, Set* range, 
                   bool bijective, MonotonicType monoType) {
  // create a new uninterpreted function description
  UninterpFunc* ufunc = new UninterpFunc(funcName, domain, range, 
                                           bijective, monoType);
  // create new environment
  Environment* env = new Environment(ufunc);
  currentEnv.append(env);

/* Adding quantified rules for functional consistency and monotonicity */
  if( domain->getArity() == 1 && range->getArity() == 1 ){

    UniQuantRule *uqRule;
    string type, tupleDecl, leftSide, rightSide;
/*
    // 3.1.1 adding Functional consistency rules:
    //   forall e1, e2 : if e1 = e2 then f(e1) = f(e2)
    type = ("FuncConsistency");
    tupleDecl = ("[e1,e2]");
    leftSide = ( "e1 = e2" );
    rightSide = ( funcName + "(e1) = " + funcName + "(e2)" );
    uqRule = new UniQuantRule(type, tupleDecl, leftSide, rightSide);
    currentEnv.addUniQuantRule( uqRule );
*/
    // 3.1.2 Adding Monotonicity rules based on:
    //   If UF monotonically strictly increasing then:
    if ( Monotonic_Increasing == monoType ){
  
      // forall e1, e2 : e1 < e2 => UF(e1) < UF(e2)
      type = ("Monotonicity");
      tupleDecl = ("[e1,e2]");
      leftSide = ( "e1 < e2" );
      rightSide = ( funcName + "(e1) < " + funcName + "(e2)" );
      uqRule = new UniQuantRule(type, tupleDecl, leftSide, rightSide);
      currentEnv.addUniQuantRule( uqRule );
/*
      // forall e1, e2 : UF(e1) = UF(e2) => e1 = e2
      type = ("Monotonicity");
      tupleDecl = ("[e1,e2]");
      leftSide = ( funcName + "(e1) = " + funcName + "(e2)" );
      rightSide = ( "e1 = e2" );
      uqRule = new UniQuantRule(type, tupleDecl, leftSide, rightSide);
      currentEnv.addUniQuantRule( uqRule );
*/
      // forall e1, e2 : UF(e1) < UF(e2) => e1 < e2
      type = ("Monotonicity");
      tupleDecl = ("[e1,e2]");
      leftSide = ( funcName + "(e1) < " + funcName + "(e2)" );
      rightSide = ( "e1 < e2" );
      uqRule = new UniQuantRule(type, tupleDecl, leftSide, rightSide);
      currentEnv.addUniQuantRule( uqRule );
/*
      // forall e1, e2 : UF(e1) <= UF(e2) => e1 <= e2
      type = ("Monotonicity");
      tupleDecl = ("[e1,e2]");
      leftSide = ( funcName + "(e1) <= " + funcName + "(e2)" );
      rightSide = ( "e1 <= e2" );
      uqRule = new UniQuantRule(type, tupleDecl, leftSide, rightSide);
      currentEnv.addUniQuantRule( uqRule );
*/
    //   If UF monotonically increasing then:
    } else  if ( Monotonic_Nondecreasing == monoType ){

      // forall e1, e2 : e1 < e2 => UF(e1) <= UF(e2)
      type = ("Monotonicity");
      tupleDecl = ("[e1,e2]");
      leftSide = ( "e1 < e2" );
      rightSide = ( funcName + "(e1) <= " + funcName + "(e2)" );
      uqRule = new UniQuantRule(type, tupleDecl, leftSide, rightSide);
      currentEnv.addUniQuantRule( uqRule );
  
    // If UF monotonically strictly decreasing then
    } else if ( Monotonic_Decreasing == monoType ){
  
      // forall e1, e2 : e1 < e2 => UF(e1) > UF(e2)
      type = ("Monotonicity");
      tupleDecl = ("[e1,e2]");
      leftSide = ( "e1 < e2" );
      rightSide = ( funcName + "(e1) > " + funcName + "(e2)" );
      uqRule = new UniQuantRule(type, tupleDecl, leftSide, rightSide);
      currentEnv.addUniQuantRule( uqRule );
/*
      // forall e1, e2 : UF(e1) = UF(e2) => e1 = e2
      type = ("Monotonicity");
      tupleDecl = ("[e1,e2]");
      leftSide = ( funcName + "(e1) = " + funcName + "(e2)" );
      rightSide = ( "e1 = e2" ); 
      uqRule = new UniQuantRule(type, tupleDecl, leftSide, rightSide);
      currentEnv.addUniQuantRule( uqRule );
*/
      // forall e1, e2 : UF(e1) < UF(e2) => e1 > e2
      type = ("Monotonicity");
      tupleDecl = ("[e1,e2]");
      leftSide = ( funcName + "(e1) < " + funcName + "(e2)" );
      rightSide = ( "e1 > e2" );
      uqRule = new UniQuantRule(type, tupleDecl, leftSide, rightSide);
      currentEnv.addUniQuantRule( uqRule );
/*
      // forall e1, e2 : UF(e1) <= UF(e2) => e1 >= e2
      type = ("Monotonicity");
      tupleDecl = ("[e1,e2]");
      leftSide = ( funcName + "(e1) <= " + funcName + "(e2)" );
      rightSide = ( "e1 >= e2" );
      uqRule = new UniQuantRule(type, tupleDecl, leftSide, rightSide);
      currentEnv.addUniQuantRule( uqRule );
*/
      // If UF monotonically decreasing then:
    } else  if ( Monotonic_Nonincreasing == monoType ){

      // forall e1, e2 : e1 < e2 => UF(e1) <= UF(e2)
      type = ("Monotonicity");
      tupleDecl = ("[e1,e2]");
      leftSide = ( "e1 < e2" );
      rightSide = ( funcName + "(e1) >= " + funcName + "(e2)" );
      uqRule = new UniQuantRule(type, tupleDecl, leftSide, rightSide);
      currentEnv.addUniQuantRule( uqRule );
    }
  }
/////////////////////////////////////////////////////////////////
}
/*
void appendCurrEnv(std::string str) {
    // parse the environment
    Environment* env = parser::parse_env(str);
    currentEnv.append(env);
}
*/

std::string queryInverseCurrEnv(const std::string funcName){
    return currentEnv.funcInverse(funcName);
}

//! search this environment for a function domain
//! returned Set is a clone
Set* queryDomainCurrEnv(const std::string funcName) {
    Set* retval = currentEnv.funcDomain(funcName);
    if (retval==NULL) {
        std::stringstream ss;
        ss << "queryDomainCurrEnv: the function " << funcName;
        ss << " has not been declared in the current environment.";
        throw assert_exception(ss.str());
    }
    return retval; 
}

//! search this environment for a function range
//! returned Set is a clone
Set* queryRangeCurrEnv(const std::string funcName) {
    Set* retval = currentEnv.funcRange(funcName);
    if (retval==NULL) {
        std::stringstream ss;
        ss << "queryRangeCurrEnv: the function " << funcName;
        ss << " has not been declared in the current environment.";
        throw assert_exception(ss.str());
    }
    return retval; 
}

//! search this environment for a function monotonicity type
MonotonicType queryMonoTypeEnv(const std::string funcName) {
    return currentEnv.funcMonoType(funcName);
}



//! search this environment for a function range arity
unsigned int queryRangeArityCurrEnv(const std::string funcName) {
    Set* range = queryRangeCurrEnv(funcName);
    unsigned int retval = range->arity();
    delete range;
    return retval;
}


//! add an universially quantified Rule to environment
void addUniQuantRule(UniQuantRule *uqRule){
    currentEnv.addUniQuantRule(uqRule);
}

// Return the number of available universially quantified Rules
int queryNoUniQuantRules(){
  return currentEnv.getNoUniQuantRules();
}

// Returns the universially quantified Rule No. idx stored in the enviroment
//! The environment still owns returned object (user should not delete it)
UniQuantRule* queryUniQuantRuleEnv(int idx){
  return currentEnv.getUniQuantRule(idx);
}

void Environment::append(Environment *other){
    mInverseMap.insert(other->mInverseMap.begin(),other->mInverseMap.end());
    // Need to do a deep copy of the UninterpFunc objects
    for (std::map<std::string, UninterpFunc*>::iterator     
            it=other->mUninterpFuncMap.begin(); 
            it!=other->mUninterpFuncMap.end(); it++) {
        // checking for duplicates
        if (mUninterpFuncMap.find(it->first)!=mUninterpFuncMap.end()) {
            throw assert_exception("Environment::append: attempting "
                    "to append duplicate UFs.");
        }
        mUninterpFuncMap[it->first] = new UninterpFunc(*(it->second));
    }
    delete other;   
}


//! Construct an Environment 
Environment::Environment(UninterpFunc* symfunc){
    // If we don't already know about this function.
    if (mUninterpFuncMap.find(symfunc->getName())==mUninterpFuncMap.end()) {
        mUninterpFuncMap[symfunc->getName()] = symfunc;
    } else {
        throw assert_exception("Environment::Environment: "
                               "UninterpFunc of same name already exists.");
    }
    
    // determine if we should create an inverse
    if (symfunc->isBijective()) {
        std::string inv_name = symfunc->getName() + "_inv";
        if (mUninterpFuncMap.find(inv_name)==mUninterpFuncMap.end()) {
            mUninterpFuncMap[inv_name] = new UninterpFunc(inv_name,
                new Set(*(symfunc->getRange())), 
                new Set(*(symfunc->getDomain())), true,Monotonic_NONE);
                // FIXME: now just setting to Monotonic_NONE but should
                // determine inverse of other function.
            mInverseMap[symfunc->getName()] = inv_name;
            mInverseMap[inv_name] = symfunc->getName();
        } else {
            throw assert_exception("Environment::Environment: "
                                   "UninterpFunc of same name already exists "
                                   "for automatically created inverse");
        }
    }
}

//! Assignment operator for Environment.
Environment& Environment::operator=(const Environment& other) {
    for (std::map<std::string, std::string>::const_iterator     
            it=other.mInverseMap.begin(); it!=other.mInverseMap.end(); it++) {
        mInverseMap[it->first] = it->second;
    }
    return *this;
}

Environment::~Environment() {
    // delete all UninterpFunc declarations
    for (std::map<std::string, UninterpFunc*>::iterator     
            it=mUninterpFuncMap.begin(); it!=mUninterpFuncMap.end(); it++) {
        delete it->second;
    }
    mUninterpFuncMap.clear();
    // delete all UninterpFunc declarations
    mInverseMap.clear();
}

// Reset the Environment to empty
void Environment::reset(){
    // delete all UninterpFunc declarations
    for (std::map<std::string, UninterpFunc*>::iterator     
            it=mUninterpFuncMap.begin(); it!=mUninterpFuncMap.end(); it++) {
        delete it->second;
    }
    mUninterpFuncMap.clear();
    // delete all UninterpFunc declarations
    mInverseMap.clear();
}

// Define the inverse for the given function.
void Environment::setInverse(std::string funcName, std::string inverseName) {
    mInverseMap[funcName] = inverseName;
    mInverseMap[inverseName] = funcName;
}

// Get the name of the inverse of the given function (or "" if none).
std::string Environment::funcInverse(const std::string funcName) const {
    if (mInverseMap.find(funcName) == mInverseMap.end()) return "";
    return mInverseMap.find(funcName)->second;
}

// Return a clone of the domain of the function.
Set* Environment::funcDomain(const std::string funcName) const {
    if (mUninterpFuncMap.find(funcName) == mUninterpFuncMap.end()) return NULL;
    return new Set(*(mUninterpFuncMap.find(funcName)->second->getDomain()));
}

// Return a clone of the domain of the function.
Set* Environment::funcRange(const std::string funcName) const {
    if (mUninterpFuncMap.find(funcName) == mUninterpFuncMap.end()) return NULL;
    return new Set(*(mUninterpFuncMap.find(funcName)->second->getRange()));
}

//! Returns whether a function is monotonistic or not and how.
MonotonicType Environment::funcMonoType(const std::string funcName) const {
    if (mUninterpFuncMap.find(funcName) == mUninterpFuncMap.end()) {
        return Monotonic_NONE;
    }
    return mUninterpFuncMap.find(funcName)->second->getMonoType();
}


std::string Environment::toString() const {
    std::stringstream ss;
    ss << "Environment:" << std::endl;
    
    std::set<std::string> keyset;
    
    // We have to visit the entries in the same order every time for testing.
    // Therefore, will put keys into a set and then print while
    // iterating over the set.
    for (std::map<std::string, std::string>::const_iterator     
            it=mInverseMap.begin(); it!=mInverseMap.end(); it++) {
        keyset.insert(it->first);
    }
    // iterating over the set
    for (std::set<std::string>::const_iterator     
            it=keyset.begin(); it!=keyset.end(); it++) {
        ss  << "    " << *it << " ==> " 
            << (mInverseMap.find(*it))->second << std::endl;
    }
    return ss.str();
}



//! Add an universially quantified Rule to the environment
void Environment::addUniQuantRule(UniQuantRule *uqRule){
    uniQuantRules.push_back (uqRule);
}

//! Get the No. of universially quantified Rules
// available in the environment
int Environment::getNoUniQuantRules(){
    return uniQuantRules.size();
}

//! Get the universially quantified Rule No. idx from 
// the environment
UniQuantRule* Environment::getUniQuantRule(int idx){
    return uniQuantRules[idx];
}


//! UniQuantRule class member functions:

UniQuantRule::UniQuantRule(string type, string tupleDecl, 
                           string leftSide, string rightSide){
    string leftSideSet, rightSideSet;

    leftSideSet = "{" + tupleDecl + " : " + leftSide + " }";
    rightSideSet = "{" + tupleDecl + " : " + rightSide + " }";

    mLeftSide = new Set (leftSideSet);
    mRightSide = new Set (rightSideSet);

    if(type == "Monotonicity")         mUniQuantRuleType = Monotonicity;
    else if (type == "CoMonotonicity") mUniQuantRuleType = CoMonotonicity;
    else if (type == "Triangularity")     mUniQuantRuleType = Triangularity;
    else if (type == "FuncConsistency")mUniQuantRuleType = FuncConsistency;
    else                               mUniQuantRuleType = TheOthers;
}

//! Copy constructor.  Performs a deep copy.
UniQuantRule::UniQuantRule(const UniQuantRule& other) {

    mUniQuantRuleType = other.mUniQuantRuleType;
    this->mLeftSide = new Set( *(other.mLeftSide) );
    this->mRightSide = new Set( *(other.mRightSide) );
}

//! Copy assignment.
UniQuantRule& UniQuantRule::operator=(const UniQuantRule& other) {

    UniQuantRule temp(other);
    temp.swap (*this); // Non-throwing swap
    return *this;
}

//! helper function for implementing copy-and-swap
void UniQuantRule::swap(UniQuantRule& second) throw() {
    std::swap(mUniQuantRuleType, second.mUniQuantRuleType);
    std::swap(mLeftSide, second.mLeftSide);
    std::swap(mRightSide, second.mRightSide);
}

UniQuantRule::~UniQuantRule(){
  delete mLeftSide;
  delete mRightSide;
}

//! prints the content of the map into a std::string, and returns it
std::string UniQuantRule::toString(){
    std::stringstream ss;

    // Functionalities in isl_str_manipulation.h/cc for breaking different 
    // parts of an IEGenLib Set/Relation (for details refer to those files):
    srParts RulesLParts,RulesRParts;
    RulesLParts = getPartsFromStr(mLeftSide->prettyPrintString());
    RulesRParts = getPartsFromStr(mRightSide->prettyPrintString());

    ss <<"Forall " + (mLeftSide->getTupleDecl()).toString()+ "  "
          + RulesLParts.constraints + " => " + RulesRParts.constraints;

    return ss.str();
}

UniQuantRuleType UniQuantRule::getType(){return mUniQuantRuleType;}
Set* UniQuantRule::getLeftSide(){ return mLeftSide; }
Set* UniQuantRule::getRightSide(){ return mRightSide; }

}//end namespace iegenlib
