#include "iegenlib.h"
#include <utility>
#include <fstream>
#include <iostream>
#include <sstream>
/*
 * Instructions to view the dot file.
 * From root make tutorial. 
 * Run the executable from root to generate dot graphs:
 * ./build/bin/tutorial/manual_graph_testing
 * Copy dot file to r2, then run this command:
 * dot -Tpng filename_dot.txt -o filename.png
 * Copy that generated png back to Onyx and download to
 * local machine to view.
*/
using iegenlib::Computation;
using namespace std;

int main(int argc, char** argv) {

    // Basic for loop with 2 statements 
    // for (i = 0; i < N; i++) /loop over rows
    //     s0:tmp = f[i];
    //     s1:tmp1 = f1[i];
    //}
/*  
   Computation* forLoopComp = new Computation();

    forLoopComp->addParameter("$f$", "int");
    forLoopComp->addParameter("$f1$", "int");
    
    forLoopComp->addDataSpace("$tmp$");
    forLoopComp->addDataSpace("$tmp1$");
    forLoopComp->addDataSpace("$N$");

    Stmt* s0 = new Stmt("$tmp$ = $f$[i];",
        "{[i]: 0 <= i < N}",
        "{[i] ->[0,i,0]}",
        {
            {"$f$", "{[i]->[i]}"}
        },
        {
            {"$tmp$", "{[i]->[0]}"}
        }
    );

    Stmt* s1 = new Stmt("$tmp1$ = $f1$[i];",
        "{[i]: 0 <= i < N}",
        "{[i] ->[0,i,1]}",
        {
            {"$f1$", "{[i]->[i]}"}
        },
        {
            {"$tmp1$", "{[i]->[0]}"}
        }
    ); 
    
    forLoopComp->addStmt(s0);
    forLoopComp->addStmt(s1);

    forLoopComp->addReturnValue("$tmp$", true);
    forLoopComp->addReturnValue("$tmp1$", true);

    //Making toDot, took example from geoac_codegen_driver
    ofstream dotFileStream("for_loop_comp_dot.txt");
    cout << "Entering toDot()" << "\n";
    string dotString = forLoopComp->toDotString(); //name of computation
    dotFileStream << dotString;
    dotFileStream.close();
    //Write codegen to a file
    ofstream outStream;
    outStream.open("for_loop_comp.c");
    outStream << forLoopComp->codeGen();
    outStream.close();
    
    //for (i = 0; i < N; i++) {
    //  for (j=0; j<M; j++) {
    //      y[i] += A[i][j] * x[j];
    //  }
    //}
  
   Computation* dsComp = new Computation();
   dsComp->addDataSpace("$y$");
   dsComp->addDataSpace("$A$");
   dsComp->addDataSpace("$x$");
   
   Stmt* ds0 = new Stmt("$y$[i] += $A$[i][j]*$x$[j];",
     "{[i,j]: 0 <= i < N && 0 <= j < M}",
     "{[i,j] -> [0,i,0,j,0]}",
     {
        {"$y$","{[i,j]->[i]}"},
        {"$A$","{[i,j]->[i,j]}"},
        {"$x$","{[i,j]->[j]}"}
     },
     {
        {"$y$","{[i,j]->[i]}"}
     }
   );
   dsComp->addStmt(ds0); 
 
    //Making toDot, took example from geoac_codegen_driver
    ofstream dotFileStream1("dense_matrix_multiply_dot.txt"); 
    cout << "Entering toDot()" << "\n";
    string dotString1 = dsComp->toDotString(); //name of computation
    dotFileStream1 << dotString;
    dotFileStream1.close();
    //Write codegen to a file
    ofstream outStream1;
    outStream1.open("dense_matrix_multipy.c");
    outStream1 << dsComp->codeGen();
    outStream1.close();
  
    //for (i = 0; i < N; i++) {
    //  for (k=rowptr[i]; k<rowptr[i+1]; k++) {
    //      j = col[k];
    //      y[i] += A[k] * x[j];
    //  }
    //} 
    Computation* spsComp = new Computation();

    spsComp->addDataSpace("$y$");
    spsComp->addDataSpace("$A$");
    spsComp->addDataSpace("$x$");
    spsComp->addDataSpace("$rowptr$");

    Stmt* sps0 = new Stmt(
      "$y$[i] += $A$[k] * $x$[j]",
      "{[i,k,j]: 0 <= i < N && rowptr(i) <= k < rowptr(i+1) && j = col(k)}",
      "{[i,k,j]->[0,i,0,k,0,j,0]}",
        {
          {"$rowptr$", "{[0]->[i]}"}, 
          {"$y$", "{[i,k,j]->[i]}"},
          {"$A$", "{[i,k,j]->[k]}"},
          {"$x$", "{[i,k,j]->[j]}"}
        },
        {
          {"$y$", "{[i,k,j]->[i]}"}
        }
    );
    spsComp->addStmt(sps0);
    
    //Making toDot, took example from geoac_codegen_driver
    ofstream dotFileStream2("sparse_matrix_multiply_dot.txt");
    cout << "Entering toDot()" << "\n";
    string dotString2 = spsComp->toDotString(); //name of computation
    dotFileStream2 << dotString2;
    dotFileStream2.close();
    //Write codegen to a file
    ofstream outStream2;
    outStream2.open("sparse_matrix_multiply.c");
    outStream2 << spsComp->codeGen();
    outStream2.close();
*/
}
