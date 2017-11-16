#include "LevelSchedule.hpp"
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
using namespace std;

extern double sqrt(int);
void ic0_csr(int n, double *val, int * row, int *col)
{
  int i, k,l, m;
  for (i = 0; i < n - 1; i++){
    val[row[i]] = sqrt(val[row[i]]);//S1

    for (m = row[i] + 1; m < row[i+1]; m++){
      val[m] = val[m] / val[row[i]];//S2
    }

    for (m = row[i] + 1; m < row[i+1]; m++) {
      for (k = row[col[m]] ; k < row[col[m]+1]; k++){
        for ( l = m; l < row[i+1] ; l++){
          if (col[l] == col[k] && col[l+1] <= col[k]){
            val[k] -= val[m]* val[l]; //S3
          }
        }
      }
    }
  }
}

int main(){
  int n =500;
  int rowptr[501];
  int diagptr[500];
  int colidx[5000];
  double values[5000];
  double b[5000];
  double y[5000];

  ic0_csr(n, values, rowptr, colidx);

  return 0;
}
