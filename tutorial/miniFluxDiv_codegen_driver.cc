#include "iegenlib.h"
#include <utility>
#include <fstream>
#include <iostream>
#include <sstream>

/*
 *This is a file represnting a portion of miniFluxdiv, the
 *mini_flux_div_lc function within miniFluxdiv-explain-serial.cpp
 *using calls to the computation api.
 */
using iegenlib::Computation;
using namespace std;

int main(int argc, char **argv)
{
    Computation *miniFD = new Computation();

    // 193 double time_spent;
    miniFD->addDataSpace("time_spent", "double");

    // 194 struct timeval  tv1, tv2;
    miniFD->addDataSpace("tv1_tv_sec", "long int");
    miniFD->addDataSpace("tv1_tv_usec", "long int");
    miniFD->addDataSpace("tv2_tv_sec", "long int");
    miniFD->addDataSpace("tv2_tv_usec", "long int");

    // 195 int idx,ix,iy,iz;
    miniFD->addDataSpace("idx", "int");
    miniFD->addDataSpace("ix", "int");
    miniFD->addDataSpace("iy", "int");
    miniFD->addDataSpace("iz", "int");

    // 196 int numCell= config.getInt("numCell");
    // TODO

    // 197 int numBox= config.getInt("numBox");
    // TODO

    // 198 int nGhost = NGHOST;
//    miniFD->addDataSpace("nGhost", "int");
//
//    Stmt *s3 = new Stmt("NGhost = 5;",
//                        "{[0]}",                                            // Iteration schedule
//                        "{[0]->[" + std::to_string(newTuplePos + 1) + "]}", // Execution schedule
//                        {},                                                 // Data reads
//                        {{"nGhost", "{[0]->[0]}"}}                        // Data writes
//    );
//
//    miniFD->addStmt(s3);

    // 199 int numComp= NCOMP;

//    // Writing header file for codegen
//    ofstream headStream;
//    headStream.open("miniCodegen.h");
//    headStream << miniLc->codeGenMemoryManagementString();
//    headStream.close();
//
//    // Write codegen to a file
//    ofstream outStream;
//    outStream.open("miniCodegen.c");
//    outStream << miniLc->codeGen();
//    outStream.close();
}

// Functions
