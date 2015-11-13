
#include <iostream>
#include <iegenlib.h>
#include <cstring>
//#include "/home/king/tools/IEGenLib/src/set_relation/set_relation.h"

using namespace std;

string ISLsubtract (string relstr, string knownstr)
{
  // Get an isl context
  isl_ctx *ctx;
  ctx = isl_ctx_alloc();
  
  // Get an isl printer and associate to an isl context
  isl_printer * ip = NULL;
  ip = isl_printer_to_str(ctx);
  
  // load Relation r into ISL map
  isl_map* imapR = NULL;
  isl_map* imapK = NULL;
  imapR = isl_map_read_from_str(ctx, relstr.c_str());
  imapK = isl_map_read_from_str(ctx, knownstr.c_str());

  imapR = isl_map_union( imapR, imapK);

  // get string back from ISL map
  char * cstr;
  isl_printer_set_output_format(ip , ISL_FORMAT_ISL);
  isl_printer_print_map(ip ,imapR);
  cstr=isl_printer_get_str(ip);
  string stringFromISL = cstr;
/*  
  // clean-up
  isl_printer_flush(ip);
  isl_printer_free(ip);
  free(cstr);
  isl_map_free(imapR);
  imapR= NULL;
  isl_map_free(imapK);
  imapK= NULL;
  isl_ctx_free(ctx); 
*/
  return stringFromISL;

}

string getRelationStringFromISL(string relstr)
{
  // Get an isl context
  isl_ctx *ctx;
  ctx = isl_ctx_alloc();
  
  // Get an isl printer and associate to an isl context
  isl_printer * ip = NULL;
  ip = isl_printer_to_str(ctx);
  
  // load Relation r into ISL map
  isl_map* imap = NULL;
  imap = isl_map_read_from_str(ctx, relstr.c_str());

  // get string back from ISL map
  char * cstr;
  isl_printer_set_output_format(ip , ISL_FORMAT_ISL);
  isl_printer_print_map(ip ,imap);
  cstr=isl_printer_get_str(ip);
  string stringFromISL = cstr;
  
  // clean-up
  isl_printer_flush(ip);
  isl_printer_free(ip);
  free(cstr);
  isl_map_free(imap);
  imap= NULL;
  isl_ctx_free(ctx); 

  return stringFromISL;
}
string getSetStringFromISL(string relstr)
{
  // Get an isl context
  isl_ctx *ctx;
  ctx = isl_ctx_alloc();
  
  // Get an isl printer and associate to an isl context
  isl_printer * ip = NULL;
  ip = isl_printer_to_str(ctx);
  
  // load Relation r into ISL map
  isl_map* imap = NULL;
  imap = isl_map_read_from_str(ctx, relstr.c_str());

  // get string back from ISL map
  char * cstr;
  isl_printer_set_output_format(ip , ISL_FORMAT_ISL);
  isl_printer_print_map(ip ,imap);
  cstr=isl_printer_get_str(ip);
  string stringFromISL = cstr;
  
  // clean-up
  isl_printer_flush(ip);
  isl_printer_free(ip);
  free(cstr);
  isl_map_free(imap);
  imap= NULL;
  isl_ctx_free(ctx); 

  return stringFromISL;
}

string project_out (string relstr, unsigned pos)
{
  // Get an isl context
  isl_ctx *ctx;
  ctx = isl_ctx_alloc();
  
  // Get an isl printer and associate to an isl context
  isl_printer * ip = NULL;
  ip = isl_printer_to_str(ctx);
  
  // load Relation r into ISL map
  isl_map* imap = NULL;
  imap = isl_map_read_from_str(ctx, relstr.c_str());

  imap = isl_map_project_out(imap, isl_dim_in, pos, 1);

  // get string back from ISL map
  char * cstr;
  isl_printer_set_output_format(ip , ISL_FORMAT_ISL);
  isl_printer_print_map(ip ,imap);
  cstr=isl_printer_get_str(ip);
  string stringFromISL = cstr;
  
  // clean-up
  isl_printer_flush(ip);
  isl_printer_free(ip);
  free(cstr);
  isl_map_free(imap);
  imap= NULL;
  isl_ctx_free(ctx); 

  return stringFromISL;
}

std::string set_project_out (std::string relstr, unsigned pos)
{
  // Get an isl context
  isl_ctx *ctx;
  ctx = isl_ctx_alloc();
  
  // Get an isl printer and associate to an isl context
  isl_printer * ip = NULL;
  ip = isl_printer_to_str(ctx);
  
  // load Relation r into ISL map
  isl_set* iset = NULL;
  iset = isl_set_read_from_str(ctx, relstr.c_str());

  iset = isl_set_project_out(iset, isl_dim_out, pos, 1);

  // get string back from ISL map
  char * cstr;
  isl_printer_set_output_format(ip , ISL_FORMAT_ISL);
  isl_printer_print_set(ip ,iset);
  cstr=isl_printer_get_str(ip);
  std::string stringFromISL = cstr;
  
  // clean-up
  isl_printer_flush(ip);
  isl_printer_free(ip);
  free(cstr);
  isl_set_free(iset);
  iset= NULL;
  isl_ctx_free(ctx); 

  return stringFromISL;
}


// Takes an UFS and returns a variable name representing it:
// col(j) -> col_j;     idx(ip+1) -> idx_ipP1
string getUF_varname (string ufs)
{
  char * cstr = new char [ufs.length()+1];
  std::strcpy (cstr, ufs.c_str());

  for(int i = 0 ; i < ufs.length() ; i++ )
  {
     if (cstr[i] == '(')  cstr[i] = '_';
     else if (cstr[i] == '+')  cstr[i] = 'P';
     else if (cstr[i] == '-')  cstr[i] = 'M';
  }
  cstr[ufs.length()-1] = cstr[ufs.length()];

  string res(cstr);
  return res;
}

int main(int argc, char **argv)
{

  string GS_F("[n] -> { [i] -> [ip] : i < ip and"
               " exists j,jp: 0 <= i,ip <= n and 0 <= j,jp <= n }");
  string isl_str_F = getRelationStringFromISL(GS_F);  

  cout<<endl;
  cout<<isl_str_F;
  cout<<endl;
/*
  string GS_F("[n,idx_I,idx_I1,idx_IP,idx_IP1,col_JP] -> "
               "{ [i,j] -> [ip,jp] : i = col_JP and i < ip "
               "and 0 <= i < n and idx_I <= j < idx_I1 "
               "and 0 <= ip <= n and idx_IP <= jp < idx_IP1 }");


//  string simp = project_out (GS_F, 1);

  cout<<endl;
//  cout<<GS_F;
  cout<<endl;

  string isl_str_F = getRelationStringFromISL(GS_F);  

  cout<<endl;
//  cout<<isl_str_F;
  cout<<endl;

  std::map<string,string> ufsTOvar;
  std::map<string,string> varTOufs;

  string ufs("idx(ip+1)");
  string var = getUF_varname(ufs);

//  varTOufs.insert(std::pair<string,string>(var,ufs));
  varTOufs[var] = ufs;
//  cout<<endl<<varTOufs[var]<<endl;


  string GSF_F("[n] -> { [i,j] -> [ip,jp] : i = col(jp) and i < ip "
               "and 0 <= i and i < n and idx(i) <= j and j < idx(i+1) "
               "and 0 <= ip and ip <= n and idx(ip+1) <= jp and jp < idx(ip+1) }");

  cout<<endl;
//  cout<<GSF_F;
  cout<<endl;

  iegenlib::Relation* rel = new iegenlib::Relation(GSF_F);

  std::cout << rel->prettyPrintString() << std::endl<< std::endl;

//  iegenlib::Set* oset = new iegenlib::Set("col(jp)");
//  std::cout << oset->prettyPrintString() << std::endl;

*/
/*
    iegenlib::setCurrEnv();
    iegenlib::appendCurrEnv("col",
        new iegenlib::Set("{[x]:0<=x &&x<n}"), 
        new iegenlib::Set("{[y]:0<=y &&y<n}"), true);
    iegenlib::appendCurrEnv("idx",
        new iegenlib::Set("{[x]:0<=x &&x<n}"), 
        new iegenlib::Set("{[y]:0<=y &&y<n}"), true);

  string GS_F("[n] -> { [i,j] -> [ip,jp] : i = col(jp) and i < ip "
               "and 0 <= i and i < n and idx(i) <= j and j < idx(i+1) "
               "and 0 <= ip and ip <= n and idx(ip+1) <= jp and jp < idx(ip+1) }");

  string ilu_F("[n] -> { [i,j1,j2,k] -> [ip,jp1,jp2,kp] : i < ip and j1 = jp2 "
               "and 0 <= i and i < n and 0 <= ip and ip <= n and k+1 <= j1 and"
               " j1 < row(i+1) and kp+1 <= jp1 and jp1 < row(ip+1) and "
               "diag(col(K)) <= j2 and j2 < row(col(k+1)) and diag(col(kp)) <="
               " jp2 and jp2 < row(col(kp+1)) and row(i) <= k and k < diag(i) "
               "and row(ip) <= kp and kp < diag(ip) }");

  iegenlib::Relation* rel = new iegenlib::Relation(GS_F);

    int count = 0;
    for (std::list<iegenlib::Conjunction*>::const_iterator i=rel->mConjunctions.begin();
        i != rel->mConjunctions.end(); i++) {
        count++;
    }


  cout << endl << endl << rel->prettyPrintString() << endl<< endl;

  cout<< "  count = "<<count<<endl<<endl;

  iegenlib::Relation* result = rel->project_out(1,2);

//  cout << endl << endl << result->prettyPrintString() << endl<< endl;


    iegenlib::Set *r1 = new iegenlib::Set("[ n ] -> { [i, j, ip, jp, tv4, tv5, tv6, tv7, tv8] : i - tv4 = 0 && i >= 0 && ip >= 0 && jp >= 0 && tv4 >= 0 && tv5 >= 0 && tv6 >= 0 && tv7 >= 0 && tv8 >= 0 && i + 1 >= 0 && j - tv5 >= 0 && -ip + n >= 0 && ip + 1 >= 0 && jp - tv6 >= 0 && -i + ip - 1 >= 0 && -i + n - 2 >= 0 && -i + n - 1 >= 0 && -j + tv7 - 1 >= 0 && -ip + n - 2 >= 0 && -jp + tv8 - 1 >= 0 && -jp + n - 1 >= 0 && -tv4 + n - 1 >= 0 && -tv5 + n - 1 >= 0 && -tv6 + n - 1 >= 0 && -tv7 + n - 1 >= 0 && -tv8 + n - 1 >= 0 }");

    std::string fromStep1 = r1->toISLString();
std::cout << "Conjunction::normalize: fromStep1 = " << fromStep1 << std::endl;
 
    // (b) send through ISL

//    string fromISL = getRelationStringFromISL(retval1->toISLString());
    string fromISL = set_project_out(fromStep1, 2);

   std::cout<<std::endl<<std::endl<<fromISL<<std::endl<<std::endl;
*/
  return 0;
}







/*
    iegenlib::setCurrEnv();
    iegenlib::appendCurrEnv("theta",
        new iegenlib::Set("{[i]:0<=i && i<N}"), 
        new iegenlib::Set("{[j]:0<=j && j<P}"), true);

    iegenlib::Relation *r1 = new iegenlib::Relation("{[tstep,i] -> [s0,t,i1,x] : "
                   "tstep = s0 && i = i1 && t = theta(i) && x = 0 }");

    iegenlib::Relation *r2 = new iegenlib::Relation("{[tstep,i] -> [tstep,t,i1,x1] : "
                   "i = i1 && t = theta(i1) && x1 = 0 }");

    r1->normalize();
//std::cout << "r1 = " << r1->toString() << std::endl;

    r2->normalize();
//std::cout << "r2 = " << r2->toString() << std::endl;
	
	delete r1;
	delete r2;
*/













/*
  string GS_F("[n,idx_I,idx_I1,idx_IP,idx_IP1,col_JP] -> \
         { [i] -> [ip] : exists j,jp : i = col_JP and i < ip and \
           0 <= i < n and idx_I <= j < idx_I1 and 0 <= ip <= n and idx_IP <= jp < idx_IP1 }");

  string GS_A("[n,idx_I,idx_I1,idx_IP,idx_IP1,col_JP] -> { [ip] -> [i] : exists j,jp : i = col_JP and ip < i and 0 <= i < n and idx_I <= j < idx_I1 and 0 <= ip <= n and idx_IP <= jp < idx_IP1 }");

  string ilu_F(
"[n,row_I,row_I1,row_IP,row_IP1,diag_I,diag_IP,diag_col_K,diag_col_K1,diag_col_KP,diag_col_KP1,row_col_K1,row_col_KP1] -> { [i] -> [ip] : exists j1,j2,jp1,jp2,k,kp : i < ip and j1 = jp2 and 0 <= i < n and 0 <= ip <= n and k+1 <= j1 < row_I1 and kp+1 <= jp1 < row_IP1 and diag_col_K <= j2 < row_col_K1 and diag_col_KP <= jp2 < row_col_KP1 and row_I <= k < diag_I and row_IP <= kp < diag_IP }");

  string ilu_A(
"[n,row_I,row_I1,row_IP,row_IP1,diag_I,diag_IP,diag_col_K,diag_col_K1,diag_col_KP,diag_col_KP1,row_col_K1,row_col_KP1] -> { [ip] -> [i] : exists j1,j2,jp1,jp2,k,kp : ip < i and j1 = jp2 and 0 <= i < n and 0 <= ip <= n and k+1 <= j1 < row_I1 and kp+1 <= jp1 < row_IP1 and diag_col_K <= j2 < row_col_K1 and diag_col_KP <= jp2 < row_col_KP1 and row_I <= k < diag_I and row_IP <= kp < diag_IP }");

  string known ("[n,row_I,row_I1,row_IP,row_IP1,diag_I,diag_IP,diag_col_K,diag_col_K1,diag_col_KP,diag_col_KP1,row_col_K1,row_col_KP1] -> { [i] -> [ip] : row_col_K1 < diag_col_K + 1 }");


  string res1 = getRelationStringFromISL(ilu_F);
  string res2 = ISLsubtract(res1 , known);
  cout<<endl;
  cout<<ilu_F;
  cout<<endl;
  
  cout<<endl;
  cout<<res2;
  cout<<endl;

  cout<<endl;
  cout<<res1;
  cout<<endl;
*/













/*
	(1) 0<=i<n 
	(2) 0<=i’<n
	(3) rowptr(i)<=k<diagptr(i)
	(4) rowptr(i’)<=k’<diagptr(i’)
	(5) k+1<=j1<rowptr(i+1)
	(6) k’+1<=j1’<rowptr(i’+1)
	(7) diagptr(col(k))+1<=j2<rowptr(col(k)+1)
	(8) diagptr(col(k'))+1<=j2'<rowptr(col(k')+1)
	(9) j1=j2'
*/
