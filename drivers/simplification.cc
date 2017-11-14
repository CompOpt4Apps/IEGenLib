/*!
     g++ -o simplifyDriver src/drivers/simplifyDriver.cc -I src build/src/libiegenlib.a -lisl -std=c++11

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

void simplify(string inputFile);

// Utility function
bool printRelation(string msg, Relation *rel);
void EXPECT_EQ(string a, string b);
void EXPECT_EQ(Relation *a, Relation *b);
int str2int(string str);


//----------------------- MAIN ---------------
int main(int argc, char **argv)
{

//CSR_Gauss_Seidel_Example();

  if (argc == 1)
  {
    cout<<"\n\nYou need to specify the input JSON files (one or more) that contain dependence relations:"
          "\n./simplifyDriver file1.json file2.json\n\n";
  } else if (argc >= 2){
    // Parsing command line arguments and reading given files.
    for(int arg = 1; arg < argc ; arg++){
      simplify(string(argv[arg]));
    }
  }

    return 0;
}

// Reads information from a JSON file (inputFile), and applies
// the simplification algorithm to the sets found in the file. 
void simplify(string inputFile)
{

  iegenlib::setCurrEnv();
  std::set<int> parallelTvs;
  // (0)
  // Read the data from inputFile
  ifstream in(inputFile);
  json data;
  in >> data;

 for(size_t p = 0; p < data.size(); ++p){    // Dependence relations (DR) found in the file

  cout<<"\n\n"<<data[p][0]["Name"].as<string>()<<"\n\n";

  for (size_t i = 0; i < data[p].size(); ++i){// Conjunctions found for one DR in the file

    // (1) Putting constraints in an iegenlib::Relation
    // Reading original set.
    Relation* rel = new Relation(data[p][i]["Relation"].as<string>());

    // (2) Introduce the uninterpreted function symbols to environment, and 
    // indicate their domain, range, whether they are bijective, or monotonic.
    if( i == 0 ){  // Read these data only once. 
                   // They are stored in the first conjunction.

      json ufcs = data[p][i];
      // Read UFCs' data for code No. p, from ith relation
      addUFCs(ufcs);

      // Add defined domain information to environment
      json uqCons = data[p][i]["User Defined"];
      addUniQuantRules(uqCons);
    }

    rel->detectUnsatOrFindEqualities();


  }

 } // End of p loop

}

const char* param = "[ m, diagptr_ip_, diagptr_i_, diagptr_colidx_k__, diagptr_colidx_kp__, colidx_j1_, colidx_j2_, colidx_j1p_, colidx_j2p_, colidx_k_, colidx_k_P1, colidx_k_P2, colidx_kp_, colidx_kp_P1, colidx_kp_P2, rowptr_colidx_k_P1_, rowptr_colidx_k_P2_, rowptr_colidx_k__, rowptr_colidx_kp_P1_, rowptr_colidx_kp_P2_, rowptr_colidx_kp__, rowptr_iM1_, rowptr_iP1_, rowptr_iP2_, rowptr_i_, rowptr_ipM1_, rowptr_ipP1_, rowptr_ipP2_, rowptr_ip_, diagptr_colidx_k_M1_, diagptr_colidx_k_P1_,  diagptr_colidx_kp_M1_, diagptr_colidx_kp_P1_, diagptr_iM1_, diagptr_iP1_,  diagptr_ipM1_, diagptr_ipP1_]  -> ";




const char* relstr = "[i,k,j1,j2] -> [ip,kp,j1p,j2p]: ";


isl_map* read_map(isl_ctx* ctx, const char* constraints) {

	char* str = (char*) malloc(strlen(param) + strlen(constraints) + 1);
	strcpy(str, param);
	strcat(str, constraints);

	return isl_map_read_from_str(ctx, str);
}

isl_map* read_instantiation(isl_ctx* ctx, const char* constraints) {

	char* str = (char*) malloc(strlen(param) + strlen(relstr) + strlen(constraints) + 3);
	strcpy(str, param);
	strcat(str, "{");
	strcat(str, relstr);
	strcat(str, constraints);
	strcat(str, "}");

	return isl_map_read_from_str(ctx, str);
}


isl_map* add_instantiation(isl_ctx* ctx, isl_map* map, const char* antecedent, const char* consequent) {
	static long count = 0;
	
	int added = 0;
	{
		isl_map* ant_map = read_instantiation(ctx, antecedent);
		ant_map = isl_map_gist(ant_map, isl_map_copy(map));
		if (isl_map_plain_is_universe(ant_map)) {
			isl_map* con_map = read_instantiation(ctx, consequent);
			map = isl_map_intersect(map, con_map);
			map = isl_map_coalesce(map);
			printf("C");
			added = 1;
		}
		isl_map_free(ant_map);
	}
	if (!added) {
		isl_map* con_map = read_instantiation(ctx, consequent);
		con_map = isl_map_complement(con_map);
		con_map = isl_map_gist(con_map, isl_map_copy(map));
		if (isl_map_plain_is_universe(con_map)) {
			isl_map* ant_map = read_instantiation(ctx, antecedent);
			ant_map = isl_map_complement(ant_map);
			map = isl_map_intersect(map, ant_map);
			map = isl_map_coalesce(map);
			printf("A");
			added = 1;
		}
		isl_map_free(con_map);
	}
	if (!added) {
		printf("_");
	}
	
	count++;
	if (count % 50 == 0) printf("\n");

	return map;
}






bool printRelation(string msg, Relation *rel){

    if ( rel ) {

        cout<<"\n\n"<<msg<<rel->toISLString()<<"\n\n";
    } else {

        cout<<"\n"<<msg<<"Not Satisfiable"<<"\n";
    }

    return true;
}

void EXPECT_EQ(string a, string b){

    if( a != b ){

        cout<<"\n\nExpected: "<<a;
        cout<<"\n\nActual:"<< b <<"\n\n";
    }
}

void EXPECT_EQ(Relation *a, Relation *b){

    if( a != b ){
        cout<<"\n\nIncorrect results: Expected or Actual is NULL.\n\n";
    }
}

int str2int(string str){
  int i;
  sscanf (str.c_str(),"%d",&i);
  return i;
}

