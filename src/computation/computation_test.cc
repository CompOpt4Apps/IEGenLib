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

#include "code_gen/parser/parser.h"
#include "omega/Relation.h"
#include "set_relation/set_relation.h"

using namespace iegenlib;

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

    //! attempt to convert the given SparseConstraints to Omega format,
    //! EXPECTing that it will equal the given Omega string
    void checkOmegaConversion(SparseConstraints iegenStructure,
                              std::string expectedOmegaResult) {
        SCOPED_TRACE(iegenStructure.prettyPrintStringForOmega());

        // do conversion
        iegenStructure.acceptVisitor(vOmegaReplacer);
        std::cout << iegenStructure.prettyPrintStringForOmega() << "\n";
        omega::Relation* omegaConverted = omega::parser::ParseRelation(
            iegenStructure.toOmegaString(vOmegaReplacer->getUFCallDecls()));
        EXPECT_EQ(expectedOmegaResult + "\n",
                  omegaConverted->print_with_subs_to_string());

        delete omegaConverted;
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

    std::string generatedCode = comp->codeGen();
    EXPECT_EQ("", generatedCode);

    delete comp;
}

#pragma mark ConvertToOmega
// Test that we can correctly convert from IEGenLib SparseConstraints to Omega
// Relations
TEST_F(ComputationTest, ConvertToOmega) {
    /* Sets */
    // basic test
    checkOmegaConversion(Set("{[i,j] : 0 <= i && i < N && 0 <= j && j < N }"),
                         "{[i,j]: 0 <= i < N && 0 <= j < N}");
    // empty set
    checkOmegaConversion(Set("{[]}"), "{ TRUE }");
    // with simple UF constraints
    checkOmegaConversion(
        Set("{[i,j] : 0 <= i && i < N && 0 <= j && j < M && i=foo(i+1)}"),
        "{[i,j]: foo_0(i) = i && 0 <= i < N && 0 <= j < M}");

    /* Relations */
    // basic test
    checkOmegaConversion(
        iegenlib::Relation("{[i]->[j]: 0 <= i && i < N && 0 <= j && j < N }"),
        "{[i] -> [j] : 0 <= i < N && 0 <= j < N}");
    // empty relation
    checkOmegaConversion(iegenlib::Relation("{[]->[]}"), "{ TRUE }");
    // with simple UF constraints
    checkOmegaConversion(
        iegenlib::Relation(
            "{[i,j]->[k]: 0 <= i && i < N && 0 <= j && j < M && i=foo(i+1)}"),
        "{[i,j] -> [k] : foo_0(i) = i && 0 <= i < N && 0 <= j < M}");
}
