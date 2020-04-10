/*!
 * \file subSetDriver.cc
 *

>> Build IEGenLib (run in the root directory):

./configure
make

>> The driver, subSetDriver, should be at build/bin/

>> Build the driver separately (in root directory run):

g++ -O3 -o subSetDriver src/drivers/subSetDriver.cc -I src build/src/libiegenlib.a -lisl -std=c++11

>> Run the driver (in root directory):

./subSetDriver

*/


#include <iostream>
#include "iegenlib.h"

using namespace iegenlib;
//using iegenlib::Set;
//using iegenlib::Relation;
//using iegenlib::UFCallTerm;
using namespace std;

// Utility function
bool printRelation(string msg, Relation *rel);
void EXPECT_EQ(string a, string b);
void EXPECT_EQ(Relation *a, Relation *b);
int str2int(string str);


//----------------------- MAIN ---------------
int main(int argc, char **argv)
{
  
  iegenlib::setCurrEnv();
  iegenlib::appendCurrEnv("idx1",
      new Set("{[i]:0<=i &&i<n}"), 
      new Set("{[j]:0<=j &&j<n}"), true, iegenlib::Monotonic_NONE);
  iegenlib::appendCurrEnv("idx2",
      new Set("{[i]:0<=i &&i<n}"), 
      new Set("{[j]:0<=j &&j<n}"), true, iegenlib::Monotonic_NONE);
  iegenlib::appendCurrEnv("idx3",
      new Set("{[i]:0<=i &&i<n}"), 
      new Set("{[j]:0<=j &&j<n}"), true, iegenlib::Monotonic_NONE);
  iegenlib::appendCurrEnv("idx4",
      new Set("{[i]:0<=i &&i<n}"), 
      new Set("{[j]:0<=j &&j<n}"), true, iegenlib::Monotonic_NONE);

  // dataDependenceRelationship is a member function of the iegenlib::Relation class:
  // SetRelationshipType Relation::dataDependenceRelationship(Relation* rightSide, int parallelLoopLevel);
  // This function determines the relaitonship between the *this and the argument, rightSide.
  // The relation can be one of the enum types in SetRelationshipType enum:
  // typedef enum {SetEqual, SubSetEqual, SubSet, SuperSet, SuperSetEqual, UnKnown, UnSatSet} SetRelationshipType;
  //
  // Note that at the moment IEGenLib does not support existential variables directly.
  // However, when doing dependence analysis, only variable representing loop level
  // targetted for parallelization is a non-existential varaible.
  // Nonetheless, variables representing loop levels not targetted for parallelizations are also
  // stored as tuple varaibles, which in common polyhedral definition is reserved for non-existentials.
  // Consequently, the second argument to the dataDependenceRelationship function, namely parallelLoopLevel, 
  // declares which of the tuple variables are representitive of the loop level targted for parallelization.
  // This will determine which of the tuple variables are existentials, and can be projected out
  // during the subset detection algorithm proceture.  
  // 

  SetRelationshipType result;

  // Cases of relation equality:

  // (1) Identical relations
  Relation *r1 = new Relation("{ [i] -> [ip] :  0 <= i < n &&  0 <= ip < n }");
  Relation *r2 = new Relation("{ [i] -> [ip] :  0 <= i < n &&  0 <= ip < n }");

  result = r1->dataDependenceRelationship(r2,0);

  if (result == SetEqual){
    std::cout<<"\nR1 and R2 are equal\nR1 = "<<r1->getString()<<"\nR2 = "<<r2->getString()<<"\n\n";
  } else {
    std::cout<<"\nThe subset detection was not able to determine that R1 and R2 are equal\nR1 = "
             <<r1->getString()<<"\nR2 = "<<r2->getString();
  }


  // (2)  Identical relations that their parallel loop level is not 
  //      the tuple variable at the start of the input and output tuple varaible list. 
  r1 = new Relation("{ [0,i] -> [0,ip] :  0 <= i < n &&  0 <= ip < n }");
  r2 = new Relation("{ [0,i] -> [0,ip] :  0 <= i < n &&  0 <= ip < n }");

  // NOTE that: we have used 1 for the second argument to dataDependenceRelationship
  //            since in "[0,i] -> [0,ip] " tuple declaration i and ip are 
  //            representitive of parallel loop level.
  result = r1->dataDependenceRelationship(r2,1);

  if (result == SetEqual){
    std::cout<<"\nR1 and R2 are equal\nR1 = "<<r1->getString()<<"\nR2 = "<<r2->getString()<<"\n\n";
  } else {
    std::cout<<"\nThe subset detection was not able to determine that R1 and R2 are equal\nR1 = "
             <<r1->getString()<<"\nR2 = "<<r2->getString();
  }


  // (3) Name of the tuple variable is not important in determining the space of a polyhedral, 
  //     only their ordering is important. 
  r1 = new Relation("{ [0,i] -> [0,ip] :  0 <= i < n &&  0 <= ip < n }");
  r2 = new Relation("{ [1,ip] -> [1,i] :  0 <= i < n &&  0 <= ip < n }");
  //   Here the general form of r1 and r2 would be identical:
  //   r1 = { [0,t2] -> [0,tv4] :  0 <= tv2 < n &&  0 <= tv4 < n }
  //   r2 = { [1,t2] -> [1,tv4] :  0 <= tv2 < n &&  0 <= tv4 < n }

  // NOTE that: we have used 1 for the second argument to dataDependenceRelationship
  //            since in "[0,i] -> [0,ip] " tuple declaration i and ip are 
  //            representitive of parallel loop level.
  result = r1->dataDependenceRelationship(r2,1);

  if (result == SetEqual){
    std::cout<<"\nR1 and R2 are equal\nR1 = "<<r1->getString()<<"\nR2 = "<<r2->getString()<<"\n\n";
  } else {
    std::cout<<"\nThe subset detection was not able to determine that R1 and R2 are equal\nR1 = "
             <<r1->getString()<<"\nR2 = "<<r2->getString();
  }

  
  // Case of subset relations


  // (4) When a relation have all the constraints of another relations, plus some extra relations,
  //     depending on the extra constraints, the relations with more constraints either is the 
  //     strict subset of the second relations, or its subset equal.  
  r1 = new Relation("{ [i] -> [ip, jp] :  0 <= i < n &&  0 <= ip < n  &&  0 <= jp < m }");
  r2 = new Relation("{ [i] -> [ip] :  0 <= i < n &&  0 <= ip < n }");

  //
  result = r1->dataDependenceRelationship(r2,0);

  if (result == SubSet){
    std::cout<<"\nR1 is subset of R2.\n"<<r1->getString()<<"\nR2 = "<<r2->getString()<<"\n\n";
  } else {
    std::cout<<"\nThe subset detection was not able to determine that R1 is subset of R2.\nR1 = "
             <<r1->getString()<<"\nR2 = "<<r2->getString();
  }

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

