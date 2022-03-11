#include "iegenlib.h"
#include <utility>
#include <fstream>
#include <iostream>

using iegenlib::Computation;
using namespace std;

int main(int argc, char **argv){


  /*void mttkrp(int I,int J, int K, int R,double *X,
   *               double *A, double *B, double *C) {
   * for (i = 0; i < I; i++)
   *   for (j = 0; j < J; j++)
   *     for (k = 0; k < K; k++)
   *       for (r = 0; r < R; r++)
   *         A[i,r] += X[i,j,k]*B[j,r]*C[k,r];
  }*/
  vector < pair<string, string> > dataReads;
  vector < pair<string, string> > dataWrites;
  Computation mttkrp;
  mttkrp.addDataSpace("A","double*");
  Stmt teststate("A(i) = 0",
                  "{[i] : 0 <= i < I}",
                  "{[i]->[0, i, 0]}",
                   dataReads,
                   dataWrites);

  mttkrp.addStmt(&teststate);


  //Calling 
  cout << "Codegen:\n";
  cout << mttkrp.codeGen();
  
  return 0;
}
