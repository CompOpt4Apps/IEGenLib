/*!
 * \file superAffSet.cc
 *

>> Build IEGenLib (run in the root directory):

./configure
make

>> The driver, superAffSet, should be at build/bin/

>> Build the driver separately (in root directory run):

g++ -O3 -o superAffSet src/drivers/setRel.cc -I src build/src/libiegenlib.a -lisl -std=c++11

>> Run the driver (in root directory):

./superAffSet SOME_EXAMPLE.json

*/


#include <iostream>
#include "iegenlib.h"
//#include "src/set_relation/Visitor.h"
#include "parser/jsoncons/json.hpp"

using jsoncons::json;
using namespace iegenlib;
//using iegenlib::Set;
//using iegenlib::Relation;
//using iegenlib::UFCallTerm;
using namespace std;


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//! If you wish not to see the output, change the value: verbose = false;
bool verbose=true;

void superAff(string inputFile);

// Utility function
bool printRelation(string msg, Relation *rel);
void EXPECT_EQ(string a, string b);
void EXPECT_EQ(Relation *a, Relation *b);
int str2int(string str);


//----------------------- MAIN ---------------
int main(int argc, char **argv)
{
  
  iegenlib::setCurrEnv();
  iegenlib::appendCurrEnv("col",
      new Set("{[i]:0<=i &&i<n}"), 
      new Set("{[j]:0<=j &&j<n}"), true, iegenlib::Monotonic_NONE);
  iegenlib::appendCurrEnv("idx",
      new Set("{[i]:0<=i &&i<n}"), 
      new Set("{[j]:0<=j &&j<n}"), true, iegenlib::Monotonic_NONE);
  iegenlib::appendCurrEnv("row",
      new Set("{[i]:0<=i &&i<n}"), 
      new Set("{[j]:0<=j &&j<n}"), true, iegenlib::Monotonic_NONE);
  iegenlib::appendCurrEnv("diag",
      new Set("{[i]:0<=i &&i<n}"), 
      new Set("{[j]:0<=j &&j<n}"), true, iegenlib::Monotonic_NONE);

/*
  Relation *r1 = new Relation("{ [i,k] -> [ip,kp] :  i = kp and col(i) < n"
                                   " and i < ip and diag(col(i))+1 <= k }");
  Relation *r2 = new Relation("{ [i,k] -> [ip,kp] :  i = col(kp) and 0 <= i < n and 0 <= ip < n"
                                   " and i < ip and row(i) <= k < row(i+1)  and row(ip) <= kp < row(ip+1)  }");
*/

  Relation *r1 = new Relation("{[i, m, k, l] -> [ip, mp] : k = mp && i < ip && 0  <=  i < n && 0 <= ip < n && col(i) + 1  <=  m  < col(i + 1) && l < col(i + 1) && m  <=  l && row(l+1)  <= row(k) && col(row(m))  <=  k < col(row(m) + 1) && row(l) = row(k) && col(ip) + 1  <=  mp < col(ip+ 1) }");
  Relation *r2 = new Relation("{[i, m, k, l] -> [ip, mp, kp, lp] : k = lp && i < ip && 0  <=  i < n && 0 <= ip < n && col(i) + 1  <=  m  < col(i + 1) && l < col(i + 1) && m  <=  l && row(l + 1)  <=  row(k) && col(row(m))  <=  k < col(row(m) + 1) && row(l) = row(k) && col(ip) + 1  <=  mp < col(ip+ 1) &&  lp < col(ip+ 1) && mp <=  lp && row(lp+ 1)  <=  row(kp) && col(row(mp)) <=  kp < col(row(mp) + 1) && row(lp) = row(kp)}");


//  json npJL = "{}";
  std::set<int> eigenLTvs;
  std::set<int> eigenRTvs;
  
  eigenLTvs.insert( 0 );
  eigenLTvs.insert( r1->inArity() );
  
  eigenRTvs.insert( 0 );
  eigenRTvs.insert( r2->inArity() );

  cout<<"\n\nR1 = "<<r1->prettyPrintString()<<"\n\n";
  r1->setRelation(r2, eigenLTvs, eigenRTvs);


  return 0;
}



void EXPECT_EQ(Relation *result, Relation *expected){

    if( result == NULL && expected == NULL ){
        cout<<"\n\nResult MATCH Expected: Not Satisfiable.\n\n";
    }

    else if( result != NULL && expected == NULL ){
        cout<<"\n\nResult DOES NOT MATCH Expected:\n"
              "Expected: Not Satisfiable\nResult:\n"<<result->toISLString()<<"\n\n";
    }

    else if( result == NULL && expected != NULL ){
        cout<<"\n\nResult DOES NOT MATCH Expected:\n"
              "Expected:\n"<<expected->toISLString()<<"\nResult: Not Satisfiable\n\n";
    }


    else if( result != NULL && expected != NULL ){
      // Replace uf calls with the variables to create an affine superset.
      Relation * supAffRes = result->superAffineRelation();
      Relation * supAffExp = expected->superAffineRelation();

      isl_ctx* ctx = isl_ctx_alloc();
      isl_map* resISL =  isl_map_read_from_str(ctx, (supAffRes->toISLString().c_str()));
      isl_map* expISL =  isl_map_read_from_str(ctx, (supAffExp->toISLString().c_str()));

      //if( result->toISLString() == expected->toISLString() ){
      if( isl_map_plain_is_equal( resISL , expISL ) ){
        cout<<"\n\nResult MATCH Expected: Which is:\n"<<result->toISLString()<<"\n\n";
      }
      else {
        cout<<"\n\nResult DOES NOT MATCH Expected:\n"
            "Expected:\n"<<expected->toISLString()<<
             "\nResult:\n"<<result->toISLString()<<"\n\n";
      }
    }

}



void EXPECT_EQ(string a, string b){

    if( a != b ){

        cout<<"\n\nExpected: "<<a;
        cout<<"\n\nActual:"<< b <<"\n\n";
    }
}

bool printRelation(string msg, Relation *rel){

    if ( rel ) {

        cout<<"\n\n"<<msg<<rel->toISLString()<<"\n\n";
    } else {

        cout<<"\n"<<msg<<"Not Satisfiable"<<"\n";
    }

    return true;
}

int str2int(string str){
  int i;
  sscanf (str.c_str(),"%d",&i);
  return i;
}

