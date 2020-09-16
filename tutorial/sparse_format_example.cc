/*!
 * \file sparse_format_example.cc
 *
 * \brief This file contains sparse format examples for IEGenLib C++ API.
 *
 * \date Date Started: 8/29/20
 *
 * \authors Floriana Ciaglia, Anna Rift, Shivani Singh
 *
 * Copyright (c) 2011-2013, Colorado State University <br>
 * Copyright (c) 2015, University of Arizona <br>
 * All rights reserved. <br>
 * See COPYING for details. <br>
 */

#include <iegenlib.h>

#include <iostream>

using iegenlib::Relation;
using iegenlib::Set;

int main(int ac, char **av) {
    // Test COO WRT DENSE
    std::cout << "=> Starting example COO_WRT_DENSE\n\n";

    Relation *r_coo = new Relation(
        "{[n] -> [i,j] : row(n) = i && col(n) = j "
        "&& 0 <= n && n < NNZ && Dense(i,j) > 0}");
    std::cout << "COO relation:\n" << r_coo->prettyPrintString() << "\n";
    std::cout << std::endl;

    delete r_coo;

    // Test CSR WRT DENSE

    std::cout << "=> Starting example CSR_WRT_DENSE\n\n";
    Relation *r_csr = new Relation(
        "{[n] -> [i,j] :n >= rptr(i) "
        "&& n < rptr(i+1) && col(n) = j "
        "&& 0 <= n && n < NNZ && Dense(i,j) > 0}");
    std::cout << "CSR relation:\n" << r_csr->prettyPrintString() << "\n";
    std::cout << std::endl;

    delete r_csr;

    // Test COO_CSR_1
    std::cout << "=> Starting example COO_CSR_1" << std::endl << std::endl;
    r_coo = new Relation(
        "{[n] -> [i,j] : row(n) = i && col(n) = j "
        "&& 0 <= n && n < NNZ }");
    std::cout << "COO relation: " << r_coo->prettyPrintString() << std::endl;
    r_csr = new Relation(
        "{[n] -> [i,j] :n >= rptr(i) "
        "&& n < rptr(i+1) && rowcol_inv(i,j) = n "
        "&& 0 <= n && n < NNZ }");
    std::cout << "CSR relation: " << r_csr->prettyPrintString() << std::endl;
    Relation *r_csr_inv = r_csr->Inverse();
    std::cout << "CSR relation inverse: " << r_csr_inv->prettyPrintString()
              << std::endl;
    Relation *r_coo_csr_comp = r_csr_inv->Compose(r_coo);
    std::cout << "Composition of CSR inverse and COO relations: "
              << r_coo_csr_comp->prettyPrintString() << std::endl;
    std::cout << std::endl;

    delete r_coo;
    delete r_csr;
    delete r_csr_inv;
    delete r_coo_csr_comp;

    // Test COO_DENSE COMPOSE
    std::cout << "=> Starting example COO_DENSE_COMPOSE" << std::endl
              << std::endl;
    Relation *dense = new Relation(
        "{ [n1] -> [i,j] :"
        " n1 = i * 9 + j }");
    std::cout << "Dense relation: " << dense->prettyPrintString() << std::endl;
    Relation *coo_inv = new Relation(
        "{[i,j] -> [n2] : rowcol_inv(n2,0) = i && rowcol_inv(n2,1) = j "
        "&& 0 <= n2 && n2 < NNZ }");
    std::cout << "COO inverse relation: " << coo_inv->prettyPrintString()
              << std::endl;
    Relation *result = coo_inv->Compose(dense);
    std::cout << "Composition of COO inverse and dense relation: "
              << result->prettyPrintString() << std::endl;
    std::cout << std::endl;
    delete dense;
    delete coo_inv;
    delete result;

    // Test COO_BSR
    std::cout << "=> Starting example COO_BCSR" << std::endl << std::endl;
    Relation *Flatten = new Relation(
        "{ [n] -> [ti,tj,iz,jz,in,zn,riz,rjz,rin,rjn] :"
        " n = iz * 9 + jz }");
    std::cout << "Flatten relation: " << Flatten->prettyPrintString()
              << std::endl;
    Relation *Zeros = new Relation(
        " { [i,j] -> [ti, tj, iz, jz, in, zn, riz, rjz, rin, rjn]: "
        " Dense (i,j) = 0 && Dense(in,jn) != 0  && iz = i && jz = j"
        " && i = ti * 9 + riz && j = ti * 7 + rjz && in = ti * 9 + rin"
        " && in = ti * 7 + rin && 0 <= riz && riz <= 9 && 0 <= rjz "
        " && rjz <= 7 && 0 <= rin && rin <= 9 && 0 <= rjz && rjz<=7"
        "}");
    std::cout << "Zeroes relation: " << Zeros->prettyPrintString() << std::endl;
    delete Flatten;
    delete Zeros;

    return 0;
}
