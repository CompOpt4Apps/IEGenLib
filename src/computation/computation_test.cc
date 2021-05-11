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
        Computation::resetNumRenames();
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
    }

    //! Test that appending a computation to another yields the correct results.
    //! The passed-in computations are modified but not adopted, and should be
    //! freed after this test.
    //! \param[in,out] appendedTo Computation that is appended to (the 'caller
    //! function')
    //! \param[in] appendedComp Computation appended onto another (the 'callee'
    //! function)
    //! \param[in] argsList list of arguments to pass the appended Computation
    //! \param[in] appendDepth loop nesting depth to append into
    //! \param[in] expectedTuplePosition expected last used tuple position at
    //! insertion level
    //! \param[in] expectedReturnValues string versions of expected return values
    //! \param[in] expectedComp Computation that appender should equal after append
    void checkAppendComputation(
        Computation* appendedTo, Computation* appendedComp,
        std::vector<std::string> argsList, unsigned int appendDepth,
        int expectedTuplePosition,
        std::vector<std::string> expectedReturnValues, Computation* expectedComp) {
        AppendComputationResult result = appendedTo->appendComputation(appendedComp, argsList,
                                                   appendDepth);

        EXPECT_EQ(expectedTuplePosition, result.tuplePosition);
        EXPECT_EQ(expectedReturnValues, result.returnValues);
        expectComputationsEqual(expectedComp, appendedTo);
    }
    
    void checkToDotString(Computation* comp, std::string expectedDot){
        EXPECT_EQ(comp->toDotString(),expectedDot);
    }

    //! EXPECT with gTest that two Computations are equal, component by component.
    void expectComputationsEqual(Computation* expectedComp, Computation* actualComp) {
        ASSERT_EQ(expectedComp->getNumStmts(), actualComp->getNumStmts());

        for (unsigned int i = 0; i < expectedComp->getNumStmts(); ++i) {
            SCOPED_TRACE("Statement " + std::to_string(i));

            Stmt* expectedStmt = expectedComp->getStmt(i);
            Stmt* actualStmt = actualComp->getStmt(i);

            EXPECT_EQ(expectedStmt->getStmtSourceCode(), actualStmt->getStmtSourceCode());

            EXPECT_EQ(expectedStmt->getIterationSpace()->prettyPrintString(),
                      actualStmt->getIterationSpace()->prettyPrintString());

            EXPECT_EQ(expectedStmt->getExecutionSchedule()->prettyPrintString(),
                      actualStmt->getExecutionSchedule()->prettyPrintString());

            ASSERT_EQ(expectedStmt->getNumReads(), actualStmt->getNumReads());
            for (unsigned int j = 0; j < expectedStmt->getNumReads(); ++j) {
                EXPECT_EQ(expectedStmt->getReadDataSpace(j),
                          actualStmt->getReadDataSpace(j));
                EXPECT_EQ(expectedStmt->getReadRelation(j)->prettyPrintString(),
                          actualStmt->getReadRelation(j)->prettyPrintString());
            }

            ASSERT_EQ(expectedStmt->getNumWrites(), actualStmt->getNumWrites());
            for (unsigned int j = 0; j < expectedStmt->getNumWrites(); ++j) {
                EXPECT_EQ(expectedStmt->getWriteDataSpace(j),
                          actualStmt->getWriteDataSpace(j));
                EXPECT_EQ(expectedStmt->getWriteRelation(j)->prettyPrintString(),
                          actualStmt->getWriteRelation(j)->prettyPrintString());
            }
        }

        EXPECT_EQ(expectedComp->getDataSpaces(), actualComp->getDataSpaces());

        ASSERT_EQ(expectedComp->getNumParams(), actualComp->getNumParams());
        for (unsigned int i = 0; i < expectedComp->getNumParams(); ++i) {
            SCOPED_TRACE("Parameter " + std::to_string(i));
            EXPECT_EQ(expectedComp->getParameterName(i), actualComp->getParameterName(i));
            EXPECT_EQ(expectedComp->getParameterType(i), actualComp->getParameterType(i));
        }

        EXPECT_EQ(expectedComp->getReturnValues(), actualComp->getReturnValues());
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

    Stmt* s0 = new Stmt("int i;", "{}", "{[]->[0,0,0,0,0]}", {}, {});
    Stmt* s1 = new Stmt("int j;", "{}", "{[]->[1,0,0,0,0]}", {}, {});
    Stmt* s2 = new Stmt("product[i] = 0;", "{[i]: i >= 0 && i < a}",
                   "{[i]->[2,i,0,0,0]}", {}, {{"product", "{[i]->[i]}"}});
    Stmt* s3 = new Stmt("product[i] += x[i][j] * y[j];",
                   "{[i,j]: i >= 0 && i < a && j >= 0 && j < b}",
                   "{[i,j]->[2,i,1,j,0]}",
                   {{"product", "{[i,j]->[i]}"},
                    {"x", "{[i,j]->[i,j]}"},
                    {"y", "{[i,j]->[j]}"}},
                   {{"product", "{[i,j]->[i]}"}});
    Stmt* s4 = new Stmt("return 0;", "{}", "{[]->[3,0,0,0,0]}", {}, {});

    Computation* comp = new Computation();
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
    dataWrites.push_back(make_pair("tmp", "{[i]->[]}"));
    dataReads.push_back(make_pair("f", "{[i]->[i]}"));
    Computation* forwardSolve = new Computation();
    Stmt* ss0 = new Stmt("tmp = f[i];", "{[i]: 0 <= i < NR}", "{[i] ->[i,0,0,0]}",
             dataReads, dataWrites);
    dataReads.clear();
    dataWrites.clear();
    dataReads.push_back(make_pair("tmp", "{[i,k]->[]}"));
    dataReads.push_back(make_pair("val", "{[i,k]->[k]}"));
    dataReads.push_back(make_pair("u", "{[i,k]->[t]: t = col(k)}"));
    dataWrites.push_back(make_pair("tmp", "{[i,k]->[]}"));

    Stmt* ss1 = new Stmt("tmp -= val[k] * u[col[k]];",
             "{[i,k]: 0 <= i && i < NR && rowptr(i) <= k && k < rowptr(i+1)-1}",
             "{[i,k] -> [i,1,k,0]}", dataReads, dataWrites);
    dataReads.clear();
    dataWrites.clear();
    dataReads.push_back(make_pair("tmp","{[i]->[]}"));
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
    checkOmegaRelationConversion("{[]->[]}", "{ TRUE }");
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

#pragma mark AppendComputation0Depth
TEST_F(ComputationTest, AppendComputation0Depth) {
    // initial Computation that will be appended to
    Computation* comp1 = new Computation();
    Stmt* s1 = new Stmt("s1;", "{[i,j]}", "{[i,j] -> [0,i,0,j,0]}", {}, {});
    Stmt* s2 = new Stmt("s2;", "{[i]}", "{[i] -> [1,i,0,0,0]}", {}, {});
    comp1->addStmt(s1);
    comp1->addStmt(s2);

    // Computation to append
    Computation* comp2 = new Computation();
    Stmt* s3 = new Stmt("s3;", "{[i]}", "{[i] -> [0,i,0,0,0]}", {}, {});
    Stmt* s4 = new Stmt("s4;", "{[0]}", "{[0] -> [1,0,0,0,0]}", {}, {});
    comp2->addStmt(s3);
    comp2->addStmt(s4);

    // Computation expected to result from appending
    Computation* expectedComp = new Computation();
    Stmt* es1 = new Stmt(*s1);
    Stmt* es2 = new Stmt(*s2);
    Stmt* es3 = new Stmt("s3;", "{[i]}", "{[i] -> [2,i,0,0,0]}", {}, {});
    Stmt* es4 = new Stmt("s4;", "{[0]}", "{[0] -> [3,0,0,0,0]}", {}, {});
    expectedComp->addStmt(es1);
    expectedComp->addStmt(es2);
    expectedComp->addStmt(es3);
    expectedComp->addStmt(es4);

    // perform test
    checkAppendComputation(comp1, comp2, {}, 0, 3, {}, expectedComp);

    delete comp1, comp2, expectedComp;
}

#pragma mark AppendComputationNonzeroDepth
TEST_F(ComputationTest, AppendComputationNonzeroDepth) {
    Computation* comp1 = new Computation();
    Stmt* s1 = new Stmt("s1;", "{[i,j]}", "{[i,j] -> [2,i,1,j,1]}", {}, {});
    comp1->addStmt(s1);

    Computation* comp2 = new Computation();
    Stmt* s2 = new Stmt("s2;", "{[k]}", "{[k] -> [0,k,1]}", {}, {});
    comp2->addStmt(s2);

    Computation* ecomp = new Computation();
    Stmt* es1 = new Stmt(*s1);
    Stmt* es2 = new Stmt("s2;", "{[i,k]}", "{[i,k] -> [2,i,2,k,1]}", {}, {});
    ecomp->addStmt(es1);
    ecomp->addStmt(es2);

    checkAppendComputation(comp1, comp2, {}, 1, 2, {}, ecomp);

    delete comp1, comp2, ecomp;
}

#pragma mark AppendComputationArgumentPassing
TEST_F(ComputationTest, AppendComputationArgumentPassing) {
    Computation* comp1 = new Computation();
    Stmt* s1 = new Stmt("s1;", "{[i,j]}", "{[i,j] -> [2,i,1,j,1]}", {}, {});
    comp1->addStmt(s1);
    comp1->addDataSpace("myInt");
    comp1->addDataSpace("myDouble");

    Computation* comp2 = new Computation();
    Stmt* s2 = new Stmt("s2;", "{[k]}", "{[k] -> [0,k,1]}", {}, {});
    comp2->addStmt(s2);
    comp2->addParameter("a", "int");
    comp2->addParameter("b", "double");
    comp2->addParameter("c", "float");

    Computation* ecomp = new Computation();
    Stmt* es1 = new Stmt(*s1);
    Stmt* e_gen_s1 = new Stmt("int _iegen_0a = myInt;", "{[i]}", "{[i] -> [2,i,2]}", {{"myInt", "{[i]->[0]}"}}, {{"_iegen_0a", "{[i]->[0]}"}});
    Stmt* e_gen_s2 = new Stmt("double _iegen_0b = myDouble;", "{[i]}", "{[i] -> [2,i,3]}", {{"myDouble", "{[i]->[0]}"}}, {{"_iegen_0b", "{[i]->[0]}"}});
    Stmt* e_gen_s3 = new Stmt("float _iegen_0c = 0;", "{[i]}", "{[i] -> [2,i,4]}", {}, {{"_iegen_0c", "{[i]->[0]}"}});
    Stmt* es2 = new Stmt("s2;", "{[i,k]}", "{[i,k] -> [2,i,5,k,1]}", {}, {});
    ecomp->addStmt(es1);
    ecomp->addStmt(e_gen_s1);
    ecomp->addStmt(e_gen_s2);
    ecomp->addStmt(e_gen_s3);
    ecomp->addStmt(es2);

    checkAppendComputation(comp1, comp2, {"myInt", "myDouble", "0"}, 1, 5, {},
                           ecomp);

    delete comp1, comp2, ecomp;
}

#pragma mark AppendComputationEmpty
TEST_F(ComputationTest, AppendComputationEmpty) {
    // without params
    Computation* comp1 = new Computation();
    Stmt* s1 = new Stmt("s1;", "{[i,j]}", "{[i,j] -> [2,i,1,j,1]}", {}, {});
    comp1->addStmt(s1);

    Computation* comp2 = new Computation();

    Computation* ecomp = new Computation();
    Stmt* es1 = new Stmt(*s1);
    ecomp->addStmt(es1);

    checkAppendComputation(comp1, comp2, {}, 1, 1, {}, ecomp);

    delete comp1, comp2, ecomp;

    // with params
    comp1 = new Computation();
    s1 = new Stmt("s1;", "{[i,j]}", "{[i,j] -> [2,i,1,j,1]}", {}, {});
    comp1->addStmt(s1);
    comp1->addDataSpace("myInt");

    comp2 = new Computation();
    comp2->addParameter("a", "int");
    comp2->addParameter("b", "double");

    ecomp = new Computation();
    es1 = new Stmt(*s1);
    Stmt* e_gen_s1 = new Stmt("int _iegen_1a = myInt;", "{[i]}", "{[i] -> [2,i,2]}", {{"myInt", "{[i]->[0]}"}}, {{"_iegen_1a", "{[i]->[0]}"}});
    Stmt* e_gen_s2 = new Stmt("double _iegen_1b = 3.14159;", "{[i]}", "{[i] -> [2,i,3]}", {}, {{"_iegen_1b", "{[i]->[0]}"}});
    ecomp->addStmt(es1);
    ecomp->addStmt(e_gen_s1);
    ecomp->addStmt(e_gen_s2);

    checkAppendComputation(comp1, comp2, {"myInt", "3.14159"}, 1, 3, {}, ecomp);

    delete comp1, comp2, ecomp;
}

#pragma mark AppendComputationReturnValues
TEST_F(ComputationTest, AppendComputationReturnValues) {
    Computation* comp1 = new Computation();
    Stmt* s1 = new Stmt("s1;", "{[i,j]}", "{[i,j] -> [2,i,1,j,1]}", {}, {});
    comp1->addStmt(s1);

    Computation* comp2 = new Computation();
    Stmt* s2 = new Stmt("s2;", "{[k]}", "{[k] -> [0,k,1]}", {}, {});
    comp2->addStmt(s2);
    comp2->addDataSpace("res");
    comp2->addReturnValue("res");
    comp2->addReturnValue("0");

    Computation* ecomp = new Computation();
    Stmt* es1 = new Stmt("s1;", "{[i,j]}", "{[i,j] -> [2,i,1,j,1]}", {}, {});
    Stmt* es2 = new Stmt("s2;", "{[i,k]}", "{[i,k] -> [2,i,2,k,1]}", {}, {});
    ecomp->addStmt(es1);
    ecomp->addStmt(es2);

    checkAppendComputation(comp1, comp2, {}, 1, 2, {"_iegen_0res", "0"}, ecomp);

    delete comp1, comp2, ecomp;
}

#pragma mark ComputationNamePrefixing
// Check creating a copy of a Computation with prefixed names works properly
TEST_F(ComputationTest, DISABLED_ComputationNamePrefixing) {
    Computation* comp1 = new Computation();
    Stmt* s0 = new Stmt("$product$[i] += $x$[i][j] * $y$[j];",
                        "{[i,j]: i >= 0 && i < a && j >= 0 && j < b}",
                        "{[i,j]->[2,i,1,j,0]}",
                        {{"$product$", "{[i,j]->[i]}"},
                         {"$x$", "{[i,j]->[i,j]}"},
                         {"$y$", "{[i,j]->[j]}"}},
                        {{"$product$", "{[i,j]->[i]}"}});
    comp1->addStmt(s0);
    Set* iterSpace = new Set("{[i,j]: i >= 0 && i < a && j >= 0 && j < b}");
    iegenlib::Relation* execSchedule = new iegenlib::Relation("{[i,j]->[2,i,1,j,0]}");
    std::string iterSpaceStr = iterSpace->prettyPrintString();
    std::string execScheduleStr = execSchedule->prettyPrintString();
    delete iterSpace;
    delete execSchedule;

    Computation* prefixedComp1 = comp1->getUniquelyNamedClone();
    EXPECT_EQ("_iegen_0$product$[i] += _iegen_0$x$[i][j] * _iegen_0$y$[j];",
              prefixedComp1->getStmt(0)->getStmtSourceCode());
    EXPECT_EQ(
        iterSpaceStr,
        prefixedComp1->getStmt(0)->getIterationSpace()->prettyPrintString());
    EXPECT_EQ(
        execScheduleStr,
        prefixedComp1->getStmt(0)->getExecutionSchedule()->prettyPrintString());
    EXPECT_EQ("_iegen_0$product$",
              prefixedComp1->getStmt(0)->getReadDataSpace(0));
    EXPECT_EQ("_iegen_0$x$", prefixedComp1->getStmt(0)->getReadDataSpace(1));
    EXPECT_EQ("_iegen_0$y$", prefixedComp1->getStmt(0)->getReadDataSpace(2));
    EXPECT_EQ("_iegen_0$product$",
              prefixedComp1->getStmt(0)->getWriteDataSpace(0));

    Computation* prefixedComp2 = comp1->getUniquelyNamedClone();
    EXPECT_EQ("_iegen_1$product$[i] += _iegen_1$x$[i][j] * _iegen_1$y$[j];",
              prefixedComp2->getStmt(0)->getStmtSourceCode());
    EXPECT_EQ(
        iterSpaceStr,
        prefixedComp2->getStmt(0)->getIterationSpace()->prettyPrintString());
    EXPECT_EQ(
        execScheduleStr,
        prefixedComp2->getStmt(0)->getExecutionSchedule()->prettyPrintString());
    EXPECT_EQ("_iegen_1$product$",
              prefixedComp2->getStmt(0)->getReadDataSpace(0));
    EXPECT_EQ("_iegen_1$x$", prefixedComp2->getStmt(0)->getReadDataSpace(1));
    EXPECT_EQ("_iegen_1$y$", prefixedComp2->getStmt(0)->getReadDataSpace(2));
    EXPECT_EQ("_iegen_1$product$",
              prefixedComp2->getStmt(0)->getWriteDataSpace(0));

    delete comp1, prefixedComp1, prefixedComp2;

    // separate Computation
    Computation* comp2 = new Computation();
    Stmt* s2 = new Stmt("$product$[i] += $x$[i][j] * $y$[j];",
                        "{[i,j]: i >= 0 && i < a && j >= 0 && j < b}",
                        "{[i,j]->[2,i,1,j,0]}",
                        {{"$product$", "{[i,j]->[i]}"},
                         {"$x$", "{[i,j]->[i,j]}"},
                         {"$y$", "{[i,j]->[j]}"}},
                        {{"$product$", "{[i,j]->[i]}"}});
    comp1->addStmt(s0);

    Computation* prefixedComp3 = comp2->getUniquelyNamedClone();
    EXPECT_EQ("_iegen_2$product$[i] += _iegen_2$x$[i][j] * _iegen_2$y$[j];",
              prefixedComp3->getStmt(0)->getStmtSourceCode());
    EXPECT_EQ(
        iterSpaceStr,
        prefixedComp3->getStmt(0)->getIterationSpace()->prettyPrintString());
    EXPECT_EQ(
        execScheduleStr,
        prefixedComp3->getStmt(0)->getExecutionSchedule()->prettyPrintString());
    EXPECT_EQ("_iegen_2$product$",
              prefixedComp3->getStmt(0)->getReadDataSpace(0));
    EXPECT_EQ("_iegen_2$x$", prefixedComp3->getStmt(0)->getReadDataSpace(1));
    EXPECT_EQ("_iegen_2$y$", prefixedComp3->getStmt(0)->getReadDataSpace(2));
    EXPECT_EQ("_iegen_2$product$",
              prefixedComp3->getStmt(0)->getWriteDataSpace(0));

    delete comp2, prefixedComp3;
}


#pragma mark TupleAssignmentUnitTest
TEST_F(ComputationTest, TupleAssignmentUnitTest) {
    checkTupleAssignments("{[i,j]: i = 3 && j = row(i)}", { "3","0"});
}

#pragma mark ToDotTest
TEST_F(ComputationTest, ToDotUnitTest){
    Computation* forwardSolve = new Computation();
    std::vector<std::pair<std::string, std::string> > dataReads;
    std::vector<std::pair<std::string, std::string> > dataWrites;
    dataWrites.push_back(make_pair("tmp", "{[i]->[]}"));
    dataReads.push_back(make_pair("f", "{[i]->[i]}"));
    Stmt* ss0 = new Stmt("tmp = f[i];", "{[i]: 0 <= i < NR}", "{[i] ->[i,0,0,0]}",
             dataReads, dataWrites);
    dataReads.clear();
    dataWrites.clear();
    dataReads.push_back(make_pair("tmp", "{[i,k]->[]}"));
    dataReads.push_back(make_pair("val", "{[i,k]->[k]}"));
    dataReads.push_back(make_pair("u", "{[i,k]->[t]: t = col(k)}"));
    dataWrites.push_back(make_pair("tmp", "{[i,k]->[]}"));

    Stmt* ss1 = new Stmt("tmp -= val[k] * u[col[k]];",
             "{[i,k]: 0 <= i && i < NR && rowptr(i) <= k && k < rowptr(i+1)-1}",
             "{[i,k] -> [i,1,k,0]}", dataReads, dataWrites);
    dataReads.clear();
    dataWrites.clear();
    dataReads.push_back(make_pair("tmp","{[i]->[]}"));
    dataReads.push_back(make_pair("val","{[i]->[t]: t = rowptr(i+1) - 1}"));
    dataWrites.push_back(make_pair("u","{[i]->[i]}"));

    Stmt* ss2  = new Stmt("u[i] = tmp/ val[rowptr[i+1]-1];",
		"{[i]: 0 <= i && i < NR}",
		"{[i] -> [i,2,0,0]}",
                dataReads,dataWrites);
    forwardSolve->addStmt(ss0);
    forwardSolve->addStmt(ss1);
    forwardSolve->addStmt(ss2);
    
}
