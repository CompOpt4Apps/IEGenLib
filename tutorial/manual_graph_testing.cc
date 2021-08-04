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
 
   Computation* forLoopComp = new Computation();

    forLoopComp->addParameter("$f$", "int");
    forLoopComp->addParameter("$f1$", "int");
    
    forLoopComp->addDataSpace("$tmp$", "int");
    forLoopComp->addDataSpace("$tmp1$", "int");
    forLoopComp->addDataSpace("$N$", "int");

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
   // string dotString = forLoopComp->toDotString(); //name of computation
   // dotFileStream << dotString;
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
   dsComp->addDataSpace("$y$", "int");
   dsComp->addDataSpace("$A$", "int");
   dsComp->addDataSpace("$x$", "int");
   
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
  //  string dotString1 = dsComp->toDotString(); //name of computation
  //  dotFileStream1 << dotString;
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

    spsComp->addDataSpace("$y$", "int");
    spsComp->addDataSpace("$A$", "int");
    spsComp->addDataSpace("$x$", "int");
    spsComp->addDataSpace("$rowptr$", "int");

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
//    string dotString2 = spsComp->toDotString(); //name of computation
//    dotFileStream2 << dotString2;
    dotFileStream2.close();
    //Write codegen to a file
    ofstream outStream2;
    outStream2.open("sparse_matrix_multiply.c");
    outStream2 << spsComp->codeGen();
    outStream2.close();

    //Phi Node Computations
    //CODE
    /* 
    int j = 2;
    int N = 1;
    int i = 2
    int foo = 0;
    if (j > N) {
        foo = 4;
    }
    if (i > N) {
        foo = 1;
    }
    int bar = foo;
    */
/*
    Computation* test1Computation = new Computation();
    // add stmts
    test1Computation->addDataSpace("$j$", "int");
    Stmt* s1 = new Stmt("$j$=2;",
        "{[0]}",
        "{[0]->[0]}",
        {},
        {{"$j$", "{[0]->[0]}"}}
    );
    test1Computation->addStmt(s1);

    test1Computation->addDataSpace("$N$", "int");
    Stmt* s2 = new Stmt("$N$=1;",
        "{[0]}",
        "{[0]->[1]}",
        {},
        {{"$N$", "{[0]->[0]}"}}
    );
    test1Computation->addStmt(s2);

    test1Computation->addDataSpace("$i$", "int");
    Stmt* s3 = new Stmt("$i$=2;",
        "{[0]}",
        "{[0]->[2]}",
        {},
        {{"$i$", "{[0]->[0]}"}}
    );
    test1Computation->addStmt(s3);

    test1Computation->addDataSpace("$foo$", "int");
    Stmt* s4 = new Stmt("$foo$=0;",
        "{[0]}",
        "{[0]->[3]}",
        {},
        {{"$foo$", "{[0]->[0]}"}}
    );
    test1Computation->addStmt(s4);

    Stmt* s5 = new Stmt("$foo$=4;",
        "{[0]: j > N}",
        "{[0]->[4]}",
        {},
        {{"$foo$", "{[0]->[0]}"}}
    );
    test1Computation->addStmt(s5);

    Stmt* s6 = new Stmt("$foo$=1;",
        "{[0]: i > N}",
        "{[0]->[5]}",
        {},
        {{"$foo$", "{[0]->[0]}"}}
    );
    test1Computation->addStmt(s6);

    test1Computation->addDataSpace("$bar$", "int");
    Stmt* s7 = new Stmt("$bar$=$foo$;",
        "{[0]}",
        "{[0]->[6]}",
        {{"$foo$", "{[0]->[0]}"}},
        {{"$bar$", "{[0]->[0]}"}}
    );
    test1Computation->addStmt(s7);

    ofstream dotFileStream3("phi_test1.txt");
    cout << "Entering toDot()" << "\n";
    string dotString3 = test1Computation->toDotString(); //name of computation
    dotFileStream3 << dotString3;
    dotFileStream3.close();
    //Write codegen to a file
    ofstream outStream3;
    outStream3.open("phi_test1.c");
    outStream3 << test1Computation->codeGen();
    outStream3.close();
    
    
   int bar = 3;
    int foo = 1;
    for(int i = 0; i < 0; i++){
        foo += 2;
    }
    if(foo < 1){ foo = bar; }
   
       
    Computation* test2Computation = new Computation();

    test2Computation->addDataSpace("$foo$", "int");
    Stmt* s21 = new Stmt("$foo$=1;",
        "{[0]}",
        "{[0]->[0]}",
        {},
        {{"$foo$", "{[0]->[0]}"}}
    );
    test2Computation->addStmt(s21);

    //Inside for loop :
    //for(int i = 0; i < 0; i++)
    Stmt* s22 = new Stmt("$foo$ += 2;",
        "{[i]: i >= 0 && i < 0}",
        "{[0]->[1]}",
        {{"$foo$", "{[0]->[0]}"}},
        {{"$foo$", "{[0]->[0]}"}}
    );
    test2Computation->addStmt(s22);
    
    Stmt* s23 = new Stmt("$foo$ = 0;",
        "{[0]: foo < 2}",
        "{[0]->[2]}",
        {},
        {{"$foo$", "{[0]->[0]}"}}
    );
    test2Computation->addStmt(s23);

    ofstream dotFileStream4("phi_test2.txt");
    cout << "Entering toDot()" << "\n";
    string dotString4 = test2Computation->toDotString(); //name of computation
    dotFileStream4 << dotString4;
    dotFileStream4.close();
    //Write codegen to a file
    ofstream outStream4;
    outStream4.open("phi_test2.c");
    outStream4 << test2Computation->codeGen();
    outStream4.close();
*/   
    return 0; 
}
