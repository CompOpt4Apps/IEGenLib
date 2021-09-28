#include "iegenlib.h"
#include <utility>
#include <fstream>
#include <iostream>
#include <sstream>
/*
 * Instructions to view the codegen file.
 * From root make tutorial. 
 * Run the executable from root to generate codegen.
*/
using iegenlib::Computation;
using namespace std;

int main(int argc, char** argv) {

/*

 for(int i = 0; i<5; i++){
    for (int j = 0; j < 5; j++){
        for (int k = 0; k < 5; k++){
            x[i] += y[j] * z[k];
        } 
    a[i] = i;
    }
}
*/

    Computation* macroComp = new Computation();

    macroComp->addDataSpace("$x$", "int");
    macroComp->addDataSpace("$y$", "int");
    macroComp->addDataSpace("$z$", "int");
    macroComp->addDataSpace("$a$", "int");
    macroComp->addDataSpace("$b$", "int");
    
    Stmt* mc0 = new Stmt(
      "x[i] = y[j] * z[k];",
      "{[i,j,k]: i >= 0 && i<5 && j >= 0 && j<5 && k>=0 && k<5}",
      "{[i,j,k]->[0,i,0,j,0,k,0]}",
        {
          {"$x$", "{[i,j,k]->[i]}"},
          {"$y$", "{[i,j,k]->[j]}"},
          {"$z$", "{[i,j,k]->[k]}"}
        },
        {
          {"$x$", "{[i,k,j]->[i]}"}
        }
    );
    macroComp->addStmt(mc0);

    Stmt* mc1 = new Stmt(
      "a[i] = i;",
      "{[i,j]: i >= 0 && i<5 && j >= 0 && j<5}",
      "{[i,j]->[0,i,0,j,1]}",
        {
          {"$i$", "{[i,j]->[i]}"},
        },
        {
          {"$a$", "{[i,j]->[i]}"}
        }
    );
    macroComp->addStmt(mc1);
    
    macroComp->finalize();
    
    ofstream outStream;
    outStream.open("macro_test.c");
    outStream << macroComp->codeGen();
    outStream.close();

return 0;
}
