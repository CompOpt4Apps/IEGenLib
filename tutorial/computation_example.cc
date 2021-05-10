#include "iegenlib.h"
#include <utility>
#include <fstream>
#include <iostream>

using iegenlib::Computation;
using namespace std;

int main(int argc, char **argv){


/*void polynomial_product(int n, int *A, int *B, int *C) {
for(int k = 0; k < 2*n-1; k++)
S: C[k] = 0;
for(int i = 0; i < n; i++)
   for(int j = 0; j < n; j++)
   T: C[i+j] += A[i] * B[j] ;
}*/
Computation* MyComp = new Computation();
vector < pair<string, string> > dataReads;
vector < pair<string, string> > dataWrites;

//Statement 1 = {"C","[k]->[k]"};
//dataReads.push_back(make_pair(" "," "));
dataWrites.push_back(make_pair("C","{[k]->[k]}"));

//Creating statement1
Stmt* statement1 = new Stmt("C[k] = 0",
                "{[k] : 0 <= k <= -2 + 2n}",
                "{[k]->[0, k, 0]}",
                 dataReads,
                 dataWrites
                 );
cout << "Source statement : " << statement1->getStmtSourceCode() << "\n\t"
     <<"- Iteration Space : "<< statement1->getIterationSpace()->prettyPrintString() << "\n\t"
     << "- Execution Schedule : "<< statement1->getExecutionSchedule()->prettyPrintString() << "\n\t" ;


//Adding statement1
MyComp->addStmt(statement1);
dataReads.clear();
dataWrites.clear();

//Statement 2 = C[i+j] += A[i] * B[j];
dataReads.push_back(make_pair("A","{[i,j]->[i]}"));
dataReads.push_back(make_pair("B","{[i,j]->[j]}"));
dataReads.push_back(make_pair("C","{[i,j]->[k]:k=i+j}"));
dataWrites.push_back(make_pair("C","{[i,j]->[k]:k=i+j}"));

//Creating statement2
Stmt* statement2 = new Stmt("C[i+j] = C[i+j] + A[i] * B[j]",
                "{[i,j] : 0<=i<n && 0<=j<n}",
                "{[i,j] -> [1, i, j]}",
                 dataReads,
                 dataWrites
                 );
cout << "Source statement : " << statement2->getStmtSourceCode() << "\n\t"
     <<"- Iteration Space : "<< statement2->getIterationSpace()->prettyPrintString() << "\n\t"
     << "- Execution Schedule : "<< statement2->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

//Adding statement2
MyComp->addStmt(statement2);
cout << "Stmt2 initialised" << "\n";

//Calling toDot() on the Computation structure
fstream dotFileStream;
cout << "Entering toDot()" << "\n";
MyComp->toDot(dotFileStream,"Example.txt");

cout << "Polynomial product Codegen:\n";
cout << MyComp->codeGen();

// Codegen SPMV
// for(int i = 0; i < NR; i++)
//    for(int k = rowptr(i); k < rowptr(i+1); k++){
//       y[i]+=A[k] * x[col[k]];
//    }
//}
dataReads.clear();
dataWrites.clear();
dataReads.push_back(make_pair("y","{[i,k]->[i]}"));
dataReads.push_back(make_pair("A","{[i,k]->[k]}"));
dataReads.push_back(make_pair("x","{[i,k]->[t]: t = col(k)}"));
dataWrites.push_back(make_pair("y","{[i,k]->[i]}"));
Computation* spmv = new Computation();
Stmt* s1 = new Stmt("y[i]+=A[k] * x[col[k]];" ,/*Statement*/
	"{[i,k]: 0 <= i && i < NR && rowptr(i) <= k && k < rowptr(i+1)}", /*domain*/
	"{[i,k] -> [0,i,k]}", /*execution schedule*/
	dataReads,
	dataWrites);
spmv->addStmt(s1);
std::string code = spmv->codeGen();
cout << "SPMV CodeGen: \n";
cout << code;
spmv->toDot(dotFileStream,"spmv.dot");
cout << "To Omega String \n"
   << spmv->toOmegaString();
// Forward Solve CSR
// for (i = 0; i < N; i++) /loop over rows
//s0:tmp = f[i]; 
//   for ( k = rowptr[i]; k < rowptr[i+1] -1 ; k++){
//s1:  tmp -= val[k] * u[col[k]];
//   }
//s2:u[i] = tmp/ val[rowptr[i+1]-1];
//}

dataReads.clear();
dataWrites.clear();
dataWrites.push_back(make_pair("tmp","{[i]->[]}"));
dataReads.push_back(make_pair("f","{[i]->[i]}"));
Computation* forwardSolve = new Computation();
Stmt* ss0 = new Stmt("tmp = f[i];", "{[i]: 0 <= i < NR}", "{[i] ->[i,0,0,0]}"
		,dataReads,dataWrites);
dataReads.clear();
dataWrites.clear();
dataReads.push_back(make_pair("tmp","{[i,k]->[]}"));
dataReads.push_back(make_pair("val","{[i,k]->[k]}"));
dataReads.push_back(make_pair("u","{[i,k]->[t]: t = col(k)}"));
dataWrites.push_back(make_pair("tmp","{[i,k]->[]}"));

Stmt* ss1 = new Stmt("tmp -= val[k] * u[col[k]];",
		"{[i,k]: 0 <= i && i < NR && rowptr(i) <= k && k < rowptr(i+1)-1}",
		"{[i,k] -> [i,1,k,0]}",
                dataReads,dataWrites);  
dataReads.clear();
dataWrites.clear();
dataReads.push_back(make_pair("tmp","{[i]->[]}"));
dataReads.push_back(make_pair("val","{[i]->[t]: t = rowptr(i+1) - 1}"));
dataWrites.push_back(make_pair("u","{[i]->[i]}"));

Stmt* ss2 = new Stmt("u[i] = tmp/ val[rowptr[i+1]-1];",
		"{[i]: 0 <= i && i < NR}",
		"{[i] -> [i,2,0,0]}",
                dataReads,dataWrites);  
forwardSolve->addStmt(ss0);							
forwardSolve->addStmt(ss1);							
forwardSolve->addStmt(ss2);							

cout << "To Omega String:\n";
cout <<forwardSolve->toOmegaString();

cout << "Forward Solve Codegen";
cout << forwardSolve->codeGen();


cout << "Forward Solve Dot File: forward_solve.dot";
forwardSolve->toDot(dotFileStream,"forward_solve.dot");


Computation* simpleComp = new Computation;
Stmt * ss5 = new Stmt("u[i] =  A[i][i]",
		"{[i]: 0 <= i && i < NR}",
		"{[i] -> [i,0,0,0]}",
		{
		   {"A", "{[i] -> [i,i]}"}
		},
		{
		   {"u","{[i]->[i]}"},
		});
simpleComp->addStmt(ss5);
cout << "To Dot \n" << simpleComp->toDot() << "\n";

cout << "Code Gen \n" << simpleComp->codeGen() << "\n";

}
