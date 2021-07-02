#include "iegenlib.h"
#include <utility>
#include <fstream>
#include <iostream>
#include <sstream>

using iegenlib::Computation;
using namespace std;

int main(int argc, char** argv) {

    // Basic for loop with 2 statements 
    // for (i = 0; i < N; i++) /loop over rows
    //     s0:tmp = f[i];
    //     s1:tmp1 = f1[i];
    //}

    std::vector<std::pair<std::string, std::string> > dataReads;
    std::vector<std::pair<std::string, std::string> > dataWrites;
    
    Computation* forLoopComp = new Computation();
    
    forLoopComp->addDataSpace("$tmp$");
    forLoopComp->addDataSpace("$f$");
    forLoopComp->addDataSpace("$tmp1$");
    forLoopComp->addDataSpace("$f1$");
    forLoopComp->addDataSpace("$N$");
    
    dataWrites.push_back(make_pair("$tmp$", "{[i]->[0]}"));
    dataReads.push_back(make_pair("$f$", "{[i]->[i]}"));
    Stmt* s0 = new Stmt("$tmp$ = $f$[i];", "{[i]: 0 <= i < N}", "{[i] ->[0,i,0]}",
             dataReads, dataWrites);
    
    dataReads.clear();
    dataWrites.clear();
    
    dataWrites.push_back(make_pair("$tmp1$", "{[i]->[0]}"));
    dataReads.push_back(make_pair("$f1$", "{[i]->[i]}"));
    Stmt* s1 = new Stmt("$tmp1$ = $f1$[i];", "{[i]: 0 <= i < N}", "{[i] ->[0,i,1]}",
             dataReads, dataWrites);
    
    dataReads.clear();
    dataWrites.clear();
    
    forLoopComp->addStmt(s0);
    forLoopComp->addStmt(s1);

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
    

    return 0;
}
