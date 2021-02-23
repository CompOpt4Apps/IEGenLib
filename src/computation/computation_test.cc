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
        /* std::cout << iegenSet->prettyPrintStringForOmega() << "\n"; */
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
        /* std::cout << iegenRel->prettyPrintStringForOmega() << "\n"; */
        omega::Relation* omegaRel = omega::parser::ParseRelation(
            iegenRel->toOmegaString(vOmegaReplacer->getUFCallDecls()));
        EXPECT_EQ(expectedOmegaResult + "\n",
                  omegaRel->print_with_subs_to_string());

        delete iegenRel;
        delete omegaRel;
        vOmegaReplacer->reset();
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

    Stmt s0 = Stmt("int i;", "{}", "{[]->[0,0,0,0,0]}", {}, {});
    Stmt s1 = Stmt("int j;", "{}", "{[]->[1,0,0,0,0]}", {}, {});
    Stmt s2 = Stmt("product[i] = 0;", "{[i]: i >= 0 && i < a}",
                   "{[i]->[2,i,0,0,0]}", {}, {{"product", "{[i]->[i]}"}});
    Stmt s3 = Stmt("product[i] += x[i][j] * y[j];",
                   "{[i,j]: i >= 0 && i < a && j >= 0 && j < b}",
                   "{[i,j]->[2,i,1,j,0]}",
                   {{"product", "{[i,j]->[i]}"},
                    {"x", "{[i,j]->[i,j]}"},
                    {"y", "{[i,j]->[j]}"}},
                   {{"product", "{[i,j]->[i]}"}});
    Stmt s4 = Stmt("return 0;", "{}", "{[]->[3,0,0,0,0]}", {}, {});

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
    Computation forwardSolve;
    Stmt ss0("tmp = f[i];", "{[i]: 0 <= i < NR}", "{[i] ->[i,0,0,0]}",
             dataReads, dataWrites);
    dataReads.clear();
    dataWrites.clear();
    dataReads.push_back(make_pair("tmp", "{[i,k]->[]}"));
    dataReads.push_back(make_pair("val", "{[i,k]->[k]}"));
    dataReads.push_back(make_pair("u", "{[i,k]->[t]: t = col(k)}"));
    dataWrites.push_back(make_pair("tmp", "{[i,k]->[]}"));

    Stmt ss1("tmp -= val[k] * u[col[k]];",
             "{[i,k]: 0 <= i && i < NR && rowptr(i) <= k && k < rowptr(i+1)-1}",
             "{[i,k] -> [i,1,k,0]}", dataReads, dataWrites);
    dataReads.clear();
    dataWrites.clear();
    dataReads.push_back(make_pair("tmp", "{[i]->[]}"));
    dataReads.push_back(make_pair("val", "{[i]->[t]: t = rowptr(i+1) - 1}"));
    dataWrites.push_back(make_pair("u", "{[i]->[i]}"));

    Stmt ss2("u[i] = tmp/ val[rowptr[i+1]-1];", "{[i]: 0 <= i && i < NR}",
             "{[i] -> [i,2,0,0]}", dataReads, dataWrites);
    forwardSolve.addStmt(ss0);
    forwardSolve.addStmt(ss1);
    forwardSolve.addStmt(ss2);
    /* std::string codegen = forwardSolve.codeGen(); */
    std::string omegString = forwardSolve.toOmegaString();

    /* EXPECT_EQ( */
    /*     "\n#Statment 0 (tmp = f[i];) \nDomain: 0\nsymbolic NR" */
    /*     "; { [i] : i >= 0 && -i + NR - 1 >= 0 };\nSchedule:" */
    /*     " 0\n{ [i] -> [i, 0, 0, 0] : i - i = 0 };\n\n#Statment" */
    /*     " 0 (tmp -= val[k] * u[col[k]];) \nDomain: 0\nsymbolic" */
    /*     " NR, rowptr_0(1), rowptr_1(1); { [i, k] : i >= 0 && k " */
    /*     "- rowptr_0(i) >= 0 && -i + NR - 1 >= 0 && -k + " */
    /*     "rowptr_1(i) - 2 >= 0 };\nSchedule: 0\n{ [i, k] ->" */
    /*     " [i, 1, k, 0] : i - i = 0 && k - k = 0 };\n\n#Statment" */
    /*     " 0 (u[i] = tmp/ val[rowptr[i+1]-1];) \nDomain: 0\n" */
    /*     "symbolic NR; { [i] : i >= 0 && -i + NR - 1 >= 0 };\n" */
    /*     "Schedule: 0\n{ [i] -> [i, 2, 0, 0] : i - i = 0 };\n\n", */
    /*     omegString); */
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
}

TEST_F(ComputationTest, GeoAcAppendComputation) {

    //Initial Computation that will be appended to
    Computation EvalSplineFComputation;
    //int k;
    //Creating s0
    Stmt s0("int k",
      "{[0]}",
      "{[0]->[0, 0, 0]}",
      {},
      {}
      );
    //Adding s0
    EvalSplineFComputation.addStmt(s0);

    //Creating s1
    //int k = Find_Segment(x, Spline.x_vals, Spline.length, Spline.accel);
    //Find_Segment(double x, double* x_vals, int length, int & prev){
    Stmt s1("double* x_vals = Spline.x_vals; int length = Spline.length; int& prev = Spline.accel",
      "{[0]}",
      "{[0]->[1, 0, 0]}",
      {},
      {}
      );
    //Adding s1
    EvalSplineFComputation.addStmt(s1);

    //Computation that gets appended
    Computation FindSegmentComputation;
    //Creating statement1
    //if(x >= x_vals[i] && x <= x_vals[i+1]) prev = i;
    Stmt s00("if(x >= x_vals[i] && x <= x_vals[i+1]) prev = i",
      "{[i]: i>=0 && i<length}",
      "{[i]->[0, i, 0]}", 
      {},
      {}
      );
    //Adding statement2
    FindSegmentComputation.addStmt(s00);

    //Append
    EvalSplineFComputation.appendComputation(&FindSegmentComputation);

    // check execution schedules of added statements are correct
    EXPECT_EQ("{[i]->[2, i, 0]}",
              EvalSplineFComputation.getStmt(2)->
              getExecutionSchedule()->prettyPrintString());
}
#pragma mark AppendComputation
// Check that the appendComputation method works correctly
TEST_F(ComputationTest, AppendComputation) {
    // initial Computation that will be appended to
    Stmt s1("s1;", "{[i,j]}", "{[i,j] -> [0,i,0,j,0]}", {}, {});
    Stmt s2("asdf();", "{[i]}", "{[i] -> [1,i,0,0,0]}", {}, {});
    Computation comp1;
    comp1.addStmt(s1);
    comp1.addStmt(s2);

    // Computation to append
    Stmt s3("s3;", "{[i]}", "{[i] -> [0,i,0,0,0]}", {}, {});
    Stmt s4("s4;", "{[]}", "{[oneOff0] -> [1,0,0,0,0]: oneOff0 = 0}",{},{});
    Computation comp2;
    comp2.addStmt(s3);
    comp2.addStmt(s4);

    comp1.appendComputation(&comp2);

    // check execution schedules of added statements are correct
    EXPECT_EQ("{[i,comp2_i]->[1,i,0,0,0]}",
             comp1.getStmt(2)->getExecutionSchedule()->prettyPrintString());
    EXPECT_EQ("{[i]->[2,0,0,0,0]}",
             comp1.getStmt(3)->getExecutionSchedule()->prettyPrintString());
}
