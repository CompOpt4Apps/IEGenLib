/*!
 * \file Computation.h
 *
 * \brief Declarations for the Computation class, and supporting classes.
 *
 * The Computation class is the SPF representation of a logical computation.
 * It contains a Stmt class for each statement, which in turn contains
 * information about that statement as mathematical objects.
 * Originally part of spf-ie.
 *
 * \date Started: 10/09/20
 *
 * \authors Anna Rift, Tobi Popoola
 *
 * Copyright (c) 2020, University of Arizona <br>
 * Copyright (c) 2020, Boise State University <br>
 * All rights reserved. <br>
 * See ../../COPYING for details. <br>
 */

#ifndef COMPUTATION_H_
#define COMPUTATION_H_

#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>


#include <basic/boolset.h>

#include "set_relation/Visitor.h"
#include "set_relation/environment.h"
#include "set_relation/expression.h"
#include "set_relation/set_relation.h"

namespace iegenlib {

class Stmt;
class AppendComputationResult;

/*!
 * \class Computation
 *
 * \brief SPF representation of a computation (group of statements such as a
 * function).
 *
 * Incoming code must have highly-unique variable names (assumed to be
 * surrounded in some non-code string such as $'s') if it may be appended to
 * another Computation.
 */
class Computation {
   public:
    //! Construct an empty Computation
    Computation();

    //! Destructor
    ~Computation();

    //! Copy constructor
    Computation(const Computation& other);

    //! Assignment operator (copy)
    Computation& operator=(const Computation& other);

    //! Equality operator
    bool operator==(const Computation& other) const;

    //! Get a copy of this Computation with uniquely-prefixed data spaces
    //! Uses string find-and-replace to change data space names, with the
    //! assumption that incoming names will be unique enough for this to work
    //! without unintended replacements occuring.
    Computation* getUniquelyNamedClone() const;
    //! Reset the counter for number of renames.
    //! **This method should only be used for testing purposes. Using it in
    //! production will cause name conflicts.**
    static void resetNumRenames();
    //! Get the new name for a data space that has the given prefix applied
    static std::string getPrefixedDataSpaceName(const std::string& originalName, const std::string& prefix);

    //! Add a statement to this Computation.
    //! Statements are numbered sequentially from 0 as they are inserted.
    //! @param[in] stmt Stmt to add (adopted)
    void addStmt(Stmt* stmt);
    //! Get a statement by index
    Stmt* getStmt(unsigned int index) const;
    //! Get the number of statements in this Computation
    unsigned int getNumStmts() const;

    //! Add a data space to this Computation
    void addDataSpace(std::string dataSpaceName);
    //! Get data spaces
    std::unordered_set<std::string> getDataSpaces() const;
    //! Check if a given string is a name of a data space of this Computation
    bool isDataSpace(std::string name) const;

    //! Add a (formal) parameter to this Computation, including adding it as a data space.
    //! @param paramName Name of the parameter
    //! @param paramType C data type of the parameter
    void addParameter(std::string paramName, std::string paramType);
    //! Get a parameter's name
    std::string getParameterName(unsigned int index) const;
    //! Get a parameter's data type
    std::string getParameterType(unsigned int index) const;
    //! Get the number of parameters this Computation has
    unsigned int getNumParams() const;

    //! Add a return value to this Computation
    void addReturnValue(std::string name);
    //! Add a return value to this Computation, specifying if it is a data space
    //! name or not (in which case it is a literal). This avoids having to check
    //! if the name is a data space before insertion, for use in cases where the
    //! caller already knows this information.
    void addReturnValue(std::string name, bool isDataSpace);
    //! Get the list of return values
    std::vector<std::string> getReturnValues() const;
    //! Get the number of return values this Computation has (some languages
    //! support multiple return)
    unsigned int getNumReturnValues() const;

    //! Get an iterator to the first Stmt in this Computation
    std::vector<Stmt*>::iterator stmtsBegin() { return stmts.begin(); }
    //! Get an iterator pointing past the last Stmt in this Computation
    std::vector<Stmt*>::iterator stmtsEnd() { return stmts.end(); }

    //! Print out all the information represented in this Computation for
    //! debug purposes
    void printInfo() const;

    //! Get whether this Computation's required information is filled out,
    //! including that of the Stmts it contains
    bool isComplete() const;

    //! Clear all data from this Computation
    void clear();

    //! Add statements from another Computation to this one for inlining.
    //! \param[in] other Computation to append to this one, which will be copied
    //! but its statements will not be modified
    //! \param[in] surroundingIterDomainStr Iteration domain of append context (not
    //! adopted)
    //! \param[in] surroundingExecScheduleStr Execution schedule of append
    //! context (not adopted). Final tuple position will be the position used by
    //! the first appended statement.
    //! \param[in] arguments Arguments to pass in
    //! to the appended Computation \return Information about the state of the
    //! appender after appending.
    AppendComputationResult appendComputation(
        const Computation* other, std::string surroundingIterDomainStr,
        std::string surroundingExecScheduleStr,
        const std::vector<std::string>& arguments = {});


    //! Function returns a dot string representing nesting
    //  and loop carrie dependency. Internally it uses
    //  a lite version of polyhedral scanning to generate
    //  subgraphs in the dot file.
    std::string toDotString();

    //! Environment used by this Computation
    Environment env;

    //! Add a transformation (adopted) to the specified Stmt
    void addTransformation(unsigned int stmtIndex, Relation* rel);

    //! Sequentially apply added transformations to all statements.
    //! Returns a list of final statement schedules after transformation.
    std::vector<Set*> applyTransformations() const;

    //! Method generates c code.
    //! Known constraints are constraints that can be considered already
    //! enforced/are known to be true without need for checking in generated
    //! code (guards).
    std::string codeGen(Set* knownConstraints = nullptr);

    //! Method returns omega strings for each statement
    std::string toOmegaString();
    

    //! Function compares two statement pair 
    //  and returns the true if a is lexicographically 
    //  lower in order to b. 
    static bool activeStatementComparator(const std::pair<int,Set*>& a, 
		   const std::pair<int,Set*>& b);
    
    //! Function splits an active statement to disjoint
    //! list of statements.
    //  \param   level
    //  \param   activeStmt current active statements.
    std::vector<std::vector<std::pair<int,Set*> > > split
	    (int level, std::vector<std::pair<int,Set*> >& activeStmt);
   
    //! Function reschudles statment s1 to come before 
    //  statement s2
    //  \param s1 First statement Id
    //  \param s2 Second statement Id.
    void reschedule(int s1, int s2);
   

    //! Function fuses two statements at some level 
    //  in their execution schedule. In the resulting 
    //  computation after fusion, S1 will be ordered before
    //  S2, other statements at that level will be adjusted accordingly
    //  so reflect the new changes.
    //
    //  \param s1        first statement id
    //  \param s2        second statement id
    //  \param fuseLevel Level at which to fuse S1 and S2 
    //  
    //
    //  Example S0: {[0,i,0,j,0] | stuff}; S1:{[1,i,0,j,0] | stuff}
    //  fuse(S0,S1,2);
    //  Result
    //  S0: {[0,i,0,j,0] | stuff}; S1:{[0,i,1,j,0] | stuff}
    void fuse (int s1, int s2, int fuseLevel);
   
    private:
    
    //! Lite version of polyhedra scanning to generate 
    //! toDot Clusters
    void toDotScan(std::vector<std::pair<int,Set*>> &activeStmts, int level,
		    std::ostringstream& ss ,
		    std::vector<std::vector<Set*> >&projectedIS);
    
    
    //! Information on all statements in the Computation
    std::vector<Stmt*> stmts;
    //! Data spaces available in the Computation
    std::unordered_set<std::string> dataSpaces;
    //! Parameters of the computation, pair of name : type. All parameters should also be data spaces.
    std::vector<std::pair<std::string, std::string>> parameters;
    //! Names of values that are returned if this Computation is called. May be
    //! data space names or literals. This is an ordered list because some
    //! languages allow multiple returns.
    //! Pair of name/literal : whether it's a data space name
    std::vector<std::pair<std::string, bool>> returnValues;

    //! List of statement transformation lists
    std::vector<std::vector<Relation*>> transformationLists;

    //! Assert that a given string would be a valid data space name, that is, it is properly delimited by $'s
    static bool assertValidDataSpaceName(const std::string& name);

    //! Number of times *any* Computation has been appended into
    //! others, for creating unique name prefixes.
    static unsigned int numRenames;
};

/*!
 * \struct AppendComputationResult
 *
 * \brief Struct containing information about the state of the Computation
 * following an append.
 */
struct AppendComputationResult {
    //! Last execution schedule value used at the appending depth.
    int tuplePosition;
    std::vector<std::string> returnValues;
};

//! Prints the dotFile for the Computation structure

/*!
 * \class Stmt
 *
 * \brief Information attached to a statement represented as mathematical
 * objects.
 */
class Stmt {
   public:
    //! Construct an empty Stmt
    Stmt() = default;

    //! Destructor
    ~Stmt();

    //! Construct a complete Stmt, given strings that will be used to
    //! construct each set/relation.
    Stmt(std::string stmtSourceCode, std::string iterationSpaceStr,
         std::string executionScheduleStr,
         std::vector<std::pair<std::string, std::string>> dataReadsStrs,
         std::vector<std::pair<std::string, std::string>> dataWritesStrs);

    //! Copy constructor
    Stmt(const Stmt& other);

    //! Assignment operator (copy)
    Stmt& operator=(const Stmt& other);

    //! Equality operator
    //! Checks equality, NOT mathematical equivalence
    bool operator==(const Stmt& other) const;

    //! Get a copy of this Stmt with the given prefix applied to all names
    Stmt* getUniquelyNamedClone(const std::string& prefix, const std::unordered_set<std::string>& dataSpaceNames) const;

    //! Get whether or not all necessary information for this Stmt is set
    bool isComplete() const;

    //! Get the source code of the statement
    std::string getStmtSourceCode() const;
    //! Set the source code of the statement
    void setStmtSourceCode(std::string newStmtSourceCode);

    //! Get the iteration space Set
    Set* getIterationSpace() const;
    //! Set the iteration space, constructing it from the given string
    void setIterationSpace(std::string newIterationSpaceStr);
    //! Set the iteration space (adopted)
    void setIterationSpace(Set* newIterationSpace);

    //! Get the execution schedule Relation
    Relation* getExecutionSchedule() const;
    //! Set the execution schedule, constructing it from the given string
    void setExecutionSchedule(std::string newExecutionScheduleStr);
    //! Set the execution schedule (adopted)
    void setExecutionSchedule(Relation* newExecutionSchedule);

    //! Add a data read
    void addRead(std::string dataSpace, std::string relationStr);
    //! Add a data read, adopting the given relation
    void addRead(std::string dataSpace, Relation* relation);
    //! Get the number of data reads for this statement
    unsigned int getNumReads() const;
    //! Get a data read's data space by index
    std::string getReadDataSpace(unsigned int index) const;
    //! Get a data read's relation by index
    Relation* getReadRelation(unsigned int index) const;

    //! Add a data write
    void addWrite(std::string dataSpace, std::string relationStr);
    //! Add a data write, adopting the given relation
    void addWrite(std::string dataSpace, Relation* relation);
    //! Get the number of data writes for this statement
    unsigned int getNumWrites() const;
    //! Get a data write's data space by index
    std::string getWriteDataSpace(unsigned int index) const;
    //! Get a data write's relation by index
    Relation* getWriteRelation(unsigned int index) const;

   private:
    //! Source code of the statement, for debugging purposes
    std::string stmtSourceCode;
    //! Iteration space of a statement
    std::unique_ptr<Set> iterationSpace;
    //! Execution schedule of a single statement
    std::unique_ptr<Relation> executionSchedule;
    //! Read dependences of a statement, pairing data space name to relation
    std::vector<std::pair<std::string, std::unique_ptr<Relation>>> dataReads;
    //! Write dependences of a statement, pairing data space name to relation
    std::vector<std::pair<std::string, std::unique_ptr<Relation>>> dataWrites;
};

/*!
 * \class VisitorChangeUFsForOmega
 *
 * \brief Visitor that makes modifications to UF call terms so that they are
 * acceptable for what Omega supports.
 *
 * Intended for use on sets/relations. Also gathers information needed to pass
 * along to Omega codegen, such as #define macros.
 */
class VisitorChangeUFsForOmega : public Visitor {
   private:
    //! UF call (re)definition macros
    std::map<std::string, std::string> macros;
    //! declarations of UF calls needed by Omega parser
    std::set<std::string> ufCallDecls;
    //! UF calls that we've seen before and do not need to rename if encountered
    //! again; mapping from call as string -> new assigned name
    std::map<std::string, std::string> knownUFs;
    //! next number to use in creating unique function names
    int nextFuncReplacementNumber;
    //! stored tuple decl for variable retrieval
    TupleDecl currentTupleDecl;

    //! Information stored for tuple variable
    //! assignments.
    std::map<int,std::string> tupleAssignments;

   public:
    //! Construct a new VisitorChangeUFsForOmega
    VisitorChangeUFsForOmega();

    //! Destructor
    ~VisitorChangeUFsForOmega();

    //! Completely reset state of Visitor for re-use, including freeing memory.
    //! This method should NOT be used between visiting connected
    //! Sets/Relations, for instance those which share some UF calls you want
    //! the Visitor to remember.
    void reset();

    //! Partially reset Visitor state so that it is prepared for use with
    //! additional interrelated Sets/Relations, such as statements in a
    //! Computation.
    void prepareForNext();

    //! Get tuple assignments. A tuple is initially assigned
    //! to zero, unless there is an equality constraint
    //! involving that tuple variable and constants
    std::map<int,std::string>& getTupleAssignments();

    //! Get the UF call macros required for the code corresponding to the
    //! set/relation to function correctly, as a string
    std::map<std::string, std::string>* getUFMacros();

    //! Get the declarations of UF calls needed by Omega parser
    std::set<std::string> getUFCallDecls();

    void preVisitSparseConstraints(SparseConstraints*);

    void preVisitConjunction(Conjunction* c);

    void postVisitUFCallTerm(UFCallTerm*);

    void preVisitExp(iegenlib::Exp * e);
};

}  // namespace iegenlib

#endif
