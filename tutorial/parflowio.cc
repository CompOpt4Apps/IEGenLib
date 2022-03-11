#include "iegenlib.h"
#include <utility>
#include <fstream>
#include <iostream>

using iegenlib::Computation;
using namespace std;

int main(int argc, char **argv){


  vector < pair<string, string> > dataReads;
  vector < pair<string, string> > dataWrites;
  Computation parflowio;
  parflowio.addDataSpace("A","double*");
  Stmt teststate("A(i) = 0",
                  "{[i] : 0 <= i < I}",
                  "{[i]->[0, i, 0]}",
                   dataReads,
                   dataWrites);

  parflowio.addStmt(&teststate);


  //Calling 
  cout << "Codegen:\n";
  cout << parflowio.codeGen();
  
  return 0;
}
