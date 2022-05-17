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
            {"x", "{[0] -> [0]}"}
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

  parflowio.addDataSpace("gx","int");

  Stmt s3("gx = x",
          "{[k,i,nsg] : 0 <= k < nz && 0<=i<ny && 0 <= nsg < m_numSubgrids && x_overlap > 0 && y_overlap >0 }",
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
  
  parflowio.addDataSpace("gy","int");

  Stmt s4("gy = y + i ",
          "{[k,i,nsg] : 0 <= k < nz && 0<=i<ny && 0 <= nsg < m_numSubgrids && x_overlap > 0 && y_overlap >0}",
          "{[nsg,k,i]->[0, nsg, 0, k, 0,i,0 ]}",
          {
            {"y", "{[0] -> [0]}"},
          },
          {
            {"gy", "{[0] -> [0]}"}
          });

    parflowio.addStmt(&s4);

//int gz = z + k;
  parflowio.addDataSpace("gz","int");

  Stmt sm("gz = z + k",
          "{[k,i,nsg] : 0 <= k < nz && 0<=i<ny && 0 <= nsg < m_numSubgrids && x_overlap > 0 && y_overlap >0}",
          "{[nsg,k,i]->[0, nsg, 0, k, 0,i,0 ]}",
          {
            {"z", "{[0] -> [0]}"},
            {"k", "{[0] -> [0]}"}
          },
          {
            {"gz", "{[0] -> [0]}"}
          });

    parflowio.addStmt(&sm);

// statement 5
// int cx = gx - clip_x;

  parflowio.addDataSpace("cx","int");

  Stmt s5("cx = gx - clip_x",
          "{[k,i,nsg] : 0 <= k < nz && 0<=i<ny && 0 <= nsg < m_numSubgrids && x_overlap > 0 && y_overlap >0 }",
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

  parflowio.addDataSpace("cy","int");

  Stmt s6("cy = gy - clip_y;",
        "{[k,i,nsg] : 0 <= k < nz && 0<=i<ny && 0 <= nsg < m_numSubgrids  && x_overlap > 0 && y_overlap >0}",
        "{[nsg,k,i]->[0, nsg, 0, k, 0,i,0 ]}",
        {
          {"gy", "{[0] -> [0]}"},
          {"clip_y", "{[0] -> [0]}"}

        },
        {
          {"cy", "{[0] -> [0]}"}
        });

    parflowio.addStmt(&s6);



// statement 8
// cz = gz;
  parflowio.addDataSpace("cz","int");

    Stmt s8("cz = gz",
          "{[k,i,nsg] : 0 <= k < nz && 0<=i<ny && 0 <= nsg < m_numSubgrids && x_overlap > 0 && y_overlap >0 }",
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

  parflowio.addDataSpace("read_count","int");

    Stmt s9("read_count = fread(buf,8,nx,m_fp)",
          "{[k,i,nsg] : 0 <= k < nz && 0<=i<ny && 0 <= nsg < m_numSubgrids && gy >=clip_y && gy< clip_y+ extent_y && x_overlap > 0 && y_overlap >0}",
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

// statement 
/*
  for(j=0;j<nx;j++){
    // if this specific y,x coordinate is within the clip
    // convert it and save it in the right position
    if((gx+j) >= clip_x && (gx+j) < clip_x+extent_x){
      // these should be valid clip coordinates
      int cxi = cx+j;
      int index = cz*(extent_y*extent_x) + cy*extent_x + cxi;
      uint64_t tmp = buf[j];
      tmp = bswap64(tmp);
      m_data[index] = *(double*)(&tmp);
    } 
  }

*/
  parflowio.addDataSpace("cxi","int");

  Stmt s10("cxi = cx+j",
        "{[k,i,nsg,j] : 0 <= k < nz && 0<=i<ny && 0 <= nsg < m_numSubgrids && 0<=j<nx && gy>=clip_y && gy< clip_y+ extent_y && (gx+j) >= clip_x && (gx+j) < clip_x+extent_x && x_overlap > 0 && y_overlap >0}",
        "{[nsg,k,i,j]->[0, nsg, 0, k, 0,i,0 ,j,0]}",
        {
          {"gx", "{[0] -> [0]}"},
          {"j", "{[0] -> [0]}"},
          {"clip_x", "{[0] -> [0]}"},
          {"extent_x", "{[0] -> [0]}"},
          {"extent_y", "{[0] -> [0]}"},
          {"cx", "{[0] -> [0]}"},
          {"clip_y", "{[0] -> [0]}"}
        },
        {
          {"cxi", "{[0] -> [0]}"}
  });
    parflowio.addStmt(&s10);


  Stmt s11("index = cz*(extent_y*extent_x) + cy*extent_x + cxi",
        "{[k,i,j,nsg] : 0 <= k < nz && 0<=i<ny && 0 <= nsg < m_numSubgrids && 0<=j<nx && gy>=clip_y && gy< clip_y+ extent_y  && (gx+j) >= clip_x && (gx+j) < clip_x+extent_x && x_overlap > 0 && y_overlap >0}",
        "{[nsg,k,i,j]->[0, nsg, 0, k, 0,i,0 ,j,0]}",
        {
          {"gx", "{[0] -> [0]}"},
          {"j", "{[0] -> [0]}"},
          {"clip_x", "{[0] -> [0]}"},
          {"extent_x", "{[0] -> [0]}"},
          {"cx", "{[0] -> [0]}"},
          {"cxi", "{[0] -> [0]}"},
          {"cz", "{[0] -> [0]}"},
          {"extent_y", "{[0] -> [0]}"},
          {"clip_y", "{[0] -> [0]}"}
        },
        {
          {"index", "{[0] -> [0]}"}
  });
    parflowio.addStmt(&s11);

  parflowio.addDataSpace("tmp","int");

  Stmt s12("tmp = buf[j]",
        "{[k,i,j,nsg] : 0 <= k < nz && 0<=i<ny && 0 <= nsg < m_numSubgrids && 0<=j<nx && gy>=clip_y && gy< clip_y+ extent_y  && (gx+j) >= clip_x && (gx+j) < clip_x+extent_x && x_overlap > 0 && y_overlap >0}",
        "{[nsg,k,i,j]->[0, nsg, 0, k, 0,i,0 ,j,0]}",
        {
          {"gx", "{[0] -> [0]}"},
          {"j", "{[0] -> [0]}"},
          {"clip_x", "{[0] -> [0]}"},
          {"extent_x", "{[0] -> [0]}"},
          {"buf", "{[j] -> [j]}"},
          {"extent_y", "{[0] -> [0]}"},
          {"clip_y", "{[0] -> [0]}"}
        },
        {
          {"tmp", "{[0] -> [0]}"}
  });
      
      parflowio.addStmt(&s12);


    Stmt s13("tmp = bswap64(tmp)",
        "{[k,i,j,nsg] : 0 <= k < nz && 0<=i<ny && 0 <= nsg < m_numSubgrids && 0<=j<nx && gy>=clip_y && gy< clip_y+ extent_y  && (gx+j) >= clip_x && (gx+j) < clip_x+extent_x && x_overlap > 0 && y_overlap >0}",
        "{[nsg,k,i,j]->[0, nsg, 0, k, 0,i,0 ,j,0]}",
        {
          {"gx", "{[0] -> [0]}"},
          {"j", "{[0] -> [0]}"},
          {"clip_x", "{[0] -> [0]}"},
          {"extent_x", "{[0] -> [0]}"},
          {"extent_y", "{[0] -> [0]}"},
          {"clip_y", "{[0] -> [0]}"}

        },
        {
          {"tmp", "{[0] -> [0]}"}
  });
    parflowio.addStmt(&s13);

// // m_data[index] = *(double*)(&tmp);

  Stmt s14("m_data[index] = *(double*)(&tmp)",
      "{[k,i,j,nsg] : 0 <= k < nz && 0<=i<ny && 0 <= nsg < m_numSubgrids && 0<=j<nx && gy>=clip_y && gy< clip_y+ extent_y  && (gx+j) >= clip_x && (gx+j) < clip_x+extent_x && x_overlap > 0 && y_overlap >0}",
      "{[nsg,k,i,j]->[0, nsg, 0, k, 0,i,0 ,j,0]}",
      {
        {"gx", "{[0] -> [0]}"},
        {"j", "{[0] -> [0]}"},
        {"clip_x", "{[0] -> [0]}"},
        {"extent_x", "{[0] -> [0]}"},
        {"tmp", "{[0]->[0]}"},
        {"clip_y", "{[0] -> [0]}"},
        {"extent_y", "{[0] -> [0]}"}
      },
      {
        {"m_data", "{[index] -> [index]}"}
    });
    
    parflowio.addStmt(&s14);


/// statement x
/*
else{
      // this y point is not of interest - so
      // seek a single pencil
      std::fseek(m_fp, 8*nx, SEEK_CUR);
    }
  */
    Stmt sx("std::fseek(m_fp, 8*nx, SEEK_CUR)",
        " {[k,i,nsg] : 0 <= k < nz && 0<=i<ny && 0 <= nsg < m_numSubgrids && x_overlap > 0} union" 
	" { [k,i,nsg] : 0 <= k < nz && 0<=i<ny && 0 <= nsg < m_numSubgrids && gy <clip_y && gy >= clip_y+ extent_y  && y_overlap >0}",
        "{[nsg,k,i]->[0, nsg, 0, k, 0,i,0 ]}",
        { },
        { });

  parflowio.addStmt(&sx);

  // statement last
  /*else{
          // this entire subgrid does not fit into our clip, so
          // scan to the next subgrid
           std::fseek(m_fp, 8*nx*ny*nz, SEEK_CUR);
        }
  */
    Stmt sy("std::fseek(m_fp, 8*nx*ny*nz, SEEK_CUR)",
      "{[k,i,nsg] : 0 <= k < nz && 0<=i<ny && 0 <= nsg < m_numSubgrids && gy >=clip_y && gy< clip_y+ extent_y }",
      "{[nsg,k,i]->[0, nsg, 0, k, 0,i,0 ]}",
      {
        {"m_fp", "{[0] -> [0]}"},
        {"nx", "{[0] -> [0]}"},
        {"ny", "{[0] -> [0]}"},
        {"nz", "{[0] -> [0]}"},
      },
      { });
    
    parflowio.addStmt(&sy);

  //Calling 
  cout << "Codegen:\n";
  cout << parflowio.codeGen();
  cout <<  parflowio.codeGenMemoryManagementString();
  
  return 0;
}
