#include <isl/map.h>
#include <string>
#include <cstring>

using namespace std;

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

int main(int argc, char** argv) {

	isl_ctx* ctx = isl_ctx_alloc();

        isl_map* map = read_map(ctx, 
"{[i,k,j1,j2] -> [ip,kp,j1p,j2p]:  ( i +1 <= ip && 0 <= i && i +1 <= m && 0 <= ip && ip +1 <= m && rowptr_i_ <= k && k +1 <= diagptr_i_ && rowptr_ip_ <= kp && kp +1 <= diagptr_ip_ && k +1 <= j1 && j1 +1 <= rowptr_iP1_ && kp +1 <= j1p && j1p +1 <= rowptr_ipP1_ && diagptr_colidx_k__ +1 <= j2 && j2 +1 <= rowptr_colidx_k_P1_ && diagptr_colidx_kp__ +1 <= j2p && j2p +1 <= rowptr_colidx_kp_P1_ && colidx_j1_ = colidx_j2_ && colidx_j1p_ = colidx_j2p_ && j1 = kp) &&  ( rowptr_i_ <= diagptr_i_ ) &&  ( rowptr_iP1_ <= diagptr_iP1_ ) &&  ( rowptr_ip_ <= diagptr_ip_ ) &&  ( rowptr_ipP1_ <= diagptr_ipP1_ ) &&  ( rowptr_colidx_k__ <= diagptr_colidx_k__ ) &&  ( rowptr_colidx_kp__ <= diagptr_colidx_kp__ ) &&  ( rowptr_colidx_k_P1_ <= diagptr_colidx_k_P1_ ) &&  ( rowptr_colidx_kp_P1_ <= diagptr_colidx_kp_P1_ )}");

	isl_map* orig = isl_map_copy(map);
	
//	map = add_instantiation(ctx, map, " i+ 1 <= i ", " rowptr_i_+ 1 <= rowptr_i_ ");
	
	
//instantiation: i+1 <= colidx(kp) -> rowptr( i+1 ) <= rowptr( colidx(kp) )
//map = add_instantiation(ctx, map, " i+1 <= colidx_kp_ ", " rowptr_iP1_ <= rowptr_colidx_kp__ ");
  
//instantiation: colidx(kp)+1 <= i -> rowptr( colidx(kp)+1 ) <= rowptr( i )
//map = add_instantiation(ctx, map, " colidx_kp_+1 <= i ", " rowptr_colidx_kp_P1_ <= rowptr_i_ ");
	

for (int i =0; i < 1; i++) {

map = add_instantiation(ctx, map, " i <= i ", " rowptr_i_ <= rowptr_i_ ");
map = add_instantiation(ctx, map, " i <= i+1 ", " rowptr_i_ <= rowptr_iP1_ ");
map = add_instantiation(ctx, map, " i <= ip ", " rowptr_i_ <= rowptr_ip_ ");
map = add_instantiation(ctx, map, " i <= ip+1 ", " rowptr_i_ <= rowptr_ipP1_ ");
map = add_instantiation(ctx, map, " i <= i-1 ", " rowptr_i_ <= rowptr_iM1_ ");
map = add_instantiation(ctx, map, " i <= ip-1 ", " rowptr_i_ <= rowptr_ipM1_ ");
map = add_instantiation(ctx, map, " i <= colidx_k_ ", " rowptr_i_ <= rowptr_colidx_k__ ");
map = add_instantiation(ctx, map, " i <= colidx_kp_ ", " rowptr_i_ <= rowptr_colidx_kp__ ");
map = add_instantiation(ctx, map, " i <= colidx_k_+1 ", " rowptr_i_ <= rowptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " i <= colidx_kp_+1 ", " rowptr_i_ <= rowptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " i <= colidx_k_+2 ", " rowptr_i_ <= rowptr_colidx_k_P2_ ");
map = add_instantiation(ctx, map, " i <= colidx_kp_+2 ", " rowptr_i_ <= rowptr_colidx_kp_P2_ ");
map = add_instantiation(ctx, map, " i+1 <= i ", " rowptr_iP1_ <= rowptr_i_ ");
map = add_instantiation(ctx, map, " i+1 <= i+1 ", " rowptr_iP1_ <= rowptr_iP1_ ");
map = add_instantiation(ctx, map, " i+1 <= ip ", " rowptr_iP1_ <= rowptr_ip_ ");
map = add_instantiation(ctx, map, " i+1 <= ip+1 ", " rowptr_iP1_ <= rowptr_ipP1_ ");
map = add_instantiation(ctx, map, " i+1 <= i-1 ", " rowptr_iP1_ <= rowptr_iM1_ ");
map = add_instantiation(ctx, map, " i+1 <= ip-1 ", " rowptr_iP1_ <= rowptr_ipM1_ ");
map = add_instantiation(ctx, map, " i+1 <= colidx_k_ ", " rowptr_iP1_ <= rowptr_colidx_k__ ");
map = add_instantiation(ctx, map, " i+1 <= colidx_kp_ ", " rowptr_iP1_ <= rowptr_colidx_kp__ ");
map = add_instantiation(ctx, map, " i+1 <= colidx_k_+1 ", " rowptr_iP1_ <= rowptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " i+1 <= colidx_kp_+1 ", " rowptr_iP1_ <= rowptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " i+1 <= colidx_k_+2 ", " rowptr_iP1_ <= rowptr_colidx_k_P2_ ");
map = add_instantiation(ctx, map, " i+1 <= colidx_kp_+2 ", " rowptr_iP1_ <= rowptr_colidx_kp_P2_ ");
map = add_instantiation(ctx, map, " ip <= i ", " rowptr_ip_ <= rowptr_i_ ");
map = add_instantiation(ctx, map, " ip <= i+1 ", " rowptr_ip_ <= rowptr_iP1_ ");
map = add_instantiation(ctx, map, " ip <= ip ", " rowptr_ip_ <= rowptr_ip_ ");
map = add_instantiation(ctx, map, " ip <= ip+1 ", " rowptr_ip_ <= rowptr_ipP1_ ");
map = add_instantiation(ctx, map, " ip <= i-1 ", " rowptr_ip_ <= rowptr_iM1_ ");
map = add_instantiation(ctx, map, " ip <= ip-1 ", " rowptr_ip_ <= rowptr_ipM1_ ");
map = add_instantiation(ctx, map, " ip <= colidx_k_ ", " rowptr_ip_ <= rowptr_colidx_k__ ");
map = add_instantiation(ctx, map, " ip <= colidx_kp_ ", " rowptr_ip_ <= rowptr_colidx_kp__ ");
map = add_instantiation(ctx, map, " ip <= colidx_k_+1 ", " rowptr_ip_ <= rowptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " ip <= colidx_kp_+1 ", " rowptr_ip_ <= rowptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " ip <= colidx_k_+2 ", " rowptr_ip_ <= rowptr_colidx_k_P2_ ");
map = add_instantiation(ctx, map, " ip <= colidx_kp_+2 ", " rowptr_ip_ <= rowptr_colidx_kp_P2_ ");
map = add_instantiation(ctx, map, " ip+1 <= i ", " rowptr_ipP1_ <= rowptr_i_ ");
map = add_instantiation(ctx, map, " ip+1 <= i+1 ", " rowptr_ipP1_ <= rowptr_iP1_ ");
map = add_instantiation(ctx, map, " ip+1 <= ip ", " rowptr_ipP1_ <= rowptr_ip_ ");
map = add_instantiation(ctx, map, " ip+1 <= ip+1 ", " rowptr_ipP1_ <= rowptr_ipP1_ ");
map = add_instantiation(ctx, map, " ip+1 <= i-1 ", " rowptr_ipP1_ <= rowptr_iM1_ ");
map = add_instantiation(ctx, map, " ip+1 <= ip-1 ", " rowptr_ipP1_ <= rowptr_ipM1_ ");
map = add_instantiation(ctx, map, " ip+1 <= colidx_k_ ", " rowptr_ipP1_ <= rowptr_colidx_k__ ");
map = add_instantiation(ctx, map, " ip+1 <= colidx_kp_ ", " rowptr_ipP1_ <= rowptr_colidx_kp__ ");
map = add_instantiation(ctx, map, " ip+1 <= colidx_k_+1 ", " rowptr_ipP1_ <= rowptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " ip+1 <= colidx_kp_+1 ", " rowptr_ipP1_ <= rowptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " ip+1 <= colidx_k_+2 ", " rowptr_ipP1_ <= rowptr_colidx_k_P2_ ");
map = add_instantiation(ctx, map, " ip+1 <= colidx_kp_+2 ", " rowptr_ipP1_ <= rowptr_colidx_kp_P2_ ");
map = add_instantiation(ctx, map, " i-1 <= i ", " rowptr_iM1_ <= rowptr_i_ ");
map = add_instantiation(ctx, map, " i-1 <= i+1 ", " rowptr_iM1_ <= rowptr_iP1_ ");
map = add_instantiation(ctx, map, " i-1 <= ip ", " rowptr_iM1_ <= rowptr_ip_ ");
map = add_instantiation(ctx, map, " i-1 <= ip+1 ", " rowptr_iM1_ <= rowptr_ipP1_ ");
map = add_instantiation(ctx, map, " i-1 <= i-1 ", " rowptr_iM1_ <= rowptr_iM1_ ");
map = add_instantiation(ctx, map, " i-1 <= ip-1 ", " rowptr_iM1_ <= rowptr_ipM1_ ");
map = add_instantiation(ctx, map, " i-1 <= colidx_k_ ", " rowptr_iM1_ <= rowptr_colidx_k__ ");
map = add_instantiation(ctx, map, " i-1 <= colidx_kp_ ", " rowptr_iM1_ <= rowptr_colidx_kp__ ");
map = add_instantiation(ctx, map, " i-1 <= colidx_k_+1 ", " rowptr_iM1_ <= rowptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " i-1 <= colidx_kp_+1 ", " rowptr_iM1_ <= rowptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " i-1 <= colidx_k_+2 ", " rowptr_iM1_ <= rowptr_colidx_k_P2_ ");
map = add_instantiation(ctx, map, " i-1 <= colidx_kp_+2 ", " rowptr_iM1_ <= rowptr_colidx_kp_P2_ ");
map = add_instantiation(ctx, map, " ip-1 <= i ", " rowptr_ipM1_ <= rowptr_i_ ");
map = add_instantiation(ctx, map, " ip-1 <= i+1 ", " rowptr_ipM1_ <= rowptr_iP1_ ");
map = add_instantiation(ctx, map, " ip-1 <= ip ", " rowptr_ipM1_ <= rowptr_ip_ ");
map = add_instantiation(ctx, map, " ip-1 <= ip+1 ", " rowptr_ipM1_ <= rowptr_ipP1_ ");
map = add_instantiation(ctx, map, " ip-1 <= i-1 ", " rowptr_ipM1_ <= rowptr_iM1_ ");
map = add_instantiation(ctx, map, " ip-1 <= ip-1 ", " rowptr_ipM1_ <= rowptr_ipM1_ ");
map = add_instantiation(ctx, map, " ip-1 <= colidx_k_ ", " rowptr_ipM1_ <= rowptr_colidx_k__ ");
map = add_instantiation(ctx, map, " ip-1 <= colidx_kp_ ", " rowptr_ipM1_ <= rowptr_colidx_kp__ ");
map = add_instantiation(ctx, map, " ip-1 <= colidx_k_+1 ", " rowptr_ipM1_ <= rowptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " ip-1 <= colidx_kp_+1 ", " rowptr_ipM1_ <= rowptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " ip-1 <= colidx_k_+2 ", " rowptr_ipM1_ <= rowptr_colidx_k_P2_ ");
map = add_instantiation(ctx, map, " ip-1 <= colidx_kp_+2 ", " rowptr_ipM1_ <= rowptr_colidx_kp_P2_ ");
map = add_instantiation(ctx, map, " colidx_k_ <= i ", " rowptr_colidx_k__ <= rowptr_i_ ");
map = add_instantiation(ctx, map, " colidx_k_ <= i+1 ", " rowptr_colidx_k__ <= rowptr_iP1_ ");
map = add_instantiation(ctx, map, " colidx_k_ <= ip ", " rowptr_colidx_k__ <= rowptr_ip_ ");
map = add_instantiation(ctx, map, " colidx_k_ <= ip+1 ", " rowptr_colidx_k__ <= rowptr_ipP1_ ");
map = add_instantiation(ctx, map, " colidx_k_ <= i-1 ", " rowptr_colidx_k__ <= rowptr_iM1_ ");
map = add_instantiation(ctx, map, " colidx_k_ <= ip-1 ", " rowptr_colidx_k__ <= rowptr_ipM1_ ");
map = add_instantiation(ctx, map, " colidx_k_ <= colidx_k_ ", " rowptr_colidx_k__ <= rowptr_colidx_k__ ");
map = add_instantiation(ctx, map, " colidx_k_ <= colidx_kp_ ", " rowptr_colidx_k__ <= rowptr_colidx_kp__ ");
map = add_instantiation(ctx, map, " colidx_k_ <= colidx_k_+1 ", " rowptr_colidx_k__ <= rowptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " colidx_k_ <= colidx_kp_+1 ", " rowptr_colidx_k__ <= rowptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " colidx_k_ <= colidx_k_+2 ", " rowptr_colidx_k__ <= rowptr_colidx_k_P2_ ");
map = add_instantiation(ctx, map, " colidx_k_ <= colidx_kp_+2 ", " rowptr_colidx_k__ <= rowptr_colidx_kp_P2_ ");
map = add_instantiation(ctx, map, " colidx_kp_ <= i ", " rowptr_colidx_kp__ <= rowptr_i_ ");
map = add_instantiation(ctx, map, " colidx_kp_ <= i+1 ", " rowptr_colidx_kp__ <= rowptr_iP1_ ");
map = add_instantiation(ctx, map, " colidx_kp_ <= ip ", " rowptr_colidx_kp__ <= rowptr_ip_ ");
map = add_instantiation(ctx, map, " colidx_kp_ <= ip+1 ", " rowptr_colidx_kp__ <= rowptr_ipP1_ ");
map = add_instantiation(ctx, map, " colidx_kp_ <= i-1 ", " rowptr_colidx_kp__ <= rowptr_iM1_ ");
map = add_instantiation(ctx, map, " colidx_kp_ <= ip-1 ", " rowptr_colidx_kp__ <= rowptr_ipM1_ ");
map = add_instantiation(ctx, map, " colidx_kp_ <= colidx_k_ ", " rowptr_colidx_kp__ <= rowptr_colidx_k__ ");
map = add_instantiation(ctx, map, " colidx_kp_ <= colidx_kp_ ", " rowptr_colidx_kp__ <= rowptr_colidx_kp__ ");
map = add_instantiation(ctx, map, " colidx_kp_ <= colidx_k_+1 ", " rowptr_colidx_kp__ <= rowptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " colidx_kp_ <= colidx_kp_+1 ", " rowptr_colidx_kp__ <= rowptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " colidx_kp_ <= colidx_k_+2 ", " rowptr_colidx_kp__ <= rowptr_colidx_k_P2_ ");
map = add_instantiation(ctx, map, " colidx_kp_ <= colidx_kp_+2 ", " rowptr_colidx_kp__ <= rowptr_colidx_kp_P2_ ");
map = add_instantiation(ctx, map, " colidx_k_+1 <= i ", " rowptr_colidx_k_P1_ <= rowptr_i_ ");
map = add_instantiation(ctx, map, " colidx_k_+1 <= i+1 ", " rowptr_colidx_k_P1_ <= rowptr_iP1_ ");
map = add_instantiation(ctx, map, " colidx_k_+1 <= ip ", " rowptr_colidx_k_P1_ <= rowptr_ip_ ");
map = add_instantiation(ctx, map, " colidx_k_+1 <= ip+1 ", " rowptr_colidx_k_P1_ <= rowptr_ipP1_ ");
map = add_instantiation(ctx, map, " colidx_k_+1 <= i-1 ", " rowptr_colidx_k_P1_ <= rowptr_iM1_ ");
map = add_instantiation(ctx, map, " colidx_k_+1 <= ip-1 ", " rowptr_colidx_k_P1_ <= rowptr_ipM1_ ");
map = add_instantiation(ctx, map, " colidx_k_+1 <= colidx_k_ ", " rowptr_colidx_k_P1_ <= rowptr_colidx_k__ ");
map = add_instantiation(ctx, map, " colidx_k_+1 <= colidx_kp_ ", " rowptr_colidx_k_P1_ <= rowptr_colidx_kp__ ");
map = add_instantiation(ctx, map, " colidx_k_+1 <= colidx_k_+1 ", " rowptr_colidx_k_P1_ <= rowptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " colidx_k_+1 <= colidx_kp_+1 ", " rowptr_colidx_k_P1_ <= rowptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " colidx_k_+1 <= colidx_k_+2 ", " rowptr_colidx_k_P1_ <= rowptr_colidx_k_P2_ ");
map = add_instantiation(ctx, map, " colidx_k_+1 <= colidx_kp_+2 ", " rowptr_colidx_k_P1_ <= rowptr_colidx_kp_P2_ ");
map = add_instantiation(ctx, map, " colidx_kp_+1 <= i ", " rowptr_colidx_kp_P1_ <= rowptr_i_ ");
map = add_instantiation(ctx, map, " colidx_kp_+1 <= i+1 ", " rowptr_colidx_kp_P1_ <= rowptr_iP1_ ");
map = add_instantiation(ctx, map, " colidx_kp_+1 <= ip ", " rowptr_colidx_kp_P1_ <= rowptr_ip_ ");
map = add_instantiation(ctx, map, " colidx_kp_+1 <= ip+1 ", " rowptr_colidx_kp_P1_ <= rowptr_ipP1_ ");
map = add_instantiation(ctx, map, " colidx_kp_+1 <= i-1 ", " rowptr_colidx_kp_P1_ <= rowptr_iM1_ ");
map = add_instantiation(ctx, map, " colidx_kp_+1 <= ip-1 ", " rowptr_colidx_kp_P1_ <= rowptr_ipM1_ ");
map = add_instantiation(ctx, map, " colidx_kp_+1 <= colidx_k_ ", " rowptr_colidx_kp_P1_ <= rowptr_colidx_k__ ");
map = add_instantiation(ctx, map, " colidx_kp_+1 <= colidx_kp_ ", " rowptr_colidx_kp_P1_ <= rowptr_colidx_kp__ ");
map = add_instantiation(ctx, map, " colidx_kp_+1 <= colidx_k_+1 ", " rowptr_colidx_kp_P1_ <= rowptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " colidx_kp_+1 <= colidx_kp_+1 ", " rowptr_colidx_kp_P1_ <= rowptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " colidx_kp_+1 <= colidx_k_+2 ", " rowptr_colidx_kp_P1_ <= rowptr_colidx_k_P2_ ");
map = add_instantiation(ctx, map, " colidx_kp_+1 <= colidx_kp_+2 ", " rowptr_colidx_kp_P1_ <= rowptr_colidx_kp_P2_ ");
map = add_instantiation(ctx, map, " colidx_k_+2 <= i ", " rowptr_colidx_k_P2_ <= rowptr_i_ ");
map = add_instantiation(ctx, map, " colidx_k_+2 <= i+1 ", " rowptr_colidx_k_P2_ <= rowptr_iP1_ ");
map = add_instantiation(ctx, map, " colidx_k_+2 <= ip ", " rowptr_colidx_k_P2_ <= rowptr_ip_ ");
map = add_instantiation(ctx, map, " colidx_k_+2 <= ip+1 ", " rowptr_colidx_k_P2_ <= rowptr_ipP1_ ");
map = add_instantiation(ctx, map, " colidx_k_+2 <= i-1 ", " rowptr_colidx_k_P2_ <= rowptr_iM1_ ");
map = add_instantiation(ctx, map, " colidx_k_+2 <= ip-1 ", " rowptr_colidx_k_P2_ <= rowptr_ipM1_ ");
map = add_instantiation(ctx, map, " colidx_k_+2 <= colidx_k_ ", " rowptr_colidx_k_P2_ <= rowptr_colidx_k__ ");
map = add_instantiation(ctx, map, " colidx_k_+2 <= colidx_kp_ ", " rowptr_colidx_k_P2_ <= rowptr_colidx_kp__ ");
map = add_instantiation(ctx, map, " colidx_k_+2 <= colidx_k_+1 ", " rowptr_colidx_k_P2_ <= rowptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " colidx_k_+2 <= colidx_kp_+1 ", " rowptr_colidx_k_P2_ <= rowptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " colidx_k_+2 <= colidx_k_+2 ", " rowptr_colidx_k_P2_ <= rowptr_colidx_k_P2_ ");
map = add_instantiation(ctx, map, " colidx_k_+2 <= colidx_kp_+2 ", " rowptr_colidx_k_P2_ <= rowptr_colidx_kp_P2_ ");
map = add_instantiation(ctx, map, " colidx_kp_+2 <= i ", " rowptr_colidx_kp_P2_ <= rowptr_i_ ");
map = add_instantiation(ctx, map, " colidx_kp_+2 <= i+1 ", " rowptr_colidx_kp_P2_ <= rowptr_iP1_ ");
map = add_instantiation(ctx, map, " colidx_kp_+2 <= ip ", " rowptr_colidx_kp_P2_ <= rowptr_ip_ ");
map = add_instantiation(ctx, map, " colidx_kp_+2 <= ip+1 ", " rowptr_colidx_kp_P2_ <= rowptr_ipP1_ ");
map = add_instantiation(ctx, map, " colidx_kp_+2 <= i-1 ", " rowptr_colidx_kp_P2_ <= rowptr_iM1_ ");
map = add_instantiation(ctx, map, " colidx_kp_+2 <= ip-1 ", " rowptr_colidx_kp_P2_ <= rowptr_ipM1_ ");
map = add_instantiation(ctx, map, " colidx_kp_+2 <= colidx_k_ ", " rowptr_colidx_kp_P2_ <= rowptr_colidx_k__ ");
map = add_instantiation(ctx, map, " colidx_kp_+2 <= colidx_kp_ ", " rowptr_colidx_kp_P2_ <= rowptr_colidx_kp__ ");
map = add_instantiation(ctx, map, " colidx_kp_+2 <= colidx_k_+1 ", " rowptr_colidx_kp_P2_ <= rowptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " colidx_kp_+2 <= colidx_kp_+1 ", " rowptr_colidx_kp_P2_ <= rowptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " colidx_kp_+2 <= colidx_k_+2 ", " rowptr_colidx_kp_P2_ <= rowptr_colidx_k_P2_ ");
map = add_instantiation(ctx, map, " colidx_kp_+2 <= colidx_kp_+2 ", " rowptr_colidx_kp_P2_ <= rowptr_colidx_kp_P2_ ");

map = add_instantiation(ctx, map, " i+ 1 <= i ", " rowptr_i_+ 1 <= rowptr_i_ ");
map = add_instantiation(ctx, map, " i+ 1 <= i+1 ", " rowptr_i_+ 1 <= rowptr_iP1_ ");
map = add_instantiation(ctx, map, " i+ 1 <= ip ", " rowptr_i_+ 1 <= rowptr_ip_ ");
map = add_instantiation(ctx, map, " i+ 1 <= ip+1 ", " rowptr_i_+ 1 <= rowptr_ipP1_ ");
map = add_instantiation(ctx, map, " i+ 1 <= colidx_k_+1 ", " rowptr_i_+ 1 <= rowptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " i+ 1 <= colidx_kp_+1 ", " rowptr_i_+ 1 <= rowptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " i+ 1 <= i+2 ", " rowptr_i_+ 1 <= rowptr_iP2_ ");
map = add_instantiation(ctx, map, " i+ 1 <= ip+2 ", " rowptr_i_+ 1 <= rowptr_ipP2_ ");
map = add_instantiation(ctx, map, " i+ 1 <= colidx_k_+2 ", " rowptr_i_+ 1 <= rowptr_colidx_k_P2_ ");
map = add_instantiation(ctx, map, " i+ 1 <= colidx_kp_+2 ", " rowptr_i_+ 1 <= rowptr_colidx_kp_P2_ ");
map = add_instantiation(ctx, map, " i+1+ 1 <= i ", " rowptr_iP1_+ 1 <= rowptr_i_ ");
map = add_instantiation(ctx, map, " i+1+ 1 <= i+1 ", " rowptr_iP1_+ 1 <= rowptr_iP1_ ");
map = add_instantiation(ctx, map, " i+1+ 1 <= ip ", " rowptr_iP1_+ 1 <= rowptr_ip_ ");
map = add_instantiation(ctx, map, " i+1+ 1 <= ip+1 ", " rowptr_iP1_+ 1 <= rowptr_ipP1_ ");
map = add_instantiation(ctx, map, " i+1+ 1 <= colidx_k_+1 ", " rowptr_iP1_+ 1 <= rowptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " i+1+ 1 <= colidx_kp_+1 ", " rowptr_iP1_+ 1 <= rowptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " i+1+ 1 <= i+2 ", " rowptr_iP1_+ 1 <= rowptr_iP2_ ");
map = add_instantiation(ctx, map, " i+1+ 1 <= ip+2 ", " rowptr_iP1_+ 1 <= rowptr_ipP2_ ");
map = add_instantiation(ctx, map, " i+1+ 1 <= colidx_k_+2 ", " rowptr_iP1_+ 1 <= rowptr_colidx_k_P2_ ");
map = add_instantiation(ctx, map, " i+1+ 1 <= colidx_kp_+2 ", " rowptr_iP1_+ 1 <= rowptr_colidx_kp_P2_ ");
map = add_instantiation(ctx, map, " ip+ 1 <= i ", " rowptr_ip_+ 1 <= rowptr_i_ ");
map = add_instantiation(ctx, map, " ip+ 1 <= i+1 ", " rowptr_ip_+ 1 <= rowptr_iP1_ ");
map = add_instantiation(ctx, map, " ip+ 1 <= ip ", " rowptr_ip_+ 1 <= rowptr_ip_ ");
map = add_instantiation(ctx, map, " ip+ 1 <= ip+1 ", " rowptr_ip_+ 1 <= rowptr_ipP1_ ");
map = add_instantiation(ctx, map, " ip+ 1 <= colidx_k_+1 ", " rowptr_ip_+ 1 <= rowptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " ip+ 1 <= colidx_kp_+1 ", " rowptr_ip_+ 1 <= rowptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " ip+ 1 <= i+2 ", " rowptr_ip_+ 1 <= rowptr_iP2_ ");
map = add_instantiation(ctx, map, " ip+ 1 <= ip+2 ", " rowptr_ip_+ 1 <= rowptr_ipP2_ ");
map = add_instantiation(ctx, map, " ip+ 1 <= colidx_k_+2 ", " rowptr_ip_+ 1 <= rowptr_colidx_k_P2_ ");
map = add_instantiation(ctx, map, " ip+ 1 <= colidx_kp_+2 ", " rowptr_ip_+ 1 <= rowptr_colidx_kp_P2_ ");
map = add_instantiation(ctx, map, " ip+1+ 1 <= i ", " rowptr_ipP1_+ 1 <= rowptr_i_ ");
map = add_instantiation(ctx, map, " ip+1+ 1 <= i+1 ", " rowptr_ipP1_+ 1 <= rowptr_iP1_ ");
map = add_instantiation(ctx, map, " ip+1+ 1 <= ip ", " rowptr_ipP1_+ 1 <= rowptr_ip_ ");
map = add_instantiation(ctx, map, " ip+1+ 1 <= ip+1 ", " rowptr_ipP1_+ 1 <= rowptr_ipP1_ ");
map = add_instantiation(ctx, map, " ip+1+ 1 <= colidx_k_+1 ", " rowptr_ipP1_+ 1 <= rowptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " ip+1+ 1 <= colidx_kp_+1 ", " rowptr_ipP1_+ 1 <= rowptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " ip+1+ 1 <= i+2 ", " rowptr_ipP1_+ 1 <= rowptr_iP2_ ");
map = add_instantiation(ctx, map, " ip+1+ 1 <= ip+2 ", " rowptr_ipP1_+ 1 <= rowptr_ipP2_ ");
map = add_instantiation(ctx, map, " ip+1+ 1 <= colidx_k_+2 ", " rowptr_ipP1_+ 1 <= rowptr_colidx_k_P2_ ");
map = add_instantiation(ctx, map, " ip+1+ 1 <= colidx_kp_+2 ", " rowptr_ipP1_+ 1 <= rowptr_colidx_kp_P2_ ");
map = add_instantiation(ctx, map, " colidx_k_+1+ 1 <= i ", " rowptr_colidx_k_P1_+ 1 <= rowptr_i_ ");
map = add_instantiation(ctx, map, " colidx_k_+1+ 1 <= i+1 ", " rowptr_colidx_k_P1_+ 1 <= rowptr_iP1_ ");
map = add_instantiation(ctx, map, " colidx_k_+1+ 1 <= ip ", " rowptr_colidx_k_P1_+ 1 <= rowptr_ip_ ");
map = add_instantiation(ctx, map, " colidx_k_+1+ 1 <= ip+1 ", " rowptr_colidx_k_P1_+ 1 <= rowptr_ipP1_ ");
map = add_instantiation(ctx, map, " colidx_k_+1+ 1 <= colidx_k_+1 ", " rowptr_colidx_k_P1_+ 1 <= rowptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " colidx_k_+1+ 1 <= colidx_kp_+1 ", " rowptr_colidx_k_P1_+ 1 <= rowptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " colidx_k_+1+ 1 <= i+2 ", " rowptr_colidx_k_P1_+ 1 <= rowptr_iP2_ ");
map = add_instantiation(ctx, map, " colidx_k_+1+ 1 <= ip+2 ", " rowptr_colidx_k_P1_+ 1 <= rowptr_ipP2_ ");
map = add_instantiation(ctx, map, " colidx_k_+1+ 1 <= colidx_k_+2 ", " rowptr_colidx_k_P1_+ 1 <= rowptr_colidx_k_P2_ ");
map = add_instantiation(ctx, map, " colidx_k_+1+ 1 <= colidx_kp_+2 ", " rowptr_colidx_k_P1_+ 1 <= rowptr_colidx_kp_P2_ ");
map = add_instantiation(ctx, map, " colidx_kp_+1+ 1 <= i ", " rowptr_colidx_kp_P1_+ 1 <= rowptr_i_ ");
map = add_instantiation(ctx, map, " colidx_kp_+1+ 1 <= i+1 ", " rowptr_colidx_kp_P1_+ 1 <= rowptr_iP1_ ");
map = add_instantiation(ctx, map, " colidx_kp_+1+ 1 <= ip ", " rowptr_colidx_kp_P1_+ 1 <= rowptr_ip_ ");
map = add_instantiation(ctx, map, " colidx_kp_+1+ 1 <= ip+1 ", " rowptr_colidx_kp_P1_+ 1 <= rowptr_ipP1_ ");
map = add_instantiation(ctx, map, " colidx_kp_+1+ 1 <= colidx_k_+1 ", " rowptr_colidx_kp_P1_+ 1 <= rowptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " colidx_kp_+1+ 1 <= colidx_kp_+1 ", " rowptr_colidx_kp_P1_+ 1 <= rowptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " colidx_kp_+1+ 1 <= i+2 ", " rowptr_colidx_kp_P1_+ 1 <= rowptr_iP2_ ");
map = add_instantiation(ctx, map, " colidx_kp_+1+ 1 <= ip+2 ", " rowptr_colidx_kp_P1_+ 1 <= rowptr_ipP2_ ");
map = add_instantiation(ctx, map, " colidx_kp_+1+ 1 <= colidx_k_+2 ", " rowptr_colidx_kp_P1_+ 1 <= rowptr_colidx_k_P2_ ");
map = add_instantiation(ctx, map, " colidx_kp_+1+ 1 <= colidx_kp_+2 ", " rowptr_colidx_kp_P1_+ 1 <= rowptr_colidx_kp_P2_ ");
map = add_instantiation(ctx, map, " i-1+ 1 <= i ", " rowptr_iM1_+ 1 <= rowptr_i_ ");
map = add_instantiation(ctx, map, " i-1+ 1 <= i+1 ", " rowptr_iM1_+ 1 <= rowptr_iP1_ ");
map = add_instantiation(ctx, map, " i-1+ 1 <= ip ", " rowptr_iM1_+ 1 <= rowptr_ip_ ");
map = add_instantiation(ctx, map, " i-1+ 1 <= ip+1 ", " rowptr_iM1_+ 1 <= rowptr_ipP1_ ");
map = add_instantiation(ctx, map, " i-1+ 1 <= colidx_k_+1 ", " rowptr_iM1_+ 1 <= rowptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " i-1+ 1 <= colidx_kp_+1 ", " rowptr_iM1_+ 1 <= rowptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " i-1+ 1 <= i+2 ", " rowptr_iM1_+ 1 <= rowptr_iP2_ ");
map = add_instantiation(ctx, map, " i-1+ 1 <= ip+2 ", " rowptr_iM1_+ 1 <= rowptr_ipP2_ ");
map = add_instantiation(ctx, map, " i-1+ 1 <= colidx_k_+2 ", " rowptr_iM1_+ 1 <= rowptr_colidx_k_P2_ ");
map = add_instantiation(ctx, map, " i-1+ 1 <= colidx_kp_+2 ", " rowptr_iM1_+ 1 <= rowptr_colidx_kp_P2_ ");
map = add_instantiation(ctx, map, " ip-1+ 1 <= i ", " rowptr_ipM1_+ 1 <= rowptr_i_ ");
map = add_instantiation(ctx, map, " ip-1+ 1 <= i+1 ", " rowptr_ipM1_+ 1 <= rowptr_iP1_ ");
map = add_instantiation(ctx, map, " ip-1+ 1 <= ip ", " rowptr_ipM1_+ 1 <= rowptr_ip_ ");
map = add_instantiation(ctx, map, " ip-1+ 1 <= ip+1 ", " rowptr_ipM1_+ 1 <= rowptr_ipP1_ ");
map = add_instantiation(ctx, map, " ip-1+ 1 <= colidx_k_+1 ", " rowptr_ipM1_+ 1 <= rowptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " ip-1+ 1 <= colidx_kp_+1 ", " rowptr_ipM1_+ 1 <= rowptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " ip-1+ 1 <= i+2 ", " rowptr_ipM1_+ 1 <= rowptr_iP2_ ");
map = add_instantiation(ctx, map, " ip-1+ 1 <= ip+2 ", " rowptr_ipM1_+ 1 <= rowptr_ipP2_ ");
map = add_instantiation(ctx, map, " ip-1+ 1 <= colidx_k_+2 ", " rowptr_ipM1_+ 1 <= rowptr_colidx_k_P2_ ");
map = add_instantiation(ctx, map, " ip-1+ 1 <= colidx_kp_+2 ", " rowptr_ipM1_+ 1 <= rowptr_colidx_kp_P2_ ");
map = add_instantiation(ctx, map, " colidx_k_+ 1 <= i ", " rowptr_colidx_k__+ 1 <= rowptr_i_ ");
map = add_instantiation(ctx, map, " colidx_k_+ 1 <= i+1 ", " rowptr_colidx_k__+ 1 <= rowptr_iP1_ ");
map = add_instantiation(ctx, map, " colidx_k_+ 1 <= ip ", " rowptr_colidx_k__+ 1 <= rowptr_ip_ ");
map = add_instantiation(ctx, map, " colidx_k_+ 1 <= ip+1 ", " rowptr_colidx_k__+ 1 <= rowptr_ipP1_ ");
map = add_instantiation(ctx, map, " colidx_k_+ 1 <= colidx_k_+1 ", " rowptr_colidx_k__+ 1 <= rowptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " colidx_k_+ 1 <= colidx_kp_+1 ", " rowptr_colidx_k__+ 1 <= rowptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " colidx_k_+ 1 <= i+2 ", " rowptr_colidx_k__+ 1 <= rowptr_iP2_ ");
map = add_instantiation(ctx, map, " colidx_k_+ 1 <= ip+2 ", " rowptr_colidx_k__+ 1 <= rowptr_ipP2_ ");
map = add_instantiation(ctx, map, " colidx_k_+ 1 <= colidx_k_+2 ", " rowptr_colidx_k__+ 1 <= rowptr_colidx_k_P2_ ");
map = add_instantiation(ctx, map, " colidx_k_+ 1 <= colidx_kp_+2 ", " rowptr_colidx_k__+ 1 <= rowptr_colidx_kp_P2_ ");
map = add_instantiation(ctx, map, " colidx_kp_+ 1 <= i ", " rowptr_colidx_kp__+ 1 <= rowptr_i_ ");
map = add_instantiation(ctx, map, " colidx_kp_+ 1 <= i+1 ", " rowptr_colidx_kp__+ 1 <= rowptr_iP1_ ");
map = add_instantiation(ctx, map, " colidx_kp_+ 1 <= ip ", " rowptr_colidx_kp__+ 1 <= rowptr_ip_ ");
map = add_instantiation(ctx, map, " colidx_kp_+ 1 <= ip+1 ", " rowptr_colidx_kp__+ 1 <= rowptr_ipP1_ ");
map = add_instantiation(ctx, map, " colidx_kp_+ 1 <= colidx_k_+1 ", " rowptr_colidx_kp__+ 1 <= rowptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " colidx_kp_+ 1 <= colidx_kp_+1 ", " rowptr_colidx_kp__+ 1 <= rowptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " colidx_kp_+ 1 <= i+2 ", " rowptr_colidx_kp__+ 1 <= rowptr_iP2_ ");
map = add_instantiation(ctx, map, " colidx_kp_+ 1 <= ip+2 ", " rowptr_colidx_kp__+ 1 <= rowptr_ipP2_ ");
map = add_instantiation(ctx, map, " colidx_kp_+ 1 <= colidx_k_+2 ", " rowptr_colidx_kp__+ 1 <= rowptr_colidx_k_P2_ ");
map = add_instantiation(ctx, map, " colidx_kp_+ 1 <= colidx_kp_+2 ", " rowptr_colidx_kp__+ 1 <= rowptr_colidx_kp_P2_ ");


map = add_instantiation(ctx, map, " i+ 1 <= i ", " diagptr_i_+ 1 <= diagptr_i_ ");
map = add_instantiation(ctx, map, " i+ 1 <= ip ", " diagptr_i_+ 1 <= diagptr_ip_ ");
map = add_instantiation(ctx, map, " i+ 1 <= colidx_k_ ", " diagptr_i_+ 1 <= diagptr_colidx_k__ ");
map = add_instantiation(ctx, map, " i+ 1 <= colidx_kp_ ", " diagptr_i_+ 1 <= diagptr_colidx_kp__ ");
map = add_instantiation(ctx, map, " i+ 1 <= i+1 ", " diagptr_i_+ 1 <= diagptr_iP1_ ");
map = add_instantiation(ctx, map, " i+ 1 <= ip+1 ", " diagptr_i_+ 1 <= diagptr_ipP1_ ");
map = add_instantiation(ctx, map, " i+ 1 <= colidx_k_+1 ", " diagptr_i_+ 1 <= diagptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " i+ 1 <= colidx_kp_+1 ", " diagptr_i_+ 1 <= diagptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " ip+ 1 <= i ", " diagptr_ip_+ 1 <= diagptr_i_ ");
map = add_instantiation(ctx, map, " ip+ 1 <= ip ", " diagptr_ip_+ 1 <= diagptr_ip_ ");
map = add_instantiation(ctx, map, " ip+ 1 <= colidx_k_ ", " diagptr_ip_+ 1 <= diagptr_colidx_k__ ");
map = add_instantiation(ctx, map, " ip+ 1 <= colidx_kp_ ", " diagptr_ip_+ 1 <= diagptr_colidx_kp__ ");
map = add_instantiation(ctx, map, " ip+ 1 <= i+1 ", " diagptr_ip_+ 1 <= diagptr_iP1_ ");
map = add_instantiation(ctx, map, " ip+ 1 <= ip+1 ", " diagptr_ip_+ 1 <= diagptr_ipP1_ ");
map = add_instantiation(ctx, map, " ip+ 1 <= colidx_k_+1 ", " diagptr_ip_+ 1 <= diagptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " ip+ 1 <= colidx_kp_+1 ", " diagptr_ip_+ 1 <= diagptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " colidx_k_+ 1 <= i ", " diagptr_colidx_k__+ 1 <= diagptr_i_ ");
map = add_instantiation(ctx, map, " colidx_k_+ 1 <= ip ", " diagptr_colidx_k__+ 1 <= diagptr_ip_ ");
map = add_instantiation(ctx, map, " colidx_k_+ 1 <= colidx_k_ ", " diagptr_colidx_k__+ 1 <= diagptr_colidx_k__ ");
map = add_instantiation(ctx, map, " colidx_k_+ 1 <= colidx_kp_ ", " diagptr_colidx_k__+ 1 <= diagptr_colidx_kp__ ");
map = add_instantiation(ctx, map, " colidx_k_+ 1 <= i+1 ", " diagptr_colidx_k__+ 1 <= diagptr_iP1_ ");
map = add_instantiation(ctx, map, " colidx_k_+ 1 <= ip+1 ", " diagptr_colidx_k__+ 1 <= diagptr_ipP1_ ");
map = add_instantiation(ctx, map, " colidx_k_+ 1 <= colidx_k_+1 ", " diagptr_colidx_k__+ 1 <= diagptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " colidx_k_+ 1 <= colidx_kp_+1 ", " diagptr_colidx_k__+ 1 <= diagptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " colidx_kp_+ 1 <= i ", " diagptr_colidx_kp__+ 1 <= diagptr_i_ ");
map = add_instantiation(ctx, map, " colidx_kp_+ 1 <= ip ", " diagptr_colidx_kp__+ 1 <= diagptr_ip_ ");
map = add_instantiation(ctx, map, " colidx_kp_+ 1 <= colidx_k_ ", " diagptr_colidx_kp__+ 1 <= diagptr_colidx_k__ ");
map = add_instantiation(ctx, map, " colidx_kp_+ 1 <= colidx_kp_ ", " diagptr_colidx_kp__+ 1 <= diagptr_colidx_kp__ ");
map = add_instantiation(ctx, map, " colidx_kp_+ 1 <= i+1 ", " diagptr_colidx_kp__+ 1 <= diagptr_iP1_ ");
map = add_instantiation(ctx, map, " colidx_kp_+ 1 <= ip+1 ", " diagptr_colidx_kp__+ 1 <= diagptr_ipP1_ ");
map = add_instantiation(ctx, map, " colidx_kp_+ 1 <= colidx_k_+1 ", " diagptr_colidx_kp__+ 1 <= diagptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " colidx_kp_+ 1 <= colidx_kp_+1 ", " diagptr_colidx_kp__+ 1 <= diagptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " i-1+ 1 <= i ", " diagptr_iM1_+ 1 <= diagptr_i_ ");
map = add_instantiation(ctx, map, " i-1+ 1 <= ip ", " diagptr_iM1_+ 1 <= diagptr_ip_ ");
map = add_instantiation(ctx, map, " i-1+ 1 <= colidx_k_ ", " diagptr_iM1_+ 1 <= diagptr_colidx_k__ ");
map = add_instantiation(ctx, map, " i-1+ 1 <= colidx_kp_ ", " diagptr_iM1_+ 1 <= diagptr_colidx_kp__ ");
map = add_instantiation(ctx, map, " i-1+ 1 <= i+1 ", " diagptr_iM1_+ 1 <= diagptr_iP1_ ");
map = add_instantiation(ctx, map, " i-1+ 1 <= ip+1 ", " diagptr_iM1_+ 1 <= diagptr_ipP1_ ");
map = add_instantiation(ctx, map, " i-1+ 1 <= colidx_k_+1 ", " diagptr_iM1_+ 1 <= diagptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " i-1+ 1 <= colidx_kp_+1 ", " diagptr_iM1_+ 1 <= diagptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " ip-1+ 1 <= i ", " diagptr_ipM1_+ 1 <= diagptr_i_ ");
map = add_instantiation(ctx, map, " ip-1+ 1 <= ip ", " diagptr_ipM1_+ 1 <= diagptr_ip_ ");
map = add_instantiation(ctx, map, " ip-1+ 1 <= colidx_k_ ", " diagptr_ipM1_+ 1 <= diagptr_colidx_k__ ");
map = add_instantiation(ctx, map, " ip-1+ 1 <= colidx_kp_ ", " diagptr_ipM1_+ 1 <= diagptr_colidx_kp__ ");
map = add_instantiation(ctx, map, " ip-1+ 1 <= i+1 ", " diagptr_ipM1_+ 1 <= diagptr_iP1_ ");
map = add_instantiation(ctx, map, " ip-1+ 1 <= ip+1 ", " diagptr_ipM1_+ 1 <= diagptr_ipP1_ ");
map = add_instantiation(ctx, map, " ip-1+ 1 <= colidx_k_+1 ", " diagptr_ipM1_+ 1 <= diagptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " ip-1+ 1 <= colidx_kp_+1 ", " diagptr_ipM1_+ 1 <= diagptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " colidx_k_-1+ 1 <= i ", " diagptr_colidx_k_M1_+ 1 <= diagptr_i_ ");
map = add_instantiation(ctx, map, " colidx_k_-1+ 1 <= ip ", " diagptr_colidx_k_M1_+ 1 <= diagptr_ip_ ");
map = add_instantiation(ctx, map, " colidx_k_-1+ 1 <= colidx_k_ ", " diagptr_colidx_k_M1_+ 1 <= diagptr_colidx_k__ ");
map = add_instantiation(ctx, map, " colidx_k_-1+ 1 <= colidx_kp_ ", " diagptr_colidx_k_M1_+ 1 <= diagptr_colidx_kp__ ");
map = add_instantiation(ctx, map, " colidx_k_-1+ 1 <= i+1 ", " diagptr_colidx_k_M1_+ 1 <= diagptr_iP1_ ");
map = add_instantiation(ctx, map, " colidx_k_-1+ 1 <= ip+1 ", " diagptr_colidx_k_M1_+ 1 <= diagptr_ipP1_ ");
map = add_instantiation(ctx, map, " colidx_k_-1+ 1 <= colidx_k_+1 ", " diagptr_colidx_k_M1_+ 1 <= diagptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " colidx_k_-1+ 1 <= colidx_kp_+1 ", " diagptr_colidx_k_M1_+ 1 <= diagptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " colidx_kp_-1+ 1 <= i ", " diagptr_colidx_kp_M1_+ 1 <= diagptr_i_ ");
map = add_instantiation(ctx, map, " colidx_kp_-1+ 1 <= ip ", " diagptr_colidx_kp_M1_+ 1 <= diagptr_ip_ ");
map = add_instantiation(ctx, map, " colidx_kp_-1+ 1 <= colidx_k_ ", " diagptr_colidx_kp_M1_+ 1 <= diagptr_colidx_k__ ");
map = add_instantiation(ctx, map, " colidx_kp_-1+ 1 <= colidx_kp_ ", " diagptr_colidx_kp_M1_+ 1 <= diagptr_colidx_kp__ ");
map = add_instantiation(ctx, map, " colidx_kp_-1+ 1 <= i+1 ", " diagptr_colidx_kp_M1_+ 1 <= diagptr_iP1_ ");
map = add_instantiation(ctx, map, " colidx_kp_-1+ 1 <= ip+1 ", " diagptr_colidx_kp_M1_+ 1 <= diagptr_ipP1_ ");
map = add_instantiation(ctx, map, " colidx_kp_-1+ 1 <= colidx_k_+1 ", " diagptr_colidx_kp_M1_+ 1 <= diagptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " colidx_kp_-1+ 1 <= colidx_kp_+1 ", " diagptr_colidx_kp_M1_+ 1 <= diagptr_colidx_kp_P1_ ");

map = add_instantiation(ctx, map, " i+ 1 <= i ", " diagptr_i_+ 1 <= rowptr_i_ ");
map = add_instantiation(ctx, map, " i+ 1 <= i+1 ", " diagptr_i_+ 1 <= rowptr_iP1_ ");
map = add_instantiation(ctx, map, " i+ 1 <= ip ", " diagptr_i_+ 1 <= rowptr_ip_ ");
map = add_instantiation(ctx, map, " i+ 1 <= ip+1 ", " diagptr_i_+ 1 <= rowptr_ipP1_ ");
map = add_instantiation(ctx, map, " i+ 1 <= colidx_k_+1 ", " diagptr_i_+ 1 <= rowptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " i+ 1 <= colidx_kp_+1 ", " diagptr_i_+ 1 <= rowptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " ip+ 1 <= i ", " diagptr_ip_+ 1 <= rowptr_i_ ");
map = add_instantiation(ctx, map, " ip+ 1 <= i+1 ", " diagptr_ip_+ 1 <= rowptr_iP1_ ");
map = add_instantiation(ctx, map, " ip+ 1 <= ip ", " diagptr_ip_+ 1 <= rowptr_ip_ ");
map = add_instantiation(ctx, map, " ip+ 1 <= ip+1 ", " diagptr_ip_+ 1 <= rowptr_ipP1_ ");
map = add_instantiation(ctx, map, " ip+ 1 <= colidx_k_+1 ", " diagptr_ip_+ 1 <= rowptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " ip+ 1 <= colidx_kp_+1 ", " diagptr_ip_+ 1 <= rowptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " colidx_k_+ 1 <= i ", " diagptr_colidx_k__+ 1 <= rowptr_i_ ");
map = add_instantiation(ctx, map, " colidx_k_+ 1 <= i+1 ", " diagptr_colidx_k__+ 1 <= rowptr_iP1_ ");
map = add_instantiation(ctx, map, " colidx_k_+ 1 <= ip ", " diagptr_colidx_k__+ 1 <= rowptr_ip_ ");
map = add_instantiation(ctx, map, " colidx_k_+ 1 <= ip+1 ", " diagptr_colidx_k__+ 1 <= rowptr_ipP1_ ");
map = add_instantiation(ctx, map, " colidx_k_+ 1 <= colidx_k_+1 ", " diagptr_colidx_k__+ 1 <= rowptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " colidx_k_+ 1 <= colidx_kp_+1 ", " diagptr_colidx_k__+ 1 <= rowptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " colidx_kp_+ 1 <= i ", " diagptr_colidx_kp__+ 1 <= rowptr_i_ ");
map = add_instantiation(ctx, map, " colidx_kp_+ 1 <= i+1 ", " diagptr_colidx_kp__+ 1 <= rowptr_iP1_ ");
map = add_instantiation(ctx, map, " colidx_kp_+ 1 <= ip ", " diagptr_colidx_kp__+ 1 <= rowptr_ip_ ");
map = add_instantiation(ctx, map, " colidx_kp_+ 1 <= ip+1 ", " diagptr_colidx_kp__+ 1 <= rowptr_ipP1_ ");
map = add_instantiation(ctx, map, " colidx_kp_+ 1 <= colidx_k_+1 ", " diagptr_colidx_kp__+ 1 <= rowptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " colidx_kp_+ 1 <= colidx_kp_+1 ", " diagptr_colidx_kp__+ 1 <= rowptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " i-1+ 1 <= i ", " diagptr_iM1_+ 1 <= rowptr_i_ ");
map = add_instantiation(ctx, map, " i-1+ 1 <= i+1 ", " diagptr_iM1_+ 1 <= rowptr_iP1_ ");
map = add_instantiation(ctx, map, " i-1+ 1 <= ip ", " diagptr_iM1_+ 1 <= rowptr_ip_ ");
map = add_instantiation(ctx, map, " i-1+ 1 <= ip+1 ", " diagptr_iM1_+ 1 <= rowptr_ipP1_ ");
map = add_instantiation(ctx, map, " i-1+ 1 <= colidx_k_+1 ", " diagptr_iM1_+ 1 <= rowptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " i-1+ 1 <= colidx_kp_+1 ", " diagptr_iM1_+ 1 <= rowptr_colidx_kp_P1_ ");
map = add_instantiation(ctx, map, " ip-1+ 1 <= i ", " diagptr_ipM1_+ 1 <= rowptr_i_ ");
map = add_instantiation(ctx, map, " ip-1+ 1 <= i+1 ", " diagptr_ipM1_+ 1 <= rowptr_iP1_ ");
map = add_instantiation(ctx, map, " ip-1+ 1 <= ip ", " diagptr_ipM1_+ 1 <= rowptr_ip_ ");
map = add_instantiation(ctx, map, " ip-1+ 1 <= ip+1 ", " diagptr_ipM1_+ 1 <= rowptr_ipP1_ ");
map = add_instantiation(ctx, map, " ip-1+ 1 <= colidx_k_+1 ", " diagptr_ipM1_+ 1 <= rowptr_colidx_k_P1_ ");
map = add_instantiation(ctx, map, " ip-1+ 1 <= colidx_kp_+1 ", " diagptr_ipM1_+ 1 <= rowptr_colidx_kp_P1_ ");}

	printf("\n");
	
	printf("%s\n", isl_map_to_str(map));
	printf("%d\n", isl_map_is_empty(map));

	//isl_basic_map *bmap = isl_map_affine_hull(map);
       // char *out = isl_basic_map_to_str(bmap);
	//printf("%s\n", out);



	isl_map_free(orig);
	//isl_basic_map_free(bmap);

	isl_ctx_free(ctx);

   return 0;
}

