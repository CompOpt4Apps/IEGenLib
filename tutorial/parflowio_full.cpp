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
  parflowio.addDataSpace("errcheck","int");
  parflowio.addDataSpace("x_overlap","int");
  parflowio.addDataSpace("clip_x","int");
  parflowio.addDataSpace("extent_x","int");

// Statement 1 
// long long qq = z*m_nx*m_ny + y*m_nx + x;
// long long k,i,j;

    Stmt s1("qq = z*m_nx*m_ny + y*m_nx + x",
          "{[nsg] : 0 <= nsg < m_numSubgrids}",
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

  parflowio.addDataSpace("y_overlap","int");
  parflowio.addDataSpace("clip_y","int");
  parflowio.addDataSpace("extent_y","int");


  // statement 2
  /*
    for (k=0; k<nz; k++){
      for(i=0;i<ny;i++){
        // read full "pencil"
        long long index = qq+k*m_nx*m_ny+i*m_nx;
        uint64_t* buf = (uint64_t*)&(m_data[index]);
        int read_count = fread(buf,8,nx,m_fp);
        if(read_count != nx){
            perror("Error Reading Data, File Ended Unexpectedly");
            return 1;
        }
  */

    Stmt s2("index = qq+k*m_nx*m_ny+i*m_nx;",
            "{[nsg,k,i] : 0 <= k < nz && 0<=i<ny && 0 <= nsg < m_numSubgrids}",
            "{[nsg,k,i]->[0, nsg, 0, k, 0,i,0 ]}",
            {
                {"x", "{[0] -> [0]}"},
                {"y", "{[0] -> [0]}"},
            },
            {
                {"gx", "{[0] -> [0]}"}
            });

    parflowio.addStmt(&s2);

  // statement 3
  /* uint64_t* buf = (uint64_t*)&(m_data[index]); */

    Stmt s3("uint64_t* buf = (uint64_t*)&(m_data[index]);",
          "{[nsg,k,i] : 0 <= k < nz && 0<=i<ny && 0 <= nsg < m_numSubgrids}",
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
// int read_count = fread(buf,8,nx,m_fp);

    Stmt s4("int read_count = fread(buf,8,nx,m_fp);",
          "{[nsg,k,i] : 0 <= k < nz && 0<=i<ny && 0 <= nsg < m_numSubgrids}",
          "{[nsg,k,i]->[0, nsg, 0, k, 0,i,0 ]}",
          {
            {"x", "{[0] -> [0]}"},
            {"y", "{[0] -> [0]}"},
          },
          {
            {"gx", "{[0] -> [0]}"}
          });

    parflowio.addStmt(&s4);

// statement 
/*
    // handle byte order
    // uint64_t* buf = (uint64_t*)&(m_data[index]);
    for(j=0;j<nx;j++){
        uint64_t tmp = buf[j];
        tmp = bswap64(tmp);
        m_data[index+j] = *(double*)(&tmp);
*/
  parflowio.addDataSpace("cxi","int");

  Stmt s10("cxi = cx+j",
        "{[nsg,k,i,j] : 0 <= k < nz && 0<=i<ny && 0 <= nsg < m_numSubgrids && 0<=j<nx && gy>=clip_y && gy< clip_y+ extent_y && (gx+j) >= clip_x && (gx+j) < clip_x+extent_x && x_overlap > 0 && y_overlap >0}",
        "{[nsg,k,i,j]->[0, nsg, 0, k, 0,i,1 ,j,0]}",
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
        "{[nsg,k,i,j] : 0 <= k < nz && 0<=i<ny && 0 <= nsg < m_numSubgrids && 0<=j<nx && gy>=clip_y && gy< clip_y+ extent_y  && (gx+j) >= clip_x && (gx+j) < clip_x+extent_x && x_overlap > 0 && y_overlap >0}",
        "{[nsg,k,i,j]->[0, nsg, 0, k, 0,i,1 ,j,0]}",
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
        "{[nsg,k,i,j] : 0 <= k < nz && 0<=i<ny && 0 <= nsg < m_numSubgrids && 0<=j<nx && gy>=clip_y && gy< clip_y+ extent_y  && (gx+j) >= clip_x && (gx+j) < clip_x+extent_x && x_overlap > 0 && y_overlap >0}",
        "{[nsg,k,i,j]->[0, nsg, 0, k, 0,i,1 ,j,0]}",
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
        "{[nsg, k,i,j] : 0 <= k < nz && 0<=i<ny && 0 <= nsg < m_numSubgrids && 0<=j<nx && gy>=clip_y && gy< clip_y+ extent_y  && (gx+j) >= clip_x && (gx+j) < clip_x+extent_x && x_overlap > 0 && y_overlap >0}",
        "{[nsg,k,i,j]->[0, nsg, 0, k, 0,i,1 ,j,0]}",
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

  Stmt s14("m_data[index] = *(double*)(&tmp); m_data[index] = 2 * m_data[index] ",
      "{[nsg,k,i,j] : 0 <= k < nz && 0<=i<ny && 0 <= nsg < m_numSubgrids && 0<=j<nx && gy>=clip_y && gy< clip_y+ extent_y  && (gx+j) >= clip_x && (gx+j) < clip_x+extent_x && x_overlap > 0 && y_overlap >0}",
      "{[nsg,k,i,j]->[0, nsg, 0, k, 0,i,1,j,0]}",
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
        " {[nsg,k,i] : 0 <= k < nz && 0<=i<ny && 0 <= nsg < m_numSubgrids && x_overlap > 0 && y_overlap > 0 && gy<  clip_y  } union" 
	      "{[nsg,k,i] : 0 <= k < nz && 0<=i<ny && 0 <= nsg < m_numSubgrids && x_overlap > 0 && y_overlap > 0 && gy >= clip_y+ extent_y}",
        "{[nsg,k,i]->[0, nsg, 0, k, 0,i, 2]}",
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
      "{[nsg] : 0 <= nsg < m_numSubgrids && x_overlap <= 0 } union" 
      "{[nsg]:  0 <= nsg < m_numSubgrids && y_overlap <=0}",
      "{[nsg]->[0, nsg, 1]}", 
      {
        {"m_fp", "{[0] -> [0]}"},
        {"nx", "{[0] -> [0]}"},
        {"ny", "{[0] -> [0]}"},
        {"nz", "{[0] -> [0]}"},
        {"x_overlap","{[0] -> [0]}" },
        {"y_overlap","{[0] -> [0]}" },

      },
      { });
    
    parflowio.addStmt(&sy);

  //Calling 
  cout << "Codegen:\n";
  cout << parflowio.codeGen();
  cout <<  parflowio.codeGenMemoryManagementString();
  
  return 0;
}
