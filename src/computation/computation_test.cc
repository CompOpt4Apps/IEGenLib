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

#pragma mark DenseMVCodeGen
TEST(ComputationTest, DenseMVCodeGen) {
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
TEST(ComputationTest, ConvertToOmega) {
    VisitorChangeUFsForOmega* vOmegaReplacer = new VisitorChangeUFsForOmega();

    /* SETS */

    // basic test
    Set* s1 = new Set("{[i,j] : 0 <= i && i < N && 0 <= j && j < N }");
    s1->acceptVisitor(vOmegaReplacer);
    omega::Relation* s1_omega = omega::parser::ParseRelation(
        s1->toOmegaString(vOmegaReplacer->getUFCallDecls()));
    EXPECT_EQ("{[i,j]: 0 <= i < N && 0 <= j < N}\n",
              s1_omega->print_with_subs_to_string());
    delete s1;
    delete s1_omega;
    vOmegaReplacer->reset();

    // with simple UF constraints
    Set* s2 = new Set(
        "{[i,j] : 0 <= i && i < N && 0 <= j && j < M && i=foo(i+1) && "
        "j=foo(4)}");
    s2->acceptVisitor(vOmegaReplacer);
    omega::Relation* s2_omega = omega::parser::ParseRelation(
        s2->toOmegaString(vOmegaReplacer->getUFCallDecls()));
    EXPECT_EQ("{[i,foo_1]: foo_0(i) = i && 0 <= foo_1 < M && 0 <= i < N}\n",
              s2_omega->print_with_subs_to_string());
    delete s2;
    delete s2_omega;
    vOmegaReplacer->reset();

    /* RELATIONS */

    // basic test
    iegenlib::Relation* r1 = new iegenlib::Relation(
        "{[i]->[j]: 0 <= i && i < N && 0 <= j && j < N }");
    r1->acceptVisitor(vOmegaReplacer);
    omega::Relation* r1_omega = omega::parser::ParseRelation(
        r1->toOmegaString(vOmegaReplacer->getUFCallDecls()));
    EXPECT_EQ("{[i] -> [j] : 0 <= i < N && 0 <= j < N}\n",
              r1_omega->print_with_subs_to_string());
    delete r1;
    delete r1_omega;
    vOmegaReplacer->reset();

    // with simple UF constraints
    iegenlib::Relation* r2 = new iegenlib::Relation(
        "{[i,j]->[k]: 0 <= i && i < N && 0 <= j && j < M && i=foo(i+1) && "
        "k=foo(4)}");
    r2->acceptVisitor(vOmegaReplacer);
    omega::Relation* r2_omega = omega::parser::ParseRelation(
        r2->toOmegaString(vOmegaReplacer->getUFCallDecls()));
    EXPECT_EQ("{[i,j] -> [foo_1] : foo_0(i) = i && 0 <= i < N && 0 <= j < M}\n",
              r2_omega->print_with_subs_to_string());
    delete r2;
    delete r2_omega;
    vOmegaReplacer->reset();

    delete vOmegaReplacer;
}
