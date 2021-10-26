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
    //We can only multiply by INT
    // 5 = BR
    // 5 = BC 
    Relation * rel200 = new Relation(
	     "{[ii,kk,jj,i,j,hr,hc,p] -> [k]: 0 <= ii  and ii < 5 "
	     " and browptr(ii) <= kk and kk < browptr(ii+1)  "
	     " and jj = bcol(kk) and 0 <= hr and hr < 5 and 0 <= hc and hc < 5 "
	     " and p = kk * 25 + hr * 5 + hc and i = ii * 5 + hr "
	     " and j= jj * 5 + hc and rowptr(i) <= k "
	     " and k < rowptr(i+ 1) and  col_inv(i,j) = k and col(k) =j and 0 <= i"
	     " and i < NR and 0 <= j and j < NC}");


    Relation * closure200 = rel200->TransitiveClosure() ;

    std::cout<< closure200->prettyPrintString() << "\n"; 
    delete closure200;
    delete rel200;

    Computation * c = new Computation();
    c->addStmt(new Stmt("ACSR[k] = ABCSR[p];",
			    "{ [ii, kk, jj, hr, hc, p, k] : jj - bcol(kk) = 0"
			    " && k - col_inv(5 ii + hr, 5 jj + hc) = 0 && 5 jj + hc"
			    " - col(k) = 0 && 25 kk + 5 hr + hc - p = 0 && ii >= 0"
			    " && hr >= 0 && hc >= 0 && kk - browptr(ii) >= 0 && -hr"
			    " + 4 >= 0 && -hc + 4 >= 0 && k - rowptr(5 ii + hr) >= 0"
			    " && -ii + NR_BR - 1 >= 0 && -kk + browptr(ii +  1) - 1 >= 0"
			    " && -k + rowptr(5 ii + hr + 1) - 1 >= 0 }",
			    "{[ii, kk, jj, hr, hc, p, k]->[ii, kk, jj, hr, hc, p, k]}",
			    {{"ABCSR" , "{[ii, kk, jj, hr, hc, p, k]->[p]}"}},
			    {{"ACSR" , "{[ii, kk, jj, hr, hc, p, k]->[k]}"}}));

    std::cout << "Header :\n" << 
	    c->codeGenMemoryManagementString() << "\n Body:\n"
	    << c->codeGen() << "\n\n\n";
    delete c;
    
    c = new Computation();
    c->addDataSpace("ACSR","int*");
    c->addDataSpace("ACOO","int*");
    c->addStmt(new Stmt("ACSR[k] = ACOO[n];",
			    "{ [n,i,j,k] : i = row1(n) && j = col1(n) && col2(k) = j &&" 
			    " rowptr(i) <= k <rowptr(i + 1) &&"
			    " col2(k) = col1(n) && 0 <= row1(n) < NR && 0 <= col1(n)"
			    " < NC && P(i,j) = k && 0 <= n < NNZ }",
			    "{[n,i,j,k]->[n,i,j,k]}",
			    {{"ACOO" , "{[n,i,j,k]->[n]}"}},
			    {{"ACSR" , "{[n,i,j,k]->[k]}"}}));

    std::cout << "COO to CSR \nHeader :\n" << 
	    c->codeGenMemoryManagementString() << "\n Body:\n"
	    << c->codeGen() << "\n\n\n";
    delete c;

    
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

    Relation *r1 = new Relation("{[a,b]->[u,v]: b = v}");
    Relation *r2 = new Relation("{[c,d]->[x,y]: y < c}");
    Relation *resultRelation = r1->Union(r2);
    delete resultRelation;
    delete r1;
    delete r2;

    Relation *r3 = new Relation("{[]->[u,v,b]: u = b}");
    Relation *r4 = new Relation("{[]->[u,v,a]: a = 0}");
    Relation *resultRelation2 = r3->Union(r4);
    delete resultRelation2;
    delete r3;
    delete r4;
    
    Relation *r5 = new Relation("{[99,1,2]->[1,2,1]: 0 = 0}");
    Relation *r6 = new Relation("{[1,2,3]->[1,5,3]: 1 = 0}");
    Relation *resultRelation3 = r5->Union(r6);
    delete resultRelation3;
    delete r5;
    delete r6;
    return 0;
}
