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

  parflowio.addDataSpace("nsg","int");
  parflowio.addDataSpace("x","int");
  parflowio.addDataSpace("y","int");
  parflowio.addDataSpace("z","int");
  parflowio.addDataSpace("nx","int");
  parflowio.addDataSpace("ny","int");
  parflowio.addDataSpace("nz","int");
  parflowio.addDataSpace("rx","int");
  parflowio.addDataSpace("ry","int");
  parflowio.addDataSpace("rz","int");
  parflowio.addDataSpace("nsg","int");
  parflowio.addDataSpace("errcheck","int");
  parflowio.addDataSpace("x_overlap","int");
  parflowio.addDataSpace("clip_x","int");
  parflowio.addDataSpace("extent_x","int");


  Stmt s1("x_overlap = fminl(clip_x+extent_x, x+nx) - fmaxl(clip_x,x)",
          "{[0] : 0 <= nsg < m_numSubgrids}",
          "{[i]->[0, i, 0]}",
          {
            {"clip_x", "{[0] -> [0]}"},
            {"extent_x", "{[0] -> [0]}"},
            {"x", "{[0] -> [0]}"},
          },
          {
            {"x_overlap", "{[0] -> [0]}"}
          });

  parflowio.addStmt(&s1);


  //Calling 
  cout << "Codegen:\n";
  cout << parflowio.codeGen();
  
  return 0;
}
