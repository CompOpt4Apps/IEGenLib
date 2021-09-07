#include "iegenlib.h"
#include <utility>
#include <fstream>
#include <iostream>
#include <sstream>

/*
 *This is a file represnting a portion of miniFluxdiv, the
 *mini_flux_div_lc function within miniFluxdiv-explain-baseline.cpp
 *using calls to the computation api.
 */
using iegenlib::Computation;
using namespace std;

int main(int argc, char** argv) {
    Computation* miniLc = new Computation();

    
    //Writing header file for codegen
    ofstream headStream;
    headStream.open("miniCodegen.h");
    headStream << miniLc->codeGenMemoryManagementString();
    headStream.close();
 
    //Write codegen to a file
    ofstream outStream;
    outStream.open("miniCodegen.c");
    outStream << miniLc->codeGen();
    outStream.close();
}

//Functions
