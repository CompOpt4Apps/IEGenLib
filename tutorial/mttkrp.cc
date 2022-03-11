#include "iegenlib.h"
#include <utility>
#include <fstream>
#include <iostream>

using iegenlib::Computation;
using namespace std;

int main(int argc, char **argv){



  // this one is dense
  //void mttkrp(int I,int J, int K, int R,double *X,
  //               double *A, double *B, double *C) {
  // for (i = 0; i < I; i++)
  //   for (j = 0; j < J; j++)
  //     for (k = 0; k < K; k++)
  //       for (r = 0; r < R; r++)
  //         A[i,r] += X[i,j,k]*B[j,r]*C[k,r];
  ///
  vector < pair<string, string> > dataReads;
  vector < pair<string, string> > dataWrites;
  Computation mttkrp;
  mttkrp.addDataSpace("X","double*");
  mttkrp.addDataSpace("A","double*");
  mttkrp.addDataSpace("B","double*");
  mttkrp.addDataSpace("C","double*");
  Stmt s0("A(i,r) += X(i,j,k)*B(j,r)*C(k,r)",
                  "{[i,j,k,r] : 0 <= i < I and 0<=j<J and 0<=k<K and 0<=r<R}",
                  "{[i,j,k,r]->[0,i,0,j,0,k,0,r,0]}",
                   dataReads,
                   dataWrites);

  mttkrp.addStmt(&s0);

  // this one is COO
  Computation mttkrp_sps;
  mttkrp_sps.addDataSpace("X","double*");
  mttkrp_sps.addDataSpace("A","double*");
  mttkrp_sps.addDataSpace("B","double*");
  mttkrp_sps.addDataSpace("C","double*");
  Stmt s1("A(x,i,j,k,r) += X(x,i,j,k,r)*B(x,i,j,k,r)*C(x,i,j,k,r)",
                  "{[x,i,j,k,r] :  0<=x< NNZ and i=UFi(x) and j=UFj(x) and k=UFk(x) and 0<=r<R}",
                  "{[x,i,j,k,r]->[0,x,0,i,0,j,0,k,0,r,0]}",
                   dataReads,
                   dataWrites);

  mttkrp_sps.addStmt(&s1);


  //Calling 
  cout << "Codegen:\n";
  cout << mttkrp_sps.codeGen();
  
  return 0;
}
