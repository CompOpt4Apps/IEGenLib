#include "iegenlib.h"
#include <utility>
#include <fstream>
#include <iostream>
#include <sstream>

using iegenlib::Computation;
using namespace std;

Computation* Find_Segment_Computation();
Computation* Eval_Spline_f_Computation();
Computation* Eval_Spline_df_Computation(); 
Computation* Eval_Spline_ddf_Computation();

Computation* c_Computation();
Computation* v_Computation();
Computation* u_Computation();
Computation* c_diff_Computation();
Computation* v_diff_Computation();
Computation* u_diff_Computation();

/**
 * Main function
 */
int main(int argc, char **argv){
   
    Computation* updateSources = new Computation(); //renamed from temp to updateSource

    updateSources->addDataSpace("$ray_length$");
    updateSources->addDataSpace("$current_values$");
    updateSources->addDataSpace("$spl$");
    updateSources->addDataSpace("$sources$");

    updateSources->addDataSpace("$r$");
    updateSources->addDataSpace("$theta$");
    updateSources->addDataSpace("$phi$");
     
    // double r = current_values[0],             theta = current_values[1],      phi = current_values[2];
    Stmt* s01 = new Stmt("double $r$ = $current_values$[0], $theta$=$current_values$[1], $phi$=$current_values$[2];",
         "{[0]}",
         "{[0]->[0]}",
         {
            {"$current_values$","{[0]->[0]}"},
            {"$current_values$","{[0]->[1]}"},
            {"$current_values$","{[0]->[2]}"}
         },
         {
            {"$r$", "{[0]->[0]}"},
            {"$theta$", "{[0]->[0]}"},
            {"$phi$", "{[0]->[0]}"}
         }
         );
    updateSources->addStmt(s01);
  
    //double nu[3] = {current_values[3],        current_values[4],              current_values[5]};
    updateSources->addDataSpace("$nu$");
    Stmt* s02 = new Stmt("double $nu$[3] = {$current_values$[3], $current_values$[4], $current_values$[5]};",
         "{[0]}",
         "{[0]->[1]}",
         {
            {"$current_values$","{[0]->[3]}"},
            {"$current_values$","{[0]->[4]}"},
            {"$current_values$","{[0]->[5]}"}
         },
         {{"$nu$", "{[0]->[3]}"}}
         );
    updateSources->addStmt(s02);
    

  
    //Add the args of the c function (computation to be appended) as data spaces to temp
    updateSources->addDataSpace("$spl.Temp_Spline$");
  
    //Args to the c_Computation
    vector<std::string> cCompArgs;
    cCompArgs.push_back("$r$");
    cCompArgs.push_back("$theta$");
    cCompArgs.push_back("$phi$");
    cCompArgs.push_back("$spl.Temp_Spline$");

    Computation* cComputation = c_Computation();

    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult cCompRes = updateSources->appendComputation(cComputation, "{[0]}", "{[0]->[2]}", cCompArgs);
 
    unsigned int newTuplePos = cCompRes.tuplePosition+1;
    updateSources->addDataSpace("$sources.c$");
    //Creating s1
    //sources.c = c(r,theta,phi,spl.Temp_Spline);
    Stmt* s1 = new Stmt("$sources.c$ = "+cCompRes.returnValues.back()+";", 
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos)+"]}",
        {{cCompRes.returnValues.back(), "{[0]->[0]}"}},
        {{"$sources.c$", "{[0]->[0]}"}}
        );
    cout << "Source statement : " << s1->getStmtSourceCode() << "\n\t"
      <<"- Iteration Space : "<< s1->getIterationSpace()->prettyPrintString() << "\n\t"
      << "- Execution Schedule : "<< s1->getExecutionSchedule()->prettyPrintString() << "\n\t" ;
  
    
    updateSources->addStmt(s1);
  
    updateSources->addDataSpace("$sources.w$");
    //Creating s2
    //sources.w = w(r,theta,phi); The w function always return 0!
    Stmt* s2 = new Stmt("$sources.w$ = 0;", 
        "{[0]}",  //Iteration schedule
        "{[0]->["+std::to_string(newTuplePos+1)+"]}",  //Execution schedule
        {}, //Data reads
        {{"$sources.w$", "{[0]->[0]}"}} //Data writes
        );
    
    updateSources->addStmt(s2);
    
    //Add the args of the v function (computation to be appended) as data spaces to temp
    //r, theta and phi have already been added
    updateSources->addDataSpace("$spl.Windv_Spline$");
    //Args to the v_Computation
    vector<std::string> vCompArgs;
    vCompArgs.push_back("$r$");
    vCompArgs.push_back("$theta$");
    vCompArgs.push_back("$phi$");
    vCompArgs.push_back("$spl.Windv_Spline$");
    
    Computation* vComputation = v_Computation();
    
    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult vCompRes = updateSources->appendComputation(vComputation, "{[0]}", "{[0]->["+std::to_string(newTuplePos+2)+"]}", vCompArgs);
    newTuplePos = vCompRes.tuplePosition+1;
    
    updateSources->addDataSpace("$sources.v$");
    //Creating s3
    //sources.v = v(r,theta,phi,spl.Temp_Spline);
    Stmt* s3 = new Stmt("$sources.v$ = "+vCompRes.returnValues.back()+";", 
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos)+"]}",
        {{vCompRes.returnValues.back(), "{[0]->[0]}"}},
        {{"$sources.v$", "{[0]->[0]}"}}
        );
    
    updateSources->addStmt(s3);
  
  
    //Add the args of the u function (computation to be appended) as data spaces to temp
    //r, theta and phi have already been added
    updateSources->addDataSpace("$spl.Windu_Spline$");
    //Args to the u_Computation
    vector<std::string> uCompArgs;
    uCompArgs.push_back("$r$");
    uCompArgs.push_back("$theta$");
    uCompArgs.push_back("$phi$");
    uCompArgs.push_back("$spl.Windu_Spline$");
    
    Computation* uComputation = u_Computation();
    
    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult uCompRes = updateSources->appendComputation(uComputation, "{[0]}", "{[0]->["+std::to_string(newTuplePos+1)+"]}", uCompArgs);
    
    newTuplePos = uCompRes.tuplePosition+1;
    updateSources->addDataSpace("$sources.u$");
    //Creating s4
    //sources.u = u(r,theta,phi, spl.Windu_Spline);
    Stmt* s4 = new Stmt("$sources.u$ = "+uCompRes.returnValues.back()+";", 
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos)+"]}",
        {{uCompRes.returnValues.back(), "{[0]->[0]}"}},
        {{"$sources.u$", "{[0]->[0]}"}}
        );
    
    updateSources->addStmt(s4);

    //Args to the c_diff
    vector<std::string> cDiffCompArgs;
    cDiffCompArgs.push_back("$r$");
    cDiffCompArgs.push_back("$theta$");
    cDiffCompArgs.push_back("$phi$");
    cDiffCompArgs.push_back("0");
    cDiffCompArgs.push_back("$spl.Temp_Spline$");
    
    Computation* cDiffComputation = c_diff_Computation();
    
    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult cDiffCompRes = updateSources->appendComputation(cDiffComputation, "{[0]}", "{[0]->["+std::to_string(newTuplePos+1)+"]}", cDiffCompArgs);
    
    newTuplePos = cDiffCompRes.tuplePosition+1;
    updateSources->addDataSpace("$sources.dc$");
    //Creating s5
    //sources.dc[0] = c_diff(r,theta,phi,0,spl.Temp_Spline);
    Stmt* s5 = new Stmt("$sources.dc$[0] = "+cDiffCompRes.returnValues.back()+";", 
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos)+"]}",
        {{cDiffCompRes.returnValues.back(), "{[0]->[0]}"}},
        {{"$sources.dc$", "{[0]->[0]}"}}
        );
    
    updateSources->addStmt(s5);

    updateSources->addDataSpace("$sources.dw$");
    //Creating s6
    //sources.dw[0] = w_diff(r,theta,phi,0); The w_diff function always returns 0!
    Stmt* s6 = new Stmt("$sources.dw$[0] = 0;", 
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos+1)+"]}",
        {},
        {{"$sources.dw$", "{[0]->[0]}"}}
        );
    
    updateSources->addStmt(s6);
    
    //Args to the v_diff
    vector<std::string> vDiffCompArgs;
    vDiffCompArgs.push_back("$r$");
    vDiffCompArgs.push_back("$theta$");
    vDiffCompArgs.push_back("$phi$");
    vDiffCompArgs.push_back("0");
    vDiffCompArgs.push_back("$spl.Windv_Spline$");
    
    Computation* vDiffComputation = v_diff_Computation();
    
    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult vDiffCompRes = updateSources->appendComputation(vDiffComputation, "{[0]}", "{[0]->["+std::to_string(newTuplePos+2)+"]}", vDiffCompArgs);   
    newTuplePos = vDiffCompRes.tuplePosition+1;
  
    updateSources->addDataSpace("$sources.dv$");
    //Creating s7
    //sources.dv[0] = v_diff(r,theta,phi,0,spl.Windv_Spline);
    Stmt* s7 = new Stmt("$sources.dv$[0] = "+vDiffCompRes.returnValues.back()+";", 
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos)+"]}",
        {{vDiffCompRes.returnValues.back(), "{[0]->[0]}"}},
        {{"$sources.dv$", "{[0]->[0]}"}}
        );
    updateSources->addStmt(s7);
    
     //Args to the u_diff
    vector<std::string> uDiffCompArgs;
    uDiffCompArgs.push_back("$r$");
    uDiffCompArgs.push_back("$theta$");
    uDiffCompArgs.push_back("$phi$");
    uDiffCompArgs.push_back("0");
    uDiffCompArgs.push_back("$spl.Windu_Spline$");
 
    Computation* uDiffComputation = u_diff_Computation();
 
    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult uDiffCompRes = updateSources->appendComputation(uDiffComputation, "{[0]}", "{[0]->["+std::to_string(newTuplePos+1)+"]}", uDiffCompArgs);

    newTuplePos = uDiffCompRes.tuplePosition+1;
    updateSources->addDataSpace("$sources.du$");
    //Creating s8
    //sources.du[0] = u_diff(r,theta,phi,0,spl.Windu_Spline);
    Stmt* s8 = new Stmt("$sources.du$[0] = "+uDiffCompRes.returnValues.back()+";",
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos)+"]}",
        {{uDiffCompRes.returnValues.back(), "{[0]->[0]}"}},
        {{"$sources.du$", "{[0]->[0]}"}}
        );
 
    updateSources->addStmt(s8);
  
    //Clearing arguments and adding new ones
    cDiffCompArgs.clear();
    cDiffCompArgs.push_back("$r$");
    cDiffCompArgs.push_back("$theta$");
    cDiffCompArgs.push_back("$phi$");
    cDiffCompArgs.push_back("1");
    cDiffCompArgs.push_back("$spl.Temp_Spline$");
    
    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult cDiff1CompRes = updateSources->appendComputation(cDiffComputation, "{[0]}", "{[0]->["+std::to_string(newTuplePos+1)+"]}", cDiffCompArgs);
    
    newTuplePos = cDiff1CompRes.tuplePosition+1;
    //Creating s9
    //sources.dc[1] = c_diff(r,theta,phi,1,spl.Temp_Spline);
    Stmt* s9 = new Stmt("$sources.dc$[1] = "+cDiff1CompRes.returnValues.back()+";", 
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos)+"]}",
        {{cDiff1CompRes.returnValues.back(), "{[0]->[0]}"}},
        {{"$sources.dc$", "{[0]->[1]}"}}
        );
    
    updateSources->addStmt(s9);
   
    //Creating s10
    //sources.dw[0] = w_diff(r,theta,phi,0); The w_diff function always returns 0!
    Stmt* s10 = new Stmt("$sources.dw$[1] = 0;", 
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos+1)+"]}",
        {},
        {{"$sources.dw$", "{[0]->[1]}"}}
        );
    
    updateSources->addStmt(s10);
   
    //Clearing arguments and adding new ones
    vDiffCompArgs.clear();
    vDiffCompArgs.push_back("$r$");
    vDiffCompArgs.push_back("$theta$");
    vDiffCompArgs.push_back("$phi$");
    vDiffCompArgs.push_back("1");
    vDiffCompArgs.push_back("$spl.Windv_Spline$");
    
    AppendComputationResult vDiff1CompRes = updateSources->appendComputation(vDiffComputation, "{[0]}", "{[0]->["+std::to_string(newTuplePos+2)+"]}", vDiffCompArgs);   
    newTuplePos = vDiff1CompRes.tuplePosition+1;
  
    //Creating s11
    //sources.dv[1] = v_diff(r,theta,phi,1,spl.Windv_Spline);
    Stmt* s11 = new Stmt("$sources.dv$[1] = "+vDiff1CompRes.returnValues.back()+";", 
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos)+"]}",
        {{vDiff1CompRes.returnValues.back(), "{[0]->[0]}"}},
        {{"$sources.dv$", "{[0]->[1]}"}}
        );
    updateSources->addStmt(s11);
    
    //Clearing arguments and adding new ones
    uDiffCompArgs.clear();
    uDiffCompArgs.push_back("$r$");
    uDiffCompArgs.push_back("$theta$");
    uDiffCompArgs.push_back("$phi$");
    uDiffCompArgs.push_back("1");
    uDiffCompArgs.push_back("$spl.Windu_Spline$");
 
    AppendComputationResult uDiff1CompRes = updateSources->appendComputation(uDiffComputation, "{[0]}", "{[0]->["+std::to_string(newTuplePos+1)+"]}", uDiffCompArgs);
    newTuplePos = uDiff1CompRes.tuplePosition+1;

    //Creating s12
    //sources.du[1] = u_diff(r,theta,phi,1,spl.Windu_Spline);
    Stmt* s12 = new Stmt("$sources.du$[1] = "+uDiff1CompRes.returnValues.back()+";",
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos)+"]}",
        {{uDiff1CompRes.returnValues.back(), "{[0]->[0]}"}},
        {{"$sources.du$", "{[0]->[1]}"}}
        );
 
    updateSources->addStmt(s12);

   //Clearing arguments and adding new ones
    cDiffCompArgs.clear();
    cDiffCompArgs.push_back("$r$");
    cDiffCompArgs.push_back("$theta$");
    cDiffCompArgs.push_back("$phi$");
    cDiffCompArgs.push_back("2");
    cDiffCompArgs.push_back("$spl.Temp_Spline$");
    
    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult cDiff2CompRes = updateSources->appendComputation(cDiffComputation, "{[0]}", "{[0]->["+std::to_string(newTuplePos+1)+"]}", cDiffCompArgs);
    
    newTuplePos = cDiff2CompRes.tuplePosition+1;
    //Creating s13
    //sources.dc[2] = c_diff(r,theta,phi,2,spl.Temp_Spline);
    Stmt* s13 = new Stmt("$sources.dc$[2] = "+cDiff2CompRes.returnValues.back()+";", 
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos)+"]}",
        {{cDiff2CompRes.returnValues.back(), "{[0]->[0]}"}},
        {{"$sources.dc$", "{[0]->[2]}"}}
        );
    
    updateSources->addStmt(s13);
   
    //Creating s14
    //sources.dw[2] = w_diff(r,theta,phi,0); The w_diff function always returns 0!
    Stmt* s14 = new Stmt("$sources.dw$[2] = 0;", 
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos+1)+"]}",
        {},
        {{"$sources.dw$", "{[0]->[2]}"}}
        );
    
    updateSources->addStmt(s14);
   
    //Clearing arguments and adding new ones
    vDiffCompArgs.clear();
    vDiffCompArgs.push_back("$r$");
    vDiffCompArgs.push_back("$theta$");
    vDiffCompArgs.push_back("$phi$");
    vDiffCompArgs.push_back("2");
    vDiffCompArgs.push_back("$spl.Windv_Spline$");
    
    AppendComputationResult vDiff2CompRes = updateSources->appendComputation(vDiffComputation, "{[0]}", "{[0]->["+std::to_string(newTuplePos+2)+"]}", vDiffCompArgs);   
    newTuplePos = vDiff2CompRes.tuplePosition+1;
  
    //Creating s15
    //sources.dv[2] = v_diff(r,theta,phi,2,spl.Windv_Spline);
    Stmt* s15 = new Stmt("$sources.dv$[2] = "+vDiff2CompRes.returnValues.back()+";", 
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos)+"]}",
        {{vDiff2CompRes.returnValues.back(), "{[0]->[0]}"}},
        {{"$sources.dv$", "{[0]->[2]}"}}
        );
    updateSources->addStmt(s15);
    
    //Clearing arguments and adding new ones
    uDiffCompArgs.clear();
    uDiffCompArgs.push_back("$r$");
    uDiffCompArgs.push_back("$theta$");
    uDiffCompArgs.push_back("$phi$");
    uDiffCompArgs.push_back("2");
    uDiffCompArgs.push_back("$spl.Windu_Spline$");
 
    AppendComputationResult uDiff2CompRes = updateSources->appendComputation(uDiffComputation, "{[0]}", "{[0]->["+std::to_string(newTuplePos+1)+"]}", uDiffCompArgs);
    newTuplePos = uDiff2CompRes.tuplePosition+1;

    //Creating s16
    //sources.du[2] = u_diff(r,theta,phi,2,spl.Windu_Spline);
    Stmt* s16 = new Stmt("$sources.du$[2] = "+uDiff2CompRes.returnValues.back()+";",
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos)+"]}",
        {{uDiff2CompRes.returnValues.back(), "{[0]->[0]}"}},
        {{"$sources.du$", "{[0]->[2]}"}}
        );
 
    updateSources->addStmt(s16); 
    
    //Data Spaces
    updateSources->addDataSpace("$sources.nu_mag$");
    //updateSources->addDataSpace("$nu$"); //Once first two lines are uncommented this will be a duplicate
    //Creating s17
    //sources.nu_mag =   sqrt( nu[0]*nu[0] + nu[1]*nu[1] + nu[2]*nu[2]);
    Stmt* s17 = new Stmt("$sources.nu_mag$ = sqrt( $nu$[0]*$nu$[0] + $nu$[1]*$nu$[1] + $nu$[2]*$nu$[2]);",
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos+1)+"]}",
        {
            {"$nu$","{[0]->[0]}"},
            {"$nu$","{[0]->[1]}"},
            {"$nu$","{[0]->[2]}"}
        }, 
        {{"$sources.nu_mag$", "{[0]->[0]}"}}
        );

    updateSources->addStmt(s17);

    //Data Spaces    
    updateSources->addDataSpace("$sources.c_gr$");
    //Creating S18
    //sources.c_gr[0] =  sources.c*nu[0]/sources.nu_mag + sources.w;
    Stmt* s18 = new Stmt("$sources.c_gr$[0] =  $sources.c$*$nu$[0]/$sources.nu_mag$ + $sources.w$;",
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos+2)+"]}",
        {
            {"$sources.c$", "{[0]->[0]}"},
            {"$nu$","{[0]->[0]}"},
            {"$sources.nu_mag$", "{[0]->[0]}"},
            {"$sources.w$", "{[0]->[0]}"}
        },
        {{"$sources.c_gr$", "{[0]->[0]}"}}
        );

    updateSources->addStmt(s18);
   
    //Creating S19
    //sources.c_gr[1] =  sources.c*nu[1]/sources.nu_mag + sources.v;
    Stmt* s19 = new Stmt("$sources.c_gr$[1] =  $sources.c$*$nu$[1]/$sources.nu_mag$ + $sources.v$;",
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos+3)+"]}",
        {
            {"$sources.c$", "{[0]->[0]}"},
            {"$nu$","{[0]->[1]}"},
            {"$sources.nu_mag$", "{[0]->[0]}"},
            {"$sources.v$", "{[0]->[0]}"}
        },
        {{"$sources.c_gr$", "{[0]->[1]}"}}
        );

    updateSources->addStmt(s19);
   
    //Creating s20
    //sources.c_gr[2] =  sources.c*nu[2]/sources.nu_mag + sources.u;
    Stmt* s20 = new Stmt("$sources.c_gr$[2] =  $sources.c$*$nu$[2]/$sources.nu_mag$ + $sources.u$;",
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos+4)+"]}",
        {
            {"$sources.c$", "{[0]->[0]}"},
            {"$nu$","{[0]->[2]}"},
            {"$sources.nu_mag$", "{[0]->[0]}"},
            {"$sources.v$", "{[0]->[0]}"}
        },
        {{"$sources.c_gr$", "{[0]->[2]}"}}
        );

    updateSources->addStmt(s20);
    
    //Data Space
    updateSources->addDataSpace("$sources.c_gr_mag$");
   
    //Creating s21
    //sources.c_gr_mag = sqrt(pow(sources.c_gr[0],2) + pow(sources.c_gr[1],2) + pow(sources.c_gr[2],2));
    Stmt* s21 = new Stmt("$sources.c_gr_mag$ = sqrt(pow($sources.c_gr$[0],2) + pow($sources.c_gr$[1],2) + pow($sources.c_gr$[2],2));",
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos+5)+"]}",
        {
            {"$sources.c_gr$", "{[0]->[0]}"},
            {"$sources.c_gr$", "{[0]->[1]}"},
            {"$sources.c_gr$", "{[0]->[2]}"}
        },
        {{"$sources.c_gr_mag$", "{[0]->[0]}"}}
        );

    updateSources->addStmt(s21);
    
    //Data Space
    updateSources->addDataSpace("$sources.GeoCoeff$");
 
    //Creating s22
    //sources.GeoCoeff[0] = 1.0;
    Stmt* s22 = new Stmt("$sources.GeoCoeff$[0] = 1.0;", 
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos+6)+"]}",
        {},
        {{"$sources.GeoCoeff$", "{[0]->[0]}"}}
        );
    
    updateSources->addStmt(s22);
   
    //Creating s23
    //sources.GeoCoeff[1] = 1.0/r;
    Stmt* s23 = new Stmt("$sources.GeoCoeff$[1] = 1.0/$r$;", 
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos+7)+"]}",
        {{"$r$", "{[0]->[0]}"}},
        {{"$sources.GeoCoeff$", "{[0]->[1]}"}}
        );
    
    updateSources->addStmt(s23);
     
    //Creating S24
    //sources.GeoCoeff[2] = 1.0/(r*cos(theta));
    Stmt* s24 = new Stmt("$sources.GeoCoeff$[2] = 1.0/($r$*cos($theta$));", 
	    "{[0]}",
	    "{[0]->["+std::to_string(newTuplePos+8)+"]}",
	    {
	        {"$r$", "{[0]->[0]}"},
	        {"$theta$", "{[0]->[0]}"}
	    },
	    {{"$sources.GeoCoeff$", "{[0]->[2]}"}}
        );
    updateSources->addStmt(s24);

    //Data Space
    updateSources->addDataSpace("$sources.GeoTerms$");

    //Creating S25
    //sources.GeoTerms[0] = 0.0;
    Stmt* s25 = new Stmt("$sources.GeoTerms$[0] = 0.0;", 
	    "{[0]}",
	    "{[0]->["+std::to_string(newTuplePos+9)+"]}",
	    {},
	    {{"$sources.GeoTerms$", "{[0]->[0]}"}}
        );
    updateSources->addStmt(s25);

    //Creating S26
    //sources.GeoTerms[1] = (nu[0]*sources.v - nu[1]*sources.w);
    Stmt* s26 = new Stmt("$sources.GeoTerms$[1] = ($nu$[0]*$sources.v$ - $nu$[1]*$sources.w$);", 
	    "{[0]}",
	    "{[0]->["+std::to_string(newTuplePos+10)+"]}",
	    {
	        {"$nu$","{[0]->[0]}"},
	        {"$sources.v$", "{[0]->[0]}"},
	        {"$nu$","{[0]->[1]}"},
	        {"$sources.w$", "{[0]->[0]}"}
	    },
	    {{"$sources.GeoTerms$", "{[0]->[1]}"}}
        );
    updateSources->addStmt(s26);

    //Creating S27
    //sources.GeoTerms[2] = (nu[0]*sources.u - nu[2]*sources.w)*cos(theta) + (nu[1]*sources.u - nu[2]*sources.v)*sin(theta);
    Stmt* s27 = new Stmt("$sources.GeoTerms$[2] = ($nu$[0]*$sources.u$ - $nu$[2]*$sources.w$)*cos($theta$) + ($nu$[1]*$sources.u$ - $nu$[2]*$sources.v$)*sin($theta$);", 
	    "{[0]}",
	    "{[0]->["+std::to_string(newTuplePos+11)+"]}",
	    {
	        {"$nu$","{[0]->[0]}"},
	        {"$sources.u$", "{[0]->[0]}"},
	        {"$nu$","{[0]->[2]}"},
	        {"$sources.w$", "{[0]->[0]}"},
	        {"$theta$", "{[0]->[0]}"},
	        {"$nu$","{[0]->[1]}"},
	        {"$sources.v$", "{[0]->[0]}"}
	    },
	    {{"$sources.GeoTerms$", "{[0]->[2]}"}}
        );
    updateSources->addStmt(s27);

    //Creating S28
    //sources.GeoTerms[0] += 1.0/r * (nu[1]*sources.c_gr[1] + nu[2]*sources.c_gr[2]);
    Stmt* s28 = new Stmt("$sources.GeoTerms$[0] += 1.0/$r$ * ($nu$[1]*$sources.c_gr$[1] + $nu$[2]*$sources.c_gr$[2]);", 
	    "{[0]}",
	    "{[0]->["+std::to_string(newTuplePos+12)+"]}",
	    {
	        {"$sources.GeoTerms$", "{[0]->[0]}"},  
	        {"$r$", "{[0]->[0]}"},
	        {"$nu$","{[0]->[1]}"},
	        {"$sources.c_gr$", "{[0]->[1]}"},
	        {"$nu$","{[0]->[2]}"},
	        {"$sources.c_gr$", "{[0]->[2]}"}
	    },
	    {{"$sources.GeoTerms$", "{[0]->[0]}"}}
        );
    updateSources->addStmt(s28);

    //Creating s29
    //sources.GeoTerms[1] += -nu[0]*sources.c_gr[1] + nu[2]*sources.c_gr[2]*tan(theta);
    Stmt* s29 = new Stmt("$sources.GeoTerms$[1] += -$nu$[0]*$sources.c_gr$[1] + $nu$[2]*$sources.c_gr$[2]*tan($theta$);", 
	    "{[0]}",
	    "{[0]->["+std::to_string(newTuplePos+13)+"]}",
	    {
	        {"$sources.GeoTerms$", "{[0]->[1]}"},  
	        {"$nu$","{[0]->[0]}"},
	        {"$sources.c_gr$", "{[0]->[1]}"},
	        {"$nu$","{[0]->[2]}"},
	        {"$sources.c_gr$", "{[0]->[1]}"},
	        {"$theta$", "{[0]->[0]}"}
	    },
	    {{"$sources.GeoTerms$", "{[0]->[1]}"}}
        );
    updateSources->addStmt(s29);

    //Creating s30
    //sources.GeoTerms[2] += -sources.c_gr[2]*(nu[0]*cos(theta) + nu[1]*sin(theta));
    Stmt* s30 = new Stmt("$sources.GeoTerms$[2] += -$sources.c_gr$[2]*($nu$[0]*cos($theta$) + $nu$[1]*sin($theta$));",
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos+14)+"]}",
        {
            {"$sources.c_gr$", "{[0]->[2]}"},
            {"$nu$", "{[0]->[0]}"},
            {"$theta$", "{[0]->[0]}"},
            {"$nu$", "{[0]->[1]}"}
        },
        {{"$sources.GeoTerms$", "{[0]->[2]}"}}
        );
    updateSources->addStmt(s30);
   
    //Data spaces
    updateSources->addDataSpace("$R_lt$");
    updateSources->addDataSpace("$R_lp$");
    updateSources->addDataSpace("$mu_lt$");
    updateSources->addDataSpace("$mu_lp$");

    //Creating s31
    //double R_lt[3], R_lp[3], mu_lt[3], mu_lp[3];
    Stmt* s31 = new Stmt("double $R_lt$[3], $R_lp$[3], $mu_lt$[3], $mu_lp$[3];",
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos+15)+"]}",
        {},
        {}
        );
    cout << "Source statement : " << s31->getStmtSourceCode() << "\n\t"
      <<"- Iteration Space : "<< s31->getIterationSpace()->prettyPrintString() << "\n\t"
      << "- Execution Schedule : "<< s31->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

    updateSources->addStmt(s31);

    //Creating s32
    //R_lt[0]  = current_values[6];       R_lt[1]  = current_values[7];       R_lt[2]  = current_values[8];
    Stmt*  s32 = new Stmt("$R_lt$[0]  = $current_values$[6];       $R_lt$[1]  = $current_values$[7];       $R_lt$[2]  = $current_values$[8];",
        "{[0]: $GeoAc_CalcAmp$ = true}",
        "{[0]->["+std::to_string(newTuplePos+16)+"]}",
        {
            {"$current_values$","{[0]->[6]}"},
            {"$current_values$","{[0]->[7]}"},
            {"$current_values$","{[0]->[8]}"}
        }, 
        {
            {"$R_lt$", "{[0]->[0]}"},
            {"$R_lt$", "{[0]->[1]}"},
            {"$R_lt$", "{[0]->[2]}"}
        }
        );  
    
    updateSources->addStmt(s32);

    
    //Creating s33
    //mu_lt[0] = current_values[9];     mu_lt[1] = current_values[10];      mu_lt[2] = current_values[11];
    Stmt* s33 = new Stmt("$mu_lt$[0] = $current_values$[9];     $mu_lt$[1] = $current_values$[10];      $mu_lt$[2] = $current_values$[11];",
        "{[0]: $GeoAc_CalcAmp$ = true}",
        "{[0]->["+std::to_string(newTuplePos+17)+"]}",
        {
            {"$current_values$","{[0]->[9]}"},
            {"$current_values$","{[0]->[10]}"},
            {"$current_values$","{[0]->[11]}"}
        }, 
        {
            {"$mu_lt$", "{[0]->[0]}"},
            {"$mu_lt$", "{[0]->[1]}"},
            {"$mu_lt$", "{[0]->[2]}"}
        }
        );  
    
    updateSources->addStmt(s33);

    //Creating s34
    //R_lp[0]  = current_values[12];      R_lp[1]  = current_values[13];      R_lp[2]  = current_values[14];
    Stmt* s34 = new Stmt("$R_lp$[0]  = $current_values$[12]; $R_lp$[1] = $current_values$[13]; $R_lp$[2]  = $current_values$[14];",        
        "{[0]: $GeoAc_CalcAmp$ = true}",
        "{[0]->["+std::to_string(newTuplePos+18)+"]}",
        {
            {"$current_values$","{[0]->[12]}"},
            {"$current_values$","{[0]->[13]}"},
            {"$current_values$","{[0]->[14]}"}
        }, 
        {
            {"$R_lp$", "{[0]->[0]}"},
            {"$R_lp$", "{[0]->[1]}"},
            {"$R_lp$", "{[0]->[2]}"}
        }
        );  
    
    updateSources->addStmt(s34);

    //Creating s35
    //mu_lp[0] = current_values[15];        mu_lp[1] = current_values[16];      mu_lp[2] = current_values[17];
    Stmt* s35 = new Stmt("$mu_lp$[0] = $current_values$[15]; $mu_lp$[1] = $current_values$[16]; $mu_lp$[2] = $current_values$[17];",
        "{[0]: $GeoAc_CalcAmp$ = true}",
        "{[0]->["+std::to_string(newTuplePos+19)+"]}",
        {
            {"$current_values$","{[0]->[15]}"},
            {"$current_values$","{[0]->[16]}"},
            {"$current_values$","{[0]->[17]}"}
        }, 
        {
            {"$mu_lp$", "{[0]->[0]}"},
            {"$mu_lp$", "{[0]->[1]}"},
            {"$mu_lp$", "{[0]->[2]}"}
        }
        );  
    
    updateSources->addStmt(s35);

     //Creating s36
     //sources.dc[3] = 0.0;  sources.dc[4] = 0.0;
     Stmt* s36 = new Stmt("$sources.dc$[3] = 0.0;  $sources.dc$[4] = 0.0;",
          "{[0]: $GeoAc_CalcAmp$ = true}",
          "{[0]->["+std::to_string(newTuplePos+20)+"]}",
          {},
          {{"$sources.dc$", "{[0]->[3]}"},
           {"$sources.dc$", "{[0]->[4]}"}
	  }
     );
     updateSources->addStmt(s36);

     //Creating s37
     //sources.dw[3] = 0.0;  sources.dw[4] = 0.0;
     Stmt* s37 = new Stmt("$sources.dw$[3] = 0.0;  $sources.dw$[4] = 0.0;",
          "{[0]: $GeoAc_CalcAmp$ = true}",
          "{[0]->["+std::to_string(newTuplePos+21)+"]}",
          {},
          {{"$sources.dw$", "{[0]->[3]}"},
           {"$sources.dw$", "{[0]->[4]}"}
          }
    );
    updateSources->addStmt(s37);
 
     //Creating s38
     //sources.dv[3] = 0.0;  sources.dv[4] = 0.0;
     Stmt* s38 = new Stmt("$sources.dv$[3] = 0.0;  $sources.dv$[4] = 0.0;",
          "{[0]: $GeoAc_CalcAmp$ = true}",
          "{[0]->["+std::to_string(newTuplePos+22)+"]}",
          {},
          {{"$sources.dv$", "{[0]->[3]}"},
           {"$sources.dv$", "{[0]->[4]}"}
          }
     );
     updateSources->addStmt(s38);
 
     //Creating s39
     //sources.du[3] = 0.0;  sources.du[4] = 0.0;
     Stmt* s39 = new Stmt("$sources.du$[3] = 0.0;  $sources.du$[4] = 0.0;",
          "{[0]: $GeoAc_CalcAmp$ = true}",
          "{[0]->["+std::to_string(newTuplePos+23)+"]}",
          {},
          {{"$sources.du$", "{[0]->[3]}"},
           {"$sources.du$", "{[0]->[4]}"}
          }
    );
    updateSources->addStmt(s39); 
    
    //Calling toDot() on the Computation structure
    /*
    ofstream dotFileStream("codegen_dot.txt");
    cout << "Entering toDot()" << "\n";
    string dotString = updateSources->toDotString();
    dotFileStream << dotString;
    dotFileStream.close();
    */
  
    //Write codegen to a file
    ofstream outStream;
    outStream.open("codegen.c");
    outStream << updateSources->codeGen();
    outStream.close();
  
    return 0;
}


/*
  double u_diff(double r, double theta, double phi int n, NaturalCubicSpline_1D &Windu_Spline){
    double r_eval = min(r, r_max);  r_eval = max(r_eval, r_min);    // Check that r_min <= r_eval <= r_max
    double u_diff_return = 0.0;
    double eval_Spline_df_return = Eval_Spline_df(r_eval, Windu_Spline);

    if(n==0){u_diff_return = eval_Spline_df_return;}
*/
Computation* u_diff_Computation(){
    Computation* uDiffComputation = new Computation();
    uDiffComputation->addParameter("$r$", "double");
    uDiffComputation->addParameter("$theta$", "double");
    uDiffComputation->addParameter("$phi$","double");
    uDiffComputation->addParameter("$n$","int");
    uDiffComputation->addParameter("$Windu_Spline$", "NaturalCubicSpline_1D &");

    uDiffComputation->addDataSpace("$r_eval$");
    //Creating statement1
    //double r_eval = min(r, r_max);  r_eval = max(r_eval, r_min);
    Stmt* s0 = new Stmt("double $r_eval$ = min($r$, r_max); $r_eval$ = max($r_eval$, r_min);",
        "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
        "{[0]->[0]}", //Execution schedule - scheduling statement to be first (scheduling function)
        {{"$r$","{[0]->[0]}"},{"$r_eval$","{[0]->[0]}"}}, //Data reads
        {{"$r_eval$","{[0]->[0]}"}} //Data writes
        );
    cout << "Source statement : " << s0->getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s0->getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s0->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

    //Adding s0 to the computation
    uDiffComputation->addStmt(s0);
   
     uDiffComputation->addDataSpace("$u_diff_return$");
    //double u_diff_return = 0.0;
    Stmt* s1 = new Stmt("double $u_diff_return$ = 0.0",
        "{[0]}",
        "{[0]->[1]}",
        {},
        {{"$u_diff_return$","{[0]->[0]}"}}
        );
    cout << "Source statement : " << s1->getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s1->getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s1->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

    //Adding s1 to the computation
    uDiffComputation->addStmt(s1);

    Computation* evalSplineDfComputation = Eval_Spline_df_Computation();
   
    //Args to the c_Computation
    vector<std::string> evalSplineDfCompArgs;
    evalSplineDfCompArgs.push_back("$r_eval$");
    evalSplineDfCompArgs.push_back("$Windu_Spline$");

    AppendComputationResult evalSplineDfCompRes = uDiffComputation->appendComputation(evalSplineDfComputation, "{[0]}", "{[0]->[2]}", evalSplineDfCompArgs);// possible 0->0?
    unsigned int  newTuplePos = evalSplineDfCompRes.tuplePosition+1;

    uDiffComputation->addDataSpace("$eval_Spline_df_return$");

    //Creating statement s3
    //double eval_Spline_df_return = Eval_Spline_df(r_eval,Windu_Spline);
    Stmt* s2 = new Stmt("double $eval_Spline_df_return$ = "+evalSplineDfCompRes.returnValues.back()+";",
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos)+"]}",
        {{evalSplineDfCompRes.returnValues.back(), "{[0]->[0]}"}},
        {{"$eval_Spline_df_return$", "{[0]->[0]}"}}
        );
    cout << "Source statement : " << s2->getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s2->getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s2->getExecutionSchedule()->prettyPrintString() << "\n\t" ;
    
    //Adding s2 to the computation
    uDiffComputation->addStmt(s2);

    //Creating statement s3
    // if(n==0){u_diff_return = eval_Spline_df_return;}
    Stmt* s3 = new Stmt("if($n$==0){$u_diff_return$ = $eval_Spline_df_return$;}",
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos+1)+"]}",
        {
            {"$n$", "{[0]->[0]}"},
            {"$eval_Spline_df_return$", "{[0]->[0]}"}
        },
        {
            {"$u_diff_return$", "{[0]->[0]}"}
        }
        );
    cout << "Source statement : " << s3->getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s3->getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s3->getExecutionSchedule()->prettyPrintString() << "\n\t";

    uDiffComputation->addStmt(s3);
    uDiffComputation->addReturnValue("$u_diff_return$", true);
          
   return uDiffComputation; 
}


/*
 * double v_diff(double r, double theta, double phi, int n, NaturalCubicSpline_1D & Windv_Spline){
 *     double r_eval = min(r, r_max);  r_eval = max(r_eval, r_min);    // Check that r_min <= r_eval <= r_max
 *     double v_diff_return = 0.0;
 *     double eval_spline_df_return = Eval_Spline_df(r_eval,Windv_Spline);
 *     if(n==0) { v_diff_return =  eval_spline_df_return; } 
 */
Computation* v_diff_Computation(){
    Computation* vDiffComputation = new Computation();
    vDiffComputation->addParameter("$r$","double");
    vDiffComputation->addParameter("$theta$","double");
    vDiffComputation->addParameter("$phi$","double");
    vDiffComputation->addParameter("$n$","int");
    vDiffComputation->addParameter("$Windv_Spline$", "NaturalCubicSpline_1D &");
    
     vDiffComputation->addDataSpace("$r_eval$");
    //Creating statement0
    //double r_eval = min(r, r_max);  r_eval = max(r_eval, r_min);
    Stmt* s0 = new Stmt("double $r_eval$ = min($r$, r_max); $r_eval$ = max($r_eval$, r_min);",
        "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
        "{[0]->[0]}", //Execution schedule - scheduling statement to be first (scheduling function)
        {
            {"$r$","{[0]->[0]}"},
            {"$r_eval$","{[0]->[0]}"}
        }, //Data reads
        {{"$r_eval$","{[0]->[0]}"}} //Data writes
    );
    //Adding s0 to the computation
    vDiffComputation->addStmt(s0);

    vDiffComputation->addDataSpace("$v_diff_return$");

    //Creating statement1
    //double v_diff_return = 0.0;
    Stmt* s1 = new Stmt("double $v_diff_return$ = 0.0",
        "{[0]}",
        "{[0]->[1]}",
        {},
        {{"$v_diff_return$","{[0]->[0]}"}}
    );
    //Adding s1 to the computation
    vDiffComputation->addStmt(s1);

    //Add the args of the Eval_spline_df function (computation to be appended) as data spaces to dDiffComputation
    Computation* evalSplineDfComputation = Eval_Spline_df_Computation();
    //Args to the Eval_Spline_df_Computation
    vector<std::string> evalSplineDfCompArgs;
    evalSplineDfCompArgs.push_back("$r_eval$");
    evalSplineDfCompArgs.push_back("$Windv_Spline$");
    
    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult evalSplineDfCompRes = vDiffComputation->appendComputation(evalSplineDfComputation, "{[0]}", "{[0]->[2]}", evalSplineDfCompArgs);
    unsigned int newTuplePos = evalSplineDfCompRes.tuplePosition+1;
    
    vDiffComputation->addDataSpace("$eval_Spline_df_return$");

    //Creating statement2
    //double eval_Spline_df_return = Eval_Spline_df(r_eval,Windv_Spline);
    Stmt* s2 = new Stmt("double $eval_Spline_df_return$ = "+evalSplineDfCompRes.returnValues.back()+";",
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos)+"]}",
        {{evalSplineDfCompRes.returnValues.back(), "{[0]->[0]}"}},
        {{"$eval_Spline_df_return$", "{[0]->[0]}"}}
    );
    vDiffComputation->addStmt(s2);

    //Creating statement3
    //if(n==0) { v_diff_return =  eval_Spline_df_return; }
    Stmt* s3 = new Stmt("if($n$==0){$v_diff_return$ = $eval_Spline_df_return$;}",
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos+1)+"]}",
        {
           {"$n$", "{[0]->[0]}"},
           {"$eval_Spline_df_return$", "{[0]->[0]}"}
        },
        {
            {"$v_diff_return$", "{[0]->[0]}"}
        }
    );
    vDiffComputation->addStmt(s3);
  
    vDiffComputation->addReturnValue("$v_diff_return$", true);
   
    return vDiffComputation;
}



/*
double c_diff(double r, double theta, double phi, int n, NaturalCubicSpline_1D &Temp_Spline){
    double r_eval = min(r, r_max);  r_eval = max(r_eval, r_min);    // Check that r_min <= r_eval <= r_max
    double c_diff_return = 0.0;
    double c_return = c(r,theta,phi, Temp_Spline);
    double eval_spline_df_return = Eval_Spline_df(r_eval,Temp_Spline);
    
    if(n==0){c_diff_return = gamR / (2.0 * c_return) * eval_spline_df_return;}
}
*/
Computation* c_diff_Computation(){
    Computation* cDiffComputation = new Computation();
    cDiffComputation->addParameter("$r$","double");
    cDiffComputation->addParameter("$theta$","double");
    cDiffComputation->addParameter("$phi$","double");
    cDiffComputation->addParameter("$n$","int");
    cDiffComputation->addParameter("$Temp_Spline$","NaturalCubicSpline_1D &");

    cDiffComputation->addDataSpace("$r_eval$");
    //Creating statement1
    //double r_eval = min(r, r_max);  r_eval = max(r_eval, r_min);
    Stmt* s0 = new Stmt("double $r_eval$ = min($r$, r_max); $r_eval$ = max($r_eval$, r_min);",
        "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
        "{[0]->[0]}", //Execution schedule - scheduling statement to be first (scheduling function)
        {{"$r$","{[0]->[0]}"},{"$r_eval$","{[0]->[0]}"}}, //Data reads
        {{"$r_eval$","{[0]->[0]}"}} //Data writes
        );
    cout << "Source statement : " << s0->getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s0->getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s0->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

    //Adding s0 to the computation
    cDiffComputation->addStmt(s0);

    cDiffComputation->addDataSpace("$c_diff_return$");
    //double c_diff_return = 0.0;
    Stmt* s1 = new Stmt("double $c_diff_return$ = 0.0",
        "{[0]}",
        "{[0]->[1]}",
        {},
        {{"$c_diff_return$","{[0]->[0]}"}}
        );

    cDiffComputation->addStmt(s1);

    //Add the args of the c function (computation to be appended) as data spaces to cDiffComputation
    Computation* cComputation = c_Computation();

    //Args to the c_Computation
    vector<std::string> cCompArgs;
    cCompArgs.push_back("$r$");
    cCompArgs.push_back("$theta$");
    cCompArgs.push_back("$phi$");
    cCompArgs.push_back("$Temp_Spline$");

    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult cCompRes = cDiffComputation->appendComputation(cComputation, "{[0]}", "{[0]->[2]}", cCompArgs);
    unsigned int newTuplePos = cCompRes.tuplePosition+1;

    cDiffComputation->addDataSpace("$c_return$");
    
    //double c_return = c(r,theta,phi, Temp_Spline);
    Stmt* s2 = new Stmt("double $c_return$ = "+cCompRes.returnValues.back()+";", 
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos)+"]}",
        {{cCompRes.returnValues.back(), "{[0]->[0]}"}},
        {{"$c_return$", "{[0]->[0]}"}}
        );
    cout << "Source statement : " << s2->getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s2->getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s2->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

    cDiffComputation->addStmt(s2);

    Computation* evalSplineDfComputation = Eval_Spline_df_Computation();

    //Args to the c_Computation
    vector<std::string> evalSplineDfCompArgs;
    evalSplineDfCompArgs.push_back("$r_eval$");
    evalSplineDfCompArgs.push_back("$Temp_Spline$");

    AppendComputationResult evalSplineDfCompRes = cDiffComputation->appendComputation(evalSplineDfComputation, "{[0]}", "{[0]->["+std::to_string(newTuplePos+1)+"]}", evalSplineDfCompArgs);
    newTuplePos = evalSplineDfCompRes.tuplePosition+1;

    cDiffComputation->addDataSpace("$eval_spline_df_return$");

    //double eval_spline_df_return = Eval_Spline_df(r_eval,Temp_Spline);
    Stmt* s3 = new Stmt("double $eval_spline_df_return$ = "+evalSplineDfCompRes.returnValues.back()+";",
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos)+"]}",
        {{evalSplineDfCompRes.returnValues.back(), "{[0]->[0]}"}},
        {{"$eval_spline_df_return$", "{[0]->[0]}"}}
        );

    cDiffComputation->addStmt(s3);

    //if(n==0){c_diff_return = gamR / (2.0 * c_return) * eval_spline_df_return;}
    Stmt* s4 = new Stmt("if($n$==0){$c_diff_return$ = gamR / (2.0 * $c_return$) * $eval_spline_df_return$;}",
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos+1)+"]}",
        {
            {"$n$", "{[0]->[0]}"},
            {"$c_return$", "{[0]->[0]}"},
            {"$eval_spline_df_return$", "{[0]->[0]}"}
        },
        {
            {"$c_diff_return$", "{[0]->[0]}"}
        }
        );

    cDiffComputation->addStmt(s4);

    cDiffComputation->addReturnValue("$c_diff_return$", true);

    return cDiffComputation;
}


/*
  double u(double r, double theta, double phi, NaturalCubicSpline_1D &Windu_Spline){
      double r_eval = min(r, r_max);  r_eval = max(r_eval, r_min);    // Check that r_min <= r_eval <= r_max
      
      return Eval_Spline_f(r_eval, Windu_Spline);
  }
*/
Computation* u_Computation(){
  
    Computation* uComputation = new Computation();
    uComputation->addParameter("$r$","double");
    uComputation->addParameter("$theta$","double");
    uComputation->addParameter("$phi$","double");
    uComputation->addParameter("$Windu_Spline$","NaturalCubicSpline_1D &");
  
    
    uComputation->addDataSpace("$r_eval$");
    //Creating statement1
    //double r_eval = min(r, r_max);  r_eval = max(r_eval, r_min);
    Stmt* s1 = new Stmt("double $r_eval$ = min($r$, r_max); $r_eval$ = max($r_eval$, r_min)",
        "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
        "{[0]->[0]}", //Execution schedule - scheduling statement to be first (scheduling function)
        {{"$r$","{[0]->[0]}"},{"$r_eval$","{[0]->[0]}"}}, //Data reads
        {{"$r_eval$","{[0]->[0]}"}} //Data writes
        );
    cout << "Source statement : " << s1->getStmtSourceCode() << "\n\t"
      <<"- Iteration Space : "<< s1->getIterationSpace()->prettyPrintString() << "\n\t"
      << "- Execution Schedule : "<< s1->getExecutionSchedule()->prettyPrintString() << "\n\t" ;
  
    //Adding statement1
    uComputation->addStmt(s1);
  
    //Args to the Eval_Spline_f_Computation
    vector<std::string> eSpFArgs;
    eSpFArgs.push_back("$r_eval$");
    eSpFArgs.push_back("$Windu_Spline$");
  
    //Call Eval_Spline_f_Computation() return values is stored in result
    Computation* EvalSplineFComputation = Eval_Spline_f_Computation();
    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult eSpFCompRes = uComputation->appendComputation(EvalSplineFComputation, "{[0]}","{[0]->[1]}",eSpFArgs);
  
    //Add return value to the current computation
    uComputation->addReturnValue(eSpFCompRes.returnValues.back(), true);
  
    return uComputation;
}


/*
  double v(double r, double theta, double phi, NaturalCubicSpline_1D &Windv_Spline){
      double r_eval = min(r, r_max);  r_eval = max(r_eval, r_min);    // Check that r_min <= r_eval <= r_max
      double v_return = Eval_Spline_f(r_eval, Windv_Spline);
      
      return v_return;
  }
*/
Computation* v_Computation(){
  
    Computation* vComputation = new Computation();
    vComputation->addParameter("$r$","double");
    vComputation->addParameter("$theta$","double");
    vComputation->addParameter("$phi$","double");
    vComputation->addParameter("$Windv_Spline$","NaturalCubicSpline_1D &");
  
    vComputation->addDataSpace("$r_eval$");
    //Creating statement1
    //double r_eval = min(r, r_max);  r_eval = max(r_eval, r_min);
    Stmt* s1 = new Stmt("double $r_eval$ = min($r$, r_max); $r_eval$ = max($r_eval$, r_min)",
        "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
        "{[0]->[0]}", //Execution schedule - scheduling statement to be first (scheduling function)
        {{"$r$","{[0]->[0]}"},{"$r_eval$","{[0]->[0]}"}}, //Data reads
        {{"$r_eval$","{[0]->[0]}"}} //Data writes
        );
    cout << "Source statement : " << s1->getStmtSourceCode() << "\n\t"
      <<"- Iteration Space : "<< s1->getIterationSpace()->prettyPrintString() << "\n\t"
      << "- Execution Schedule : "<< s1->getExecutionSchedule()->prettyPrintString() << "\n\t" ;
  
    //Adding statement1
    vComputation->addStmt(s1);
  
    //Args to the Eval_Spline_f_Computation
    vector<std::string> eSpFArgs;
    eSpFArgs.push_back("$r_eval$");
    eSpFArgs.push_back("$Windv_Spline$");
  
    //Call Eval_Spline_f_Computation() returen values is stored in result
    Computation* EvalSplineFComputation = Eval_Spline_f_Computation();
    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult eSpFCompRes = vComputation->appendComputation(EvalSplineFComputation, "{[0]}","{[0]->[1]}",eSpFArgs);

    unsigned int newTuplePos = eSpFCompRes.tuplePosition+1;

    vComputation->addDataSpace("$v_return$");

    //double v_return = Eval_Spline_f(r_eval, Windv_Spline);
    Stmt* s2 = new Stmt("double $v_return$ ="+eSpFCompRes.returnValues.back()+";",
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos)+"]}",
        {{eSpFCompRes.returnValues.back(), "{[0]->[0]}"}},
        {{"$v_return$", "{[0]->[0]}"}}
        );

    vComputation->addStmt(s2);
  
    //Add return value to the current computation
    vComputation->addReturnValue("$v_return$", true);
  
    return vComputation;
}


/*
  double c(double r, double theta, double phi, NaturalCubicSpline_1D &Temp_Spline){
    //cout << "c: r_max = " << r_max << ", r_min = " << r_min << ", gamR = " << gamR << endl;
    double r_eval = min(r, r_max);  r_eval = max(r_eval, r_min);    // Check that r_min <= r_eval <= r_max
    //cout << "c: r_eval = " << r_eval << endl;
    double c_result = sqrt(gamR * Eval_Spline_f(r_eval,Temp_Spline));
    //cout << "c: result = " << c_result << endl;
    return c_result;
  }
*/
Computation* c_Computation(){
  
    Computation* cComputation = new Computation();
    cComputation->addParameter("$r$","double");
    cComputation->addParameter("$theta$","double");
    cComputation->addParameter("$phi$","double");
    cComputation->addParameter("$Temp_Spline$","NaturalCubicSpline_1D &");
  
    //Add data space explicitly to the computation
    cComputation->addDataSpace("$r_eval$");
    //Creating statement1
    //double r_eval = min(r, r_max);  r_eval = max(r_eval, r_min);
    Stmt* s1 = new Stmt("double $r_eval$ = min($r$, r_max);",
        "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
        "{[0]->[0]}", //Execution schedule - scheduling statement to be first (scheduling function)
        {{"$r$","{[0]->[0]}"}}, //Data reads
        {{"$r_eval$","{[0]->[0]}"}} //Data writes
        );
    cout << "Source statement : " << s1->getStmtSourceCode() << "\n\t"
      <<"- Iteration Space : "<< s1->getIterationSpace()->prettyPrintString() << "\n\t"
      << "- Execution Schedule : "<< s1->getExecutionSchedule()->prettyPrintString() << "\n\t" ;
  
    //Adding statement1
    cComputation->addStmt(s1);
  
    //Creating statement2
    //r_eval = max(r_eval, r_min);
    Stmt* s2 = new Stmt("$r_eval$ = max($r_eval$, r_min);",
        "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
        "{[0]->[1]}", //Execution schedule - scheduling statement to be first (scheduling function)
        {{"$r_eval$","{[0]->[0]}"}}, //Data reads
        {{"$r_eval$","{[0]->[0]}"}} //Data writes
        );
    cout << "Source statement : " << s2->getStmtSourceCode() << "\n\t"
      <<"- Iteration Space : "<< s2->getIterationSpace()->prettyPrintString() << "\n\t"
      << "- Execution Schedule : "<< s2->getExecutionSchedule()->prettyPrintString() << "\n\t" ;
  
    //Adding statement2
    cComputation->addStmt(s2);
  
    //Args to the Eval_Spline_f_Computation
    vector<std::string> eSpFArgs;
    eSpFArgs.push_back("$r_eval$");
    eSpFArgs.push_back("$Temp_Spline$");
  
    //Call Eval_Spline_f_Computation() 
    //Call this computation only once and reuse when needed
    Computation* EvalSplineFComputation = Eval_Spline_f_Computation();
  
    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult eSpFCompRes = cComputation->appendComputation(EvalSplineFComputation, "{[0]}","{[0]->[2]}",eSpFArgs);
 
    unsigned int newTuplePos = eSpFCompRes.tuplePosition+1;
    cComputation->addDataSpace("$c_result$");
    //Creating statement3
    //double c_result = sqrt(gamR * Eval_Spline_f(r_eval,Temp_Spline));
    Stmt* s3 = new Stmt("double $c_result$ = sqrt(gamR * "+eSpFCompRes.returnValues.back()+");",
        "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
        "{[0]->["+std::to_string(newTuplePos)+"]}", //Execution schedule - scheduling statement to be first (scheduling function)
        {{eSpFCompRes.returnValues.back(), "{[0]->[0]}"}}, //Data reads
        {{"$c_result$", "{[0]->[0]}"}} //Data writes
        );
    cout << "Source statement : " << s3->getStmtSourceCode() << "\n\t"
      <<"- Iteration Space : "<< s3->getIterationSpace()->prettyPrintString() << "\n\t"
      << "- Execution Schedule : "<< s3->getExecutionSchedule()->prettyPrintString() << "\n\t" ;
  
    //Adding statement3b
    cComputation->addStmt(s3);
    cComputation->addReturnValue("$c_result$", true);
  
    
    return cComputation;
}


/*
  double Eval_Spline_f(double x, struct NaturalCubicSpline_1D & Spline){
      //cout << "Called Eval_Spline_f" << endl;
      //cout << "Eval: x = " << x << endl;
      int k = Find_Segment(x, Spline.x_vals, Spline.length, Spline.accel);
   
      //cout << "Eval: k = " << k << " spline length = " << Spline.length << endl;   
      //cout << "Passed Find_Segment()" << endl;
      
      if(k < Spline.length){
          double X = (x - Spline.x_vals[k])/(Spline.x_vals[k+1] - Spline.x_vals[k]);
          double A = Spline.slopes[k] * (Spline.x_vals[k+1] - Spline.x_vals[k]) - (Spline.f_vals[k+1] - Spline.f_vals[k]);
          double B = -Spline.slopes[k+1] * (Spline.x_vals[k+1] - Spline.x_vals[k]) + (Spline.f_vals[k+1] - Spline.f_vals[k]);
          //cout << "Eval: X = " << X << " A = " << A << " B = " << B << endl;
          
          //cout << "Ready to return" << endl;
          double result = (1.0 - X) * Spline.f_vals[k] + X * Spline.f_vals[k+1] + X * (1.0 - X) * (A * (1.0 - X ) + B * X);
          //cout << "Eval: result = " << result << endl;
          return result;
      } else { return 0.0;}
  }
*/
Computation* Eval_Spline_f_Computation(){

    Computation* evalSplineFComputation = new Computation();
    evalSplineFComputation->addParameter("$x$", "double");
    evalSplineFComputation->addParameter("$Spline$", "struct NaturalCubicSpline_1D &");
    
  
    Computation* FindSegmentComputation= Find_Segment_Computation();
  
    //Add the args of the FindSegment function (computation to be appended) as data spaces to evalSplineFComputation
    // evalSplineFComputation->addDataSpace("$x$");
    evalSplineFComputation->addDataSpace("$Spline.x_vals$");
    evalSplineFComputation->addDataSpace("$Spline.length$");
    evalSplineFComputation->addDataSpace("$Spline.accel$");
  
    //Args to the Find_Segment_Computation
    vector<std::string> findSegArgs;
    findSegArgs.push_back("$x$");
    findSegArgs.push_back("$Spline.x_vals$");
    findSegArgs.push_back("$Spline.length$");
    findSegArgs.push_back("$Spline.accel$");
  
    // Return values are stored in a struct of the AppendComputationResult data structure
    AppendComputationResult fSegCompRes = evalSplineFComputation->appendComputation(FindSegmentComputation, "{[0]}", "{[0]->[0]}", findSegArgs);
  
    unsigned int newTuplePos = fSegCompRes.tuplePosition+1;
    evalSplineFComputation->addDataSpace("$k$");
    //Creating s1
    Stmt* s1a = new Stmt("int $k$ = "+fSegCompRes.returnValues.back()+";",
        "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
        "{[0]->["+std::to_string(newTuplePos)+"]}", //Execution schedule - scheduling statement to be first (scheduling function)
        {{fSegCompRes.returnValues.back(), "{[0]->[0]}"}}, 
        {{"$k$", "{[0]->[0]}"}}  
        );  
    cout << "Source statement : " << s1a->getStmtSourceCode() << "\n\t"
      <<"- Iteration Space : "<< s1a->getIterationSpace()->prettyPrintString() << "\n\t"
      << "- Execution Schedule : "<< s1a->getExecutionSchedule()->prettyPrintString() << "\n\t" ;
  
    //Adding s1
    evalSplineFComputation->addStmt(s1a);
  
  
    evalSplineFComputation->addDataSpace("$result$");
    //Creating s2
    Stmt* s2 = new Stmt("double $result$ = 0.0;",
        "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
        "{[0]->["+std::to_string(newTuplePos+1)+"]}", //Execution schedule - scheduling statement to be first (scheduling function)
        {}, 
        {{"$result$", "{[0]->[0]}"}}  
        );  
    cout << "Source statement : " << s2->getStmtSourceCode() << "\n\t"
      <<"- Iteration Space : "<< s2->getIterationSpace()->prettyPrintString() << "\n\t"
      << "- Execution Schedule : "<< s2->getExecutionSchedule()->prettyPrintString() << "\n\t" ;
  
    //Adding s2
    evalSplineFComputation->addStmt(s2);  
  
    evalSplineFComputation->addDataSpace("$X$");
    evalSplineFComputation->addDataSpace("$A$");
    evalSplineFComputation->addDataSpace("$B$");
    evalSplineFComputation->addDataSpace("$Spline.slopes$");
    evalSplineFComputation->addDataSpace("$Spline.f_vals$");
  
    //Creating s3 --> the entire if block, slightly modified
    Stmt* s3 = new Stmt("if($k$ < $Spline.length$) { double $X$ = ($x$ - $Spline.x_vals$[$k$])/($Spline.x_vals$[$k$+1] - $Spline.x_vals$[$k$]); double $A$ = $Spline.slopes$[$k$] * ($Spline.x_vals$[$k$+1] - $Spline.x_vals$[$k$]) - ($Spline.f_vals$[$k$+1] - $Spline.f_vals$[$k$]); double $B$ = -$Spline.slopes$[$k$+1] * ($Spline.x_vals$[$k$+1] - $Spline.x_vals$[$k$]) + ($Spline.f_vals$[$k$+1] - $Spline.f_vals$[$k$]); $result$ = (1.0 - $X$) * $Spline.f_vals$[$k$] + $X$ * $Spline.f_vals$[$k$+1] + $X$ * (1.0 - $X$) * ($A$ * (1.0 - $X$ ) + $B$ * $X$);}",
        "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
        "{[0]->["+std::to_string(newTuplePos+2)+"]}", //Execution schedule - scheduling statement to be first (scheduling function)
        {
            {"$k$","{[0]->[0]}"},
            {"$Spline.length$", "{[0]->[0]}"},
            {"$x$","{[0]->[0]}"},
            {"$Spline.x_vals$","{[0]->[k1]: k1 = $k$}"},
            {"$Spline.x_vals$","{[0]->[kp1]: kp1 = $k$+1}"}, 
            {"$Spline.slopes$","{[0]->[x1]: x1 = $k$}"}, 
            {"$Spline.f_vals$","{[0]->[k1]: k1 = $k$}"},
            {"$Spline.f_vals$","{[0]->[kp1]: kp1 = $k$+1}"}, 
            {"$Spline.slopes$","{[0]->[kp1]: kp1 = $k$+1}"}, 
            {"$X$","{[0]->[0]}"}, 
            {"$A$","{[0]->[0]}"}, 
            {"$B$","{[0]->[0]}"}
        },//Data reads
        {
            {"$X$", "{[0]->[0]}"},
            {"$A$", "{[0]->[0]}"},
            {"$B$", "{[0]->[0]}"},
            {"$result$", "{[0]->[0]}"},
        } //Data writes
        );
    cout << "Source statement : " << s3->getStmtSourceCode() << "\n\t"
      <<"- Iteration Space : "<< s3->getIterationSpace()->prettyPrintString() << "\n\t"
      << "- Execution Schedule : "<< s3->getExecutionSchedule()->prettyPrintString() << "\n\t" ;
  
    //Adding s3
    evalSplineFComputation->addStmt(s3);
    evalSplineFComputation->addReturnValue("$result$", true);
  
  
    return evalSplineFComputation;
}


/*
   double Eval_Spline_df(double x, struct NaturalCubicSpline_1D & Spline){
       int k = Find_Segment(x, Spline.x_vals, Spline.length, Spline.accel);
       double eval_spline_df_return = 0.0;
        
       if(k < Spline.length){
           double X = (x - Spline.x_vals[k])/(Spline.x_vals[k+1] - Spline.x_vals[k]);
           double A = Spline.slopes[k] * (Spline.x_vals[k+1] - Spline.x_vals[k]) - (Spline.f_vals[k+1] - Spline.f_vals[k]);
           double B = -Spline.slopes[k+1] * (Spline.x_vals[k+1] - Spline.x_vals[k]) + (Spline.f_vals[k+1] - Spline.f_vals[k]);
   
           eval_spline_df_return = (Spline.f_vals[k+1] - Spline.f_vals[k])/(Spline.x_vals[k+1] - Spline.x_vals[k])
           + (1.0 - 2.0 * X) * (A * (1.0 - X) + B * X)/(Spline.x_vals[k+1] - Spline.x_vals[k])
           + X * (1.0 - X) * (B - A)/(Spline.x_vals[k+1] - Spline.x_vals[k]);
        } 
   }
 */
Computation* Eval_Spline_df_Computation(){
    Computation* evalSplineDfComputation = new Computation();
    evalSplineDfComputation->addParameter("$x$", "double");
    evalSplineDfComputation->addParameter("$Spline$", "NaturalCubicSpline_1D &");
   
    //Add the args of the FindSegment function (computation to be appended) as data spaces to EvalSplinedFComputation
    evalSplineDfComputation->addDataSpace("$Spline.x_vals$");
    evalSplineDfComputation->addDataSpace("$Spline.length$");
    evalSplineDfComputation->addDataSpace("$Spline.accel$");
  
    Computation* findSegmentComputation= Find_Segment_Computation();
    vector<std::string> findSegArgs;
    findSegArgs.push_back("$x$");
    findSegArgs.push_back("$Spline.x_vals$");
    findSegArgs.push_back("$Spline.length$");
    findSegArgs.push_back("$Spline.accel$");
    //Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult fSegCompRes = evalSplineDfComputation->appendComputation(findSegmentComputation, "{[0]}", "{[0]->[0]}", findSegArgs);
  
    unsigned int newTuplePos = fSegCompRes.tuplePosition+1;

    evalSplineDfComputation->addDataSpace("$k$");
    //Creating s0
    //int k = Find_Segment(x, Spline.x_vals, Spline.length, Spline.accel);
    Stmt* s0 = new Stmt("int $k$ = "+fSegCompRes.returnValues.back()+";",
      "{[0]}",  //Iteration schedule
      "{[0]->["+std::to_string(newTuplePos)+"]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {{fSegCompRes.returnValues.back(), "{[0]->[0]}"}},//Reads 
      {{"$k$", "{[0]->[0]}"}} //Writes
      );  
    cout << "Source statement : " << s0->getStmtSourceCode() << "\n\t"
      <<"- Iteration Space : "<< s0->getIterationSpace()->prettyPrintString() << "\n\t"
      << "- Execution Schedule : "<< s0->getExecutionSchedule()->prettyPrintString() << "\n\t" ;
  
    //Adding statement S0 
    evalSplineDfComputation->addStmt(s0);  
  
    evalSplineDfComputation->addDataSpace("$eval_spline_df_return$");
    //Creating s1
    Stmt* s1 = new Stmt("double $eval_spline_df_return$ = 0.0;",
        "{[0]}",  
        "{[0]->["+std::to_string(newTuplePos+1)+"]}",
        {}, 
        {{"$eval_spline_df_return$", "{[0]->[0]}"}}  
    );  
    cout << "Source statement : " << s1->getStmtSourceCode() << "\n\t"
      <<"- Iteration Space : "<< s1->getIterationSpace()->prettyPrintString() << "\n\t"
      << "- Execution Schedule : "<< s1->getExecutionSchedule()->prettyPrintString() << "\n\t" ;
    
    //Adding statement s1      
    evalSplineDfComputation->addStmt(s1);                                     
   
    evalSplineDfComputation->addDataSpace("$X$");
    evalSplineDfComputation->addDataSpace("$A$");
    evalSplineDfComputation->addDataSpace("$B$");
    evalSplineDfComputation->addDataSpace("$Spline.slopes$");
    evalSplineDfComputation->addDataSpace("$Spline.f_vals$");
   
    Stmt* s2 = new Stmt("if($k$ < $Spline.length$){double $X$ = ($x$ - $Spline.x_vals$[$k$])/($Spline.x_vals$[$k$+1] - $Spline.x_vals$[$k$]); double $A$ = $Spline.slopes$[$k$] * ($Spline.x_vals$[$k$+1] -$Spline.x_vals$[$k$]) - ($Spline.f_vals$[$k$+1] - $Spline.f_vals$[$k$]); double $B$ = -$Spline.slopes$[$k$+1] * ($Spline.x_vals$[$k$+1] - $Spline.x_vals$[$k$]) + ($Spline.f_vals$[$k$+1] - $Spline.f_vals$[$k$]); $eval_spline_df_return$ = ($Spline.f_vals$[$k$+1] - $Spline.f_vals$[$k$])/($Spline.x_vals$[$k$+1] - $Spline.x_vals$[$k$]) + (1.0 - 2.0 * $X$) * ($A$ * (1.0 - $X$) + $B$ * $X$)/($Spline.x_vals$[$k$+1] - $Spline.x_vals$[$k$])+ $X$ * (1.0 - $X$) * ($B$ - $A$)/($Spline.x_vals$[$k$+1] - $Spline.x_vals$[$k$]);}",
        "{[0]}", //Iteration Schedule
        "{[0]->["+std::to_string(newTuplePos+2)+"]}", //Execution schedule
        {
            {"$k$","{[0]->[0]}"},
            {"$Spline.length$", "{[0]->[0]}"},
            {"$x$","{[0]->[0]}"},
            {"$Spline.x_vals$","{[0]->[k1]: k1 = $k$}"},
            {"$Spline.x_vals$","{[0]->[kp1]: kp1 = $k$+1}"}, 
            {"$Spline.slopes$","{[0]->[k1]: k1 = $k$}"},
            {"$Spline.f_vals$","{[0]->[k1]: k1 = $k$}"},
            {"$Spline.slopes$","{[0]->[kp1]: kp1 = $k$+1}"}, 
            {"$Spline.f_vals$","{[0]->[kp1]: kp1 = $k$+1}"}, 
            {"$X$","{[0]->[0]}"},
            {"$A$","{[0]->[0]}"},
            {"$B$","{[0]->[0]}"}
        },//Data reads
        {
            {"$X$", "{[0]->[0]}"},
            {"$A$", "{[0]->[0]}"},
            {"$B$", "{[0]->[0]}"},
            {"$eval_spline_df_return$", "{[0]->[0]}"
        }} //Data writes
    );
    cout << "Source statement : " << s2->getStmtSourceCode() << "\n\t"
      <<"- Iteration Space : "<< s2->getIterationSpace()->prettyPrintString() << "\n\t"
      << "- Execution Schedule : "<< s2->getExecutionSchedule()->prettyPrintString() << "\n\t" ;
    //Adding statement S2
    evalSplineDfComputation->addStmt(s2);
    evalSplineDfComputation->addReturnValue("$eval_spline_df_return$",true);
  
    return evalSplineDfComputation;
}


/*
double Eval_Spline_ddf(double x, struct NaturalCubicSpline_1D & Spline){
    int k = Find_Segment(x, Spline.x_vals, Spline.length, Spline.accel);
    double result = 0.0;    
    if(k < Spline.length){
        double X = (x - Spline.x_vals[k])/(Spline.x_vals[k+1] - Spline.x_vals[k]);
        double A = Spline.slopes[k] * (Spline.x_vals[k+1] - Spline.x_vals[k]) - (Spline.f_vals[k+1] - Spline.f_vals[k]);
        double B = -Spline.slopes[k+1] * (Spline.x_vals[k+1] - Spline.x_vals[k]) + (Spline.f_vals[k+1] - Spline.f_vals[k]);
        
        result =  2.0 * (B - 2.0 * A + (A - B) * 3.0 * X)/pow(Spline.x_vals[k+1] - Spline.x_vals[k],2);
    } 
}
*/
Computation* Eval_Spline_ddf_Computation(){
    Computation* evalSplineDdfComputation = new Computation();
    evalSplineDdfComputation->addParameter("$x$", "double");
    evalSplineDdfComputation->addParameter("$Spline$", "struct NaturalCubicSpline_1D &");
    
    evalSplineDdfComputation->addDataSpace("$Spline.x_vals$");
    evalSplineDdfComputation->addDataSpace("$Spline.length$");
    evalSplineDdfComputation->addDataSpace("$Spline.accel$");
  
    Computation* findSegmentComputation= Find_Segment_Computation();
    //Args to the Find_Segment_Computation
    vector<std::string> findSegArgs;
    findSegArgs.push_back("$x$");
    findSegArgs.push_back("$Spline.x_vals$");
    findSegArgs.push_back("$Spline.length$");
    findSegArgs.push_back("$Spline.accel$"); 
    
    //Return values are stored in a struct of the AppendComputationResult data structure
    AppendComputationResult fSegCompRes = evalSplineDdfComputation->appendComputation(findSegmentComputation, "{[0]}", "{[0]->[0]}", findSegArgs);
   
    unsigned int newTuplePos = fSegCompRes.tuplePosition + 1;
    evalSplineDdfComputation->addDataSpace("$k$");
    //Creating s1
    //int k = Find_Segment(x, Spline.x_vals, Spline.length, Spline.accel);
    Stmt* s1 = new Stmt("int $k$ = "+fSegCompRes.returnValues.back()+";",
        "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
        "{[0]->["+std::to_string(newTuplePos)+"]}", //Execution schedule - scheduling statement to be first (scheduling function)
        {{fSegCompRes.returnValues.back(), "{[0]->[0]}"}}, 
        {{"$k$", "{[0]->[0]}"}}  
    );   
    //Adding s1
    evalSplineDdfComputation->addStmt(s1);
  
    evalSplineDdfComputation->addDataSpace("$eval_Spline_ddf_return$");
    //Creating s2
    Stmt* s2 = new Stmt(
        "double $eval_Spline_ddf_return$ = 0.0;",
        "{[0]}",  //Iteration schedule
        "{[0]->["+std::to_string(newTuplePos+1)+"]}", //Execution schedule
        {}, 
        {{"$eval_Spline_ddf_return$", "{[0]->[0]}"}}  
    );
    //Adding s2
    evalSplineDdfComputation->addStmt(s2);
    
  
    evalSplineDdfComputation->addDataSpace("$X$");
    evalSplineDdfComputation->addDataSpace("$A$");
    evalSplineDdfComputation->addDataSpace("$B$");
    evalSplineDdfComputation->addDataSpace("$Spline.slopes$");
    evalSplineDdfComputation->addDataSpace("$Spline.f_vals$");
    //Creating s3 --> the entire if block, slightly modified
    Stmt* s3 = new Stmt(
    "if($k$ < $Spline.length$){ double $X$ = ($x$ - $Spline.x_vals$[$k$])/($Spline.x_vals$[$k$+1] - $Spline.x_vals$[$k$]);double $A$ = $Spline.slopes$[$k$] * ($Spline.x_vals$[$k$+1] - $Spline.x_vals$[$k$]) - ($Spline.f_vals$[$k$+1] - $Spline.f_vals$[$k$]);double $B$ = -$Spline.slopes$[$k$+1] * ($Spline.x_vals$[$k$+1] - $Spline.x_vals$[$k$]) + ($Spline.f_vals$[$k$+1] - $Spline.f_vals$[$k$]); $eval_Spline_ddf_return$ = 2.0 * ($B$ - 2.0 * $A$ + ($A$ - $B$) * 3.0 * $X$) / pow($Spline.x_vals$[$k$+1] - $Spline.x_vals$[$k$, 2);}", 
        "{[0]}",  //Iteration schedule
        "{[0]->["+std::to_string(newTuplePos+2)+"]}", //Execution schedule
        {
            {"$k$","{[0]->[0]}"},
            {"$x$","{[0]->[0]}"}, 
            {"$Spline.length$", "{[0]->[0]}"},
            {"$Spline.x_vals$","{[0]->[k1]: k1 = $k$}"}, 
            {"$Spline.x_vals$","{[0]->[kp1]: kp1 = $k$+1}"}, 
            {"$Spline.slopes$","{[0]->[x1]: x1 = $k$}"},
            {"$Spline.f_vals$","{[0]->[kp1]: kp1 = $k$+1}"}, 
            {"$Spline.f_vals$","{[0]->[k1]: k1 = $k$}"}, 
            {"$Spline.slopes$","{[0]->[kp1]: kp1 = $k$+1}"}, 
            {"$B$","{[0]->[0]}"},
            {"$A$","{[0]->[0]}"},
            {"$X$","{[0]->[0]}"}
        },//Data reads
        {
            {"$X$", "{[0]->[0]}"},
            {"$A$", "{[0]->[0]}"},
            {"$B$", "{[0]->[0]}"},
            {"$eval_Spline_ddf_return$", "{[0]->[0]}"}
        } //Data writes
    );
    //Adding s3
    evalSplineDdfComputation->addStmt(s3);
    
    evalSplineDdfComputation->addReturnValue("$eval_Spline_ddf_return$", true);
    return evalSplineDdfComputation;
}


/*
  int Find_Segment(double x, double* x_vals, int length, int & prev){
      for(int i = 0; i < length; i++){
          if(x >= x_vals[i] && x <= x_vals[i+1]){
              prev = i;
          }
      }
      return prev;        
  }
*/
Computation* Find_Segment_Computation(){

    Computation* findSegmentComputation = new Computation();
    findSegmentComputation->addParameter("$x$", "double");
    findSegmentComputation->addParameter("$x_vals$", "double*");
    findSegmentComputation->addParameter("$length$", "int");
    findSegmentComputation->addParameter("$prev$", "int&");
  
    //Creating s0
    //if(x >= x_vals[i] && x <= x_vals[i+1]) prev = i;
    Stmt*  s0 = new Stmt("if($x$ >= $x_vals$[i] && $x$ <= $x_vals$[i+1]) $prev$ = i;",
        "{[i]: i>=0 && i<$length$}",  //Iteration schedule 
        "{[i]->[0, i, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
        {{"$x$","{[0]->[0]}"}, {"$x_vals$","{[i]->[i]}"}, {"$x_vals$","{[i]->[ip1]: ip1 = i+1}"}}, //Reads
        {{"$prev$","{[0]->[0]}"}}   //writes
        );  
    cout << "Source statement : " << s0->getStmtSourceCode() << "\n\t"
      <<"- Iteration Space : "<< s0->getIterationSpace()->prettyPrintString() << "\n\t"
      << "- Execution Schedule : "<< s0->getExecutionSchedule()->prettyPrintString() << "\n\t" ;
  
    //Adding s0
    findSegmentComputation->addStmt(s0);
    findSegmentComputation->addReturnValue("$prev$", true);
  
    return findSegmentComputation;
}



