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
    std::cout << "generated code:\n" << generatedCode;
    /* EXPECT_EQ(originalCode, generatedCode); */

    delete comp;
}
