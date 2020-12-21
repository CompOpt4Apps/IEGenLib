#include "Computation.h"
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
Computation MyComp;
vector < pair<string, string> > dataReads;
vector < pair<string, string> > dataWrites;

//Statement 1 = {"C","[k]->[k]"};
//dataReads.push_back(make_pair(" "," "));
dataWrites.push_back(make_pair("C","{[k]->[k]}"));

//Creating statement1
Stmt statement1("C[k] = 0",
                "{[k] : 0 <= k <= -2 + 2n}",
                "{[k]->[0, k, 0]}",
                 dataReads,
                 dataWrites
                 );
cout << "Source statement : " << statement1.getStmtSourceCode() << "\n\t"
     <<"- Iteration Space : "<< statement1.getIterationSpace()->prettyPrintString() << "\n\t"
     << "- Execution Schedule : "<< statement1.getExecutionSchedule()->prettyPrintString() << "\n\t" ;


//Adding statement1
MyComp.addStmt(statement1);
dataReads.clear();
dataWrites.clear();

//Statement 2 = C[i+j] += A[i] * B[j];
dataReads.push_back(make_pair("A","{[i,j]->[i]}"));
dataReads.push_back(make_pair("B","{[i,j]->[j]}"));
dataReads.push_back(make_pair("C","{[i,j]->[k]:k=i+j}"));
dataWrites.push_back(make_pair("C","{[i,j]->[k]:k=i+j}"));

//Creating statement2
Stmt statement2("C[i+j] = C[i+j] + A[i] * B[j]",
                "{[i,j] : 0<=i<n && 0<=j<n}",
                "{[i,j] -> [1, i, j]}",
                 dataReads,
                 dataWrites
                 );
cout << "Source statement : " << statement2.getStmtSourceCode() << "\n\t"
     <<"- Iteration Space : "<< statement2.getIterationSpace()->prettyPrintString() << "\n\t"
     << "- Execution Schedule : "<< statement2.getExecutionSchedule()->prettyPrintString() << "\n\t" ;

//Adding statement2
MyComp.addStmt(statement2);
cout << "Stmt2 initialised" << "\n";

//Calling toDot() on the Computation structure
fstream dotFileStream;
cout << "Entering toDot()" << "\n";
MyComp.toDot(dotFileStream,"Example.txt");


}