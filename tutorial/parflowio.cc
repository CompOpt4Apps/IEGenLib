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

// Statement 1 
// int x_overlap = fminl(clip_x+extent_x, x+nx) - fmaxl(clip_x,x); 

  Stmt s1("x_overlap = fminl(clip_x+extent_x, x+nx) - fmaxl(clip_x,x)",
          "{[0] : 0 <= nsg < m_numSubgrids}",
          "{[nsg]->[0, nsg, 0]}",
          {
            {"clip_x", "{[0] -> [0]}"},
            {"extent_x", "{[0] -> [0]}"},
            {"x", "{[0] -> [0]}"},
          },
          {
            {"x_overlap", "{[0] -> [0]}"}
          });

  parflowio.addStmt(&s1);


 //  Statement 2  
 // int y_overlap = fminl(clip_y+extent_y, y+ny) - fmaxl(clip_y,y); 

  parflowio.addDataSpace("y_overlap","int");
  parflowio.addDataSpace("clip_y","int");
  parflowio.addDataSpace("extent_y","int");

  Stmt s2("y_overlap = fminl(clip_y+extent_y, y+ny) - fmaxl(clip_y,y)",
          "{[0] : 0 <= nsg < m_numSubgrids}",
          "{[nsg]->[0, nsg, 0]}",
          {
            {"clip_y", "{[0] -> [0]}"},
            {"extent_y", "{[0] -> [0]}"},
            {"y", "{[0] -> [0]}"},
          },
          {
            {"y_overlap", "{[0] -> [0]}"}
          });

  parflowio.addStmt(&s2);

  // statement 3 
  /*
    for (k=0; k<nz; k++){
      for(i=0;i<ny;i++){
          // Determine the indices of the first element of the pencil in the
          // global space
          int gx = x;
          int gy = y + i  
  */

  Stmt s3("gx = x",
          "{[k,i,nsg] : 0 <= k < nz && 0<=i<ny && 0 <= nsg < m_numSubgrids }",
          "{[nsg,k,i]->[0, nsg, 0, k, 0,i,0 ]}",
          {
            {"x", "{[0] -> [0]}"},
            {"y", "{[0] -> [0]}"},
          },
          {
            {"gx", "{[0] -> [0]}"}
          });

  parflowio.addStmt(&s3);

  // statement 4
  /* gy = y + i  */

  Stmt s4("gy = y + i ",
          "{[k,i,nsg] : 0 <= k < nz && 0<=i<ny && 0 <= nsg < m_numSubgrids }",
          "{[nsg,k,i]->[0, nsg, 0, k, 0,i,0 ]}",
          {
            {"y", "{[0] -> [0]}"},
          },
          {
            {"gy", "{[0] -> [0]}"}
          });

    parflowio.addStmt(&s4);

// statement 5
// int cx = gx - clip_x;
  Stmt s5("cx = gx - clip_x",
          "{[k,i,nsg] : 0 <= k < nz && 0<=i<ny && 0 <= nsg < m_numSubgrids }",
          "{[nsg,k,i]->[0, nsg, 0, k, 0,i,0 ]}",
          {
            {"gx", "{[0] -> [0]}"},
            {"clip_x", "{[0] -> [0]}"} 
          },
          {
            {"cx", "{[0] -> [0]}"}
          });

    parflowio.addStmt(&s5);

// statement 6
// cy = gy - clip_y;

  Stmt s6("gy - clip_y;",
        "{[k,i,nsg] : 0 <= k < nz && 0<=i<ny && 0 <= nsg < m_numSubgrids }",
        "{[nsg,k,i]->[0, nsg, 0, k, 0,i,0 ]}",
        {
          {"gy", "{[0] -> [0]}"},
          {"clip_y", "{[0] -> [0]}"}

        },
        {
          {"cy", "{[0] -> [0]}"}
        });

    parflowio.addStmt(&s6);

// statement 7
// cy = gy - clip_y;

  Stmt s7("gy - clip_y;",
      "{[k,i,nsg] : 0 <= k < nz && 0<=i<ny && 0 <= nsg < m_numSubgrids }",
      "{[nsg,k,i]->[0, nsg, 0, k, 0,i,0 ]}",
      {
        {"gy", "{[0] -> [0]}"},
        {"clip_y", "{[0] -> [0]}"}

      },
      {
        {"cy", "{[0] -> [0]}"}
      });

    parflowio.addStmt(&s7);

// statement 9
// cz = gz;

    Stmt s8("cz = gz",
          "{[k,i,nsg] : 0 <= k < nz && 0<=i<ny && 0 <= nsg < m_numSubgrids }",
          "{[nsg,k,i]->[0, nsg, 0, k, 0,i,0 ]}",
          {
            {"gz", "{[0] -> [0]}"}
          },
          {
            {"cz", "{[0] -> [0]}"}
          });

    parflowio.addStmt(&s8);
  
// statement 9 
//if(gy>=clip_y && gy<clip_y+extent_y)
// int read_count = fread(buf,8,nx,m_fp);

    Stmt s9("read_count = fread(buf,8,nx,m_fp)",
          "{[k,i,nsg] : 0 <= k < nz && 0<=i<ny && 0 <= nsg < m_numSubgrids && gy >=clip_y && gy< clip_y+ extent_y }",
          "{[nsg,k,i]->[0, nsg, 0, k, 0,i,0 ]}",
          {
            {"clip_y", "{[0] -> [0]}"},
            {"extent_y", "{[0] -> [0]}"},
            {"gy", "{[0] -> [0]}"},        
          },
          {
            {"read_count", "{[0] -> [0]}"}
          });

  parflowio.addStmt(&s9);

  //Calling 
  cout << "Codegen:\n";
  cout << parflowio.codeGen();
  
  return 0;
}
