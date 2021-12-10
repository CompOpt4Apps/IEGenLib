/*!
 * \file computation_test.cc
 *
 * \brief Tests for the Computation class.
 *
 * \date Started: 12/10/20
 *
 * \authors Anna Rift
 *
 * Copyright (c) 2020, University of Arizona <br>
 * Copyright (c) 2020, Boise State University <br>
 * All rights reserved. <br>
 * See ../../COPYING for details. <br>
 */

#include "Computation.h"

#include <gtest/gtest.h>

#include <string>
#include <utility>
#include <vector>

#include "code_gen/parser/parser.h"
#include "omega/Relation.h"
#include "set_relation/set_relation.h"

using namespace iegenlib;
using namespace std;

/*!
 * \class ComputationTest
 *
 * \brief gtest fixture for computation testing
 */
class ComputationTest : public ::testing::Test {
   private:
    VisitorChangeUFsForOmega* vOmegaReplacer;

   protected:
    virtual void SetUp() override {
        vOmegaReplacer = new VisitorChangeUFsForOmega();
        //! Reset the Computation class's running rename counter so generated name prefixes do not depend on test order.
        Computation::resetNumRenamesCounters();
    }
    virtual void TearDown() override { delete vOmegaReplacer; }

    //! attempt to convert the given Set string to Omega format,
    //! EXPECTing that it will equal the given Omega string
    void checkOmegaSetConversion(std::string iegenSetStr,
                                 std::string expectedOmegaResult) {
        SCOPED_TRACE(iegenSetStr);
        Set* iegenSet = new Set(iegenSetStr);

        // do conversion
        iegenSet->acceptVisitor(vOmegaReplacer);
        omega::Relation* omegaSet = omega::parser::ParseRelation(
            iegenSet->toOmegaString(vOmegaReplacer->getUFCallDecls()));
        EXPECT_EQ(expectedOmegaResult + "\n",
                  omegaSet->print_with_subs_to_string());

        delete iegenSet;
        delete omegaSet;
        vOmegaReplacer->reset();
    }

    //! Tests if a transformation is valid.
    //! \param comp        Computation
    //! \param expectedSet Transformations apply to each of the statement
    //                     internally, and these statements are indexed from
    //                     zero to N-1 where N is the number of statements.
    //                     Transformation affect statemennts in a computation
    //                     This paramter is the expected result of an applied
    //                     transformation on each statement. Each set in the
    //                     vector represent the corresponding result of
    //                     transformation on a statement id.
    void checkTransformation(Computation*comp,
		                  std::vector<std::string> expectedSet){
        std::vector<Set*> transforms=  comp->applyTransformations();
        for(int i =0 ; i < transforms.size() ; i++){
	    Set* s = transforms[i];
	    Set* expected= new Set(expectedSet[i]);
	    SCOPED_TRACE(("S"+std::to_string(i) +" Actual: "+ s->toString()+
			" Expected: "+expected->toString() ).c_str());
	    EXPECT_TRUE(*expected==*s);
	    delete s;
	    delete expected;
	}
    }

    //! attempt to convert the given Relation to Omega format,
    //! EXPECTing that it will equal the given Omega string
    void checkOmegaRelationConversion(std::string iegenRelStr,
                                      std::string expectedOmegaResult) {
        SCOPED_TRACE(iegenRelStr);
        iegenlib::Relation* iegenRel = new iegenlib::Relation(iegenRelStr);

        // do conversion
        iegenRel->acceptVisitor(vOmegaReplacer);
	omega::Relation* omegaRel = omega::parser::ParseRelation(
            iegenRel->toOmegaString(vOmegaReplacer->getUFCallDecls()));
        EXPECT_EQ(expectedOmegaResult + "\n",
                  omegaRel->print_with_subs_to_string());

        delete iegenRel;
        delete omegaRel;
        vOmegaReplacer->reset();
    }

    //! Test that for a set, codegen tuple variables are correctly initialized.
    //
    void checkTupleAssignments(
        std::string setString, std::vector<std::string> expectedAssignments){
        SCOPED_TRACE(setString);

	iegenlib::Set* set = new iegenlib::Set(setString);
        set->acceptVisitor(vOmegaReplacer);
        for(auto& t : vOmegaReplacer->getTupleAssignments()){
            EXPECT_EQ(expectedAssignments[t.first],t.second);
	}
        delete set;
	vOmegaReplacer->reset();
    }

    //! Test that appending a computation to another yields the correct results.
    //! The passed-in computations are modified but not adopted, and should be
    //! freed after this test.
    //! \param[in,out] appendedTo Computation that is appended to (the 'caller
    //! function')
    //! \param[in] appendedComp Computation appended onto another (the 'callee'
    //! function)
    //! \param[in] surroundingIterDomain Iteration domain of append context
    //! \param[in] surroundingExecSchedule Execution schedule of append context
    //! \param[in] argsList list of arguments to pass the appended Computation
    //! \param[in] expectedTuplePosition expected last used tuple position at
    //! insertion level
    //! \param[in] expectedReturnValues string versions of expected return values
    //! \param[in] expectedComp Computation that appender should equal after append
    void checkAppendComputation(Computation* appendedTo,
                                const Computation* appendedComp,
                                std::string surroundingIterDomainStr,
                                std::string surroundingExecScheduleStr,
                                const std::vector<std::string>& argsList,
                                int expectedTuplePosition,
                                std::vector<std::string> expectedReturnValues,
                                Computation* expectedComp) {

        AppendComputationResult result =
            appendedTo->appendComputation(appendedComp, surroundingIterDomainStr,
                                          surroundingExecScheduleStr, argsList);

        EXPECT_EQ(expectedTuplePosition, result.tuplePosition);
        EXPECT_EQ(expectedReturnValues, result.returnValues);
        expectComputationsEqual(appendedTo, expectedComp);
    }

    void checkToDotString(Computation* comp, std::string expectedDot){
        EXPECT_EQ(expectedDot, comp->toDotString());
    }

    //! EXPECT with gTest that two Computations are equal, component by component.
	void expectComputationsEqual(const Computation* actual, const Computation* expected) {
        ASSERT_EQ(expected->getName(), actual->getName());
        SCOPED_TRACE("Computation '" + actual->getName() + "'");

        ASSERT_EQ(expected->isComplete(), actual->isComplete());

        const auto expectedTransformations = expected->getTransformations();
        const auto actualTransformations = actual->getTransformations();
        ASSERT_EQ(expectedTransformations.size(), actualTransformations.size());
        for (unsigned int i = 0; i < expectedTransformations.size(); ++i) {
            EXPECT_EQ(*expectedTransformations[i], *actualTransformations[i]);
        }

        ASSERT_EQ(expected->getNumStmts(), actual->getNumStmts());
        for (unsigned int i = 0; i < actual->getNumStmts(); ++i) {
          SCOPED_TRACE("Statement " + std::to_string(i));
          expectStmtsEqual(actual->getStmt(i), expected->getStmt(i));
        }

        EXPECT_EQ(expected->getDataSpaces(), actual->getDataSpaces());

        ASSERT_EQ(expected->getNumParams(), actual->getNumParams());
        for (unsigned int i = 0; i < actual->getNumParams(); ++i) {
          SCOPED_TRACE("Parameter " + std::to_string(i));
          EXPECT_EQ(expected->getParameterName(i), actual->getParameterName(i));
          EXPECT_EQ(expected->getParameterType(i), actual->getParameterType(i));
        }

        EXPECT_EQ(expected->getReturnValues(), actual->getReturnValues());
        EXPECT_EQ(expected->getActiveOutValues(), actual->getActiveOutValues());

        EXPECT_TRUE(*actual == *expected);
	}

    //! EXPECT with gTest that two Stmts are equal, component by component.
    void expectStmtsEqual(const Stmt* actual, const Stmt* expected) {
        ASSERT_EQ(expected->isComplete(), actual->isComplete());
        ASSERT_EQ(expected->isDelimited(), actual->isDelimited());

        EXPECT_EQ(expected->getStmtSourceCode(), actual->getStmtSourceCode());

        EXPECT_EQ(expected->getIterationSpace()->prettyPrintString(),
                  actual->getIterationSpace()->prettyPrintString());

        EXPECT_EQ(expected->getExecutionSchedule()->prettyPrintString(),
                  actual->getExecutionSchedule()->prettyPrintString());

        ASSERT_EQ(expected->getNumReads(), actual->getNumReads());
        for (unsigned int j = 0; j < actual->getNumReads(); ++j) {
          EXPECT_EQ(expected->getReadDataSpace(j),
                    actual->getReadDataSpace(j));
          EXPECT_EQ(expected->getReadRelation(j)->prettyPrintString(),
                    actual->getReadRelation(j)->prettyPrintString());
        }

        ASSERT_EQ(expected->getNumWrites(), actual->getNumWrites());
        for (unsigned int j = 0; j < actual->getNumWrites(); ++j) {
          EXPECT_EQ(expected->getWriteDataSpace(j),
                    actual->getWriteDataSpace(j));
          EXPECT_EQ(expected->getWriteRelation(j)->prettyPrintString(),
                    actual->getWriteRelation(j)->prettyPrintString());
        }

        EXPECT_EQ(expected->getAllDebugStr(), actual->getAllDebugStr());
        EXPECT_EQ(expected->isPhiNode(), actual->isPhiNode());
        EXPECT_EQ(expected->isArrayAccess(), actual->isArrayAccess());

        EXPECT_TRUE(*actual == *expected);
    }
};

#pragma mark DenseMVCodeGen
TEST_F(ComputationTest, DenseMVCodeGen) {
    std::string originalCode =
        "\
int i; \
int j; \
for (i = 0; i < a; i++) { \
    product[i] = 0; \
    for (j = 0; j < b; j++) { \
        product[i] += x[i][j] * y[j]; \
    } \
} \
\
return 0; \
";

    Computation* comp = new Computation();
    comp->addDataSpace("product", "int");
    comp->addDataSpace("x", "int");
    comp->addDataSpace("y", "int");
    Stmt* s0 = new Stmt("int i;", "{}", "{[0]->[0,0,0,0,0]}", {}, {});
    Stmt* s1 = new Stmt("int j;", "{}", "{[0]->[1,0,0,0,0]}", {}, {});
    Stmt* s2 = new Stmt("product[i] = 0;", "{[i]: i >= 0 && i < a}",
                   "{[i]->[2,i,0,0,0]}", {}, {{"product", "{[i]->[i]}"}});
    Stmt* s3 = new Stmt("product[i] += x[i][j] * y[j];",
                   "{[i,j]: i >= 0 && i < a && j >= 0 && j < b}",
                   "{[i,j]->[2,i,1,j,0]}",
                   {{"product", "{[i,j]->[i]}"},
                    {"x", "{[i,j]->[i,j]}"},
                    {"y", "{[i,j]->[j]}"}},
                   {{"product", "{[i,j]->[i]}"}});
    Stmt* s4 = new Stmt("return 0;", "{}", "{[0]->[3,0,0,0,0]}", {}, {});
    comp->addStmt(s0);
    comp->addStmt(s1);
    comp->addStmt(s2);
    comp->addStmt(s3);
    comp->addStmt(s4);

    /* std::string generatedCode = comp->codeGen(); */

    //   EXPECT_EQ(
    //     "#undef s0() \n#define s0()   int i; \n#undef s1() \n#define s1()"
    //     "   int j; \n#undef s2(i) \n#define s2(i)   product[i] = 0; \n#undef"
    //     " s3(i, j) \n#define s3(i, j)   product[i] += x[i][j] * y[j];"
    //     " \n#undef s4() \n#define s4()   return 0; \n\ns0();\ns1();\n"
    //     "for(t2 = 0; t2 <= a-1; t2++) {\n  s2(t2);\n  if (a >= t2+1) {\n"
    //     "    for(t4 = 0; t4 <= b-1; t4++) {\n      s3(t2,t4);\n    }\n "
    //     " }\n}\nif (b >= 1) {\n  for(t2 = max(a',0); t2 <= a-1; t2++) {\n"
    //     "    for(t4 = 0; t4 <= b-1; t4++) {\n      s3(t2,t4);\n "
    //     "   }\n  }\n}\ns4();\n\n", generatedCode);

    delete comp;
}

TEST_F(ComputationTest, ForwardTriangularSolve) {
    // Forward Solve CSR
    // for (i = 0; i < N; i++) /loop over rows
    // s0:tmp = f[i];
    //   for ( k = rowptr[i]; k < rowptr[i+1] -1 ; k++){
    // s1:  tmp -= val[k] * u[col[k]];
    //   }
    // s2:u[i] = tmp/ val[rowptr[i+1]-1];
    //}

    std::vector<std::pair<std::string, std::string> > dataReads;
    std::vector<std::pair<std::string, std::string> > dataWrites;
    dataWrites.push_back(make_pair("tmp", "{[i]->[0]}"));
    dataReads.push_back(make_pair("f", "{[i]->[i]}"));
    Computation* forwardSolve = new Computation();
    forwardSolve->addDataSpace("tmp", "int");
    forwardSolve->addDataSpace("f", "int");
    Stmt* ss0 = new Stmt("tmp = f[i];", "{[i]: 0 <= i < NR}", "{[i] ->[i,0,0,0]}",
             dataReads, dataWrites);
    dataReads.clear();
    dataWrites.clear();

    forwardSolve->addDataSpace("val", "int");
    forwardSolve->addDataSpace("u", "int");
    dataReads.push_back(make_pair("tmp", "{[i,k]->[0]}"));
    dataReads.push_back(make_pair("val", "{[i,k]->[k]}"));
    dataReads.push_back(make_pair("u", "{[i,k]->[t]: t = col(k)}"));
    dataWrites.push_back(make_pair("tmp", "{[i,k]->[0]}"));

    Stmt* ss1 = new Stmt("tmp -= val[k] * u[col[k]];",
             "{[i,k]: 0 <= i && i < NR && rowptr(i) <= k && k < rowptr(i+1)-1}",
             "{[i,k] -> [i,1,k,0]}", dataReads, dataWrites);
    dataReads.clear();

    dataReads.push_back(make_pair("tmp","{[i]->[0]}"));
    dataReads.push_back(make_pair("val","{[i]->[t]: t = rowptr(i+1) - 1}"));
    dataWrites.push_back(make_pair("u","{[i]->[i]}"));

    Stmt* ss2  = new Stmt("u[i] = tmp/ val[rowptr[i+1]-1];",
		"{[i]: 0 <= i && i < NR}",
		"{[i] -> [i,2,0,0]}",
                dataReads,dataWrites);
    forwardSolve->addStmt(ss0);
    forwardSolve->addStmt(ss1);
    forwardSolve->addStmt(ss2);
    /* std::string codegen = forwardSolve->codeGen(); */
    std::string omegString= forwardSolve->toOmegaString();

    /*EXPECT_EQ("\n#Statment 0 (tmp = f[i];) \nDomain: 0\nsymbolic NR"
		    "; { [i] : i >= 0 && -i + NR - 1 >= 0 };\nSchedule:"
		    " 0\n{ [i] -> [i, 0, 0, 0] : i - i = 0 };\n\n#Statment"
		    " 0 (tmp -= val[k] * u[col[k]];) \nDomain: 0\nsymbolic"
		    " NR, rowptr_0(1), rowptr_1(1); { [i, k] : i >= 0 && k "
		    "- rowptr_0(i) >= 0 && -i + NR - 1 >= 0 && -k + "
		    "rowptr_1(i) - 2 >= 0 };\nSchedule: 0\n{ [i, k] ->"
		    " [i, 1, k, 0] : i - i = 0 && k - k = 0 };\n\n#Statment"
		    " 0 (u[i] = tmp/ val[rowptr[i+1]-1];) \nDomain: 0\n"
		    "symbolic NR; { [i] : i >= 0 && -i + NR - 1 >= 0 };\n"
		    "Schedule: 0\n{ [i] -> [i, 2, 0, 0] : i - i = 0 };\n\n",
		    omegString);*/

    delete forwardSolve;
}

TEST_F(ComputationTest, BasicForLoop) {
    // Basic for loop with 2 statements
    // for (i = 0; i < N; i++) /loop over rows
    //     s0:tmp = f[i];
    //     s1:tmp1 = f1[i];
    //}

    std::vector<std::pair<std::string, std::string> > dataReads;
    std::vector<std::pair<std::string, std::string> > dataWrites;

    Computation* forLoopComp = new Computation();

    forLoopComp->addDataSpace("tmp", "int");
    forLoopComp->addDataSpace("f", "int");
    forLoopComp->addDataSpace("tmp1", "int");
    forLoopComp->addDataSpace("f1", "int");
    forLoopComp->addDataSpace("N", "int");

    dataWrites.push_back(make_pair("tmp", "{[i]->[0]}"));
    dataReads.push_back(make_pair("f", "{[i]->[i]}"));
    Stmt* s0 = new Stmt("tmp = f[i];", "{[i]: 0 <= i < N}", "{[i] ->[0,i,0]}",
             dataReads, dataWrites);

    dataReads.clear();
    dataWrites.clear();

    dataWrites.push_back(make_pair("tmp1", "{[i]->[0]}"));
    dataReads.push_back(make_pair("f1", "{[i]->[i]}"));
    Stmt* s1 = new Stmt("tmp1 = f1[i];", "{[i]: 0 <= i < N}", "{[i] ->[0,i,1]}",
             dataReads, dataWrites);

    dataReads.clear();
    dataWrites.clear();

    forLoopComp->addStmt(s0);
    forLoopComp->addStmt(s1);

    std::string omegString= forLoopComp->toOmegaString();
    std::string codegen = forLoopComp->codeGen();

    EXPECT_EQ("s0\n"
              "tmp = f[i];\n"
              "Domain\n"
              "symbolic N; { [__x0, i, __x2] : __x0 = 0 && __x2 = 0 && i >= 0 && -i + N - 1 >= 0 };\n"
              "s1\n"
              "tmp1 = f1[i];\n"
              "Domain\n"
              "symbolic N; { [__x0, i, __x2] : __x0 = 0 && __x2 - 1 = 0 && i >= 0 && -i + N - 1 >= 0 };\n",
              omegString);

    EXPECT_EQ("#undef s0\n"
	      "#undef s_0\n"
	      "#undef s1\n"
	      "#undef s_1\n"
	      "#define s_0(i)   tmp = f[i]; \n"
	      "#define s0(__x0, i, __x2)   s_0(i);\n"
	      "#define s_1(i)   tmp1 = f1[i]; \n"
	      "#define s1(__x0, i, __x2)   s_1(i);\n\n\n"
	      "t1 = 0; \n"
	      "t2 = 0; \n"
	      "t3 = 1; \n\n"
	      "for(t2 = 0; t2 <= N-1; t2++) {\n"
	      "  s0(0,t2,0);\n"
	      "  s1(0,t2,1);\n"
	      "}\n\n"
	      "#undef s0\n"
	      "#undef s_0\n"
	      "#undef s1\n"
	      "#undef s_1\n",
              codegen);

    delete forLoopComp;
}

// Test that omega codegen function generates correct code
// given an omega iteration string
TEST_F(ComputationTest, OmegaCodeGenFromString){
    std::vector<int> arity;
    std::vector<std::string> omegaIterationSpaces;

    arity.push_back(3);
    arity.push_back(3);
    omegaIterationSpaces.push_back("symbolic N; { [__x0, i, __x2] : __x0 = 0 && __x2 = 0 && i >= 0 && -i + N - 1 >= 0 };");
    omegaIterationSpaces.push_back("symbolic N; { [__x0, i, __x2] : __x0 = 0 && __x2 - 1 = 0 && i >= 0 && -i + N - 1 >= 0 };");

    Computation* myComp =  new Computation();
    std::string codegen = myComp->omegaCodeGenFromString(arity, omegaIterationSpaces,"{ [] }");
    EXPECT_EQ("for(t2 = 0; t2 <= N-1; t2++) {\n  s0(0,t2,0);\n  s1(0,t2,1);\n}\n\n",codegen);

    delete myComp;
}

TEST_F(ComputationTest, FirstWriteIndex) {
    // Basic for loop with 2 statements
    // for (i = 0; i < N; i++) /loop over rows
    //     s0:tmp = f[i];
    //     s1:tmp1 = f1[i];
    //}

    Computation* forLoopComp = new Computation();
    forLoopComp->addDataSpace("tmp", "int");
    forLoopComp->addDataSpace("f", "int");
    forLoopComp->addDataSpace("tmp1", "int");
    forLoopComp->addDataSpace("f1", "int");
    forLoopComp->addDataSpace("N", "int");

    Stmt* s0 = new Stmt("tmp = f[i];", "{[i]: 0 <= i < N}", "{[i] ->[0,i,0]}",
                        {{"f", "{[i]->[i]}"}}, {{"tmp", "{[i]->[0]}"}});
    forLoopComp->addStmt(s0);

    Stmt* s1 = new Stmt("tmp1 = f1[i];", "{[i]: 0 <= i < N}", "{[i] ->[0,i,1]}",
                        {{"f1", "{[i]->[i]}"}}, {{"tmp1", "{[i]->[0]}"}});
    forLoopComp->addStmt(s1);

    EXPECT_EQ(0, forLoopComp->firstWriteIndex("$tmp$"));
    EXPECT_EQ(-1, forLoopComp->firstWriteIndex("tmp"));
    EXPECT_EQ(1, forLoopComp->firstWriteIndex("$tmp1$"));
    EXPECT_EQ(-1, forLoopComp->firstWriteIndex("unknown"));

    delete forLoopComp;
}

TEST_F(ComputationTest, ReplaceDataSpaceName) {
    // Basic for loop with 2 statements
    // for (i = 0; i < N; i++) /loop over rows
    //     s0:tmp = f[i];
    //     s1:tmp1 = f1[i];
    //}

    std::vector<std::pair<std::string, std::string> > dataReads;
    std::vector<std::pair<std::string, std::string> > dataWrites;

    Computation* forLoopComp = new Computation();

    forLoopComp->addDataSpace("tmp", "int");
    forLoopComp->addDataSpace("f", "int");
    forLoopComp->addDataSpace("tmp1", "int");
    forLoopComp->addDataSpace("f1", "int");
    forLoopComp->addDataSpace("N", "int");

    dataWrites.push_back(make_pair("tmp", "{[i]->[0]}"));
    dataReads.push_back(make_pair("f", "{[i]->[i]}"));
    Stmt* s0 = new Stmt("tmp = f[i];", "{[i]: 0 <= i < N}", "{[i] ->[0,i,0]}",
             dataReads, dataWrites);

    dataReads.clear();
    dataWrites.clear();

    dataWrites.push_back(make_pair("tmp1", "{[i]->[0]}"));
    dataReads.push_back(make_pair("f1", "{[i]->[i]}"));
    Stmt* s1 = new Stmt("tmp1 = f1[i];", "{[i]: 0 <= i < N}", "{[i] ->[0,i,1]}",
             dataReads, dataWrites);

    dataReads.clear();
    dataWrites.clear();

    forLoopComp->addStmt(s0);
    forLoopComp->addStmt(s1);

    forLoopComp->replaceDataSpaceName("tmp", "notTmp");
    forLoopComp->replaceDataSpaceName("notHere", "here");
    forLoopComp->replaceDataSpaceName("f1", "g1");

    EXPECT_EQ(s0->getStmtSourceCode(), "$notTmp$ = $f$[i];");
    EXPECT_EQ(s1->getStmtSourceCode(), "$tmp1$ = $g1$[i];");
    EXPECT_FALSE(forLoopComp->isDataSpace("$notTmp$"));
    EXPECT_FALSE(forLoopComp->isDataSpace("$here$"));
    EXPECT_FALSE(forLoopComp->isDataSpace("tmp"));
    EXPECT_FALSE(forLoopComp->isDataSpace("notHere"));

    delete forLoopComp;
}

TEST_F(ComputationTest, IsParameterOrReturn) {
    Computation* testComp = new Computation();

    testComp->addParameter("Hi", "int");
    testComp->addDataSpace("how", "int");
    testComp->addDataSpace("are", "int");
    testComp->addReturnValue("you", true);

    EXPECT_TRUE(testComp->isParameter("$Hi$"));
    EXPECT_FALSE(testComp->isParameter("$you$"));
    EXPECT_FALSE(testComp->isParameter("$oops$"));

    EXPECT_TRUE(testComp->isReturnValue("$you$"));
    EXPECT_FALSE(testComp->isReturnValue("$are$"));
    EXPECT_FALSE(testComp->isReturnValue("$oops$"));

    delete testComp;
}

TEST_F(ComputationTest, SSARenaming) {

    Computation* computation = new Computation();

    computation->addParameter("foo", "int");
    computation->addDataSpace("bar", "int");

    Stmt* s1 = new Stmt("bar = foo;",
             "{[0]}",
             "{[0]->[0]}",
             {{"foo", "{[0]->[0]}"}},
             {{"bar", "{[0]->[0]}"}}
         );
    computation->addStmt(s1);

    Stmt* s2 =new Stmt("foo = bar + 1",
             "{[0]}", "{[0]->[1]}",
             {{"bar", "{[0]->[0]}"}},
             {{"foo", "{[0]->[0]}"}}
         );
    computation->addStmt(s2);

    std::string codeGen = computation->codeGen();

    EXPECT_EQ("#undef s0\n#undef s_0\n#undef s1\n#undef s_1\n#define s_0(0)   bar = foo; \n#define s0(__x0)   s_0(0);\n#define s_1(0)   foo = bar + 1 \n#define s1(__x0)   s_1(0);\n\n\nt1 = 1; \n\ns0(0);\ns1(1);\n\n#undef s0\n#undef s_0\n#undef s1\n#undef s_1\n",codeGen);


    delete computation;
}

TEST_F(ComputationTest, AppendComputationSSA) {
    /* Code:
     * useMe =  80;
     * input = useMe * 8;
     * useMe = useMe + input;
     * input = func(input); // inlined
     * useMe = useMe + input;
     *
     * func(int foo):
     * bar = foo;
     * foo = bar + 1;
     * bar = foo + func1(foo); // inlined
     * foo = foo - 1;
     * return foo;
     *
     * func1(int foo):
     * foo = 3 * 21;
     * foo = foo + 1;
     * return foo
     */

    std::vector<std::string> args;
    int pos = 0;
    AppendComputationResult res;
    std::ofstream file;
    file.open("out.c");

    Computation* comp1 = new Computation();

    comp1->addDataSpace("input", "int");
    comp1->addDataSpace("useMe", "int");

    comp1->addStmt(new Stmt (
        "useMe = 80;", "{[0]}", "{[0]->[0]}", {}, {{"useMe", "{[0]->[0]}"}}
    ));

    comp1->addStmt(new Stmt (
        "input = useMe * 8;", "{[0]}", "{[0]->[1]}", {{"useMe", "{[0]->[0]}"}}, {{"input", "{[0]->[0]}"}}
    ));

    comp1->addStmt(new Stmt (
        "useMe = useMe + input;", "{[0]}", "{[0]->[2]}", {{"useMe", "{[0]->[0]}"}, {"input", "{[0]->[0]}"}}, {{"useMe", "{[0]->[0]}"}}
    ));


    Computation* comp2 = new Computation();

    comp2->addParameter("foo", "int");
    comp2->addReturnValue("foo");
    comp2->addDataSpace("bar", "int");

    comp2->addStmt(new Stmt(
        "bar = foo;", "{[0]}", "{[0]->[0]}", {{"foo", "{[0]->[0]}"}}, {{"bar", "{[0]->[0]}"}}
    ));
    comp2->addStmt(new Stmt(
        "foo = bar + 1;", "{[0]}", "{[0]->[1]}", {{"bar", "{[0]->[0]}"}}, {{"foo", "{[0]->[0]}"}}
    ));

    Computation* comp3 = new Computation();

    comp3->addParameter("foo", "int");
    comp3->addReturnValue("foo");

    comp3->addStmt(new Stmt(
        "foo = 3 * 21;", "{[0]}", "{[0]->[0]}", {}, {{"foo", "{[0]->[0]}"}}
    ));
    comp3->addStmt(new Stmt(
        "foo = foo + 1;", "{[0]}", "{[0]->[1]}", {{"foo", "{[0]->[0]}"}}, {{"foo", "{[0]->[0]}"}}
    ));

    args = {"foo"};
    res = comp2->appendComputation(comp3, "{[0]}", "{[0]->[2]}", args);
    pos = res.tuplePosition + 1;

    comp2->addStmt(new Stmt(
        "bar = foo + "+res.returnValues.back()+";", "{[0]}", "{[0]->["+std::to_string(pos++)+"]}",
        {{"foo", "{[0]->[0]}"},{res.returnValues.back(), "{[0]->[0]}"}}, {{"bar", "{[0]->[0]}"}}
    ));
    comp2->addStmt(new Stmt(
        "foo = foo - 1;", "{[0]}", "{[0]->["+std::to_string(pos++)+"]}", {{"foo", "{[0]->[0]}"}}, {{"foo", "{[0]->[0]}"}}
    ));

    args = {"input"};
    res = comp1->appendComputation(comp2, "{[0]}", "{[0]->[3]}", args);
    pos = res.tuplePosition + 1;

    comp1->addStmt(new Stmt("input = "+res.returnValues.back()+";",
        "{[0]}",
        "{[0]->["+std::to_string(pos++)+"]}",
        {{res.returnValues.back(), "{[0]->[0]}"}},
        {{"input", "{[0]->[0]}"}}
    ));

    comp1->addStmt(new Stmt (
        "useMe = useMe + input;", "{[0]}", "{[0]->["+std::to_string(pos++)+"]}", {{"useMe", "{[0]->[0]}"}, {"input", "{[0]->[0]}"}}, {{"useMe", "{[0]->[0]}"}}
    ));

    std::stringstream ss;
    ss << "#undef s0\n#undef s1\n#undef s2\n#undef s3\n#undef s4\n#undef s5\n#undef s6\n#undef s7\n#undef s8\n#undef s9\n#undef s10\n#undef s11\n#undef s12\n";
    ss << "#define s0(__x0)   useMe__w__0 = 80; \n" << "#define s1(__x0)   input__w__6 = useMe__w__0 * 8; \n" << "#define s2(__x0)   useMe__w__7 = useMe__w__0 + input__w__6; \n";
    ss << "#define s3(__x0)   int _iegen_1foo__w__5 = input__w__6; \n" << "#define s4(__x0)   _iegen_1bar__w__3 = _iegen_1foo__w__4; \n";
    ss << "#define s5(__x0)   _iegen_1foo__w__4 = _iegen_1bar__w__3 + 1; \n" << "#define s6(__x0)   int _iegen_0foo__w__2 = _iegen_1foo__w__4; \n";
    ss << "#define s7(__x0)   _iegen_1_iegen_0foo__w__1 = 3 * 21; \n" << "#define s8(__x0)   _iegen_1_iegen_0foo = _iegen_1_iegen_0foo__w__1 + 1; \n";
    ss << "#define s9(__x0)   _iegen_1bar = _iegen_1foo__w__4 + _iegen_1_iegen_0foo; \n" << "#define s10(__x0)   _iegen_1foo = _iegen_1foo__w__4 - 1; \n";
    ss << "#define s11(__x0)   input = _iegen_1foo; \n" << "#define s12(__x0)   useMe = useMe__w__7 + input; \n\n\n";
    ss << "t1 = 0; \n\ns0(0);\ns1(1);\ns2(2);\ns3(3);\ns4(4);\ns5(5);\ns6(6);\ns7(7);\ns8(8);\ns9(9);\ns10(10);\ns11(11);\ns12(12);\n\n";
    ss << "#undef s0\n#undef s1\n#undef s2\n#undef s3\n#undef s4\n#undef s5\n#undef s6\n#undef s7\n#undef s8\n#undef s9\n#undef s10\n#undef s11\n#undef s12\n";
    EXPECT_EQ(comp1->codeGen(), ss.str());

    delete comp1;
    delete comp2;
    delete comp3;

    comp1 = new Computation();

    comp1->addDataSpace("input", "int");

    comp1->addStmt(new Stmt (
        "input = 4 * 8;", "{[0]}", "{[0]->[0]}", {}, {{"input", "{[0]->[0]}"}}
    ));


    comp2 = new Computation();

    comp2->addParameter("foo", "int");
    comp2->addReturnValue("foo");
    comp2->addDataSpace("bar", "int");

    comp2->addStmt(new Stmt(
        "bar = foo;", "{[0]}", "{[0]->[0]}", {{"foo", "{[0]->[0]}"}}, {{"bar", "{[0]->[0]}"}}
    ));

    args = {"input"};
    res = comp1->appendComputation(comp2, "{[0]}", "{[0]->[1]}", args);
    pos = res.tuplePosition + 1;

    comp1->addStmt(new Stmt("input = "+res.returnValues.back()+";",
        "{[0]}",
        "{[0]->["+std::to_string(pos++)+"]}",
        {{res.returnValues.back(), "{[0]->[0]}"}},
        {{"input", "{[0]->[0]}"}}
    ));

    ss.str("");
    ss << "#undef s0\n#undef s1\n#undef s2\n#undef s3\n";
    ss << "#define s0(__x0)   input__w__8 = 4 * 8; \n" << "#define s1(__x0)   int _iegen_2foo = input__w__8; \n";
    ss << "#define s2(__x0)   _iegen_2bar = _iegen_2foo; \n" << "#define s3(__x0)   input = _iegen_2foo; \n\n\n";
    ss << "t1 = 0; \n\ns0(0);\ns1(1);\ns2(2);\ns3(3);\n\n#undef s0\n#undef s1\n#undef s2\n#undef s3\n";
    EXPECT_EQ(comp1->codeGen(), ss.str());

    delete comp1;
    delete comp2;
}

TEST_F(ComputationTest, Colors) {
    // Basic for loop with 2 statements
    // for (i = 0; i < N; i++) /loop over rows
    //     s0:tmp = f[i];
    //     s1:tmp1 = f1[i];
    //}

    std::vector<std::pair<std::string, std::string> > dataReads;
    std::vector<std::pair<std::string, std::string> > dataWrites;

    Computation* forLoopComp = new Computation();

    forLoopComp->addReturnValue("tmp", true);
    forLoopComp->addParameter("f", "int");
    forLoopComp->addDataSpace("tmp1", "int");
    forLoopComp->addParameter("f1", "int");
    forLoopComp->addDataSpace("N", "int");

    dataWrites.push_back(make_pair("tmp", "{[i]->[0]}"));
    dataReads.push_back(make_pair("f", "{[i]->[i]}"));
    Stmt* s0 = new Stmt("tmp = f[i];", "{[i]: 0 <= i < N}", "{[i] ->[0,i,0]}",
             dataReads, dataWrites);

    dataReads.clear();
    dataWrites.clear();

    dataWrites.push_back(make_pair("tmp1", "{[i]->[0]}"));
    dataReads.push_back(make_pair("f1", "{[i]->[i]}"));
    Stmt* s1 = new Stmt("tmp1 = f1[i];", "{[i]: 0 <= i < N}", "{[i] ->[0,i,1]}",
             dataReads, dataWrites);

    dataReads.clear();
    dataWrites.clear();

    forLoopComp->addStmt(s0);
    forLoopComp->addStmt(s1);

    string dotString = forLoopComp->toDotString();

    delete forLoopComp;
}

#pragma mark ConvertToOmega
// Test that we can correctly convert from IEGenLib SparseConstraints to Omega
// Relations
TEST_F(ComputationTest, ConvertToOmega) {
    /* Sets */
    // basic test
    checkOmegaSetConversion("{[i,j] : 0 <= i && i < N && 0 <= j && j < N }",
                            "{[i,j]: 0 <= i < N && 0 <= j < N}");
    // empty set
    checkOmegaSetConversion("{[]}", "{ TRUE }");
    // with simple UF constraints
    checkOmegaSetConversion(
        "{[i,j] : 0 <= i && i < N && 0 <= j && j < M && i=foo(i+1)}",
        "{[i,j]: foo_0(i) = i && 0 <= i < N && 0 <= j < M}");
    // multiple uses of same UF
    checkOmegaSetConversion(
        "{[i,j]: A(i) = A(j) && A(i,j) = A(j)}",
        "{[i,j]: A_0(i) = A_1(i,j) && A_2(i,j) = A_1(i,j)}");

    // replacing constants with variables in tuple.
    checkOmegaSetConversion(
        "{[0,i,j]: A(i) = A(j) && A(i,j) = A(j)}",
        "{[__x0,i,j]: A_0(__x0,i) = A_2(__x0,i,j) && A_1(__x0,i,j) = A_2(__x0,i,j) && __x0 = 0}");

    checkOmegaSetConversion(
        "{[0,i,0,j,1]: A(i) = A(j) && A(i,j) = A(j)}",
        "{[__x0,i,__x2,j,__x4]: A_2(__x0,i,__x2,j) = A_0(__x0,i) && \
A_1(__x0,i,__x2,j) = A_0(__x0,i) && __x4 = 1 && __x0 = 0 && __x2 = 0}");



    checkOmegaSetConversion(
        "{[0,i,2,j,1]: A(i) = A(j) && A(i,j) = A(j)}",
        "{[__x0,i,__x2,j,__x4]: A_2(__x0,i,__x2,j) = A_0(__x0,i) && \
A_1(__x0,i,__x2,j) = A_0(__x0,i) && __x4 = 1 && __x0 = 0 && __x2 = 2}");

    checkOmegaSetConversion(
        "{[0,i,0,j,1,k,0]: A(i) = A(j) && A(i,j) = A(j) && B(j,k) = 0 }",
        "{[__x0,i,__x2,j,__x4,k,__x6]: A_3(__x0,i,__x2,j) = \
A_1(__x0,i) && A_2(__x0,i,__x2,j) = A_1(__x0,i) && \
__x6 = 0 && B_0(__x0,i,__x2,j,__x4,k) = 0 && __x4 = 1 \
&& __x0 = 0 && __x2 = 0}");

    /* Relations */
    // basic test
    checkOmegaRelationConversion(
        "{[i]->[j]: 0 <= i && i < N && 0 <= j && j < N }",
        "{[i] -> [j] : 0 <= i < N && 0 <= j < N}");
    // empty relation
    checkOmegaRelationConversion("{[0]->[0]}", "{[0] -> [0] }");
    // with simple UF constraints
    checkOmegaRelationConversion(
        "{[i,j]->[k]: 0 <= i && i < N && 0 <= j && j < M && i=foo(i+1)}",
        "{[i,j] -> [k] : foo_0(i) = i && 0 <= i < N && 0 <= j < M}");
    // replacement of constants with tuplevariable
    checkOmegaRelationConversion(
        "{[0,i,j]->[k]: 0 <= i && i < N && 0 <= j && j < M && i=foo(i+1)}",
        "{[__x0,i,j] -> [k] : foo_0(__x0,i) = i && __x0 = 0 && 0 <= i < N && 0 <= j < M}");
    // TODO: multiple uses of same UF in a Relation?
}

#pragma mark AppendComputationBasic
TEST_F(ComputationTest, AppendComputationBasic) {
    Computation* comp1 = new Computation();
    Stmt* s1 = new Stmt("s1;", "{[i,j]}", "{[i,j] -> [2,i,1,j,1]}", {}, {});
    comp1->addStmt(s1);

    Computation* comp2 = new Computation();
    Stmt* s2 = new Stmt("s2;", "{[k]: 0 <= k < M}", "{[k] -> [0,k,1]}", {}, {});
    comp2->addStmt(s2);

    Computation* ecomp = new Computation();
    Stmt* es1 = new Stmt(*s1);
    Stmt* es2 = new Stmt("s2;", "{[i,k]: 0 <= i < N && 0 <= k < M}", "{[i,k] -> [2,i,2,k,1]}", {}, {});
    ecomp->addStmt(es1);
    ecomp->addStmt(es2);

    checkAppendComputation(comp1, comp2, "{[i]: 0 <= i < N}", "{[i]->[2,i,2]}", {}, 2,
                           {}, ecomp);

    delete comp1;
    delete comp2;
    delete ecomp;
}

#pragma mark AppendComputationArgumentPassing
TEST_F(ComputationTest, AppendComputationArgumentPassing) {
    Computation* comp1 = new Computation();
    comp1->addDataSpace("myInt", "int");
    comp1->addDataSpace("myDouble", "double");

    Computation* comp2 = new Computation();
    Stmt* s2 = new Stmt("s2;", "{[k]}", "{[k] -> [0,k,1]}", {}, {});
    comp2->addStmt(s2);
    comp2->addParameter("a", "int");
    comp2->addParameter("b", "double");
    comp2->addParameter("c", "float");

    Computation* ecomp = new Computation();
    ecomp->addDataSpace("myInt", "int");
    ecomp->addDataSpace("myDouble", "double");
    ecomp->addDataSpace("_iegen_0a", "int");
    ecomp->addDataSpace("_iegen_0b", "double");
    ecomp->addDataSpace("_iegen_0c", "float");
    Stmt* e_gen_s1 = new Stmt("_iegen_0a = myInt;", "{[i]}", "{[i] -> [2,i,2]}", {{"myInt", "{[i]->[0]}"}}, {{"_iegen_0a", "{[i]->[0]}"}});
    Stmt* e_gen_s2 = new Stmt("_iegen_0b = myDouble;", "{[i]}", "{[i] -> [2,i,3]}", {{"myDouble", "{[i]->[0]}"}}, {{"_iegen_0b", "{[i]->[0]}"}});
    Stmt* e_gen_s3 = new Stmt("_iegen_0c = 0;", "{[i]}", "{[i] -> [2,i,4]}", {}, {{"_iegen_0c", "{[i]->[0]}"}});
    Stmt* es1 = new Stmt("s2;", "{[i,k]}", "{[i,k] -> [2,i,5,k,1]}", {}, {});
    ecomp->addStmt(e_gen_s1);
    ecomp->addStmt(e_gen_s2);
    ecomp->addStmt(e_gen_s3);
    ecomp->addStmt(es1);

    checkAppendComputation(comp1, comp2, "{[i]}", "{[i]->[2,i,2]}", {"myInt", "myDouble", "0"}, 5, {},
                           ecomp);

    delete comp1;
    delete comp2;
    delete ecomp;
}

#pragma mark AppendComputationEmpty
TEST_F(ComputationTest, AppendComputationEmpty) {
    // without params
    Computation* comp1 = new Computation();

    Computation* comp2 = new Computation();

    Computation* ecomp = new Computation();

    checkAppendComputation(comp1, comp2, "{[0]}", "{[0]->[0]}", {}, 0, {}, ecomp);

    delete comp1;
    delete comp2;
    delete ecomp;

    // with params
    comp1 = new Computation();
    comp1->addDataSpace("myInt", "int");

    comp2 = new Computation();
    comp2->addParameter("a", "int");
    comp2->addParameter("b", "double");

    ecomp = new Computation();
    ecomp->addDataSpace("myInt", "int");
    ecomp->addDataSpace("_iegen_1a", "int");
    ecomp->addDataSpace("_iegen_1b", "double");
    Stmt* e_gen_s1 = new Stmt("_iegen_1a = myInt;", "{[i]}", "{[i] -> [2,i,2]}", {{"myInt", "{[i]->[0]}"}}, {{"_iegen_1a", "{[i]->[0]}"}});
    Stmt* e_gen_s2 = new Stmt("_iegen_1b = 3.14159;", "{[i]}", "{[i] -> [2,i,3]}", {}, {{"_iegen_1b", "{[i]->[0]}"}});
    ecomp->addStmt(e_gen_s1);
    ecomp->addStmt(e_gen_s2);

    checkAppendComputation(comp1, comp2, "{[i]}", "{[i]->[2,i,2]}", {"myInt", "3.14159"}, 3, {}, ecomp);

    delete comp1;
    delete comp2;
    delete ecomp;
}


#pragma mark AppendComputationReturnValues
TEST_F(ComputationTest, AppendComputationReturnValues) {
    Computation* comp1 = new Computation();

    Computation* comp2 = new Computation();
    Stmt* s1 = new Stmt("s2;", "{[k]}", "{[k] -> [0,k,1]}", {}, {});
    comp2->addStmt(s1);
    comp2->addDataSpace("res", "int");
    comp2->addReturnValue("res");
    comp2->addReturnValue("0");

    Computation* ecomp = new Computation();
    ecomp->addDataSpace("_iegen_0res", "int");
    Stmt* es1 = new Stmt("s2;", "{[i,k]}", "{[i,k] -> [2,i,2,k,1]}", {}, {});
    ecomp->addStmt(es1);

    checkAppendComputation(comp1, comp2, "{[i]}", "{[i]->[2,i,2]}", {}, 2, {"_iegen_0res", "0"}, ecomp);

    delete comp1;
    delete comp2;
    delete ecomp;
}

#pragma mark AppendComputationComplex
TEST_F(ComputationTest, AppendComputationComplex) {
    // This is an attempt to simulate a more realistic inlining situation,
    // where all features are exercised. Not all details of what we actually
    // want are ironed out yet so it will change.
    // comp3 is appended onto comp2, which is then appended on to comp1.
    Computation* comp1 = new Computation();
    comp1->addDataSpace("index", "int");
    comp1->addDataSpace("N", "int");
    comp1->addDataSpace("tmp", "int");
    Stmt* s1 = new Stmt("tmp = index[0] + 1;", "{[0]}", "{[0]->[0]}",
                        {{"index", "{[0]->[0]}"}}, {{"tmp", "{[0]->[0]}"}});
    Stmt* s2 =
        new Stmt("index[i+1] = i;", "{[i]: 0 <= i < N}", "{[i]->[1,i,0]}",
                 {}, {{"index", "{[i]->[r]: r=i+1}"}});
    comp1->addStmt(s1);
    comp1->addStmt(s2);

    Computation* comp2 = new Computation();
    comp2->addDataSpace("A", "int");
    comp2->addDataSpace("tmp", "int");
    comp2->addParameter("someInteger", "int");
    comp2->addReturnValue("tmp");
    Stmt* s3 =
        new Stmt("A[k] += A[k+1];", "{[k]: 0 <= k < 5}", "{[k]->[0,k,0]}",
                 {{"A", "{[k]->[k]}"}, {"A", "{[k]->[r]: r=k+1}"}},
                 {{"A", "{[k]->[k]}"}});
    Stmt* s4 =
        new Stmt("tmp = A[2] + someInteger;", "{[0]}", "{[0]->[1]}",
                 {{"A", "{[0]->[2]}"}, {"someInteger", "{[0]->[0]}"}},
                 {{"tmp", "{[0]->[0]}"}});
    comp2->addStmt(s3);
    comp2->addStmt(s4);

    Computation* comp3 = new Computation();
    comp3->addDataSpace("asdf", "int");
    Stmt* s5 = new Stmt("asdf = 2;", "{[0]}", "{[0]->[0]}", {}, {{"asdf", "{[0]->[0]}"}});
    comp3->addStmt(s5);

    Computation* ecomp2 = new Computation();
    ecomp2->addDataSpace("A", "int");
    ecomp2->addDataSpace("tmp", "int");
    ecomp2->addDataSpace("_iegen_0asdf", "int");
    ecomp2->addParameter("someInteger", "int");
    ecomp2->addReturnValue("tmp");
    Stmt* e1s1 = new Stmt(*s3);
    Stmt* e1s2 = new Stmt(*s4);
    Stmt* e1s3 = new Stmt("_iegen_0asdf = 2;", "{[0]}", "{[0]->[2]}", {}, {{"_iegen_0asdf", "{[0]->[0]}"}});
    ecomp2->addStmt(e1s1);
    ecomp2->addStmt(e1s2);
    ecomp2->addStmt(e1s3);

    // append comp3 onto comp2
    checkAppendComputation(comp2, comp3, "{[0]}", "{[0]->[2]}", {}, 2, {}, ecomp2);

    Computation* ecomp1 = new Computation();
    ecomp1->addDataSpace("index", "int");
    ecomp1->addDataSpace("N", "int");
    ecomp1->addDataSpace("tmp", "int");
    ecomp1->addDataSpace("_iegen_1A", "int");
    ecomp1->addDataSpace("_iegen_1tmp", "int");
    ecomp1->addDataSpace("_iegen_1someInteger", "int");
    ecomp1->addDataSpace("_iegen_1_iegen_0asdf", "int");
    Stmt* e2s1 = new Stmt(*s1);
    Stmt* e2s2 = new Stmt(*s2);
    Stmt* e2s3 = new Stmt("_iegen_1someInteger = tmp;", "{[i]: 0<=i<N}", "{[i]->[1,i,1]}", {{"tmp", "{[i]->[0]}"}}, {{"_iegen_1someInteger", "{[i]->[0]}"}});
    Stmt* e2s4 = new Stmt("_iegen_1A[k] += _iegen_1A[k+1];",
                         "{[i,k]: 0<=i<N && 0<=k<5}", "{[i,k]->[1,i,2,k,0]}",
                         {{"_iegen_1A", "{[i,k]->[k]}"},
                          {"_iegen_1A", "{[i,k]->[r]: r=k+1}"}},
                         {{"_iegen_1A", "{[i,k]->[k]}"}});
    Stmt* e2s5 = new Stmt("_iegen_1tmp = _iegen_1A[2] + _iegen_1someInteger;", "{[i]: 0<=i<N}",
                         "{[i]->[1,i,3]}", {{"_iegen_1A", "{[i]->[2]}"}, {"_iegen_1someInteger", "{[i]->[0]}"}},
                         {{"_iegen_1tmp", "{[i]->[0]}"}});
    Stmt* e2s6 = new Stmt("_iegen_1_iegen_0asdf = 2;", "{[i]: 0<=i<N}", "{[i]->[1,i,4]}", {}, {{"_iegen_1_iegen_0asdf", "{[i]->[0]}"}});
    ecomp1->addStmt(e2s1);
    ecomp1->addStmt(e2s2);
    ecomp1->addStmt(e2s3);
    ecomp1->addStmt(e2s4);
    ecomp1->addStmt(e2s5);
    ecomp1->addStmt(e2s6);

    // append comp2 (which already has comp3 appended on) onto comp1
    checkAppendComputation(comp1, comp2, "{[i]: 0 <= i < N}",
                           "{[i]->[1,i,1]}", {"tmp"}, 4, {"_iegen_1tmp"}, ecomp1);

    delete comp1;
    delete comp2;
    delete comp3;
    delete ecomp2;
    delete ecomp1;
}

#pragma mark ComputationNamePrefixing
// Check creating a copy of a Computation with prefixed names works properly
TEST_F(ComputationTest, ComputationNamePrefixing) {
    Computation* comp1 = new Computation();
    comp1->addDataSpace("product", "int");
    comp1->addDataSpace("x", "int");
    comp1->addDataSpace("y", "int");
    Stmt* s0 = new Stmt("product[i] += x[i][j] * y[j];",
                        "{[i,j]: i >= 0 && i < a && j >= 0 && j < b}",
                        "{[i,j]->[2,i,1,j,0]}",
                        {{"product", "{[i,j]->[i]}"},
                         {"x", "{[i,j]->[i,j]}"},
                         {"y", "{[i,j]->[j]}"}},
                        {{"product", "{[i,j]->[i]}"}});
    comp1->addStmt(s0);
    Set* iterSpace = new Set("{[i,j]: i >= 0 && i < a && j >= 0 && j < b}");
    iegenlib::Relation* execSchedule = new iegenlib::Relation("{[i,j]->[2,i,1,j,0]}");
    std::string iterSpaceStr = iterSpace->prettyPrintString();
    std::string execScheduleStr = execSchedule->prettyPrintString();
    delete iterSpace;
    delete execSchedule;

    Computation* prefixedComp1 = comp1->getUniquelyNamedClone();
    std::map<std::string, std::string> mapPrefixedComp1 = {
                                                  {"$_iegen_0product$", "int"},
                                                  {"$_iegen_0x$", "int"},
                                                  {"$_iegen_0y$", "int"}
                                                };
    EXPECT_EQ(mapPrefixedComp1, prefixedComp1->getDataSpaces());
    EXPECT_EQ("$_iegen_0product$[i] += $_iegen_0x$[i][j] * $_iegen_0y$[j];",
              prefixedComp1->getStmt(0)->getStmtSourceCode());
    EXPECT_EQ(
        iterSpaceStr,
        prefixedComp1->getStmt(0)->getIterationSpace()->prettyPrintString());
    EXPECT_EQ(
        execScheduleStr,
        prefixedComp1->getStmt(0)->getExecutionSchedule()->prettyPrintString());
    EXPECT_EQ("$_iegen_0product$",
              prefixedComp1->getStmt(0)->getReadDataSpace(0));
    EXPECT_EQ("$_iegen_0x$", prefixedComp1->getStmt(0)->getReadDataSpace(1));
    EXPECT_EQ("$_iegen_0y$", prefixedComp1->getStmt(0)->getReadDataSpace(2));
    EXPECT_EQ("$_iegen_0product$",
              prefixedComp1->getStmt(0)->getWriteDataSpace(0));

    Computation* prefixedComp2 = comp1->getUniquelyNamedClone();
    std::map<std::string, std::string> mapPrefixedComp2 = {
                                                  {"$_iegen_1product$", "int"},
                                                  {"$_iegen_1x$", "int"},
                                                  {"$_iegen_1y$", "int"}
                                                };
    EXPECT_EQ(mapPrefixedComp2,
              prefixedComp2->getDataSpaces());
    EXPECT_EQ("$_iegen_1product$[i] += $_iegen_1x$[i][j] * $_iegen_1y$[j];",
              prefixedComp2->getStmt(0)->getStmtSourceCode());
    EXPECT_EQ(
        iterSpaceStr,
        prefixedComp2->getStmt(0)->getIterationSpace()->prettyPrintString());
    EXPECT_EQ(
        execScheduleStr,
        prefixedComp2->getStmt(0)->getExecutionSchedule()->prettyPrintString());
    EXPECT_EQ("$_iegen_1product$",
              prefixedComp2->getStmt(0)->getReadDataSpace(0));
    EXPECT_EQ("$_iegen_1x$", prefixedComp2->getStmt(0)->getReadDataSpace(1));
    EXPECT_EQ("$_iegen_1y$", prefixedComp2->getStmt(0)->getReadDataSpace(2));
    EXPECT_EQ("$_iegen_1product$",
              prefixedComp2->getStmt(0)->getWriteDataSpace(0));

    // prefix an already-prefixed Computation
    Computation* prefixedComp3 = prefixedComp1->getUniquelyNamedClone();
    std::map<std::string, std::string> mapPrefixedComp3 = {
                                                  {"$_iegen_2_iegen_0product$", "int"},
                                                  {"$_iegen_2_iegen_0x$", "int"},
                                                  {"$_iegen_2_iegen_0y$", "int"}
                                                };
    EXPECT_EQ(mapPrefixedComp3,
              prefixedComp3->getDataSpaces());
    EXPECT_EQ("$_iegen_2_iegen_0product$[i] += $_iegen_2_iegen_0x$[i][j] * $_iegen_2_iegen_0y$[j];",
              prefixedComp3->getStmt(0)->getStmtSourceCode());
    EXPECT_EQ(
        iterSpaceStr,
        prefixedComp3->getStmt(0)->getIterationSpace()->prettyPrintString());
    EXPECT_EQ(
        execScheduleStr,
        prefixedComp3->getStmt(0)->getExecutionSchedule()->prettyPrintString());
    EXPECT_EQ("$_iegen_2_iegen_0product$",
              prefixedComp3->getStmt(0)->getReadDataSpace(0));
    EXPECT_EQ("$_iegen_2_iegen_0x$", prefixedComp3->getStmt(0)->getReadDataSpace(1));
    EXPECT_EQ("$_iegen_2_iegen_0y$", prefixedComp3->getStmt(0)->getReadDataSpace(2));
    EXPECT_EQ("$_iegen_2_iegen_0product$",
              prefixedComp3->getStmt(0)->getWriteDataSpace(0));

    delete comp1;
    delete prefixedComp1;
    delete prefixedComp2;
    delete prefixedComp3;
}


#pragma mark TupleAssignmentUnitTest
TEST_F(ComputationTest, TupleAssignmentUnitTest) {
    checkTupleAssignments("{[i,j]: i = 3 && j = row(i)}", { "3","0"});
}

/*
#pragma mark PolyhedralSplit
TEST_F(ComputationTest, PolyhedralSplit){
    Computation c;
    Set * s0 = new Set("{[0,i,0]: 0 <= i && i < N}");
    Set * s1 = new Set("{[0,i,1]:  0 <= i && i < N}");
    Set * s2 = new Set("{[0,i,2]:   0 <= i && i < N}");
    std::vector< std::pair<int,Set*> > PS
	    = {
		    std::make_pair(0,s0),
		    std::make_pair(1,s1),
		    std::make_pair(2,s2)
	    };

    std::vector<std::vector<std::pair<int,Set*> > > res =
	    c.split(2,PS);
    EXPECT_EQ(res.size(),3);

    res = c.split(0,PS);
    EXPECT_EQ(res.size(),1);

    res = c.split(1,PS);
    EXPECT_EQ(res.size(),1);

    delete s0;
    delete s1;
    delete s2;

    s0 = new Set("{[0,i,1]: 0 <= i && i < N}");
    s1 = new Set("{[0,i,1]:  0 <= i && i < N}");
    s2 = new Set("{[0,i,2]:   0 <= i && i < N}");
    PS = {
		    std::make_pair(0,s0),
		    std::make_pair(1,s1),
		    std::make_pair(2,s2),
	    };


    res = c.split(2,PS);
    EXPECT_EQ(res.size(),2);

    res = c.split(0,PS);
    EXPECT_EQ(res.size(),1);

    res = c.split(1,PS);
    EXPECT_EQ(res.size(),1);

    delete s0;
    delete s1;
    delete s2;
}*/

// TODO: This needs to be updated for the new toDotString() function
TEST_F(ComputationTest, ToDotUnitTest){
    Computation* comp  = new Computation();
    comp->addDataSpace("f", "int[]");
    comp->addDataSpace("tmp", "int[]");
    comp->addStmt(new Stmt("S0: tmp(i) = f(i)", "{[i]: 0 <= i and i < N}",
                  "{[i] -> [0,i,0,0,0]}",
                  { {"f","{[i]->[i]}"}},
                  { {"tmp","{[i]->[i]}"}}));

    comp->addDataSpace("A", "int[][]");
    comp->addDataSpace("u", "int[]");
    comp->addStmt(new Stmt("S1: tmp(i) -= A(i,j) * u(j)",
                  "{[i,j]: 0 <= i and i < N and 0 <= j and  j < i }",
                  "{[i,j] -> [0,i,1,j,0]}",
                  {
                     {"A"  ,"{[i,j]->[i,j]}"},
		     {"tmp","{[i,j]->[i]}"},
		     {"u"  ,"{[i,j]->[j]}"},
                  },
                 { {"tmp","{[i,j]->[i]}"}}));

    comp->addStmt(new Stmt("S2: u(i) = tmp(i)/A(i,i)",
                  "{[i]: 0 <= i and i < N }",
                  "{[i] -> [0,i,2,0,0]}",
                  {
                     {"A"  ,"{[i]->[i,i]}"},
		     {"tmp","{[i]->[i]}"},
		     {"u"  ,"{[i]->[i]}"},
                  },
                  { {"u"  ,"{[i]->[i]}"}}));

    EXPECT_EQ(comp->toDotString(),
              "digraph dataFlowGraph_1{"
              " \nsubgraph cluster2"
	      " {\nstyle = filled;\n"
	      " color = \"\";\n "
	      "label = \"Domain :\\{ [0, i] : i \\>= 0"
	      " && N - 1 \\>= 0 && -i + N - 1 \\>= 0 \\} \""
	      " \nS0[label=\"\\{ [0, i, 0, 0, 0] : i \\>= 0 && -i"
	      " + N - 1 \\>= 0 \\}\\n S0: tmp(i) = f(i)\"][shape=Mrecord]"
	      "[style=bold]  [color=grey];\nS1[label=\"\\{ [0, i, 1, j, 0]"
	      " : i \\>= 0 && j \\>= 0 && -i + N - 1 \\>= 0 &&"
	      " i - j - 1 \\>= 0 \\}\\n"
	      " S1: tmp(i) -= A(i,j) * u(j)\"][shape=Mrecord][style=bold]"
	      "  [color=grey];"
	      "\nS2[label=\"\\{ [0, i, 2, 0, 0] : i \\>= 0 &&"
	      " -i + N - 1 \\>= 0 \\}\\n S2: u(i) = tmp(i)/A(i,i)\"]"
	      "[shape=Mrecord][style=bold]  [color=grey];"
	      "\n}\n"
	      "f[label=\"f[] \"] [shape=box][style=bold][color=grey];\n"
	      "\t\tf->S0[label=\"[i]\"]\n"
	      "tmp[label=\"tmp[] \"] [shape=box][style=bold][color=grey];\n"
	      "\t\tS0->tmp[label=\"[i]\"]\n"
	      "A[label=\"A[] \"] [shape=box][style=bold][color=grey];\n"
	      "\t\tA->S1[label=\"[i, j]\"]\n\t\t"
	      "tmp->S1[label=\"[i]\"]\n"
	      "u[label=\"u[] \"] [shape=box][style=bold][color=grey];\n"
	      "\t\tu->S1[label=\"[j]\"]\n\t\tS1->tmp[label=\"[i]\"]\n\t\t"
	      "A->S2[label=\"[i, i]\"]\n\t\ttmp->S2[label=\"[i]\"]\n\t\t"
	      "u->S2[label=\"[i]\"]\n\t\t"
	      "S2->u[label=\"[i]\"]\n}");
    delete comp;
}


TEST_F(ComputationTest, RescheduleUnitTest){
    Computation * comp1 = new Computation();
    //S0
    comp1->addDataSpace("A", "int[][]");
    comp1->addDataSpace("u", "int[]");
    comp1->addStmt(new Stmt("u[i] =  A[i][i]",
		"{[i]: 0 <= i && i < NR}",
		"{[i] -> [0,i,0,0,0]}",
		{
		   {"A", "{[i] -> [i,i]}"}
		},
		{
		   {"u","{[i]->[i]}"},
		}));
    //S1
    comp1->addDataSpace("t", "int[]");
    comp1->addStmt(new Stmt("t[i] =  A[i][i]",
		"{[i]: 0 <= i && i < NR}",
		"{[i] -> [1,i,0,0,0]}",
		{
		   {"A", "{[i] -> [i,i]}"}
		},
		{
		   {"t","{[i]->[i]}"},
		}));
    //S2
    comp1->addStmt(new Stmt("t[i] =  A[i][i]",
		"{[i]: 0 <= i && i < NR}",
		"{[i] -> [2,i,0,0,0]}",
		{
		   {"A", "{[i] -> [i,i]}"}
		},
		{
		   {"t","{[i]->[i]}"},
		}));

    // Test out of bounds.
    EXPECT_ANY_THROW(comp1->reschedule(-5,10));
    // Test equal size
    EXPECT_ANY_THROW(comp1->reschedule(0,0));

    // Reschedule statement S0 to come just before S2
    comp1->reschedule(0,2);
    checkTransformation(comp1,
		    {"{[1,t1,0,0,0]:0 <= t1 && t1 < NR}",
		    "{[0,t1,0,0,0]:0 <= t1 && t1 < NR}",
		    "{[2,t1,0,0,0]:0 <= t1 && t1 < NR}"});

    // Reschedule statement S1 to come just before S2
    comp1->reschedule(1,2);
    checkTransformation(comp1,
		   {"{[0,t1,0,0,0]:0 <= t1 && t1 < NR}",
		    "{[1,t1,0,0,0]:0 <= t1 && t1 < NR}",
		    "{[2,t1,0,0,0]:0 <= t1 && t1 < NR}"});

    // Reschedule statement S1 to come just before S0
    comp1->reschedule(1,0);
    checkTransformation(comp1,
		   {"{[1,t1,0,0,0]:0 <= t1 && t1 < NR}",
		    "{[0,t1,0,0,0]:0 <= t1 && t1 < NR}",
		    "{[2,t1,0,0,0]:0 <= t1 && t1 < NR}"});

    // Reschdule statement S2 to come just befoer S0
    comp1->reschedule(2,0);
    checkTransformation(comp1,
           {"{[1,t1,0,0,0]:0 <= t1 && t1 < NR}",
		    "{[2,t1,0,0,0]:0 <= t1 && t1 < NR}",
		    "{[0,t1,0,0,0]:0 <= t1 && t1 < NR}"});

    // Reschdule statement S2 to come just befoer S1
    comp1->reschedule(2,1);
    checkTransformation(comp1,
           {"{[2,t1,0,0,0]:0 <= t1 && t1 < NR}",
		    "{[1,t1,0,0,0]:0 <= t1 && t1 < NR}",
		    "{[0,t1,0,0,0]:0 <= t1 && t1 < NR}"});

    // Second Computation Test
    Computation * comp2 = new Computation();
    //S0
    comp2->addStmt(new Stmt("u[i] = A[i][i]",
                "{[i]: 0<= i && i < NR}",
	        "{[i] -> [0,i,0]}",
	        {
		   {"A", "{[i] -> [i,i]}"}
   		},
		{
		   {"u","{[i]->[i]}"},
		}));
    //S1
    comp2->addStmt(new Stmt("t[i] = A[i][i]",
                "{[i]: 0 <= i && i < NR}",
	        "{[i] -> [1,i,0]}",
	        {
		   {"A", "{[i] -> [i,i]}"}
   		},
		{
		   {"t","{[i]->[i]}"},
		}));
    //S2
    comp2->addStmt(new Stmt("t[i] = A[i][i]",
                "{[i]: 0 <= i && i < NR}",
	        "{[i] -> [2,i,0]",
	        {
		   {"A", "{[i] -> [i,i]}"}
   		},
		{
		   {"t","{[i]->[i]}"},
		}));
    //Reschedule statement S0 to come just before S2
    comp2->reschedule(0,2);
    checkTransformation(comp2,
		    {"{[1,t1,0]:0 <= t1 && t1 < NR}",
		     "{[0,t1,0]:0 <= t1 && t1 < NR}",
		     "{[2,t1,0]:0 <= t1 && t1 < NR}"});
    //Reschedule statement S1 to come just before S2
    comp2->reschedule(1,2);
    checkTransformation(comp2,
	    	   {"{[0,t1,0]:0 <= t1 && t1 < NR}",
		    "{[1,t1,0]:0 <= t1 && t1 < NR}",
		    "{[2,t1,0]:0 <= t1 && t1 < NR}"});
    //Reschedule statement S2 to come just before S
    comp2->reschedule(2,0);
    checkTransformation(comp2,
	    	   {"{[1,t1,0]:0 <= t1 && t1 < NR}",
		    "{[2,t1,0]:0 <= t1 && t1 < NR}",
		    "{[0,t1,0]:0 <= t1 && t1 < NR}"});

    //Third computation test
    Computation * comp3 = new Computation();
    //S0
    comp3->addStmt(new Stmt("u[i] = A[i][i]",
                "{[i]: 0<= i && i < NR}",
	        "{[i] -> [0,0,i,0,0]}",
	        {
		   {"A", "{[i] -> [i,i]}"}
   		},
		{
		   {"u","{[i]->[i]}"},
		}));
    //S1
    comp3->addStmt(new Stmt("t[i] = A[i][i]",
                "{[i]: 0 <= i && i < NR}",
	        "{[i] -> [1,0,i,0,0]}",
	        {
		   {"A", "{[i] -> [i,i]}"}
   		},
		{
		   {"t","{[i]->[i]}"},
		}));
    //S2
    comp3->addStmt(new Stmt("t[i] = A[i][i]",
                "{[i]: 0 <= i && i < NR}",
	        "{[i] -> [2,0,i,0,0]}",
	        {
		   {"A", "{[i] -> [i,i]}"}
   		},
		{
		   {"t","{[i]->[i]}"},
		}));
    //S3
    comp3->addStmt(new Stmt("t[i] = A[i][i]",
                "{[i]: 0 <= i && i < NR}",
	        "{[i] -> [2,0,i,0,1]}",
	        {
		   {"A", "{[i] -> [i,i]}"}
   		},
		{
		   {"t","{[i]->[i]}"},
		}));
    //Reschedule statement S0 to come just before S2
    comp3->reschedule(0,2);
    checkTransformation(comp3,
                   {"{[1,0,t1,0,0]:0 <= t1 && t1 < NR}",
		    "{[0,0,t1,0,0]:0 <= t1 && t1 < NR}",
		    "{[2,0,t1,0,0]:0 <= t1 && t1 < NR}",
		    "{[2,0,t1,0,1]:0 <= t1 && t1 < NR}"});
    //Reschedule statement S0 to come just before S1
    comp3->reschedule(0,1);
    checkTransformation(comp3,
                   {"{[0,0,t1,0,0]:0 <= t1 && t1 < NR}",
		    "{[1,0,t1,0,0]:0 <= t1 && t1 < NR}",
		    "{[2,0,t1,0,0]:0 <= t1 && t1 < NR}",
		    "{[2,0,t1,0,1]:0 <= t1 && t1 < NR}"});
    //Reschedule statement S3 to come just before S2
    comp3->reschedule(3,2);
    checkTransformation(comp3,
                   {"{[0,0,t1,0,0]:0 <= t1 && t1 < NR}",
		    "{[1,0,t1,0,0]:0 <= t1 && t1 < NR}",
		    "{[2,0,t1,0,1]:0 <= t1 && t1 < NR}",
		    "{[2,0,t1,0,0]:0 <= t1 && t1 < NR}"});
    //Reschedule statement S2 to come just before S3
    comp3->reschedule(2,3);
    checkTransformation(comp3,
                   {"{[0,0,t1,0,0]:0 <= t1 && t1 < NR}",
		    "{[1,0,t1,0,0]:0 <= t1 && t1 < NR}",
		    "{[2,0,t1,0,0]:0 <= t1 && t1 < NR}",
		    "{[2,0,t1,0,1]:0 <= t1 && t1 < NR}"});
    //Reschedule statement S1 to come just before S3
    comp3->reschedule(1,3);
    checkTransformation(comp3,
                   {"{[0,0,t1,0,0]:0 <= t1 && t1 < NR}",
		    "{[2,0,t1,0,0]:0 <= t1 && t1 < NR}",
		    "{[1,0,t1,0,0]:0 <= t1 && t1 < NR}",
		    "{[2,0,t1,0,1]:0 <= t1 && t1 < NR}"});
    //Reschedule statement S1 to come just before S2
    comp3->reschedule(1,2);
    checkTransformation(comp3,
                   {"{[0,0,t1,0,0]:0 <= t1 && t1 < NR}",
		    "{[1,0,t1,0,0]:0 <= t1 && t1 < NR}",
		    "{[2,0,t1,0,0]:0 <= t1 && t1 < NR}",
		    "{[2,0,t1,0,1]:0 <= t1 && t1 < NR}"});
    delete comp1;
    delete comp2;
    delete comp3;
}


TEST_F(ComputationTest, FusionUnitTest){
    Computation* comp = new Computation();
    //S0
    comp->addStmt(new Stmt("S0",
		"{[i,j]: 0 <= i && i < NR && 0 <= j && j < NC}",
		"{[i,j] -> [0,i,0,j,0]}",
		{},
		{}));

    //S1
    comp->addStmt(new Stmt("S1",
		"{[i,j]: 0 <= i && i < NR && 0 <= j && j < NQ}",
		"{[i,j] -> [1,i,0,j,0]}",
		{},
		{}));

    //S2
    comp->addStmt(new Stmt("S2",
		"{[i,j]: 0 <= i && i < NR && 0 <= j && j < NP}",
		"{[i,j] -> [2,i,0,j,0]}",
		{},
		{}));


    //S3
    comp->addStmt(new Stmt("S3",
		"{[i,j]: 0 <= i && i < NR && 0 <= j && j < NT}",
		"{[i,j] -> [3,i,0,j,0]}",
		{},
		{}));
    // Fise statements S0 and S1 at level 2.
    comp->fuse(0,1,2);
    checkTransformation(comp,
		    {"{[0,t1,0,t2,0]:0 <= t1 && t1 < NR && 0 <= t2 && t2 < NC}",
		    "{[0,t1,1,t2,0]:0 <= t1 && t1 < NR && 0 <= t2 && t2 < NQ}",
		    "{[1,t1,0,t2,0]:0 <= t1 && t1 < NR && 0 <= t2 && t2 < NP}",
		    "{[2,t1,0,t2,0]:0 <= t1 && t1 < NR && 0 <= t2 && t2 < NT}"
		    });
    // Fuse statements S2 and S3 at level 4
    comp->fuse(2,3,4);
    checkTransformation(comp,
		    {"{[0,t1,0,t2,0]:0 <= t1 && t1 < NR && 0 <= t2 && t2 < NC}",
		    "{[0,t1,1,t2,0]:0 <= t1 && t1 < NR && 0 <= t2 && t2 < NQ}",
		    "{[1,t1,0,t2,0]:0 <= t1 && t1 < NR && 0 <= t2 && t2 < NP}",
		    "{[1,t1,0,t2,1]:0 <= t1 && t1 < NR && 0 <= t2 && t2 < NT}"
		    });
    // Fuse statements S3 and S1 at level 4

    comp->fuse(3,1,4);


    checkTransformation(comp,
		    {"{[0,t1,0,t2,0]:0 <= t1 && t1 < NR && 0 <= t2 && t2 < NC}",
		    "{[1,t1,0,t2,2]:0 <= t1 && t1 < NR && 0 <= t2 && t2 < NQ}",
		    "{[1,t1,0,t2,0]:0 <= t1 && t1 < NR && 0 <= t2 && t2 < NP}",
		    "{[1,t1,0,t2,1]:0 <= t1 && t1 < NR && 0 <= t2 && t2 < NT}"
		    });

    delete comp;
}

TEST_F(ComputationTest, DelimitDataSpacesTest) {
    auto *comp = new Computation();
    comp->addDataSpace("asdf", "int");
    comp->addDataSpace("a", "int");
    comp->addDataSpace("sd", "int");
    comp->addDataSpace("x", "int");
    comp->addDataSpace("z", "string");
    comp->addDataSpace("N", "int");

    EXPECT_EQ("int $asdf$ = 3;", comp->delimitDataSpacesInString("int asdf = 3;"));
    EXPECT_EQ("int $asdf$ = 3*$x$;", comp->delimitDataSpacesInString("int asdf = 3*x;"));
    EXPECT_EQ("string $z$ = \"asdf\";", comp->delimitDataSpacesInString("string z = \"asdf\";"));
    EXPECT_EQ("string $z$ = \"\\\"asdf\";", comp->delimitDataSpacesInString("string z = \"\\\"asdf\";"));
    EXPECT_EQ("string $z$ = \"\\\"asdf\\\"\";", comp->delimitDataSpacesInString("string z = \"\\\"asdf\\\"\";"));
    EXPECT_EQ("\"asdf\" == $z$;", comp->delimitDataSpacesInString("\"asdf\" == z;"));
    EXPECT_EQ("if ($z$ == \"\\\"asdf\") {\
    $asdf$ = 5;\
}", comp->delimitDataSpacesInString("if (z == \"\\\"asdf\") {\
    asdf = 5;\
}"));
    EXPECT_EQ("{[i,j]: 0 < i < j && 0 < j < 10}", comp->delimitDataSpacesInString("{[i,j]: 0 < i < j && 0 < j < 10}"));
    EXPECT_EQ("{[i]: 0 < i < $N$}", comp->delimitDataSpacesInString("{[i]: 0 < i < N}"));

    delete comp;
}

TEST_F(ComputationTest, ActiveOutTest) {
    auto *comp = new Computation();
    comp->addParameter("intPtr", "int&");
    comp->addParameter("intCopied", "int");
    comp->addDataSpace("asdf", "int");
    comp->addReturnValue("asdf");
    comp->addReturnValue("0");

    std::unordered_set<std::string> expectedActiveOut = {"$intPtr$", "$asdf$"};
    EXPECT_EQ(comp->getActiveOutValues(), expectedActiveOut);

    delete comp;
}
TEST_F(ComputationTest, NestedUFTest) {
    auto vOmegaReplacer = new VisitorChangeUFsForOmega();
    Set * s = new Set("{[n]: 0 < n && n < NNZ && rowptr(row(n)) <= NNZ }");
    
    EXPECT_NO_THROW(s->acceptVisitor(vOmegaReplacer));
    EXPECT_EQ("{ [n, _x1] : _x1 - row_1(n) = 0 && n - 1 >="
	" 0 && NNZ - rowptr_0(n, _x1) >= 0 && -n + NNZ - 1 >= 0 }",
	      s->prettyPrintString());
    auto ufMaps = vOmegaReplacer->getUFMap();
    ASSERT_EQ(2,ufMaps.size());
    auto ufMapIter = ufMaps.begin(); 
    EXPECT_EQ("row_1", (*ufMapIter).first);
    EXPECT_EQ("row(n)",  (*ufMapIter).second->prettyPrintString(s->getTupleDecl())); 
    
    ufMapIter++;
    EXPECT_EQ("rowptr_0", (*ufMapIter).first);
    EXPECT_EQ("rowptr(_x1)", (*ufMapIter).second->prettyPrintString(s->getTupleDecl())); 
    delete s;
    
    vOmegaReplacer->reset();
    
    

    
    // Tests with expressions in UF parameter.
    s = new Set("{[n]: 0 < n && n < NNZ && rowptr(row(n) + NNZ) <= NNZ }");
    
    EXPECT_NO_THROW(s->acceptVisitor(vOmegaReplacer));
    EXPECT_EQ("{ [n, _x1] : _x1 - row_1(n) = 0 && n - 1 >= 0 &&"
	  " NNZ - rowptr_0(n, _x1) >= 0 && -n + NNZ - 1 >= 0 }",
	      s->prettyPrintString());
    ufMaps = vOmegaReplacer->getUFMap();
    ASSERT_EQ(2,ufMaps.size());
    
    ufMapIter = ufMaps.begin(); 
    EXPECT_EQ("row_1", (*ufMapIter).first);
    EXPECT_EQ("row(n)",  (*ufMapIter).second->prettyPrintString(s->getTupleDecl())); 
    
    ufMapIter++;
    EXPECT_EQ("rowptr_0", (*ufMapIter).first);
    EXPECT_EQ("rowptr(_x1 + NNZ)", (*ufMapIter).second->prettyPrintString(s->getTupleDecl())); 
    vOmegaReplacer->reset();
    
    delete s;
}

TEST_F(ComputationTest,InfiniteNestingTest){
    // Tests on infinite nesting..
    auto vOmegaReplacer = new VisitorChangeUFsForOmega();
    // Doubly nested UF test
    Set* s = new Set("{[n]: 0 < n && n < NNZ && rowptr(row(col(n))) <= NNZ }");
    EXPECT_NO_THROW(s->acceptVisitor(vOmegaReplacer));
    EXPECT_EQ("{ [n, tv1, _x1] : tv1 - col_2(n) = 0 && "
	      "_x1 - row_1(n, tv1) = 0 && n - 1 >= 0 &&"
	      " NNZ - rowptr_0(n, _x1) >= 0 && -n + NNZ"
	      " - 1 >= 0 }",s->prettyPrintString());

    auto ufMaps = vOmegaReplacer->getUFMap();
    ASSERT_EQ(3,ufMaps.size());
    auto ufMapIter = ufMaps.begin(); 
    EXPECT_EQ("col_2", (*ufMapIter).first);
    EXPECT_EQ("col(n)",  (*ufMapIter).second->prettyPrintString(s->getTupleDecl())); 
    
    ufMapIter++;
    EXPECT_EQ("row_1", (*ufMapIter).first);
    EXPECT_EQ("row(tv1)", (*ufMapIter).second->prettyPrintString(s->getTupleDecl())); 
   
    ufMapIter++; 
    EXPECT_EQ("rowptr_0", (*ufMapIter).first);
    EXPECT_EQ("rowptr(tv1)", (*ufMapIter).second->prettyPrintString(s->getTupleDecl())); 

    vOmegaReplacer->reset();
    delete s;
}
TEST_F(ComputationTest, NestedUFComputationTest) {
     Computation* comp = new Computation();
     comp->addStmt(new Stmt("s0","{ [n,k]: 0 <= n && n < NNZ && "
			     "rowptr(row(n) + 1) <= k < P }",
			     "{[n,k]->[0,n,0,k,0]}", {},{}));
     comp->padExecutionSchedules();
     std::string codeGenStr = "";
     EXPECT_NO_THROW(codeGenStr = comp->codeGen());
     EXPECT_EQ("#undef s0\n#undef s_0\n#define s_0(n, k)   s0 \n#define s0(__x0, a1, tv2, __x2, a3, __x4)   s_0(a1, a3);\n\n#undef row(t0)\n#undef row_1(__tv0, __tv1)\n#undef rowptr(t0)\n#undef rowptr_0(__tv0, __tv1, __tv2)\n#define row(t0) row[t0]\n#define row_1(__tv0, __tv1) row(__tv1)\n#define rowptr(t0) rowptr[t0]\n#define rowptr_0(__tv0, __tv1, __tv2) rowptr(__tv2 + 1)\n\nt1 = 0; \nt2 = 0; \nt3 = 0; \nt4 = 0; \nt5 = 0; \nt6 = 0; \n\nfor(t2 = 0; t2 <= NNZ-1; t2++) {\n  t3=row_1(t1,t2);\n  for(t5 = rowptr_0(t1,t2,t3); t5 <= P-1; t5++) {\n    s0(0,t2,t3,0,t5,0);\n  }\n}\n\n#undef s0\n#undef s_0\n#undef row(t0)\n#undef row_1(__tv0, __tv1)\n#undef rowptr(t0)\n#undef rowptr_0(__tv0, __tv1, __tv2)\n",codeGenStr);

    delete comp;

    // Test is directly dumped from synthesis 
    // result
    comp = new Computation();
    comp->addStmt(new Stmt("P.insert(row(n), col2(n))", 
			 "{ [tv0, tv1, tv2] : tv1 - row(tv0) = 0 && "
			 "tv2 - col2(tv0) = 0 && tv0 >= 0 &&"
			 " col2(tv0) >= 0 && row(tv0) >= 0 &&"
			 " NC - 1 >= 0 && NNZ - 1 >= 0 &&"
			 " NR - 1 >= 0 && -tv0 + NNZ - 1 >="
			 " 0 && NC - col2(tv0) - 1 >= 0 &&"
			 " NR - row(tv0) - 1 >= 0 }",
		       	" { [tv0, tv1, tv2] -> [0, a1, 0, a3, 0, a5, 0]"
			" : tv0 - a1 = 0 && tv1 - a3 = 0 &&"
			" tv2 - a5 = 0 }",
			{{"row", "{ [tv0, tv1, tv2] -> [tv3] : tv0"
			" - tv3 = 0 }"},
			{"col2", "{ [tv0, tv1, tv2] -> [tv3] : tv0"
			" - tv3 = 0 }"}},
		        {{"P", "{ [tv0, tv1, tv2] -> [0] }"}}));
     comp->addStmt(new Stmt("ACSR(n,k) = ACOO(n,k)",
			    "{ [n, k] : k - P(row(n), col2(n)) = 0"
			    " && col1(k) - col2(n) = 0 && n >= 0"
			    " && col2(n) >= 0 && row(n) >= 0 &&"
			    " k - rowptr(row(n)) >= 0 && -n + NNZ"
			    " - 1 >= 0 && -k + rowptr(row(n) + 1)"
			    " - 1 >= 0 && NC - col2(n) - 1 >= 0"
			    " && NR - row(n) - 1 >= 0 }",
			    "{ [tv0, tv1] -> [1, a1, 0, a3, 0, 0, 0]"
			    " : tv0 - a1 = 0 && tv1 - a3 = 0 }",
			    {{"ACOO", "{ [n, k] -> [n] : n - n = 0 }"}},
			    {{"ACSR", "{ [n, k] -> [k] : k - k = 0 }"}}));
     comp->padExecutionSchedules();
     codeGenStr = "";
     EXPECT_NO_THROW(codeGenStr = comp->codeGen());
     EXPECT_EQ("#undef s0\n#undef s_0\n#undef s1\n#"
	"undef s_1\n#define s_0(tv0, tv1, tv2)   P.insert(row(n), col2(n)) \n"
	"#define s0(__x0, a1, __x2, a3, __x4, a5, __x6)   s_0(a1, a3, a5);\n"
	"#define s_1(n, k)   ACSR(n,k) = ACOO(n,k) \n"
	"#define s1(__x0, a1, tv2, __x2, a3, __x4, __x5, __x6, _x7)   s_1(a1, a3);\n\n"
	"#undef P(t0,t1)\n#undef P_2(__tv0, __tv1, __tv2, __tv3)\n"
	"#undef col1(t0)\n#undef col1_3(__tv0, __tv1, __tv2, __tv3, __tv4, __tv5)\n"
	"#undef col2(t0)\n#undef col2_1(__tv0, __tv1)\n"
	"#undef row(t0)\n#undef row_0(__tv0, __tv1)\n"
	"#undef rowptr(t0)\n#undef rowptr_4(__tv0, __tv1, __tv2)\n"
	"#undef rowptr_5(__tv0, __tv1, __tv2)\n"
	"#define P(t0,t1) P[t0][t1]\n"
	"#define P_2(__tv0, __tv1, __tv2, __tv3) P(__tv3, __tv2)\n"
	"#define col1(t0) col1[t0]\n"
	"#define col1_3(__tv0, __tv1, __tv2, __tv3, __tv4, __tv5) col1(__tv5)\n"
	"#define col2(t0) col2[t0]\n"
	"#define col2_1(__tv0, __tv1) col2(__tv1)\n"
	"#define row(t0) row[t0]\n"
	"#define row_0(__tv0, __tv1) row(__tv1)\n"
	"#define rowptr(t0) rowptr[t0]\n"
	"#define rowptr_4(__tv0, __tv1, __tv2) rowptr(__tv2)\n"
	"#define rowptr_5(__tv0, __tv1, __tv2) rowptr(__tv2 + 1)\n\n"
	"t1 = 1; \nt2 = 0; \nt3 = 0; \nt4 = 0; \nt5 = 0; \nt6 = 0; \nt7 = 0; \n"
	"t8 = 0; \nt9 = 0; \n\n"
	"if (NR >= 1 && NC >= 1) {\n"
	"  for(t2 = 0; t2 <= NNZ-1; t2++) {\n"
	"    if (row_0(t1,t2) >= 0 && NC >= col2_1(t1,t2)+1 && NR >= "
	"row_0(t1,t2)+1 && col2_1(t1,t2) >= 0) {\n"
	"      t4=row_0(t1,t2);\n"
	"      t6=col2_1(t1,t2);\n"
	"      s0(0,t2,0,t4,0,t6,0);\n"
	"    }\n"
	"  }\n"
	"  for(t2 = 0; t2 <= NNZ-1; t2++) {\n"
	"    if (col2_1(t1,t2) >= 0 && row_0(t1,t2) >= 0 &&"
	" NC >= col2_1(t1,t2)+1 && NR >= row_0(t1,t2)+1) {\n"
	"      t3=row_0(t1,t2);\n"
	"      if (rowptr_5(t1,t2,t3) >= rowptr_4(t1,t2,t3)+1) {\n"
	"        t4=col2_1(t1,t2);\n"
	"        if (rowptr_5(t1,t2,t3) >= P_2(t1,t2,t3,t4)+1 &&"
	" P_2(t1,t2,t3,t4) >= rowptr_4(t1,t2,t3)) {\n"
	"          t6=P_2(t1,t2,t3,t4);\n"
	"          if (col1_3(t1,t2,t3,t4,t5,t6) == col2_1(t1,t2)) {\n"
	"            s1(1,t2,t3,t4,0,t6,0,0,0);\n"
	"          }\n"
	"        }\n"
	"      }\n"
	"    }\n"
	"  }\n"
	"}\n\n"
	"#undef s0\n"
	"#undef s_0\n"
	"#undef s1\n"
	"#undef s_1\n"
	"#undef P(t0,t1)\n"
	"#undef P_2(__tv0, __tv1, __tv2, __tv3)\n"
	"#undef col1(t0)\n"
	"#undef col1_3(__tv0, __tv1, __tv2, __tv3, __tv4, __tv5)\n"
	"#undef col2(t0)\n#undef col2_1(__tv0, __tv1)\n"
	"#undef row(t0)\n"
	"#undef row_0(__tv0, __tv1)\n"
	"#undef rowptr(t0)\n"
	"#undef rowptr_4(__tv0, __tv1, __tv2)\n"
	"#undef rowptr_5(__tv0, __tv1, __tv2)\n",codeGenStr);

}

