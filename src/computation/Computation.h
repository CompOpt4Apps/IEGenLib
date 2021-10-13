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
 * \authors Anna Rift, Tobi Popoola, Aaron Orenstein
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

#include "CompGraph.h"
#include "set_relation/Visitor.h"
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
    Computation() = default;

    //! Construct an empty Computation with the given name
    explicit Computation(std::string& name);

    //! Destructor
    ~Computation();

    //! Copy constructor
    Computation(const Computation& other);

    //! Assignment operator (copy)
    Computation& operator=(const Computation& other);

    //! Equality operator
    bool operator==(const Computation& other) const;

	//! Get the name of this Computation
	std::string getName() const;
	//! Set the name of this Computation
	void setName(std::string newName);

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

    //! Takes in an iteration space set and removes all "$"
    //  all trimmed data spaces are stored in dataSpaces if passed in
    //  returns the new trimmed set, which must be deallocated by the caller
    static Set* trimISDataSpaces(Set* set);
    static Set* trimISDataSpaces(Set* set, std::set<std::string> &trimmedNames);

    //! Returns true if stmt1 is guaranteed to have executed by
    //  the time stmt2 is reached, false otherwise
    static bool isGuaranteedExecute(Stmt* stmt1, Stmt* stmt2);

    //! Returns a list of all the constraints of the given set
    static std::vector<std::string> getSetConstraints(Set* set);

    //! Add statement to the end of this Computation
    void addStmt(Stmt* stmt);
    //! Insert statement at given index.
    //! This may modify the statement to fit the needs of the Computation.
    void addStmt(Stmt* stmt, int stmtIdx);

    //! Get a statement by index
    Stmt* getStmt(unsigned int index) const;
    //! Get the number of statements in this Computation
    unsigned int getNumStmts() const;

    //! Add a data space to this Computation
    void addDataSpace(std::string dataSpaceName, std::string dataSpaceType);
    //! Get data spaces
    std::map<std::string, std::string> getDataSpaces() const;
    //! Get data space's type
    std::string getDataSpaceType(std::string) const;
    //! Check if a given string is a name of a data space of this Computation
    bool isDataSpace(std::string name) const;
    //! Returns statement index of write if written to, else -1
    int firstWriteIndex(std::string dataSpace);
    //! Replace data space name if written to
    void replaceDataSpaceName(std::string original, std::string newString);
    //! Produces a rename for the inputted data space, incrementing dataRenameCnt
    std::string getDataSpaceRename(std::string dataSpaceName);
    //! unroll  - $arrName__ati1__ati2...__atiN$
    //  access  - $arrName$[i1][i2]...[iN]
    //  tuple   - [i1,i2,...,iN]
    static void getArrayAccessStrs(std::string& unroll, std::string& access,
        std::string& tuple, std::string arrName, const std::list<int>& idxs);
    //! Checks if the given data space is an array with only constant accesses
    //bool isConstArray(std::string dataSpaceName);

    //! Trims dollar signs off of data space
    static std::string trimDataSpaceName(std::string dataSpaceName);
    //! Gets original data space name by trimming off final "__w__#" sequence
    static std::string getOriginalDataSpaceName(std::string dataSpaceName);
    //! Returns if a and b are renames of the same dataspace
    static bool areEquivalentRenames(std::string a, std::string b);

    //! Add a (formal) parameter to this Computation, including adding it as a data space.
    //! @param paramName Name of the parameter
    //! @param paramType C data type of the parameter
    void addParameter(std::string paramName, std::string paramType);
    //! Get a parameter's name
    std::string getParameterName(unsigned int index) const;
    //! Get a parameter's data type
    std::string getParameterType(unsigned int index) const;
    //! Get the list of parameters
    std::vector<std::string> getParameters() const { return parameters; }
    //! Check if a data space is a parameter
    bool isParameter(std::string dataSpaceName) const;
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
    //! Gets all active out data spaces - return values and reference parameters
    std::vector<std::string> getActiveOutValues() const;
    //! Check if a data space is a return values
    bool isReturnValue(std::string dataSpaceName) const;
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

    //Returns a string declaring all data spaces in the computation
    std::string codeGenMemoryManagementString();


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
        std::vector<std::string> arguments = {});

    //! Performs operations with the assumption that no more statements will be added
    void finalize(bool deleteDeadNodes = false);

    //! Returns true if all input sets have the same arity, false otherwise
    bool consistentSetArity(const std::vector<Set*>& sets);

    //! Pads each statement's execution schedule with 0's such the all
    //  execution schedules have the same arity
    void padExecutionSchedules();

    //! Performs special SSA renaming for constant access arrays
    //  on all statements
    void enforceArraySSA();

    //! Adjusts all statement execution schedules to accouunt for 
    //  dynamically added statements.
    //  Ordering of inputted statements is maintained.
    void adjustExecutionSchedules();

    //! Deletes statements that writes 
    //! to nodes that are never read from 
    //! in the computation.
    void deleteDeadStatements();

    //! Function returns a dot string representing nesting
    //  and loop carrie dependency.
    //  TODO: Add graph options as parameters
    std::string toDotString(bool reducePCRelations = false,
		    bool toPoint = false,
            bool onlyLoopLevels = false,
		    bool addDebugStmts = false,
		    std::string subgraphSrc = "",
		    bool subgraphReads = false,
		    bool subgraphWrites = false);

    //! Compiles each statement's debug string into a list
    std::vector<std::pair<int, std::string>> getStmtDebugStrings();

    //! Add a transformation (adopted) to the specified Stmt
    void addTransformation(unsigned int stmtIndex, Relation* rel);

    //! Sequentially apply added transformations to all statements.
    //! Returns a list of final statement schedules after transformation.
    std::vector<Set*> applyTransformations();

    //! Sequentially composes addes transfomations onto each statement's
    //  execution schedule (scheduling function)
    //  Caller is responsible for deallocating Relation*
    std::vector<Relation*> getTransformations() const;

    //! Returns a list of iteration spaces after padding and applying transformations
    std::vector<std::pair<int, Set*>> getIterSpaces();

    //Creates a mapping from the variables of the in tuple to those of the out tuple
    static std::string getRelationMapping(Relation* r);

    //! Method generates c code.
    //! Known constraints are constraints that can be considered already
    //! enforced/are known to be true without need for checking in generated
    //! code (guards).
    std::string codeGen(Set* knownConstraints = nullptr);

    //!! Generates code from an omega string
    std::string omegaCodeGenFromString(
            std::vector<int> relationArity,
            std::vector<std::string> iterSpacesStr, std::string known);

    //! Method returns omega strings for each statement
    std::string toOmegaString();

    //! Function compares two statement pair
    //  and returns the true if a is lexicographically
    //  lower in order to b.
    static bool activeStatementComparator(const std::pair<int,Set*>& a,
		   const std::pair<int,Set*>& b);

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

    //! Delimit data spaces in a statement so that it may be used properly in this Computation.
    //! Modifies the given statement in-place.
    void delimitDataSpacesInStmt(Stmt* stmt);
    //! Go through the string, potentially consisting of C source code, and delimit all data space names.
    std::string delimitDataSpacesInString(std::string originalString);
    //! Wrap the given data space name in delimiters
    static std::string delimitDataSpaceName(std::string dataSpaceName);
    //! Get a new string with all instances of the data space delimiter removed from the original string
    static std::string stripDataSpaceDelimiter(std::string delimitedStr);
    //! Check if a data space name is delimited, excepting if it is only delimited on one side.
    static bool nameIsDelimited(std::string name);

    private:

  	//! Human-readable name of Computation
  	std::string name;

    //! maps array name : true - has constant accesses at all dimensions
    //std::map<std::string, bool> arrays;
    //! Performs special SSA renaming on arrays
    //  splits constant-access arrays into separate data spaces
    //void enforceArraySSA(Stmt* stmt);
    //! Performs special SSA renaming for the data space
    //  at the specified read/write index in stmt
    //  returns true if successful, false otherwise
    //bool enforceArraySSA(Stmt* stmt, int dataIdx, bool isRead);

    //! Locates phi nodes for a given statement. These occur when a data space
    //  which is being read can take multiple values based on the control flow
    int locatePhiNodes(int stmtIdx);
    //! Returns true if a phi nodes is created, false otherwise
    bool locatePhiNode(int stmtIdx, std::string dataSpace);
    //! Adds in a phi node
    //! Returns true if a phi nodes is created, false otherwise
    bool addPhiNode(int stmtIdx, std::pair<int, std::string>& first,
        std::pair<int, std::string>& guaranteed);

    //! For each write in the new statement, rename the written dataspace in all
    //  statements from the last statement to the last write to that data space.
    void enforceSSA(int stmtIdx);

    //! Information on all statements in the Computation
    std::vector<Stmt*> stmts;

    //! Data spaces available in the Computation, pairs of name : type
    std::map<std::string, std::string> dataSpaces;
    //! Non-delimited data space names
    std::unordered_set<std::string> undelimitedDataSpaceNames;

    //! Parameters of the computation. All parameters should also be data spaces.
    std::vector<std::string> parameters;
    //! Names of values that are returned if this Computation is called. May be
    //! data space names or literals. This is an ordered list because some
    //! languages allow multiple returns.
    //! Pair of name/literal : whether it's a data space name
    std::vector<std::pair<std::string, bool>> returnValues;

    //! List of statement transformation lists
    std::vector<std::vector<Relation*>> transformationLists;

    //! Assert that a given string would be a valid data space name
    static bool assertValidDataSpaceName(const std::string& name, bool alreadyDelimited);

  //! Number of times *any* Computation has been appended into
    //! others, for creating unique name prefixes.
    static unsigned int numRenames;

    static unsigned int dataRenameCnt;
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
    //! construct each set/relation. Data spaces in the incoming strings
    //! will be delimited automatically.
    Stmt(std::string stmtSourceCode, std::string iterationSpaceStr,
         std::string executionScheduleStr,
         std::vector<std::pair<std::string, std::string>> dataReadsStrs,
         std::vector<std::pair<std::string, std::string>> dataWritesStrs);

    //! Copy constructor
    Stmt(const Stmt& other);

    //! Replaces read everywhere in the Stmt
    void replaceRead(std::string searchStr, std::string replaceStr);
    //! Replaces read data space
    void replaceReadDataSpace(std::string searchStr, std::string replaceStr);
    //! Replaces searchStr in the read portion of the source code
    void replaceReadSourceCode(std::string searchStr, std::string replaceStr);

    //! Replaces write everywhere in the Stmt
    void replaceWrite(std::string searchStr, std::string replaceStr);
    //! Replaces write data space
    void replaceWriteDataSpace(std::string searchStr, std::string replaceStr);
    //! Replaces searchStr in the write portion of the source code
    void replaceWriteSourceCode(std::string searchStr, std::string replaceStr);

    //! Replace data space everywhere in the Stmt
    void replaceDataSpace(std::string searchString, std::string replaceString);

    //! Assignment operator (copy)
    Stmt& operator=(const Stmt& other);

    //! Equality operator
    //! Checks equality, NOT mathematical equivalence
    bool operator==(const Stmt& other) const;

    //! Get a copy of this Stmt with the given prefix applied to all names
    //Stmt* getUniquelyNamedClone(const std::string& prefix, const std::unordered_set<std::string>& dataSpaceNames) const;
    Stmt* getUniquelyNamedClone(const std::string& prefix, const std::map<std::string, std::string>& dataSpaceNames) const;

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
    //! Update read
    void updateRead(int idx, std::string dataSpace, std::string relationStr);
    //! Get the number of data reads for this statement
    unsigned int getNumReads() const;
    //! Get a data read's data space by index
    std::string getReadDataSpace(unsigned int index) const;
    //std::string getReadDataSpaceType(unsigned int index) const;
    //! Get a data read's relation by index
    Relation* getReadRelation(unsigned int index) const;

    //! Add a data write
    void addWrite(std::string dataSpace, std::string relationStr);
    //! Add a data write, adopting the given relation
    void addWrite(std::string dataSpace, Relation* relation);
    //! Update write
    void updateWrite(int idx, std::string dataSpace, std::string relationStr);
    //! Get the number of data writes for this statement
    unsigned int getNumWrites() const;
    //! Get a data write's data space by index
    std::string getWriteDataSpace(unsigned int index) const;
    //std::string getWriteDataSpaceType(unsigned int index) const;
    //! Get a data write's relation by index
    Relation* getWriteRelation(unsigned int index) const;

    //! Checks if a data read/write is a constant access array
    std::list<int> getConstArrayAccesses(unsigned int index, bool read) const;

    //Set debug string for the statement
    void setDebugStr(std::string str);
    //Returns first debug string for the statement
    std::string getDebugStr() const;
    //Returns all added debug strings for the statement
    std::string getAllDebugStr() const;

    std::string prettyPrintString() const;

    // Getter/setters for Stmt booleans
    bool isPhiNode() const { return phiNode; }
    void setPhiNode(bool val) { phiNode = val; };
    bool isArrayAccess() const { return arrayAccess; }
    void setArrayAccess(bool val) { arrayAccess = val; };

    //! Check if this Stmt has its data spaces delimited already
    bool isDelimited() const { return delimited; }
    //! Mark that this Stmt's data spaces have been delimited
    void setDelimited() { delimited = true; }

	//! EXPECT with gTest this Stmt equals the given one, component by component.
	//! This method is for testing with gTest only.
	void expectEqualTo(const Stmt* other) const;

   private:
    //! Is the statement a phi node or array access
    bool phiNode = false, arrayAccess = false;
    //! Debug string
    std::vector<string> debug;
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
    //! Whether this Stmt's references to data spaces are delimited
    bool delimited = false;
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

    std::vector<std::string> arrayAccessUFs;

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




