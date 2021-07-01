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

Computation* c_ddiff_Computation();
Computation* v_ddiff_Computation();
Computation* u_ddiff_Computation();

/**
 * Main function
 */
int main(int argc, char **argv){

    Computation* updateSources = new Computation(); 

    updateSources->addDataSpace("$ray_length$");
    updateSources->addDataSpace("$current_values$");
    updateSources->addDataSpace("$spl$");
    updateSources->addDataSpace("$sources$");

    // double r = current_values[0], theta = current_values[1], phi = current_values[2];
    updateSources->addDataSpace("$r$");
    updateSources->addDataSpace("$theta$");
    updateSources->addDataSpace("$phi$");

    //Temp var assignment because IEGenLib can't handle periods
    // int spl_Temp_Spline_length = spl.Temp_Spline.length;
    // int spl_Temp_Spline_accel = spl.Temp_Spline.accel;
    // double* spl_Temp_Spline_x_vals = spl.Temp_Spline.x_vals;
    // double* spl_Temp_Spline_f_vals = spl.Temp_Spline.f_vals;
    // double* spl_Temp_Spline_slopes = spl.Temp_Spline.slopes;
    updateSources->addDataSpace("$spl.Temp_Spline.length$");
    updateSources->addDataSpace("$spl.Temp_Spline.accel$");
    updateSources->addDataSpace("$spl.Temp_Spline.x_vals$");
    updateSources->addDataSpace("$spl.Temp_Spline.f_vals$");
    updateSources->addDataSpace("$spl.Temp_Spline.slopes$");

    updateSources->addDataSpace("$spl_Temp_Spline_length$");
    updateSources->addDataSpace("$spl_Temp_Spline_accel$");
    updateSources->addDataSpace("$spl_Temp_Spline_x_vals$");
    updateSources->addDataSpace("$spl_Temp_Spline_f_vals$");
    updateSources->addDataSpace("$spl_Temp_Spline_slopes$");    
    Stmt* s0a = new Stmt("int $spl_Temp_Spline_length$ = $spl.Temp_Spline.length$;",
         "{[0]}",
         "{[0]->[0]}",
         {
            {"$spl.Temp_Spline.length$","{[0]->[0]}"}
         },
         {
            {"$spl_Temp_Spline_length$", "{[0]->[0]}"}
         }
         );
    updateSources->addStmt(s0a);

    Stmt* s0b = new Stmt("int $spl_Temp_Spline_accel$ = $spl.Temp_Spline.accel$;",
     "{[0]}",
     "{[0]->[1]}",
     {
        {"$spl.Temp_Spline.accel$","{[0]->[0]}"}
     },
     {
        {"$spl_Temp_Spline_accel$", "{[0]->[0]}"}
     }
     );
    updateSources->addStmt(s0b);

    Stmt* s0c = new Stmt("double* $spl_Temp_Spline_x_vals$ = $spl.Temp_Spline.x_vals$;",
     "{[0]}",
     "{[0]->[2]}",
     {
        {"$spl.Temp_Spline.x_vals$","{[0]->[0]}"}
     },
     {
        {"$spl_Temp_Spline_x_vals$", "{[0]->[0]}"}
     }
     );
    updateSources->addStmt(s0c);

    Stmt* s0d = new Stmt("double* $spl_Temp_Spline_f_vals$ = $spl.Temp_Spline.f_vals$;",
     "{[0]}",
     "{[0]->[3]}",
     {
        {"$spl.Temp_Spline.f_vals$","{[0]->[0]}"}
     },
     {
        {"$spl_Temp_Spline_f_vals$", "{[0]->[0]}"}
     }
     );
    updateSources->addStmt(s0d);

    Stmt* s0e = new Stmt("double* spl_Temp_Spline_slopes$ = $spl.Temp_Spline.slopes$;",
     "{[0]}",
     "{[0]->[4]}",
     {
        {"$spl.Temp_Spline.slopes$","{[0]->[0]}"}
     },
     {
        {"$spl_Temp_Spline_slopes$", "{[0]->[0]}"}
     }
     );
    updateSources->addStmt(s0e);
    
    Stmt* s01 = new Stmt("double $r$ = $current_values$[0], $theta$=$current_values$[1], $phi$=$current_values$[2];",
         "{[0]}",
         "{[0]->[5]}",
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

    //double nu[3] = {current_values[3], current_values[4], current_values[5]};
    updateSources->addDataSpace("$nu$");
    Stmt* s02 = new Stmt("double $nu$[3] = {$current_values$[3], $current_values$[4], $current_values$[5]};",
         "{[0]}",
         "{[0]->[6]}",
         {
            {"$current_values$","{[0]->[3]}"},
            {"$current_values$","{[0]->[4]}"},
            {"$current_values$","{[0]->[5]}"}
         },
         {{"$nu$", "{[0]->[3]}"}} //correct?
         );
    updateSources->addStmt(s02);

    //Args to the c_Computation
    vector<std::string> cCompArgs;
    cCompArgs.push_back("$r$");
    cCompArgs.push_back("$theta$");
    cCompArgs.push_back("$phi$");
    //cCompArgs.push_back("$spl.Temp_Spline$");
    cCompArgs.push_back("$spl_Temp_Spline_length$");
    cCompArgs.push_back("$spl_Temp_Spline_accel$");
    cCompArgs.push_back("$spl_Temp_Spline_x_vals$");
    cCompArgs.push_back("$spl_Temp_Spline_f_vals$");
    cCompArgs.push_back("$spl_Temp_Spline_slopes$");

    Computation* cComputation = c_Computation();

    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult cCompRes = updateSources->appendComputation(cComputation, "{[0]}", "{[0]->[7]}", cCompArgs);
    unsigned int newTuplePos = cCompRes.tuplePosition+1;
    
    //Creating s1
    //sources.c = c(r,theta,phi,spl.Temp_Spline);
    updateSources->addDataSpace("$sources.c$");
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

    //Creating s2
    //sources.w = w(r,theta,phi); The w function always return 0!
    updateSources->addDataSpace("$sources.w$");
    Stmt* s2 = new Stmt("$sources.w$ = 0;",
        "{[0]}",  //Iteration schedule
        "{[0]->["+std::to_string(newTuplePos+1)+"]}",  //Execution schedule
        {}, //Data reads
        {{"$sources.w$", "{[0]->[0]}"}} //Data writes
        );

    updateSources->addStmt(s2);

    //Add the args of the v function (computation to be appended) as data spaces to temp
    //r, theta and phi have already been added
    //updateSources->addDataSpace("$spl.Windv_Spline");
    
    //Temp var assignment because IEGenLib can't handle periods
    // int spl_Windv_Spline_length = spl.Windv_Spline.length;
    // int spl_Windv_Spline_accel = spl.Windv_Spline.accel;
    // double* spl_Windv_Spline_x_vals = spl.Windv_Spline.x_vals;
    // double* spl_Windv_Spline_f_vals = spl.Windv_Spline.f_vals;
    // double* spl_Windv_Spline_slopes = spl.Windv_Spline.slopes;
    updateSources->addDataSpace("$spl.Windv_Spline.length$");
    updateSources->addDataSpace("$spl.Windv_Spline.accel$");
    updateSources->addDataSpace("$spl.Windv_Spline.x_vals$");
    updateSources->addDataSpace("$spl.Windv_Spline.f_vals$");
    updateSources->addDataSpace("$spl.Windv_Spline.slopes$");

    updateSources->addDataSpace("$spl_Windv_Spline_length$");
    updateSources->addDataSpace("$spl_Windv_Spline_accel$");
    updateSources->addDataSpace("$spl_Windv_Spline_x_vals$");
    updateSources->addDataSpace("$spl_Windv_Spline_f_vals$");
    updateSources->addDataSpace("$spl_Windv_Spline_slopes$"); 
    Stmt* s3a = new Stmt("int $spl_Windv_Spline_length$ = $spl.Windv_Spline.length$; int $spl_Windv_Spline_accel$ = $spl.Windv_Spline.accel$; double* $spl_Windv_Spline_x_vals$ = $spl.Windv_Spline.x_vals$; double* $spl_Windv_Spline_f_vals$ = $spl.Windv_Spline.f_vals$; double* spl_Windv_Spline_slopes$ = $spl.Windv_Spline.slopes$;",
         "{[0]}",
         "{[0]->["+std::to_string(newTuplePos+2)+"]}",
         {
            {"$spl.Windv_Spline.length$","{[0]->[0]}"},
            {"$spl.Windv_Spline.accel$","{[0]->[0]}"},
            {"$spl.Windv_Spline.x_vals$","{[0]->[0]}"},
            {"$spl.Windv_Spline.f_vals$","{[0]->[0]}"},
            {"$spl.Windv_Spline.slopes$","{[0]->[0]}"}
         },
         {
            {"$spl_Windv_Spline_length$", "{[0]->[0]}"},
            {"$spl_Windv_Spline_accel$", "{[0]->[0]}"},
            {"$spl_Windv_Spline_x_vals$", "{[0]->[0]}"},
            {"$spl_Windv_Spline_f_vals$", "{[0]->[0]}"},
            {"$spl_Windv_Spline_slopes$", "{[0]->[0]}"}
         }
         );
    updateSources->addStmt(s3a);


    //Args to the v_Computation
    vector<std::string> vCompArgs;
    vCompArgs.push_back("$r$");
    vCompArgs.push_back("$theta$");
    vCompArgs.push_back("$phi$");
    //vCompArgs.push_back("$spl.Windv_Spline$");
    vCompArgs.push_back("$spl_Windv_Spline_length$");
    vCompArgs.push_back("$spl_Windv_Spline_accel$");
    vCompArgs.push_back("$spl_Windv_Spline_x_vals$");
    vCompArgs.push_back("$spl_Windv_Spline_f_vals$");
    vCompArgs.push_back("$spl_Windv_Spline_slopes$");

    Computation* vComputation = v_Computation();

    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult vCompRes = updateSources->appendComputation(vComputation, "{[0]}", "{[0]->["+std::to_string(newTuplePos+3)+"]}", vCompArgs);
    newTuplePos = vCompRes.tuplePosition+1;

    //Creating s3b
    //sources.v = v(r,theta,phi,spl.Temp_Spline);
    updateSources->addDataSpace("$sources.v$");
    Stmt* s3b = new Stmt("$sources.v$ = "+vCompRes.returnValues.back()+";",
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos)+"]}",
        {{vCompRes.returnValues.back(), "{[0]->[0]}"}},
        {{"$sources.v$", "{[0]->[0]}"}}
        );

    updateSources->addStmt(s3b);

    //Add the args of the u function (computation to be appended) as data spaces to temp
    //r, theta and phi have already been added
    //updateSources->addDataSpace("$spl.Windu_Spline$");

    //Temp var assignment because IEGenLib can't handle periods
    // int spl_Windu_Spline_length = spl.Windu_Spline.length;
    // int spl_Windu_Spline_accel = spl.Windu_Spline.accel;
    // double* spl_Windu_Spline_x_vals = spl.Windu_Spline.x_vals;
    // double* spl_Windu_Spline_f_vals = spl.Windu_Spline.f_vals;
    // double* spl_Windu_Spline_slopes = spl.Windu_Spline.slopes;
    updateSources->addDataSpace("$spl.Windu_Spline.length$");
    updateSources->addDataSpace("$spl.Windu_Spline.accel$");
    updateSources->addDataSpace("$spl.Windu_Spline.x_vals$");
    updateSources->addDataSpace("$spl.Windu_Spline.f_vals$");
    updateSources->addDataSpace("$spl.Windu_Spline.slopes$");

    updateSources->addDataSpace("$spl_Windu_Spline_length$");
    updateSources->addDataSpace("$spl_Windu_Spline_accel$");
    updateSources->addDataSpace("$spl_Windu_Spline_x_vals$");
    updateSources->addDataSpace("$spl_Windu_Spline_f_vals$");
    updateSources->addDataSpace("$spl_Windu_Spline_slopes$"); 
    Stmt* s3c = new Stmt("int $spl_Windu_Spline_length$ = $spl.Windu_Spline.length$; int $spl_Windu_Spline_accel$ = $spl.Windu_Spline.accel$; double* $spl_Windu_Spline_x_vals$ = $spl.Windu_Spline.x_vals$; double* $spl_Windu_Spline_f_vals$ = $spl.Windu_Spline.f_vals$; double* spl_Windu_Spline_slopes$ = $spl.Windu_Spline.slopes$;",
         "{[0]}",
         "{[0]->["+std::to_string(newTuplePos+1)+"]}",
         {
            {"$spl.Windv_Spline.length$","{[0]->[0]}"},
            {"$spl.Windv_Spline.accel$","{[0]->[0]}"},
            {"$spl.Windv_Spline.x_vals$","{[0]->[0]}"},
            {"$spl.Windv_Spline.f_vals$","{[0]->[0]}"},
            {"$spl.Windv_Spline.slopes$","{[0]->[0]}"}
         },
         {
            {"$spl_Windv_Spline_length$", "{[0]->[0]}"},
            {"$spl_Windv_Spline_accel$", "{[0]->[0]}"},
            {"$spl_Windv_Spline_x_vals$", "{[0]->[0]}"},
            {"$spl_Windv_Spline_f_vals$", "{[0]->[0]}"},
            {"$spl_Windv_Spline_slopes$", "{[0]->[0]}"}
         }
         );
    updateSources->addStmt(s3c);

    
    //Args to the u_Computation
    vector<std::string> uCompArgs;
    uCompArgs.push_back("$r$");
    uCompArgs.push_back("$theta$");
    uCompArgs.push_back("$phi$");
    //uCompArgs.push_back("$spl.Windu_Spline$");
    uCompArgs.push_back("$spl_Windu_Spline_length$");
    uCompArgs.push_back("$spl_Windu_Spline_accel$");
    uCompArgs.push_back("$spl_Windu_Spline_x_vals$");
    uCompArgs.push_back("$spl_Windu_Spline_f_vals$");
    uCompArgs.push_back("$spl_Windu_Spline_slopes$");

    Computation* uComputation = u_Computation();

    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult uCompRes = updateSources->appendComputation(uComputation, "{[0]}", "{[0]->["+std::to_string(newTuplePos+2)+"]}", uCompArgs);

    newTuplePos = uCompRes.tuplePosition+1;
    
    //Creating s4
    //sources.u = u(r,theta,phi, spl_Windu_Spline_;
    updateSources->addDataSpace("$sources.u$");
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
    //cDiffCompArgs.push_back("$spl.Temp_Spline$");
    cDiffCompArgs.push_back("$spl_Temp_Spline_length$");
    cDiffCompArgs.push_back("$spl_Temp_Spline_accel$");
    cDiffCompArgs.push_back("$spl_Temp_Spline_x_vals$");
    cDiffCompArgs.push_back("$spl_Temp_Spline_f_vals$");
    cDiffCompArgs.push_back("$spl_Temp_Spline_slopes$");

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
    
    //Creating s6
    //sources.dw[0] = w_diff(r,theta,phi,0); The w_diff function always returns 0!
    updateSources->addDataSpace("$sources.dw$");
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
    //vDiffCompArgs.push_back("$spl_Windv_Spline_");
    vDiffCompArgs.push_back("$spl_Windv_Spline_length$");
    vDiffCompArgs.push_back("$spl_Windv_Spline_accel$");
    vDiffCompArgs.push_back("$spl_Windv_Spline_x_vals$");
    vDiffCompArgs.push_back("$spl_Windv_Spline_f_vals$");
    vDiffCompArgs.push_back("$spl_Windv_Spline_slopes$");
    Computation* vDiffComputation = v_diff_Computation();

    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult vDiffCompRes = updateSources->appendComputation(vDiffComputation, "{[0]}", "{[0]->["+std::to_string(newTuplePos+2)+"]}", vDiffCompArgs);
    newTuplePos = vDiffCompRes.tuplePosition+1;

    //Creating s7
    //sources.dv[0] = v_diff(r,theta,phi,0,spl_Windv_Spline_;
    updateSources->addDataSpace("$sources.dv$");
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
    ///uDiffCompArgs.push_back("$spl_Windu_Spline_");
    uDiffCompArgs.push_back("$spl_Windu_Spline_length$");
    uDiffCompArgs.push_back("$spl_Windu_Spline_accel$");
    uDiffCompArgs.push_back("$spl_Windu_Spline_x_vals$");
    uDiffCompArgs.push_back("$spl_Windu_Spline_f_vals$");
    uDiffCompArgs.push_back("$spl_Windu_Spline_slopes$");

    Computation* uDiffComputation = u_diff_Computation();

    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult uDiffCompRes = updateSources->appendComputation(uDiffComputation, "{[0]}", "{[0]->["+std::to_string(newTuplePos+1)+"]}", uDiffCompArgs);

    newTuplePos = uDiffCompRes.tuplePosition+1;
    
    //Creating s8
    //sources.du[0] = u_diff(r,theta,phi,0,spl_Windu_Spline_;
    updateSources->addDataSpace("$sources.du$");
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
    //cDiffCompArgs.push_back("$spl.Temp_Spline$");
    cDiffCompArgs.push_back("$spl_Temp_Spline_length$");
    cDiffCompArgs.push_back("$spl_Temp_Spline_accel$");
    cDiffCompArgs.push_back("$spl_Temp_Spline_x_vals$");
    cDiffCompArgs.push_back("$spl_Temp_Spline_f_vals$");
    cDiffCompArgs.push_back("$spl_Temp_Spline_slopes$");

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
    //vDiffCompArgs.push_back("$spl_Windv_Spline_");
    vDiffCompArgs.push_back("$spl_Windv_Spline_length$");
    vDiffCompArgs.push_back("$spl_Windv_Spline_accel$");
    vDiffCompArgs.push_back("$spl_Windv_Spline_x_vals$");
    vDiffCompArgs.push_back("$spl_Windv_Spline_f_vals$");
    vDiffCompArgs.push_back("$spl_Windv_Spline_slopes$");

    AppendComputationResult vDiff1CompRes = updateSources->appendComputation(vDiffComputation, "{[0]}", "{[0]->["+std::to_string(newTuplePos+2)+"]}", vDiffCompArgs);
    newTuplePos = vDiff1CompRes.tuplePosition+1;

    //Creating s11
    //sources.dv[1] = v_diff(r,theta,phi,1,spl_Windv_Spline_;
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
    //uDiffCompArgs.push_back("$spl_Windu_Spline_");
    uDiffCompArgs.push_back("$spl_Windu_Spline_length$");
    uDiffCompArgs.push_back("$spl_Windu_Spline_accel$");
    uDiffCompArgs.push_back("$spl_Windu_Spline_x_vals$");
    uDiffCompArgs.push_back("$spl_Windu_Spline_f_vals$");
    uDiffCompArgs.push_back("$spl_Windu_Spline_slopes$");

    AppendComputationResult uDiff1CompRes = updateSources->appendComputation(uDiffComputation, "{[0]}", "{[0]->["+std::to_string(newTuplePos+1)+"]}", uDiffCompArgs);
    newTuplePos = uDiff1CompRes.tuplePosition+1;

    //Creating s12
    //sources.du[1] = u_diff(r,theta,phi,1,spl_Windu_Spline_;
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
    //cDiffCompArgs.push_back("$spl.Temp_Spline$");
    cDiffCompArgs.push_back("$spl_Temp_Spline_length$");
    cDiffCompArgs.push_back("$spl_Temp_Spline_accel$");
    cDiffCompArgs.push_back("$spl_Temp_Spline_x_vals$");
    cDiffCompArgs.push_back("$spl_Temp_Spline_f_vals$");
    cDiffCompArgs.push_back("$spl_Temp_Spline_slopes$");

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
    //vDiffCompArgs.push_back("$spl_Windv_Spline_");
    vDiffCompArgs.push_back("$spl_Windv_Spline_length$");
    vDiffCompArgs.push_back("$spl_Windv_Spline_accel$");
    vDiffCompArgs.push_back("$spl_Windv_Spline_x_vals$");
    vDiffCompArgs.push_back("$spl_Windv_Spline_f_vals$");
    vDiffCompArgs.push_back("$spl_Windv_Spline_slopes$");

    AppendComputationResult vDiff2CompRes = updateSources->appendComputation(vDiffComputation, "{[0]}", "{[0]->["+std::to_string(newTuplePos+2)+"]}", vDiffCompArgs);
    newTuplePos = vDiff2CompRes.tuplePosition+1;

    //Creating s15
    //sources.dv[2] = v_diff(r,theta,phi,2,spl_Windv_Spline_;
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
    //uDiffCompArgs.push_back("$spl_Windu_Spline_");
    uDiffCompArgs.push_back("$spl_Windu_Spline_length$");
    uDiffCompArgs.push_back("$spl_Windu_Spline_accel$");
    uDiffCompArgs.push_back("$spl_Windu_Spline_x_vals$");
    uDiffCompArgs.push_back("$spl_Windu_Spline_f_vals$");
    uDiffCompArgs.push_back("$spl_Windu_Spline_slopes$");

    AppendComputationResult uDiff2CompRes = updateSources->appendComputation(uDiffComputation, "{[0]}", "{[0]->["+std::to_string(newTuplePos+1)+"]}", uDiffCompArgs);
    newTuplePos = uDiff2CompRes.tuplePosition+1;

    //Creating s16
    //sources.du[2] = u_diff(r,theta,phi,2,spl_Windu_Spline_;
    Stmt* s16 = new Stmt("$sources.du$[2] = "+uDiff2CompRes.returnValues.back()+";",
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos)+"]}",
        {{uDiff2CompRes.returnValues.back(), "{[0]->[0]}"}},
        {{"$sources.du$", "{[0]->[2]}"}}
        );
    updateSources->addStmt(s16);

    //Creating s17
    //sources.nu_mag =   sqrt( nu[0]*nu[0] + nu[1]*nu[1] + nu[2]*nu[2]);
    updateSources->addDataSpace("$sources.nu_mag$");
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

    //Creating S18
    //sources.c_gr[0] =  sources.c*nu[0]/sources.nu_mag + sources.w;
    updateSources->addDataSpace("$sources.c_gr$");
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

    //Creating s21
    //sources.c_gr_mag = sqrt(pow(sources.c_gr[0],2) + pow(sources.c_gr[1],2) + pow(sources.c_gr[2],2));
    updateSources->addDataSpace("$sources.c_gr_mag$");
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

    //Creating s22
    //sources.GeoCoeff[0] = 1.0;
    updateSources->addDataSpace("$sources.GeoCoeff$");
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

    //Creating S25
    //sources.GeoTerms[0] = 0.0;
    updateSources->addDataSpace("$sources.GeoTerms$");
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

    //Creating s31
    //double R_lt[3], R_lp[3], mu_lt[3], mu_lp[3];
    updateSources->addDataSpace("$R_lt$");
    updateSources->addDataSpace("$R_lp$");
    updateSources->addDataSpace("$mu_lt$");
    updateSources->addDataSpace("$mu_lp$");
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
        "{[0]: GeoAc_CalcAmp = 1}",
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
        "{[0]: GeoAc_CalcAmp = 1}",
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
        "{[0]: GeoAc_CalcAmp = 1}",
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
        "{[0]: GeoAc_CalcAmp = 1}",
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
          "{[0]: GeoAc_CalcAmp = 1}",
          "{[0]->["+std::to_string(newTuplePos+20)+"]}",
          {},
          {
            {"$sources.dc$", "{[0]->[3]}"},
            {"$sources.dc$", "{[0]->[4]}"}
          }
     );
    updateSources->addStmt(s36);

     //Creating s37
     //sources.dw[3] = 0.0;  sources.dw[4] = 0.0;
     Stmt* s37 = new Stmt("$sources.dw$[3] = 0.0;  $sources.dw$[4] = 0.0;",
          "{[0]: GeoAc_CalcAmp = 1}",
          "{[0]->["+std::to_string(newTuplePos+21)+"]}",
          {},
          {
              {"$sources.dw$", "{[0]->[3]}"},
              {"$sources.dw$", "{[0]->[4]}"}
          }
    );
    updateSources->addStmt(s37);

     //Creating s38
     //sources.dv[3] = 0.0;  sources.dv[4] = 0.0;
     Stmt* s38 = new Stmt("$sources.dv$[3] = 0.0;  $sources.dv$[4] = 0.0;",
          "{[0]: GeoAc_CalcAmp = 1}",
          "{[0]->["+std::to_string(newTuplePos+22)+"]}",
          {},
          {
              {"$sources.dv$", "{[0]->[3]}"},
              {"$sources.dv$", "{[0]->[4]}"}
          }
     );
    updateSources->addStmt(s38);

     //Creating s39
     //sources.du[3] = 0.0;  sources.du[4] = 0.0;
     Stmt* s39 = new Stmt("$sources.du$[3] = 0.0;  $sources.du$[4] = 0.0;",
          "{[0]: GeoAc_CalcAmp = 1}",
          "{[0]->["+std::to_string(newTuplePos+23)+"]}",
          {},
          {
              {"$sources.du$", "{[0]->[3]}"},
              {"$sources.du$", "{[0]->[4]}"}
          }
    );
    updateSources->addStmt(s39);    

    //Creating s40 - inside the first for loop in the if condition
    //for(int n = 0; n < 3; n++){
    //sources.ddc[n][0] = 0.0;
    updateSources->addDataSpace("$sources.ddc$");
    Stmt* s40 = new Stmt("$sources.ddc$[n][0] = 0.0;",
        "{[n]: GeoAc_CalcAmp = 1 && n>=0 && n<3}",
        "{[n]->["+std::to_string(newTuplePos+24)+",n,0]}",
        {},
        {{"$sources.ddc$", "{[n]->[n,0]}"}}
        );

    updateSources->addStmt(s40);

    //Creating s41 - inside the first for loop in the if condition
    //sources.ddw[n][0] = 0.0;
    updateSources->addDataSpace("$sources.ddw$");
    Stmt* s41 = new Stmt("$sources.ddw$[n][0] = 0.0;",
        "{[n]: GeoAc_CalcAmp = 1 && n>=0 && n<3}",
        "{[n]->["+std::to_string(newTuplePos+24)+",n,1]}",
        {},
        {{"$sources.ddw$", "{[n]->[n,0]}"}}
        );

    updateSources->addStmt(s41);

    //Creating s42 - inside the first for loop in the if condition
    //sources.ddv[n][0] = 0.0;
    updateSources->addDataSpace("$sources.ddv$");
    Stmt* s42 = new Stmt("$sources.ddv$[n][0] = 0.0;",
        "{[n]: GeoAc_CalcAmp = 1 && n>=0 && n<3}",
        "{[n]->["+std::to_string(newTuplePos+24)+",n,2]}",
        {},
        {{"$sources.ddv$", "{[n]->[n,0]}"}}
        );

    updateSources->addStmt(s42);

    //Adding data space
    updateSources->addDataSpace("$sources.ddu$");
    //Creating s43 - inside the first for loop in the if condition
    //sources.ddu[n][0] = 0.0;
    Stmt* s43 = new Stmt("$sources.ddu$[n][0] = 0.0;",
        "{[n]: GeoAc_CalcAmp = 1 && n>=0 && n<3}",
        "{[n]->["+std::to_string(newTuplePos+24)+",n,3]}",
        {},
        {{"$sources.ddu$", "{[n]->[n,0]}"}}
        );

    updateSources->addStmt(s43);

    //Creating s44
    //sources.ddc[n][1] = 0.0;
    Stmt* s44 = new Stmt("$sources.ddc$[n][1] = 0.0;",
          "{[n]: GeoAc_CalcAmp = 1 && n>=0 && n<3}",
          "{[n]->["+std::to_string(newTuplePos+24)+", n, 4]}",
          {},
          {
              {"$sources.ddc$", "{[n]->[n,1]}"},
          }
    );
    updateSources->addStmt(s44);

    //Creating s45
    //sources.ddw[n][1] = 0.0;
    Stmt* s45 = new Stmt("$sources.ddw$[n][1] = 0.0;",
          "{[n]: GeoAc_CalcAmp = 1 && n>=0 && n<3}",
          "{[n]->["+std::to_string(newTuplePos+24)+", n, 5]}",
          {},
          {
              {"$sources.ddw$", "{[n]->[n,1]}"},
          }
    );
    updateSources->addStmt(s45);

    //Creating s46
    //sources.ddv[n][1] = 0.0;
    Stmt* s46 = new Stmt("$sources.ddv$[n][1] = 0.0;",
          "{[n]: GeoAc_CalcAmp = 1 && n>=0 && n<3}",
          "{[n]->["+std::to_string(newTuplePos+24)+", n, 6]}",
          {},
          {
              {"$sources.ddv$", "{[n]->[n,1]}"},
          }
    );
    updateSources->addStmt(s46);

    //Creating s47
    //sources.ddu[n][1] = 0.0;
    Stmt* s47 = new Stmt("$sources.ddu$[n][1] = 0.0;",
          "{[n]: GeoAc_CalcAmp = 1 && n>=0 && n<3}",
          "{[n]->["+std::to_string(newTuplePos+24)+", n, 7]}",
          {},
          {
              {"$sources.ddu$", "{[n]->[n,1]}"},
          }
    );
    updateSources->addStmt(s47);

    //Creating s47a
    //for(int n = 0; n < 3; n++){
    //    int loopN = n;
    updateSources->addDataSpace("$loopN$");
    Stmt* s47a = new Stmt("int $loopN$ = n;",
          "{[n]: GeoAc_CalcAmp = 1 && n>=0 && n<3}",
          "{[n]->["+std::to_string(newTuplePos+25)+", n, 0]}",
          {},
          {
              {"$loopN$", "{[n]->[0]}"},
          }
    );
    updateSources->addStmt(s47a);

    //Clearing arguments and adding new ones
    cDiffCompArgs.clear();
    cDiffCompArgs.push_back("$r$");
    cDiffCompArgs.push_back("$theta$");
    cDiffCompArgs.push_back("$phi$");
    cDiffCompArgs.push_back("$loopN$");
    //cDiffCompArgs.push_back("$spl.Temp_Spline$");
    cDiffCompArgs.push_back("$spl_Temp_Spline_length$");
    cDiffCompArgs.push_back("$spl_Temp_Spline_accel$");
    cDiffCompArgs.push_back("$spl_Temp_Spline_x_vals$");
    cDiffCompArgs.push_back("$spl_Temp_Spline_f_vals$");
    cDiffCompArgs.push_back("$spl_Temp_Spline_slopes$");

    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult cDiff3CompRes = updateSources->appendComputation(cDiffComputation, "{[n]: GeoAc_CalcAmp = 1 && n>=0 && n<3}", "{[n]->["+std::to_string(newTuplePos+25)+",n,1]}", cDiffCompArgs);
    unsigned int nTuplePos = cDiff3CompRes.tuplePosition+1;

    //Creating s48
    //sources.dc[3] += R_lt[n]*c_diff(r,theta,phi,loopN,spl.Temp_Spline);
    Stmt* s48 = new Stmt("$sources.dc$[3] += $R_lt$[n]*"+cDiff3CompRes.returnValues.back()+";",
        "{[n]: GeoAc_CalcAmp = 1 && n>=0 && n<3}",
        "{[n]->["+std::to_string(newTuplePos+25)+",n,"+std::to_string(nTuplePos)+"]}",
        {
            {"$sources.dc$", "{[n]->[3]}"},
            {"$R_lt$","{[n]->[n]}"},
            {cDiff3CompRes.returnValues.back(), "{[n]->[0]}"}
        },
        {{"$sources.dc$", "{[n]->[3]}"}}
        );
    updateSources->addStmt(s48);

    //Creating s49
    //sources.dw[3] += 0.0; //The w_diff function always returns 0
    Stmt* s49 = new Stmt("$sources.dw$[3] += 0;",
        "{[n]: GeoAc_CalcAmp = 1 && n>=0 && n<3}",
        "{[n]->["+std::to_string(newTuplePos+25)+",n,"+std::to_string(nTuplePos+1)+"]}",
        {{"$sources.dw$", "{[n]->[3]}"}},
        {{"$sources.dw$", "{[n]->[3]}"}}
        );
    updateSources->addStmt(s49);

    //Clearing agruments and adding new ones
    vDiffCompArgs.clear();
    vDiffCompArgs.push_back("$r$");
    vDiffCompArgs.push_back("$theta$");
    vDiffCompArgs.push_back("$phi$");
    vDiffCompArgs.push_back("$loopN$");
    //vDiffCompArgs.push_back("$spl_Windv_Spline_");
    vDiffCompArgs.push_back("$spl_Windv_Spline_length$");
    vDiffCompArgs.push_back("$spl_Windv_Spline_accel$");
    vDiffCompArgs.push_back("$spl_Windv_Spline_x_vals$");
    vDiffCompArgs.push_back("$spl_Windv_Spline_f_vals$");
    vDiffCompArgs.push_back("$spl_Windv_Spline_slopes$");

    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult vDiff3CompRes = updateSources->appendComputation(vDiffComputation, "{[n]: GeoAc_CalcAmp = 1 && n>=0 && n<3}", "{[n]->["+std::to_string(newTuplePos+25)+",n,"+std::to_string(nTuplePos+2)+"]}", vDiffCompArgs);
    nTuplePos = vDiff3CompRes.tuplePosition+1;

    //Creating s50
    //sources.dv[3] += R_lt[n]*v_diff(r,theta,phi,loopN,spl_Windv_Spline_;
    Stmt* s50 = new Stmt("$sources.dv$[3] += $R_lt$[n]*"+vDiff3CompRes.returnValues.back()+";",
        "{[n]: GeoAc_CalcAmp = 1 && n>=0 && n<3}",
        "{[n]->["+std::to_string(newTuplePos+25)+",n,"+std::to_string(nTuplePos)+"]}",
        {
            {"$sources.dv$", "{[n]->[3]}"},
            {"$R_lt$","{[n]->[n]}"},
            {vDiff3CompRes.returnValues.back(), "{[n]->[0]}"}
        },
        {{"$sources.dv$", "{[n]->[3]}"}}
        );
    updateSources->addStmt(s50);

    //Clearing arguments and adding new ones
    uDiffCompArgs.clear();
    uDiffCompArgs.push_back("$r$");
    uDiffCompArgs.push_back("$theta$");
    uDiffCompArgs.push_back("$phi$");
    uDiffCompArgs.push_back("$loopN$");
    //uDiffCompArgs.push_back("$spl_Windu_Spline_");
    uDiffCompArgs.push_back("$spl_Windu_Spline_length$");
    uDiffCompArgs.push_back("$spl_Windu_Spline_accel$");
    uDiffCompArgs.push_back("$spl_Windu_Spline_x_vals$");
    uDiffCompArgs.push_back("$spl_Windu_Spline_f_vals$");
    uDiffCompArgs.push_back("$spl_Windu_Spline_slopes$");

    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult uDiff3CompRes = updateSources->appendComputation(uDiffComputation, "{[n]: GeoAc_CalcAmp = 1 && n>=0 && n<3}", "{[n]->["+std::to_string(newTuplePos+25)+",n,"+std::to_string(nTuplePos+1)+"]}", uDiffCompArgs);
    nTuplePos = uDiff3CompRes.tuplePosition+1;

    //Creating s51
    //sources.du[3] += R_lt[n]*u_diff(r,theta,phi,n,spl_Windu_Spline_;
    Stmt* s51 = new Stmt("$sources.du$[3] += $R_lt$[n]*"+uDiff3CompRes.returnValues.back()+";",
        "{[n]: GeoAc_CalcAmp = 1 && n>=0 && n<3}",
        "{[n]->["+std::to_string(newTuplePos+25)+",n,"+std::to_string(nTuplePos)+"]}",
        {
            {"$sources.du$", "{[n]->[3]}"},
            {"$R_lt$","{[n]->[n]}"},
            {uDiff3CompRes.returnValues.back(), "{[n]->[0]}"}
        },
        {{"$sources.du$", "{[n]->[3]}"}}
        );
    updateSources->addStmt(s51);

    //Clearing arguments and adding new ones
    cDiffCompArgs.clear();
    cDiffCompArgs.push_back("$r$");
    cDiffCompArgs.push_back("$theta$");
    cDiffCompArgs.push_back("$phi$");
    cDiffCompArgs.push_back("$loopN$");
    //cDiffCompArgs.push_back("$spl.Temp_Spline$");
    cDiffCompArgs.push_back("$spl_Temp_Spline_length$");
    cDiffCompArgs.push_back("$spl_Temp_Spline_accel$");
    cDiffCompArgs.push_back("$spl_Temp_Spline_x_vals$");
    cDiffCompArgs.push_back("$spl_Temp_Spline_f_vals$");
    cDiffCompArgs.push_back("$spl_Temp_Spline_slopes$");

    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult cDiff4CompRes = updateSources->appendComputation(cDiffComputation, "{[n]: GeoAc_CalcAmp = 1 && n>=0 && n<3}", "{[n]->["+std::to_string(newTuplePos+25)+",n,"+std::to_string(nTuplePos+1)+"]}", cDiffCompArgs);
    nTuplePos = cDiff4CompRes.tuplePosition+1;

    //Creating s52
    //sources.dc[4] += R_lp[n]*c_diff(r,theta,phi,n,spl.Temp_Spline);
    Stmt* s52 = new Stmt("$sources.dc$[4] += $R_lp$[n]*"+cDiff4CompRes.returnValues.back()+";",
        "{[n]: GeoAc_CalcAmp = 1 && n>=0 && n<3}",
        "{[n]->["+std::to_string(newTuplePos+25)+",n,"+std::to_string(nTuplePos)+"]}",
        {
            {"$sources.dc$", "{[n]->[4]}"},
            {"$R_lp$","{[n]->[n]}"},
            {cDiff4CompRes.returnValues.back(), "{[n]->[0]}"}
        },
        {{"$sources.dc$", "{[n]->[4]}"}}
        );
    updateSources->addStmt(s52);

    //Creating s53
    //sources.dw[4] += 0.0; //The w_diff function always returns 0
    Stmt* s53 = new Stmt("$sources.dw$[4] += 0;",
        "{[n]: GeoAc_CalcAmp = 1 && n>=0 && n<3}",
        "{[n]->["+std::to_string(newTuplePos+25)+",n,"+std::to_string(nTuplePos+1)+"]}",
        {{"$sources.dw$", "{[n]->[4]}"}},
        {{"$sources.dw$", "{[n]->[4]}"}}
        );
    updateSources->addStmt(s53);

    //Clearing agruments and adding new ones
    vDiffCompArgs.clear();
    vDiffCompArgs.push_back("$r$");
    vDiffCompArgs.push_back("$theta$");
    vDiffCompArgs.push_back("$phi$");
    vDiffCompArgs.push_back("$loopN$");
    //vDiffCompArgs.push_back("$spl_Windv_Spline_");
    vDiffCompArgs.push_back("$spl_Windv_Spline_length$");
    vDiffCompArgs.push_back("$spl_Windv_Spline_accel$");
    vDiffCompArgs.push_back("$spl_Windv_Spline_x_vals$");
    vDiffCompArgs.push_back("$spl_Windv_Spline_f_vals$");
    vDiffCompArgs.push_back("$spl_Windv_Spline_slopes$");

    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult vDiff4CompRes = updateSources->appendComputation(vDiffComputation, "{[n]: GeoAc_CalcAmp = 1 && n>=0 && n<3}", "{[n]->["+std::to_string(newTuplePos+25)+",n,"+std::to_string(nTuplePos+2)+"]}", vDiffCompArgs);
    nTuplePos = vDiff4CompRes.tuplePosition+1;

    //Creating s54
    //sources.dv[4] += R_lp[n]*v_diff(r,theta,phi,loopN,spl_Windv_Spline_;
    Stmt* s54 = new Stmt("$sources.dv$[4] += $R_lp$[n]*"+vDiff4CompRes.returnValues.back()+";",
        "{[n]: GeoAc_CalcAmp = 1 && n>=0 && n<3}",
        "{[n]->["+std::to_string(newTuplePos+25)+",n,"+std::to_string(nTuplePos)+"]}",
        {
            {"$sources.dv$", "{[n]->[4]}"},
            {"$R_lp$","{[n]->[n]}"},
            {vDiff4CompRes.returnValues.back(), "{[n]->[0]}"}
        },
        {{"$sources.dv$", "{[n]->[4]}"}}
        );
    updateSources->addStmt(s54);

    //Clearing arguments and adding new ones
    uDiffCompArgs.clear();
    uDiffCompArgs.push_back("$r$");
    uDiffCompArgs.push_back("$theta$");
    uDiffCompArgs.push_back("$phi$");
    uDiffCompArgs.push_back("$loopN$");
    //uDiffCompArgs.push_back("$spl_Windu_Spline_");
    uDiffCompArgs.push_back("$spl_Windu_Spline_length$");
    uDiffCompArgs.push_back("$spl_Windu_Spline_accel$");
    uDiffCompArgs.push_back("$spl_Windu_Spline_x_vals$");
    uDiffCompArgs.push_back("$spl_Windu_Spline_f_vals$");
    uDiffCompArgs.push_back("$spl_Windu_Spline_slopes$");

    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult uDiff4CompRes = updateSources->appendComputation(uDiffComputation, "{[n]: GeoAc_CalcAmp = 1 && n>=0 && n<3}", "{[n]->["+std::to_string(newTuplePos+25)+",n,"+std::to_string(nTuplePos+1)+"]}", uDiffCompArgs);
    nTuplePos = uDiff4CompRes.tuplePosition+1;

    //Creating s55
    //sources.du[4] += R_lp[n]*u_diff(r,theta,phi,n,spl_Windu_Spline_;
    Stmt* s55 = new Stmt("$sources.du$[4] += $R_lp$[n]*"+uDiff4CompRes.returnValues.back()+";",
        "{[n]: GeoAc_CalcAmp = 1 && n>=0 && n<3}",
        "{[n]->["+std::to_string(newTuplePos+25)+",n,"+std::to_string(nTuplePos)+"]}",
        {
            {"$sources.du$", "{[n]->[4]}"},
            {"$R_lp$","{[n]->[n]}"},
            {uDiff4CompRes.returnValues.back(), "{[n]->[0]}"}
        },
        {{"$sources.du$", "{[n]->[4]}"}}
        );
    updateSources->addStmt(s55);

    //Creating s56a
    //for(int m = 0; m < 3; m++){
    //    int loopM = m;
    updateSources->addDataSpace("$loopM$");
    Stmt* s56a = new Stmt("int $loopM$ = m;",
          "{[n,m]: GeoAc_CalcAmp = 1 && n>=0 && n<3 && m>=0 && m<3}",
          "{[n,m]->["+std::to_string(newTuplePos+25)+", n, "+std::to_string(nTuplePos)+",m,0]}",
          {},
          {
              {"$loopM$", "{[n,m]->[0]}"},
          }
    );
    updateSources->addStmt(s56a);

    //Args to c_ddiff_Computation
    vector<std::string> cDdiffCompArgs;
    cDdiffCompArgs.push_back("$r$");
    cDdiffCompArgs.push_back("$theta$");
    cDdiffCompArgs.push_back("$phi$");
    cDdiffCompArgs.push_back("$loopM$");
    cDdiffCompArgs.push_back("$loopN$");
    //cDdiffCompArgs.push_back("$spl.Temp_Spline$");
    cDdiffCompArgs.push_back("$spl_Temp_Spline_length$");
    cDdiffCompArgs.push_back("$spl_Temp_Spline_accel$");
    cDdiffCompArgs.push_back("$spl_Temp_Spline_x_vals$");
    cDdiffCompArgs.push_back("$spl_Temp_Spline_f_vals$");
    cDdiffCompArgs.push_back("$spl_Temp_Spline_slopes$");

    Computation* cDdiffComputation = c_ddiff_Computation();

    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult cDdiffCompRes = updateSources->appendComputation(cDdiffComputation, "{[n,m]: GeoAc_CalcAmp = 1 && n>=0 && n<3 && m>=0 && m<3}", "{[n,m]->["+std::to_string(newTuplePos+25)+", n, "+std::to_string(nTuplePos)+",m,1]}", cDdiffCompArgs);
    unsigned int mTuplePos = cDdiffCompRes.tuplePosition+1;

    //Creating s56
    //sources.ddc[m][0] += R_lt[n]*c_ddiff(r, theta, phi, m, n, spl.Temp_Spline);
    Stmt* s56 = new Stmt("$sources.ddc$[m][0] += $R_lt$[n]*"+cDdiffCompRes.returnValues.back()+";",
        "{[n,m]: GeoAc_CalcAmp = 1 && n>=0 && n<3 && m>=0 && m<3}",
        "{[n,m]->["+std::to_string(newTuplePos+25)+",n,"+std::to_string(nTuplePos)+",m,"+std::to_string(mTuplePos)+"]}",
        {
            {"$sources.ddc$", "{[n,m]->[m,0]}"},
            {"$R_lt$","{[n,m]->[n]}"},
            {cDdiffCompRes.returnValues.back(), "{[n,m]->[0]}"}
        },
        {{"$sources.ddc$", "{[n,m]->[m,0]}"}}
        );
    updateSources->addStmt(s56);

    //Creating s57
    //sources.ddw[m][0] += R_lt[n]*w_ddiff(r, theta, phi, m, n); //The w_ddiff function always returns 0
    Stmt* s57 = new Stmt("$sources.ddw$[m][0] += 0;",
        "{[n,m]: GeoAc_CalcAmp = 1 && n>=0 && n<3 && m>=0 && m<3}",
        "{[n,m]->["+std::to_string(newTuplePos+25)+",n,"+std::to_string(nTuplePos)+",m,"+std::to_string(mTuplePos+1)+"]}",
        {
            {"$sources.ddw$", "{[n,m]->[m,0]}"},
            {"$R_lt$","{[n,m]->[n]}"},
            {cDdiffCompRes.returnValues.back(), "{[n,m]->[0]}"}
        },
        {{"$sources.ddw$", "{[n,m]->[m,0]}"}}
        );
    updateSources->addStmt(s57);

    //Args to v_ddiff_Computation
    vector<std::string> vDdiffCompArgs;
    vDdiffCompArgs.push_back("$r$");
    vDdiffCompArgs.push_back("$theta$");
    vDdiffCompArgs.push_back("$phi$");
    vDdiffCompArgs.push_back("$loopM$");
    vDdiffCompArgs.push_back("$loopN$");
    //vDdiffCompArgs.push_back("$spl_Windv_Spline_");
    vDdiffCompArgs.push_back("$spl_Windv_Spline_length$");
    vDdiffCompArgs.push_back("$spl_Windv_Spline_accel$");
    vDdiffCompArgs.push_back("$spl_Windv_Spline_x_vals$");
    vDdiffCompArgs.push_back("$spl_Windv_Spline_f_vals$");
    vDdiffCompArgs.push_back("$spl_Windv_Spline_slopes$");

    Computation* vDdiffComputation = v_ddiff_Computation();

    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult vDdiffCompRes = updateSources->appendComputation(vDdiffComputation, "{[n,m]: GeoAc_CalcAmp = 1 && n>=0 && n<3 && m>=0 && m<3}", "{[n,m]->["+std::to_string(newTuplePos+25)+", n, "+std::to_string(nTuplePos)+",m,"+std::to_string(mTuplePos+2)+"]}", vDdiffCompArgs);
    mTuplePos = vDdiffCompRes.tuplePosition+1;

    //Creating s58
    //sources.ddv[m][0] += R_lt[n]*v_ddiff(r, theta, phi, m, n, spl_Windv_Spline_;
    Stmt* s58 = new Stmt("$sources.ddv$[m][0] += $R_lt$[n]*"+vDdiffCompRes.returnValues.back()+";",
        "{[n,m]: GeoAc_CalcAmp = 1 && n>=0 && n<3 && m>=0 && m<3}",
        "{[n,m]->["+std::to_string(newTuplePos+25)+",n,"+std::to_string(nTuplePos)+",m,"+std::to_string(mTuplePos)+"]}",
        {
            {"$sources.ddv$", "{[n,m]->[m,0]}"},
            {"$R_lt$","{[n,m]->[n]}"},
            {vDdiffCompRes.returnValues.back(), "{[n,m]->[0]}"}
        },
        {{"$sources.ddv$", "{[n,m]->[m,0]}"}}
        );
    updateSources->addStmt(s58);

    //Args to u_ddiff_Computation
    vector<std::string> uDdiffCompArgs;
    uDdiffCompArgs.push_back("$r$");
    uDdiffCompArgs.push_back("$theta$");
    uDdiffCompArgs.push_back("$phi$");
    uDdiffCompArgs.push_back("$loopM$");
    uDdiffCompArgs.push_back("$loopN$");
    //uDdiffCompArgs.push_back("$spl_Windu_Spline_");
    uDdiffCompArgs.push_back("$spl_Windu_Spline_length$");
    uDdiffCompArgs.push_back("$spl_Windu_Spline_accel$");
    uDdiffCompArgs.push_back("$spl_Windu_Spline_x_vals$");
    uDdiffCompArgs.push_back("$spl_Windu_Spline_f_vals$");
    uDdiffCompArgs.push_back("$spl_Windu_Spline_slopes$");

    Computation* uDdiffComputation = u_ddiff_Computation();

    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult uDdiffCompRes = updateSources->appendComputation(uDdiffComputation, "{[n,m]: GeoAc_CalcAmp = 1 && n>=0 && n<3 && m>=0 && m<3}", "{[n,m]->["+std::to_string(newTuplePos+25)+", n, "+std::to_string(nTuplePos)+",m,"+std::to_string(mTuplePos+1)+"]}", vDdiffCompArgs);
    mTuplePos = uDdiffCompRes.tuplePosition+1;

    //Creating s59
    //sources.ddu[m][0] += R_lt[n]*v_ddiff(r, theta, phi, m, n, spl_Windv_Spline_;
    Stmt* s59 = new Stmt("$sources.ddu$[m][0] += $R_lt$[n]*"+uDdiffCompRes.returnValues.back()+";",
        "{[n,m]: GeoAc_CalcAmp = 1 && n>=0 && n<3 && m>=0 && m<3}",
        "{[n,m]->["+std::to_string(newTuplePos+25)+",n,"+std::to_string(nTuplePos)+",m,"+std::to_string(mTuplePos)+"]}",
        {
            {"$sources.ddu$", "{[n,m]->[m,0]}"},
            {"$R_lt$","{[n,m]->[n]}"},
            {uDdiffCompRes.returnValues.back(), "{[n,m]->[0]}"}
        },
        {{"$sources.ddu$", "{[n,m]->[m,0]}"}}
        );
    updateSources->addStmt(s59);

    cDdiffCompArgs.clear();
    cDdiffCompArgs.push_back("$r$");
    cDdiffCompArgs.push_back("$theta$");
    cDdiffCompArgs.push_back("$phi$");
    cDdiffCompArgs.push_back("$loopM$");
    cDdiffCompArgs.push_back("$loopN$");
    //cDdiffCompArgs.push_back("$spl.Temp_Spline$");
    cDdiffCompArgs.push_back("$spl_Temp_Spline_length$");
    cDdiffCompArgs.push_back("$spl_Temp_Spline_accel$");
    cDdiffCompArgs.push_back("$spl_Temp_Spline_x_vals$");
    cDdiffCompArgs.push_back("$spl_Temp_Spline_f_vals$");
    cDdiffCompArgs.push_back("$spl_Temp_Spline_slopes$");

    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult cDdiff1CompRes = updateSources->appendComputation(cDdiffComputation, "{[n,m]: GeoAc_CalcAmp = 1 && n>=0 && n<3 && m>=0 && m<3}", "{[n,m]->["+std::to_string(newTuplePos+25)+", n, "+std::to_string(nTuplePos)+",m,"+std::to_string(mTuplePos+1)+"]}", cDdiffCompArgs);
    mTuplePos = cDdiff1CompRes.tuplePosition+1;

    //Creating s60
    //sources.ddc[m][1] += R_lp[n]*c_ddiff(r, theta, phi, m, n, spl.Temp_Spline);
    Stmt* s60 = new Stmt("$sources.ddc$[m][1] += $R_lp$[n]*"+cDdiff1CompRes.returnValues.back()+";",
        "{[n,m]: GeoAc_CalcAmp = 1 && n>=0 && n<3 && m>=0 && m<3}",
        "{[n,m]->["+std::to_string(newTuplePos+25)+",n,"+std::to_string(nTuplePos)+",m,"+std::to_string(mTuplePos)+"]}",
        {
            {"$sources.ddc$", "{[n,m]->[m,1]}"},
            {"$R_lp$","{[n,m]->[n]}"},
            {cDdiff1CompRes.returnValues.back(), "{[n,m]->[0]}"}
        },
        {{"$sources.ddc$", "{[n,m]->[m,1]}"}}
        );
    updateSources->addStmt(s60);

    //Creating s61
    //sources.ddw[m][1] += R_lp[n]*w_ddiff(r, theta, phi, m, n); //The w_ddiff function always returns 0
    Stmt* s61 = new Stmt("$sources.ddw$[m][1] += 0;",
        "{[n,m]: GeoAc_CalcAmp = 1 && n>=0 && n<3 && m>=0 && m<3}",
        "{[n,m]->["+std::to_string(newTuplePos+25)+",n,"+std::to_string(nTuplePos)+",m,"+std::to_string(mTuplePos+1)+"]}",
        {
            {"$sources.ddw$", "{[n,m]->[m,1]}"},
            {"$R_lp$","{[n,m]->[n]}"},
            {cDdiff1CompRes.returnValues.back(), "{[n,m]->[0]}"}
        },
        {{"$sources.ddw$", "{[n,m]->[m,1]}"}}
        );
    updateSources->addStmt(s61);

    vDdiffCompArgs.clear();
    vDdiffCompArgs.push_back("$r$");
    vDdiffCompArgs.push_back("$theta$");
    vDdiffCompArgs.push_back("$phi$");
    vDdiffCompArgs.push_back("$loopM$");
    vDdiffCompArgs.push_back("$loopN$");
    //vDdiffCompArgs.push_back("$spl_Windv_Spline_");
    vDdiffCompArgs.push_back("$spl_Windv_Spline_length$");
    vDdiffCompArgs.push_back("$spl_Windv_Spline_accel$");
    vDdiffCompArgs.push_back("$spl_Windv_Spline_x_vals$");
    vDdiffCompArgs.push_back("$spl_Windv_Spline_f_vals$");
    vDdiffCompArgs.push_back("$spl_Windv_Spline_slopes$");

    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult vDdiff1CompRes = updateSources->appendComputation(vDdiffComputation, "{[n,m]: GeoAc_CalcAmp = 1 && n>=0 && n<3 && m>=0 && m<3}", "{[n,m]->["+std::to_string(newTuplePos+25)+", n, "+std::to_string(nTuplePos)+",m,"+std::to_string(mTuplePos+2)+"]}", vDdiffCompArgs);
    mTuplePos = vDdiff1CompRes.tuplePosition+1;

    //Creating s62
    //sources.ddv[m][1] += R_lp[n]*v_ddiff(r, theta, phi, m, n, spl_Windv_Spline_;
    Stmt* s62 = new Stmt("$sources.ddv$[m][1] += $R_lp$[n]*"+vDdiff1CompRes.returnValues.back()+";",
        "{[n,m]: GeoAc_CalcAmp = 1 && n>=0 && n<3 && m>=0 && m<3}",
        "{[n,m]->["+std::to_string(newTuplePos+25)+",n,"+std::to_string(nTuplePos)+",m,"+std::to_string(mTuplePos)+"]}",
        {
            {"$sources.ddv$", "{[n,m]->[m,1]}"},
            {"$R_lp$","{[n,m]->[n]}"},
            {vDdiff1CompRes.returnValues.back(), "{[n,m]->[0]}"}
        },
        {{"$sources.ddv$", "{[n,m]->[m,1]}"}}
        );
    updateSources->addStmt(s62);

    uDdiffCompArgs.clear();
    uDdiffCompArgs.push_back("$r$");
    uDdiffCompArgs.push_back("$theta$");
    uDdiffCompArgs.push_back("$phi$");
    uDdiffCompArgs.push_back("$loopM$");
    uDdiffCompArgs.push_back("$loopN$");
    //uDdiffCompArgs.push_back("$spl_Windu_Spline_");
    uDdiffCompArgs.push_back("$spl_Windu_Spline_length$");
    uDdiffCompArgs.push_back("$spl_Windu_Spline_accel$");
    uDdiffCompArgs.push_back("$spl_Windu_Spline_x_vals$");
    uDdiffCompArgs.push_back("$spl_Windu_Spline_f_vals$");
    uDdiffCompArgs.push_back("$spl_Windu_Spline_slopes$");

    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult uDdiff1CompRes = updateSources->appendComputation(uDdiffComputation, "{[n,m]: GeoAc_CalcAmp = 1 && n>=0 && n<3 && m>=0 && m<3}", "{[n,m]->["+std::to_string(newTuplePos+25)+", n, "+std::to_string(nTuplePos)+",m,"+std::to_string(mTuplePos+1)+"]}", vDdiffCompArgs);
    mTuplePos = uDdiff1CompRes.tuplePosition+1;

    //Creating s63
    //sources.ddu[m][1] += R_lp[n]*u_ddiff(r, theta, phi, m, n, spl_Windu_Spline_;
    Stmt* s63 = new Stmt("$sources.ddu$[m][1] += $R_lp$[n]*"+uDdiff1CompRes.returnValues.back()+";",
        "{[n,m]: GeoAc_CalcAmp = 1 && n>=0 && n<3 && m>=0 && m<3}",
        "{[n,m]->["+std::to_string(newTuplePos+25)+",n,"+std::to_string(nTuplePos)+",m,"+std::to_string(mTuplePos)+"]}",
        {
            {"$sources.ddu$", "{[n,m]->[m,1]}"},
            {"$R_lp$","{[n,m]->[n]}"},
            {uDdiff1CompRes.returnValues.back(), "{[n,m]->[0]}"}
        },
        {{"$sources.ddu$", "{[n,m]->[m,1]}"}}
        );
    updateSources->addStmt(s63);

    //Creating s64
    //sources.dnu_mag[0] = (nu[0]*mu_lt[0] + nu[1]*mu_lt[1] + nu[2]*mu_lt[2])/sources.nu_mag;
    updateSources->addDataSpace("$sources.dnu_mag$");
    Stmt* s64 = new Stmt("$sources.dnu_mag$[0] = ($nu$[0]*$mu_lt$[0] + $nu$[1]*$mu_lt$[1] + $nu$[2]*$mu_lt$[2])/$sources.nu_mag$;",
        "{[0]: GeoAc_CalcAmp = 1}",
        "{[0]->["+std::to_string(newTuplePos+26)+"]}",
        {
            {"$nu$", "{[0]->[0]}"},
            {"$mu_lt$", "{[0]->[0]}"},
            {"$nu$","{[0]->[1]}"},
            {"$mu_lt$", "{[0]->[1]}"},
            {"$nu$","{[0]->[2]}"},
            {"$mu_lt$", "{[0]->[2]}"},
            {"$sources.nu_mag$", "{[0]->[0]}"}
        },
        {{"$sources.dnu_mag$", "{[0]->[0]}"}}
        );
    updateSources->addStmt(s64);

    //Creating s65
    //sources.dnu_mag[1] = (nu[0]*mu_lp[0] + nu[1]*mu_lp[1] + nu[2]*mu_lp[2])/sources.nu_mag;
    Stmt* s65 = new Stmt("$sources.dnu_mag$[1] = ($nu$[0]*$mu_lp$[0] + $nu$[1]*$mu_lp$[1] + $nu$[2]*$mu_lp$[2])/$sources.nu_mag$;",
        "{[0]: GeoAc_CalcAmp = 1}",
        "{[0]->["+std::to_string(newTuplePos+27)+"]}",
        {
            {"$nu$", "{[0]->[0]}"},
            {"$mu_lp$", "{[0]->[0]}"},
            {"$nu$","{[0]->[1]}"},
            {"$mu_lp$", "{[0]->[1]}"},
            {"$nu$","{[0]->[2]}"},
            {"$mu_lp$", "{[0]->[2]}"},
            {"$sources.nu_mag$", "{[0]->[0]}"}
        },
        {{"$sources.dnu_mag$", "{[0]->[1]}"}}
        );
    updateSources->addStmt(s65);

    //Creating s66
    //sources.dc_gr[0][0] = nu[0]/sources.nu_mag*sources.dc[3] + sources.c*mu_lt[0]/sources.nu_mag - sources.c*nu[0]/pow(sources.nu_mag,2) * sources.dnu_mag[0] + sources.dw[3];
    updateSources->addDataSpace("$sources.dc_gr$");
    Stmt* s66 = new Stmt("$sources.dc_gr$[0][0] = $nu$[0]/$sources.nu_mag$*$sources.dc$[3] + $sources.c$*$mu_lt$[0]/$sources.nu_mag$ - $sources.c$*$nu$[0]/pow($sources.nu_mag$,2) * $sources.dnu_mag$[0] + $sources.dw$[3];",
        "{[0]: GeoAc_CalcAmp = 1}",
        "{[0]->["+std::to_string(newTuplePos+28)+"]}",
        {
            {"$nu$", "{[0]->[0]}"},
            {"$sources.nu_mag$", "{[0]->[0]}"},
            {"$sources.dc$","{[0]->[3]}"},
            {"$sources.c$", "{[0]->[0]}"},
            {"$mu_lt$","{[0]->[0]}"},
            {"$sources.dnu_mag$", "{[0]->[0]}"},
            {"$sources.dw$", "{[0]->[3]}"}
        },
        {{"$sources.dc_gr$", "{[0]->[0,0]}"}}
        );
    updateSources->addStmt(s66);

    //Creating s67
    //sources.dc_gr[1][0] = nu[1]/sources.nu_mag*sources.dc[3] + sources.c*mu_lt[1]/sources.nu_mag - sources.c*nu[1]/pow(sources.nu_mag,2) * sources.dnu_mag[0] + sources.dw[3];
    Stmt* s67 = new Stmt("$sources.dc_gr$[1][0] = $nu$[1]/$sources.nu_mag$*$sources.dc$[3] + $sources.c$*$mu_lt$[1]/$sources.nu_mag$ - $sources.c$*$nu$[1]/pow($sources.nu_mag$,2) * $sources.dnu_mag$[0] + $sources.dv$[3];",
        "{[0]: GeoAc_CalcAmp = 1}",
        "{[0]->["+std::to_string(newTuplePos+29)+"]}",
        {
            {"$nu$", "{[0]->[1]}"},
            {"$sources.nu_mag$", "{[0]->[0]}"},
            {"$sources.dc$","{[0]->[3]}"},
            {"$sources.c$", "{[0]->[0]}"},
            {"$mu_lt$","{[0]->[1]}"},
            {"$sources.dnu_mag$", "{[0]->[0]}"},
            {"$sources.dv$", "{[0]->[3]}"}
        },
        {{"$sources.dc_gr$", "{[0]->[1,0]}"}}
        );
    updateSources->addStmt(s67);

    //Creating s68
    //sources.dc_gr[2][0] = nu[2]/sources.nu_mag*sources.dc[3] + sources.c*mu_lt[2]/sources.nu_mag - sources.c*nu[2]/pow(sources.nu_mag,2) * sources.dnu_mag[0] + sources.du[3];
    Stmt* s68 = new Stmt("$sources.dc_gr$[2][0] = $nu$[2]/$sources.nu_mag$*$sources.dc$[3] + $sources.c$*$mu_lt$[2]/$sources.nu_mag$ - $sources.c$*$nu$[2]/pow($sources.nu_mag$,2) * $sources.dnu_mag$[0] + $sources.du$[3];",
        "{[0]: GeoAc_CalcAmp = 1}",
        "{[0]->["+std::to_string(newTuplePos+30)+"]}",
        {
            {"$nu$", "{[0]->[2]}"},
            {"$sources.nu_mag$", "{[0]->[0]}"},
            {"$sources.dc$","{[0]->[3]}"},
            {"$sources.c$", "{[0]->[0]}"},
            {"$mu_lt$","{[0]->[2]}"},
            {"$sources.dnu_mag$", "{[0]->[0]}"},
            {"$sources.du$", "{[0]->[3]}"}
        },
        {{"$sources.dc_gr$", "{[0]->[2,0]}"}}
        );
    updateSources->addStmt(s68);

    //Creating s69
    //sources.dc_gr[0][1] = nu[0]/sources.nu_mag*sources.dc[4] + sources.c*mu_lp[0]/sources.nu_mag - sources.c*nu[0]/pow(sources.nu_mag,2) * sources.dnu_mag[1] + sources.dw[4];
    Stmt* s69 = new Stmt("$sources.dc_gr$[0][1] = $nu$[0]/$sources.nu_mag$*$sources.dc$[4] + $sources.c$*$mu_lp$[0]/$sources.nu_mag$ - $sources.c$*$nu$[0]/pow($sources.nu_mag$,2) * $sources.dnu_mag$[1] + $sources.dw$[4];",
        "{[0]: GeoAc_CalcAmp = 1}",
        "{[0]->["+std::to_string(newTuplePos+31)+"]}",
        {
            {"$nu$", "{[0]->[0]}"},
            {"$sources.nu_mag$", "{[0]->[0]}"},
            {"$sources.dc$","{[0]->[4]}"},
            {"$sources.c$", "{[0]->[0]}"},
            {"$mu_lp$","{[0]->[0]}"},
            {"$sources.dnu_mag$", "{[0]->[1]}"},
            {"$sources.dw$", "{[0]->[4]}"}
        },
        {{"$sources.dc_gr$", "{[0]->[0,1]}"}}
        );
    updateSources->addStmt(s69);

    //Creating s70
    //sources.dc_gr[1][1] = nu[1]/sources.nu_mag*sources.dc[4] + sources.c*mu_lp[1]/sources.nu_mag - sources.c*nu[1]/pow(sources.nu_mag,2) * sources.dnu_mag[1] + sources.dv[4];
    Stmt* s70 = new Stmt("$sources.dc_gr$[1][1] = $nu$[1]/$sources.nu_mag$*$sources.dc$[4] + $sources.c$*$mu_lp$[1]/$sources.nu_mag$ - $sources.c$*$nu$[1]/pow($sources.nu_mag$,2) * $sources.dnu_mag$[1] + $sources.dv$[4];",
        "{[0]: GeoAc_CalcAmp = 1}",
        "{[0]->["+std::to_string(newTuplePos+32)+"]}",
        {
            {"$nu$", "{[0]->[1]}"},
            {"$sources.nu_mag$", "{[0]->[0]}"},
            {"$sources.dc$","{[0]->[4]}"},
            {"$sources.c$", "{[0]->[0]}"},
            {"$mu_lp$","{[0]->[1]}"},
            {"$sources.dnu_mag$", "{[0]->[1]}"},
            {"$sources.dv$", "{[0]->[4]}"}
        },
        {{"$sources.dc_gr$", "{[0]->[1,1]}"}}
        );
    updateSources->addStmt(s70);

    //Creating s71
    //sources.dc_gr[2][1] = nu[2]/sources.nu_mag*sources.dc[4] + sources.c*mu_lp[2]/sources.nu_mag - sources.c*nu[2]/pow(sources.nu_mag,2) * sources.dnu_mag[1] + sources.du[4];
    Stmt* s71 = new Stmt("$sources.dc_gr$[2][1] = $nu$[2]/$sources.nu_mag$*$sources.dc$[4] + $sources.c$*$mu_lp$[2]/$sources.nu_mag$ - $sources.c$*$nu$[2]/pow($sources.nu_mag$,2) * $sources.dnu_mag$[1] + $sources.du$[4];",
        "{[0]: GeoAc_CalcAmp = 1}",
        "{[0]->["+std::to_string(newTuplePos+33)+"]}",
        {
            {"$nu$", "{[0]->[2]}"},
            {"$sources.nu_mag$", "{[0]->[0]}"},
            {"$sources.dc$","{[0]->[4]}"},
            {"$sources.c$", "{[0]->[0]}"},
            {"$mu_lp$","{[0]->[2]}"},
            {"$sources.dnu_mag$", "{[0]->[1]}"},
            {"$sources.du$", "{[0]->[4]}"}
        },
        {{"$sources.dc_gr$", "{[0]->[2,1]}"}}
        );
    updateSources->addStmt(s71);

    //Creating s72
    //sources.dc_gr_mag[0] = (sources.c_gr[0]*sources.dc_gr[0][0] + sources.c_gr[1]*sources.dc_gr[1][0] + sources.c_gr[2]*sources.dc_gr[2][0])/sources.c_gr_mag;
    updateSources->addDataSpace("$sources.dc_gr_mag$");
    Stmt* s72 = new Stmt("$sources.dc_gr_mag$[0] = ($sources.c_gr$[0]*$sources.dc_gr$[0][0] + $sources.c_gr$[1]*$sources.dc_gr$[1][0] + $sources.c_gr$[2]*$sources.dc_gr$[2][0])/$sources.c_gr_mag$;",
        "{[0]: GeoAc_CalcAmp = 1}",
        "{[0]->["+std::to_string(newTuplePos+34)+"]}",
        {
            {"$sources.c_gr$", "{[0]->[0]}"},
            {"$sources.dc_gr$", "{[0]->[0,0]}"},
            {"$sources.c_gr$","{[0]->[1]}"},
            {"$sources.dc_gr$", "{[0]->[1,0]}"},
            {"$sources.c_gr$","{[0]->[2]}"},
            {"$sources.dc_gr$", "{[0]->[2,0]}"},
            {"$sources.c_gr_mag$", "{[0]->[0]}"}
        },
        {{"$sources.dc_gr_mag$", "{[0]->[0]}"}}
        );
    updateSources->addStmt(s72);

    //Creating s73 - line 347 in Global Equation Sets
    //sources.dc_gr_mag[1] = (sources.c_gr[0]*sources.dc_gr[0][1] + sources.c_gr[1]*sources.dc_gr[1][1] + sources.c_gr[2]*sources.dc_gr[2][1])/sources.c_gr_mag;
    Stmt* s73 = new Stmt("$sources.dc_gr_mag$[1] = ($sources.c_gr$[0]*$sources.dc_gr$[0][1] + $sources.c_gr$[1]*$sources.dc_gr$[1][1] + $sources.c_gr$[2]*$sources.dc_gr$[2][1])/$sources.c_gr_mag$;",
        "{[0]: GeoAc_CalcAmp = 1}",
        "{[0]->["+std::to_string(newTuplePos+35)+"]}",
        {
            {"$sources.c_gr$", "{[0]->[0]}"},
            {"$sources.dc_gr$", "{[0]->[0,1]}"},
            {"$sources.c_gr$","{[0]->[1]}"},
            {"$sources.dc_gr$", "{[0]->[1,1]}"},
            {"$sources.c_gr$","{[0]->[2]}"},
            {"$sources.dc_gr$", "{[0]->[2,1]}"},
            {"$sources.c_gr_mag$", "{[0]->[1]}"}
        },
        {{"$sources.dc_gr_mag$", "{[0]->[0]}"}}
        );
    updateSources->addStmt(s73);

    //Creating s74
    //sources.d_GeoCoeff[0][0] = 0.0;
    updateSources->addDataSpace("$sources.d_GeoCoeff$");
    Stmt* s74 = new Stmt("$sources.d_GeoCoeff$[0][0] = 0.0;",
        "{[0]: GeoAc_CalcAmp = 1}",
        "{[0]->["+std::to_string(newTuplePos+36)+"]}",
        {},
        {{"$sources.d_GeoCoeff$", "{[0]->[0,0]}"}}
    );
    updateSources->addStmt(s74);

    //Creating s75
    //sources.d_GeoCoeff[1][0] = -R_lt[0]/(pow(r,2));
    Stmt* s75 = new Stmt("$sources.d_GeoCoeff$[1][0] = -$R_lt$[0]/(pow($r$,2));",
        "{[0]: GeoAc_CalcAmp = 1}",
        "{[0]->["+std::to_string(newTuplePos+37)+"]}",
        {{"$R_lt$", "{[0]->[0]}"}},
        {{"$sources.d_GeoCoeff$", "{[0]->[1,0]}"}}
    );
    updateSources->addStmt(s75);

    //Creating s76
    //sources.d_GeoCoeff[2][0] = -R_lt[0]/(pow(r,2)*cos(theta)) + sin(theta)/(r*pow(cos(theta),2))*R_lt[1];
    Stmt* s76 = new Stmt("$sources.d_GeoCoeff$[2][0] = -$R_lt$[0]/(pow($r$,2)*cos($theta$)) + sin($theta$)/($r$*pow(cos($theta$),2))*$R_lt$[1];",
        "{[0]: GeoAc_CalcAmp = 1}",
        "{[0]->["+std::to_string(newTuplePos+38)+"]}",
        {
         {"$R_lt$", "{[0]->[0]}"},
         {"$r$", "{[0]->[0]}"},
         {"$theta$", "{[0]->[0]}"}
    },
        {{"$sources.d_GeoCoeff$", "{[0]->[2,0]}"}}
    );
    updateSources->addStmt(s76);

    //Creating s77
    //sources.d_GeoCoeff[0][1] = 0.0;
    Stmt* s77 = new Stmt("$sources.d_GeoCoeff$[0][1] = 0.0;",
        "{[0]: GeoAc_CalcAmp = 1}",
        "{[0]->["+std::to_string(newTuplePos+39)+"]}",
        {},
        {{"$sources.d_GeoCoeff$", "{[0]->[0,1]}"}}
    );
    updateSources->addStmt(s77);

    //Creating s78
    //sources.d_GeoCoeff[1][1] = -R_lp[0]/(pow(r,2));
    Stmt* s78 = new Stmt("$sources.d_GeoCoeff$[1][1] = -$R_lp$[0]/(pow($r$,2));",
        "{[0]: GeoAc_CalcAmp = 1}",
        "{[0]->["+std::to_string(newTuplePos+40)+"]}",
        {{"$R_lp$", "{[0]->[0]}"}},
        {{"$sources.d_GeoCoeff$", "{[0]->[1,1]}"}}
    );
    updateSources->addStmt(s78);

    //Creating s79
    //sources.d_GeoCoeff[2][1] = -R_lp[0]/(pow(r,2)*cos(theta)) + sin(theta)/(r*pow(cos(theta),2))*R_lp[1];
    Stmt* s79 = new Stmt("$sources.d_GeoCoeff$[2][1] = -$R_lp$[0]/(pow($r$,2)*cos($theta$)) + sin($theta$)/($r$*pow(cos($theta$),2))*$R_lp$[1];",
        "{[0]: GeoAc_CalcAmp = 1}",
        "{[0]->["+std::to_string(newTuplePos+41)+"]}",
        {
            {"$R_lp$", "{[0]->[0]}"},
            {"$r$", "{[0]->[0]}"},
            {"$theta$", "{[0]->[0]}"}
        },
        {{"$sources.d_GeoCoeff$", "{[0]->[2,1]}"}}
    );
    updateSources->addStmt(s79);

    //Creating s80
    //sources.d_GeoTerms[0][0] = 0.0;
    updateSources->addDataSpace("$sources.d_GeoTerms$");
    Stmt* s80 = new Stmt("$sources.d_GeoTerms$[0][0] = 0.0;",
        "{[0]: GeoAc_CalcAmp = 1}",
        "{[0]->["+std::to_string(newTuplePos+42)+"]}",
        {},
        {{"$sources.d_GeoTerms$", "{[0]->[0,0]}"}}
    );
    updateSources->addStmt(s80);

    //Creating s81
    //sources.d_GeoTerms[1][0] = (mu_lt[0]*sources.v + nu[0]*sources.dv[3] - mu_lt[1]*sources.w - nu[1] * sources.dw[3]);
    Stmt* s81 = new Stmt("$sources.d_GeoTerms$[1][0] = ($mu_lt$[0]*$sources.v$ + $nu$[0]*$sources.dv$[3] - $mu_lt$[1]*$sources.w$ - $nu$[1] * $sources.dw$[3]);",
        "{[0]: GeoAc_CalcAmp = 1}",
        "{[0]->["+std::to_string(newTuplePos+43)+"]}",
        {
            {"$mu_lt$", "{[0]->[0]}"},
            {"$sources.v$", "{[0]->[0]}"},
            {"$nu$", "{[0]->[0]}"},
            {"$sources.dv$", "{[0]->[3]}"},
            {"$mu_lt$", "{[0]->[1]}"},
            {"$sources.w$", "{[0]->[0]}"},
            {"$nu$", "{[0]->[1]}"},
            {"$sources.dw$", "{[0]->[3]}"}
        },
        {{"$sources.d_GeoTerms$", "{[0]->[1,0]}"}}
    );
    updateSources->addStmt(s81);

    //Creating s82
    //sources.d_GeoTerms[2][0] = (mu_lt[0]*sources.u + nu[0]*sources.du[3] - mu_lt[2]*sources.w - nu[2] * sources.dw[3])*cos(theta) - (nu[0]*sources.u - nu[2]*sources.w)*R_lt[1]*sin(theta)
    //                                       + (mu_lt[1]*sources.u + nu[1]*sources.du[3] - mu_lt[2]*sources.v - nu[2] * sources.dv[3])*sin(theta) + (nu[1]*sources.u - nu[2]*sources.v)*R_lt[1]*cos(theta);
    Stmt* s82 = new Stmt("$sources.d_GeoTerms$[2][0] = ($mu_lt$[0]*$sources.u$ + $nu$[0]*$sources.du$[3] - $mu_lt$[2]*$sources.w$ - $nu$[2] * $sources.dw$[3])*cos($theta$) - ($nu$[0]*$sources.u$ - $nu$[2]*$sources.w$)*$R_lt$[1]*sin($theta$)+ ($mu_lt$[1]*$sources.u$ + $nu$[1]*$sources.du$[3] - $mu_lt$[2]*$sources.v$ - $nu$[2] * $sources.dv$[3])*sin($theta$) + ($nu$[1]*$sources.u$ - $nu$[2]*$sources.v$)*$R_lt$[1]*cos($theta$);",
        "{[0]: GeoAc_CalcAmp = 1}",
        "{[0]->["+std::to_string(newTuplePos+44)+"]}",
        {
            {"$mu_lt$", "{[0]->[0]}"},
            {"$sources.u$", "{[0]->[0]}"},
            {"$nu$", "{[0]->[0]}"},
            {"$sources.du$", "{[0]->[3]}"},
            {"$mu_lt$", "{[0]->[2]}"},
            {"$sources.w$", "{[0]->[0]}"},
            {"$nu$", "{[0]->[2]}"},
            {"$sources.dw$", "{[0]->[3]}"},
            {"$theta$", "{[0]->[0]}"},
            {"$R_lt$", "{[0]->[1]}"},
            {"$mu_lt$", "{[0]->[1]}"},
            {"$nu$", "{[0]->[1]}"},
            {"$sources.v$", "{[0]->[0]}"},
            {"$sources.dv$", "{[0]->[3]}"}
        },
        {{"$sources.d_GeoTerms$", "{[0]->[2,0]}"}}
    );
    updateSources->addStmt(s82);

    //Creating s83
    //sources.d_GeoTerms[0][0] += -R_lt[0]/pow(r,2)*(nu[1]*sources.c_gr[1] + nu[2]*sources.c_gr[2])
    //                                        + 1.0/r*(mu_lt[1]*sources.c_gr[1] + nu[1]*sources.dc_gr[1][0] + mu_lt[2]*sources.c_gr[2] + nu[2]*sources.dc_gr[2][0]);
    Stmt* s83 = new Stmt("$sources.d_GeoTerms$[0][0] += -$R_lt$[0]/pow($r$,2)*($nu$[1]*$sources.c_gr$[1] + $nu$[2]*$sources.c_gr$[2])+ 1.0/$r$*($mu_lt$[1]*$sources.c_gr$[1] + $nu$[1]*$sources.dc_gr$[1][0] + $mu_lt$[2]*$sources.c_gr$[2] + $nu$[2]*$sources.dc_gr$[2][0]);",
        "{[0]: GeoAc_CalcAmp = 1}",
        "{[0]->["+std::to_string(newTuplePos+45)+"]}",
        {
            {"$sources.d_GeoTerms$", "{[0]->[0, 0]}"},
            {"$R_lt$", "{[0]->[0]}"},
            {"$r$", "{[0]->[0]}"},
            {"$nu$", "{[0]->[1]}"},
            {"$sources.c_gr$", "{[0]->[1]}"},
            {"$nu$", "{[0]->[2]}"},
            {"$sources.c_gr$", "{[0]->[2]}"},
            {"$mu_lt$", "{[0]->[1]}"},
            {"$sources.dc_gr$", "{[0]->[1, 0]}"},
            {"$mu_lt$", "{[0]->[2]}"},
            {"$sources.c_gr$", "{[0]->[2]}"},
            {"$sources.dc_gr$", "{[0]->[2, 0]}"}
        },
        {{"$sources.d_GeoTerms$", "{[0]->[0,0]}"}}
    );
    updateSources->addStmt(s83);

    //Creating s84
    //sources.d_GeoTerms[1][0] += -mu_lt[0]*sources.c_gr[1] - nu[0]*sources.dc_gr[1][0] + mu_lt[2]*sources.c_gr[2]*tan(theta) + nu[2]*sources.dc_gr[2][0]*tan(theta) + nu[2]*sources.c_gr[2]*R_lt[1]/pow(cos(theta),2);
    Stmt* s84 = new Stmt("$sources.d_GeoTerms$[1][0] += -$mu_lt$[0]*$sources.c_gr$[1] - $nu$[0]*$sources.dc_gr$[1][0] + $mu_lt$[2]*$sources.c_gr$[2]*tan($theta$) + $nu$[2]*$sources.dc_gr$[2][0]*tan($theta$) + $nu$[2]*$sources.c_gr$[2]*$R_lt$[1]/pow(cos($theta$),2);",
        "{[0]: GeoAc_CalcAmp = 1}",
        "{[0]->["+std::to_string(newTuplePos+46)+"]}",
        {
            {"$sources.d_GeoTerms$", "{[0]->[1, 0]}"},
            {"$mu_lt$", "{[0]->[0]}"},
            {"$sources.c_gr$", "{[0]->[1]}"},
            {"$nu$", "{[0]->[0]}"},
            {"$sources.dc_gr$", "{[0]->[1, 0]}"},
            {"$mu_lt$", "{[0]->[2]}"},
            {"$sources.c_gr$", "{[0]->[2]}"},
            {"$theta$", "{[0]->[0]}"},
            {"$nu$", "{[0]->[2]}"},
            {"$sources.dc_gr$", "{[0]->[2, 0]}"},
            {"$R_lt$", "{[0]->[1]}"}
        },
        {{"$sources.d_GeoTerms$", "{[0]->[1,0]}"}}
    );
    updateSources->addStmt(s84);

    //Creating s85
    //sources.d_GeoTerms[2][0] += -sources.dc_gr[2][0]*(nu[0]*cos(theta) + nu[1]*sin(theta)) - sources.c_gr[2]*(mu_lt[0]*cos(theta) - nu[0]*R_lt[1]*sin(theta) + mu_lt[1]*sin(theta) + nu[1]*R_lt[1]*cos(theta));
    Stmt* s85 = new Stmt("$sources.d_GeoTerms$[2][0] += -$sources.dc_gr$[2][0]*($nu$[0]*cos($theta$) + $nu$[1]*sin($theta$)) - $sources.c_gr$[2]*($mu_lt$[0]*cos($theta$) - $nu$[0]*$R_lt$[1]*sin($theta$) + $mu_lt$[1]*sin($theta$) + $nu$[1]*$R_lt$[1]*cos($theta$));",
        "{[0]: GeoAc_CalcAmp = 1}",
        "{[0]->["+std::to_string(newTuplePos+47)+"]}",
        {
            {"$sources.d_GeoTerms$", "{[0]->[2, 0]}"},
            {"$sources.dc_gr$", "{[0]->[2, 0]}"},
            {"$nu$", "{[0]->[0]}"},
            {"$theta$", "{[0]->[0]}"},
            {"$nu$", "{[0]->[1]}"},
            {"$sources.c_gr$", "{[0]->[2]}"},
            {"$mu_lt$", "{[0]->[0]}"},
            {"$R_lt$", "{[0]->[1]}"},
            {"$mu_lt$", "{[0]->[1]}"}
        },
        {{"$sources.d_GeoTerms$", "{[0]->[2,0]}"}}
    );
    updateSources->addStmt(s85);

    //Creating s86
    //sources.d_GeoTerms[0][1] = 0.0;
    Stmt* s86 = new Stmt("$sources.d_GeoTerms$[0][1] = 0.0;",
        "{[0]: GeoAc_CalcAmp = 1}",
        "{[0]->["+std::to_string(newTuplePos+48)+"]}",
        {},
        {{"$sources.d_GeoTerms$", "{[0]->[0,1]}"}}
    );
    updateSources->addStmt(s86);

    //Creating s87
    //sources.d_GeoTerms[1][1] = (mu_lp[0]*sources.v + nu[0]*sources.dv[4] - mu_lp[1]*sources.w - nu[1] * sources.dw[4]);
    Stmt* s87 = new Stmt("$sources.d_GeoTerms$[1][1] = ($mu_lp$[0]*$sources.v$ + $nu$[0]*$sources.dv$[4] - $mu_lp$[1]*$sources.w$ - $nu$[1] * $sources.dw$[4]);",
        "{[0]: GeoAc_CalcAmp = 1}",
        "{[0]->["+std::to_string(newTuplePos+49)+"]}",
        {
            {"$mu_lp$", "{[0]->[0]}"},
            {"$sources.v$", "{[0]->[0]}"},
            {"$nu$", "{[0]->[0]}"},
            {"$sources.dv$", "{[0]->[4]}"},
            {"$mu_lp$", "{[0]->[1]}"},
            {"$sources.w$", "{[0]->[0]}"},
            {"$nu$", "{[0]->[1]}"},
            {"$sources.dw$", "{[0]->[4]}"}
        },
        {{"$sources.d_GeoTerms$", "{[0]->[1,1]}"}}
    );
    updateSources->addStmt(s87);

    //Creating s88
    //sources.d_GeoTerms[2][1] = (mu_lp[0]*sources.u + nu[0]*sources.du[4] - mu_lp[2]*sources.w - nu[2] * sources.dw[4])*cos(theta) - (nu[0]*sources.u - nu[2]*sources.w)*R_lp[1]*sin(theta)
    //                                       + (mu_lp[1]*sources.u + nu[1]*sources.du[4] - mu_lp[2]*sources.v - nu[2] * sources.dv[4])*sin(theta) + (nu[1]*sources.u - nu[2]*sources.v)*R_lp[1]*cos(theta);
    Stmt* s88 = new Stmt("$sources.d_GeoTerms$[2][1] = ($mu_lp$[0]*$sources.u$ + $nu$[0]*$sources.du$[4] - $mu_lp$[2]*$sources.w$ - $nu$[2] * $sources.dw$[4])*cos($theta$) - ($nu$[0]*$sources.u$ - $nu$[2]*$sources.w$)*$R_lp$[1]*sin($theta$)+ ($mu_lp$[1]*$sources.u$ + $nu$[1]*$sources.du$[4] - $mu_lp$[2]*$sources.v$ - $nu$[2] * $sources.dv$[4])*sin($theta$) + ($nu$[1]*$sources.u$ - $nu$[2]*$sources.v$)*$R_lp$[1]*cos($theta$);",
        "{[0]: GeoAc_CalcAmp = 1}",
        "{[0]->["+std::to_string(newTuplePos+50)+"]}",
        {
            {"$mu_lp$", "{[0]->[0]}"},
            {"$sources.u$", "{[0]->[0]}"},
            {"$nu$", "{[0]->[0]}"},
            {"$sources.du$", "{[0]->[4]}"},
            {"$mu_lp$", "{[0]->[2]}"},
            {"$sources.w$", "{[0]->[0]}"},
            {"$nu$", "{[0]->[2]}"},
            {"$sources.dw$", "{[0]->[4]}"},
            {"$theta$", "{[0]->[0]}"},
            {"$R_lp$", "{[0]->[1]}"},
            {"$mu_lp$", "{[0]->[1]}"},
            {"$nu$", "{[0]->[1]}"},
            {"$sources.v$", "{[0]->[0]}"},
            {"$sources.dv$", "{[0]->[4]}"}
        },
        {{"$sources.d_GeoTerms$", "{[0]->[2,1]}"}}
    );
    updateSources->addStmt(s88);

    //Creating s89
    //sources.d_GeoTerms[0][1] += -R_lp[0]/pow(r,2)*(nu[1]*sources.c_gr[1] + nu[2]*sources.c_gr[2])
    //                                        + 1.0/r*(mu_lp[1]*sources.c_gr[1] + nu[1]*sources.dc_gr[1][1] + mu_lp[2]*sources.c_gr[2] + nu[2]*sources.dc_gr[2][1]);
    Stmt* s89 = new Stmt("$sources.d_GeoTerms$[0][1] += -$R_lp$[0]/pow($r$,2)*($nu$[1]*$sources.c_gr$[1] + $nu$[2]*$sources.c_gr$[2])+ 1.0/$r$*($mu_lp$[1]*$sources.c_gr$[1] + $nu$[1]*$sources.dc_gr$[1][1] + $mu_lp$[2]*$sources.c_gr$[2] + $nu$[2]*$sources.dc_gr$[2][1]);",
        "{[0]: GeoAc_CalcAmp = 1}",
        "{[0]->["+std::to_string(newTuplePos+51)+"]}",
        {
            {"$sources.d_GeoTerms$", "{[0]->[0, 1]}"},
            {"$R_lp$", "{[0]->[0]}"},
            {"$r$", "{[0]->[0]}"},
            {"$nu$", "{[0]->[1]}"},
            {"$sources.c_gr$", "{[0]->[1]}"},
            {"$nu$", "{[0]->[2]}"},
            {"$sources.c_gr$", "{[0]->[2]}"},
            {"$mu_lp$", "{[0]->[1]}"},
            {"$sources.dc_gr$", "{[0]->[1, 1]}"},
            {"$mu_lp$", "{[0]->[2]}"},
            {"$sources.c_gr$", "{[0]->[2]}"},
            {"$sources.dc_gr$", "{[0]->[2, 1]}"}
        },
        {{"$sources.d_GeoTerms$", "{[0]->[0,1]}"}}
    );
    updateSources->addStmt(s89);

    //Creating s90
    //sources.d_GeoTerms[1][1] += -mu_lp[0]*sources.c_gr[1] - nu[0]*sources.dc_gr[1][1] + mu_lp[2]*sources.c_gr[2]*tan(theta) + nu[2]*sources.dc_gr[2][1]*tan(theta) + nu[2]*sources.c_gr[2]*R_lp[1]/pow(cos(theta),2);
    Stmt* s90 = new Stmt("$sources.d_GeoTerms$[1][1] += -$mu_lp$[0]*$sources.c_gr$[1] - $nu$[0]*$sources.dc_gr$[1][1] + $mu_lp$[2]*$sources.c_gr$[2]*tan($theta$) + $nu$[2]*$sources.dc_gr$[2][1]*tan($theta$) + $nu$[2]*$sources.c_gr$[2]*$R_lp$[1]/pow(cos($theta$),2);",
        "{[0]: GeoAc_CalcAmp = 1}",
        "{[0]->["+std::to_string(newTuplePos+52)+"]}",
        {
            {"$sources.d_GeoTerms$", "{[0]->[1, 1]}"},
            {"$mu_lp$", "{[0]->[0]}"},
            {"$sources.c_gr$", "{[0]->[1]}"},
            {"$nu$", "{[0]->[0]}"},
            {"$sources.dc_gr$", "{[0]->[1, 1]}"},
            {"$mu_lp$", "{[0]->[2]}"},
            {"$sources.c_gr$", "{[0]->[2]}"},
            {"$theta$", "{[0]->[0]}"},
            {"$nu$", "{[0]->[2]}"},
            {"$sources.dc_gr$", "{[0]->[2, 1]}"},
            {"$R_lp$", "{[0]->[1]}"}
        },
        {{"$sources.d_GeoTerms$", "{[0]->[1,1]}"}}
    );
    updateSources->addStmt(s90);

    //Creating s91
    //sources.d_GeoTerms[2][1] += -sources.dc_gr[2][1]*(nu[0]*cos(theta) + nu[1]*sin(theta)) - sources.c_gr[2]*(mu_lp[0]*cos(theta) - nu[0]*R_lp[1]*sin(theta) + mu_lp[1]*sin(theta) + nu[1]*R_lp[1]*cos(theta));
    Stmt* s91 = new Stmt("$sources.d_GeoTerms$[2][1] += -$sources.dc_gr$[2][1]*($nu$[0]*cos($theta$) + $nu$[1]*sin($theta$)) - $sources.c_gr$[2]*($mu_lp$[0]*cos($theta$) - $nu$[0]*$R_lp$[1]*sin($theta$) + $mu_lp$[1]*sin($theta$) + $nu$[1]*$R_lp$[1]*cos($theta$));",
        "{[0]: GeoAc_CalcAmp = 1}",
        "{[0]->["+std::to_string(newTuplePos+53)+"]}",
        {
            {"$sources.d_GeoTerms$", "{[0]->[2, 1]}"},
            {"$sources.dc_gr$", "{[0]->[2, 1]}"},
            {"$nu$", "{[0]->[0]}"},
            {"$theta$", "{[0]->[0]}"},
            {"$nu$", "{[0]->[1]}"},
            {"$sources.c_gr$", "{[0]->[2]}"},
            {"$mu_lp$", "{[0]->[0]}"},
            {"$R_lp$", "{[0]->[1]}"},
            {"$mu_lp$", "{[0]->[1]}"}
        },
        {{"$sources.d_GeoTerms$", "{[0]->[2,1]}"}}
    );
    updateSources->addStmt(s91);
  
    //Calling toDot() on the Computation structure
    ofstream dotFileStream("codegen_dot.txt");
    cout << "Entering toDot()" << "\n";
    string dotString = updateSources->toDotString();
    dotFileStream << dotString;
    dotFileStream.close();
    
    //Write codegen to a file
    ofstream outStream;
    outStream.open("codegen.c");
    outStream << updateSources->codeGen();
    outStream.close();

    return 0;
}

/*
double u_ddiff(double r, double theta, double phi, int n1, int n2, NaturalCubicSpline_1D &Windu_Spline){
    double r_eval = min(r, r_max);  r_eval = max(r_eval, r_min);    // Check that r_min <= r_eval <= r_max
    double u_ddiff_return = 0.0;
    double eval_Spline_ddf_return = Eval_Spline_ddf(r_eval, Windu_Spline);
    if(n1==0&&n2==0){ u_ddiff_return = eval_Spline_ddf_return;}
}
*/
Computation* u_ddiff_Computation(){
    Computation* uDdiffComputation = new Computation();

    uDdiffComputation->addParameter("$r$","double");
    uDdiffComputation->addParameter("$theta$","double");
    uDdiffComputation->addParameter("$phi$","double");
    uDdiffComputation->addParameter("$n1$","int");
    uDdiffComputation->addParameter("$n2$","int");
    //uDdiffComputation->addParameter("$Windu_Spline$", "NaturalCubicSpline_1D &");
    uDdiffComputation->addParameter("$Windu_Spline_length$", "int");
    uDdiffComputation->addParameter("$Windu_Spline_accel$", "int");
    uDdiffComputation->addParameter("$Windu_Spline_x_vals$", "double*");
    uDdiffComputation->addParameter("$Windu_Spline_f_vals$", "double*");
    uDdiffComputation->addParameter("$Windu_Spline_slopes$", "double*");


    //Creating statement0
    //double r_eval = min(r, r_max);  r_eval = max(r_eval, r_min);
    uDdiffComputation->addDataSpace("$r_eval$");
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
    uDdiffComputation->addStmt(s0);

    //Creating statement1
    //double u_ddiff_return = 0.0;
    uDdiffComputation->addDataSpace("$u_ddiff_return$");
    Stmt* s1 = new Stmt("double $u_ddiff_return$ = 0.0",
        "{[0]}",
        "{[0]->[1]}",
        {},
        {{"$u_ddiff_return$","{[0]->[0]}"}}
    );
    uDdiffComputation->addStmt(s1);

    Computation* evalSplineDdfComputation = Eval_Spline_ddf_Computation();

    //Args to the Eval_Spline_ddf_Computation
    vector<std::string> evalSplineDdfCompArgs;
    evalSplineDdfCompArgs.push_back("$r_eval$");
    //evalSplineDdfCompArgs.push_back("$Windu_Spline$");
    evalSplineDdfCompArgs.push_back("$Windu_Spline_length$");
    evalSplineDdfCompArgs.push_back("$Windu_Spline_accel$");
    evalSplineDdfCompArgs.push_back("$Windu_Spline_x_vals$");
    evalSplineDdfCompArgs.push_back("$Windu_Spline_f_vals$");
    evalSplineDdfCompArgs.push_back("$Windu_Spline_slopes$");

    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult evalSplineDdfCompRes = uDdiffComputation->appendComputation(evalSplineDdfComputation, "{[0]}", "{[0]->[2]}", evalSplineDdfCompArgs);
    unsigned int newTuplePos = evalSplineDdfCompRes.tuplePosition+1;

    //Creating statement2
    //double eval_spline_ddf_return = Eval_Spline_ddf(r_eval,Windu_Spline);
    uDdiffComputation->addDataSpace("$eval_Spline_ddf_return$");
    Stmt* s2 = new Stmt("double $eval_Spline_ddf_return$ = "+evalSplineDdfCompRes.returnValues.back()+";",
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos)+"]}",
        {{evalSplineDdfCompRes.returnValues.back(), "{[0]->[0]}"}},
        {{"$eval_Spline_ddf_return$", "{[0]->[0]}"}}
    );
    uDdiffComputation->addStmt(s2);

    //Creating statement3
    //if(n1==0&&n2==0){u_ddiff_return = eval_spline_ddf_return;}
    Stmt* s3 = new Stmt("if($n1$==0 && $n2$==0){$u_ddiff_return$ = $eval_Spline_ddf_return$;}",
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos+1)+"]}",
        {
            {"$n1$", "{[0]->[0]}"},
            {"$n2$", "{[0]->[0]}"},
            {"$eval_Spline_ddf_return$", "{[0]->[0]}"}
        },
        {
            {"$u_ddiff_return$", "{[0]->[0]}"}
        }
    );
    uDdiffComputation->addStmt(s3);

    uDdiffComputation->addReturnValue("$u_ddiff_return$", true);

    return uDdiffComputation;
}


/*
double v_ddiff(double r, double theta, double phi, int n1, int n2, NaturalCubicSpline_1D &Windv_Spline){
    double r_eval = min(r, r_max);  r_eval = max(r_eval, r_min);    // Check that r_min <= r_eval <= r_max
    double v_ddiff_return = 0.0;
    double eval_spline_ddf_return = Eval_Spline_ddf(r_eval,Windv_Spline);
    if(n1==0&&n2==0){v_ddiff_return = eval_spline_ddf_return;}
}
*/
Computation* v_ddiff_Computation(){
    Computation* vDdiffComputation = new Computation();

    vDdiffComputation->addParameter("$r$","double");
    vDdiffComputation->addParameter("$theta$","double");
    vDdiffComputation->addParameter("$phi$","double");
    vDdiffComputation->addParameter("$n1$","int");
    vDdiffComputation->addParameter("$n2$","int");
    //vDdiffComputation->addParameter("$Windv_Spline$", "NaturalCubicSpline_1D &");
    vDdiffComputation->addParameter("$Windv_Spline_length$", "int");
    vDdiffComputation->addParameter("$Windv_Spline_accel$", "int");
    vDdiffComputation->addParameter("$Windv_Spline_x_vals$", "double*");
    vDdiffComputation->addParameter("$Windv_Spline_f_vals$", "double*");
    vDdiffComputation->addParameter("$Windv_Spline_slopes$", "double*");

    //Creating statement0
    //double r_eval = min(r, r_max);  r_eval = max(r_eval, r_min);
    vDdiffComputation->addDataSpace("$r_eval$");
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
    vDdiffComputation->addStmt(s0);

    //Creating statement1
    //double v_ddiff_return = 0.0;
    vDdiffComputation->addDataSpace("$v_ddiff_return$");
    Stmt* s1 = new Stmt("double $v_ddiff_return$ = 0.0",
        "{[0]}",
        "{[0]->[1]}",
        {},
        {{"$v_ddiff_return$","{[0]->[0]}"}}
    );
    vDdiffComputation->addStmt(s1);

    Computation* evalSplineDdfComputation = Eval_Spline_ddf_Computation();

    //Args to the Eval_Spline_ddf_Computation
    vector<std::string> evalSplineDdfCompArgs;
    evalSplineDdfCompArgs.push_back("$r_eval$");
    //evalSplineDdfCompArgs.push_back("$Windv_Spline$");
    evalSplineDdfCompArgs.push_back("$Windv_Spline_length$");
    evalSplineDdfCompArgs.push_back("$Windv_Spline_accel$");
    evalSplineDdfCompArgs.push_back("$Windv_Spline_x_vals$");
    evalSplineDdfCompArgs.push_back("$Windv_Spline_f_vals$");
    evalSplineDdfCompArgs.push_back("$Windv_Spline_slopes$");

    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult evalSplineDdfCompRes = vDdiffComputation->appendComputation(evalSplineDdfComputation, "{[0]}", "{[0]->[2]}", evalSplineDdfCompArgs);
    unsigned int newTuplePos = evalSplineDdfCompRes.tuplePosition+1;

    //Creating statement2
    //double eval_spline_ddf_return = Eval_Spline_ddf(r_eval,Windv_Spline);
    vDdiffComputation->addDataSpace("$eval_Spline_ddf_return$");
    Stmt* s2 = new Stmt("double $eval_Spline_ddf_return$ = "+evalSplineDdfCompRes.returnValues.back()+";",
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos)+"]}",
        {{evalSplineDdfCompRes.returnValues.back(), "{[0]->[0]}"}},
        {{"$eval_Spline_ddf_return$", "{[0]->[0]}"}}
    );
    vDdiffComputation->addStmt(s2);

    //Creating statement3
    //if(n1==0&&n2==0){v_ddiff_return = eval_spline_ddf_return;}
    Stmt* s3 = new Stmt("if($n1$==0 && $n2$==0){$v_ddiff_return$ = $eval_Spline_ddf_return$;}",
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos+1)+"]}",
        {
            {"$n1$", "{[0]->[0]}"},
            {"$n2$", "{[0]->[0]}"},
            {"$eval_Spline_ddf_return$", "{[0]->[0]}"}
        },
        {
            {"$v_ddiff_return$", "{[0]->[0]}"}
        }
    );
    vDdiffComputation->addStmt(s3);

    vDdiffComputation->addReturnValue("$v_ddiff_return$", true);

    return vDdiffComputation;
}


/*
double c_ddiff(double r, double theta, double phi, int n1, int n2, NaturalCubicSpline_1D &Temp_Spline){
    double r_eval = min(r, r_max);  r_eval = max(r_eval, r_min);    // Check that r_min <= r_eval <= r_max
    double c_ddiff_return$ = 0.0
    double SndSpd = c(r, theta, phi, Temp_Spline);
    double eval_spline_ddf_return = Eval_Spline_ddf(r_eval,Temp_Spline);
    double eval_spline_df_return = Eval_Spline_df(r_eval,Temp_Spline);

    if(n1==0 && n2==0){
        c_ddiff_return = gamR / (2.0 * SndSpd) * eval_spline_ddf_return
                - pow(gamR,2)/(4.0 * pow(SndSpd,3)) * pow(eval_spline_df_return,2);
    }
}
*/
Computation* c_ddiff_Computation(){
    Computation* cDdiffComputation = new Computation();
    cDdiffComputation->addParameter("$r$","double");
    cDdiffComputation->addParameter("$theta$","double");
    cDdiffComputation->addParameter("$phi$","double");
    cDdiffComputation->addParameter("$n1$","int");
    cDdiffComputation->addParameter("$n2$","int");
    //cDdiffComputation->addParameter("$Temp_Spline$","NaturalCubicSpline_1D &");
    cDdiffComputation->addParameter("$Temp_Spline_length$", "int");
    cDdiffComputation->addParameter("$Temp_Spline_accel$", "int");
    cDdiffComputation->addParameter("$Temp_Spline_x_vals$", "double*");
    cDdiffComputation->addParameter("$Temp_Spline_f_vals$", "double*");
    cDdiffComputation->addParameter("$Temp_Spline_slopes$", "double*");

    //Creating statement s0
    //double r_eval = min(r, r_max);  r_eval = max(r_eval, r_min);
    cDdiffComputation->addDataSpace("$r_eval$");
    Stmt* s0 = new Stmt("double $r_eval$ = min($r$, r_max); $r_eval$ = max($r_eval$, r_min);",
        "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
        "{[0]->[0]}", //Execution schedule - scheduling statement to be first (scheduling function)
        {{"$r$","{[0]->[0]}"},{"$r_eval$","{[0]->[0]}"}}, //Data reads
        {{"$r_eval$","{[0]->[0]}"}} //Data writes
        );
    //Adding s0 to the computation
    cDdiffComputation->addStmt(s0);

    //Creating statement s1
    //double c_ddiff_return = 0.0;
    cDdiffComputation->addDataSpace("$c_ddiff_return$");
    Stmt* s1 = new Stmt("double $c_ddiff_return$ = 0.0;",
        "{[0]}",
        "{[0]->[1]}",
        {},
        {{"$c_ddiff_return$","{[0]->[0]}"}}
        );
    cDdiffComputation->addStmt(s1);

    //Add the args of the c function (computation to be appended) as data spaces to cDiffComputation
    Computation* cComputation = c_Computation();

    //Args to the c_Computation
    vector<std::string> cCompArgs;
    cCompArgs.push_back("$r$");
    cCompArgs.push_back("$theta$");
    cCompArgs.push_back("$phi$");
    //cCompArgs.push_back("$Temp_Spline$");
    cCompArgs.push_back("$Temp_Spline_length$");
    cCompArgs.push_back("$Temp_Spline_accel$");
    cCompArgs.push_back("$Temp_Spline_x_vals$");
    cCompArgs.push_back("$Temp_Spline_f_vals$");
    cCompArgs.push_back("$Temp_Spline_slopes$");


    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult cCompRes = cDdiffComputation->appendComputation(cComputation, "{[0]}", "{[0]->[2]}", cCompArgs);
    unsigned int newTuplePos = cCompRes.tuplePosition+1;

    //Creating statement s2
    //double SndSpd = c(r, theta, phi, Temp_Spline);
    cDdiffComputation->addDataSpace("$SndSpd$");
    Stmt* s2 = new Stmt("double $SndSpd$ = "+cCompRes.returnValues.back()+";",
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos)+"]}",
        {{cCompRes.returnValues.back(), "{[0]->[0]}"}},
        {{"$SndSpd$", "{[0]->[0]}"}}
        );
    cDdiffComputation->addStmt(s2);

    Computation* evalSplineDdfComputation = Eval_Spline_ddf_Computation();

    //Args to the eval_Spline_ddf_Computation
    vector<std::string> evalSplineDdfCompArgs;
    evalSplineDdfCompArgs.push_back("$r_eval$");
    //evalSplineDdfCompArgs.push_back("$Temp_Spline$");
    evalSplineDdfCompArgs.push_back("$Temp_Spline_length$");
    evalSplineDdfCompArgs.push_back("$Temp_Spline_accel$");
    evalSplineDdfCompArgs.push_back("$Temp_Spline_x_vals$");
    evalSplineDdfCompArgs.push_back("$Temp_Spline_f_vals$");
    evalSplineDdfCompArgs.push_back("$Temp_Spline_slopes$");

    AppendComputationResult evalSplineDdfCompRes = cDdiffComputation->appendComputation(evalSplineDdfComputation, "{[0]}", "{[0]->["+std::to_string(newTuplePos+1)+"]}", evalSplineDdfCompArgs);
    newTuplePos = evalSplineDdfCompRes.tuplePosition+1;

    //Creating statement s3
    //double eval_spline_ddf_return = Eval_Spline_ddf(r_eval,Temp_Spline);
    cDdiffComputation->addDataSpace("$eval_spline_ddf_return$");
    Stmt* s3 = new Stmt("double $eval_spline_ddf_return$ = "+evalSplineDdfCompRes.returnValues.back()+";",
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos)+"]}",
        {{evalSplineDdfCompRes.returnValues.back(), "{[0]->[0]}"}},
        {{"$eval_spline_ddf_return$", "{[0]->[0]}"}}
        );
    cDdiffComputation->addStmt(s3);

    Computation* evalSplineDfComputation = Eval_Spline_df_Computation();

    //Args to the eval_Spline_df_Computation
    vector<std::string> evalSplineDfCompArgs;
    evalSplineDfCompArgs.push_back("$r_eval$");
    //evalSplineDfCompArgs.push_back("$Temp_Spline$");
    evalSplineDfCompArgs.push_back("$Temp_Spline_length$");
    evalSplineDfCompArgs.push_back("$Temp_Spline_accel$");
    evalSplineDfCompArgs.push_back("$Temp_Spline_x_vals$");
    evalSplineDfCompArgs.push_back("$Temp_Spline_f_vals$");
    evalSplineDfCompArgs.push_back("$Temp_Spline_slopes$");
    AppendComputationResult evalSplineDfCompRes = cDdiffComputation->appendComputation(evalSplineDfComputation, "{[0]}", "{[0]->["+std::to_string(newTuplePos+1)+"]}", evalSplineDfCompArgs);
    newTuplePos = evalSplineDfCompRes.tuplePosition+1;

    //Creating statement s4
    //double eval_spline_df_return = Eval_Spline_df(r_eval,Temp_Spline);
    cDdiffComputation->addDataSpace("$eval_spline_df_return$");
    Stmt* s4 = new Stmt("double $eval_spline_df_return$ = "+evalSplineDfCompRes.returnValues.back()+";",
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos)+"]}",
        {{evalSplineDfCompRes.returnValues.back(), "{[0]->[0]}"}},
        {{"$eval_spline_df_return$", "{[0]->[0]}"}}
        );
    cDdiffComputation->addStmt(s4);

    //Creating statement s4
    //if(n1==0 && n2==0){
    //    return gamR / (2.0 * SndSpd) * eval_spline_ddf_return
    //            - pow(gamR,2)/(4.0 * pow(SndSpd,3)) * pow(eval_spline_df_return,2);
    //}
    Stmt* s5 = new Stmt("if($n1$==0 && $n2$==0){$c_ddiff_return$ = gamR / (2.0 * $SndSpd$) * $eval_spline_ddf_return$ - pow(gamR,2)/(4.0*pow($SndSpd$,3)) * pow($eval_spline_df_return$,2);}",
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos+1)+"]}",
        {
            {"$n1$", "{[0]->[0]}"},
            {"$n2$", "{[0]->[0]}"},
            {"$SndSpd$", "{[0]->[0]}"},
            {"$eval_spline_ddf_return$", "{[0]->[0]}"},
            {"$eval_spline_df_return$", "{[0]->[0]}"}
        },
        {
            {"$c_ddiff_return$", "{[0]->[0]}"}
        }
        );
    cDdiffComputation->addStmt(s5);

    cDdiffComputation->addReturnValue("$c_ddiff_return$", true);

    return cDdiffComputation;
}



/*
double u_diff(double r, double theta, double phi int n, NaturalCubicSpline_1D &Windu_Spline){
    double r_eval = min(r, r_max);  r_eval = max(r_eval, r_min);    // Check that r_min <= r_eval <= r_max
    double u_diff_return = 0.0;
    double eval_Spline_df_return = Eval_Spline_df(r_eval, Windu_Spline);

    if(n==0){u_diff_return = eval_Spline_df_return;}
  }
*/
Computation* u_diff_Computation(){
    Computation* uDiffComputation = new Computation();

    uDiffComputation->addParameter("$r$", "double");
    uDiffComputation->addParameter("$theta$", "double");
    uDiffComputation->addParameter("$phi$","double");
    uDiffComputation->addParameter("$n$","int");
    //uDiffComputation->addParameter("$Windu_Spline$", "NaturalCubicSpline_1D &");
    uDiffComputation->addParameter("$Windu_Spline_length$", "int");
    uDiffComputation->addParameter("$Windu_Spline_accel$", "int");
    uDiffComputation->addParameter("$Windu_Spline_x_vals$", "double*");
    uDiffComputation->addParameter("$Windu_Spline_f_vals$", "double*");
    uDiffComputation->addParameter("$Windu_Spline_slopes$", "double*");

    //Creating statement1
    //double r_eval = min(r, r_max);  r_eval = max(r_eval, r_min);
    uDiffComputation->addDataSpace("$r_eval$");
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

    //double u_diff_return = 0.0;
    uDiffComputation->addDataSpace("$u_diff_return$");
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

    //Args to the eval_Spline_df_Computation
    vector<std::string> evalSplineDfCompArgs;
    evalSplineDfCompArgs.push_back("$r_eval$");
    //evalSplineDfCompArgs.push_back("$Windu_Spline$");
    evalSplineDfCompArgs.push_back("$Windu_Spline_length$");
    evalSplineDfCompArgs.push_back("$Windu_Spline_accel$");
    evalSplineDfCompArgs.push_back("$Windu_Spline_x_vals$");
    evalSplineDfCompArgs.push_back("$Windu_Spline_f_vals$");
    evalSplineDfCompArgs.push_back("$Windu_Spline_slopes$");
    
    AppendComputationResult evalSplineDfCompRes = uDiffComputation->appendComputation(evalSplineDfComputation, "{[0]}", "{[0]->[2]}", evalSplineDfCompArgs);// possible 0->0?
    unsigned int  newTuplePos = evalSplineDfCompRes.tuplePosition+1;

    //Creating statement s3
    //double eval_Spline_df_return = Eval_Spline_df(r_eval,Windu_Spline);
    uDiffComputation->addDataSpace("$eval_Spline_df_return$");
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
double v_diff(double r, double theta, double phi, int n, NaturalCubicSpline_1D & Windv_Spline){
  double r_eval = min(r, r_max);  r_eval = max(r_eval, r_min);    // Check that r_min <= r_eval <= r_max
  double v_diff_return = 0.0;
  double eval_spline_df_return = Eval_Spline_df(r_eval,Windv_Spline);
  if(n==0) { v_diff_return =  eval_spline_df_return;}
}
 */
Computation* v_diff_Computation(){
    Computation* vDiffComputation = new Computation();
    vDiffComputation->addParameter("$r$","double");
    vDiffComputation->addParameter("$theta$","double");
    vDiffComputation->addParameter("$phi$","double");
    vDiffComputation->addParameter("$n$","int");
    //vDiffComputation->addParameter("$Windv_Spline$", "NaturalCubicSpline_1D &");
    vDiffComputation->addParameter("$Windv_Spline_length$", "int");
    vDiffComputation->addParameter("$Windv_Spline_accel$", "int");
    vDiffComputation->addParameter("$Windv_Spline_x_vals$", "double*");
    vDiffComputation->addParameter("$Windv_Spline_f_vals$", "double*");
    vDiffComputation->addParameter("$Windv_Spline_slopes$", "double*");

    //Creating statement0
    //double r_eval = min(r, r_max);  r_eval = max(r_eval, r_min);
    vDiffComputation->addDataSpace("$r_eval$");
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

    //Creating statement1
    //double v_diff_return = 0.0;
    vDiffComputation->addDataSpace("$v_diff_return$");
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
    //evalSplineDfCompArgs.push_back("$Windv_Spline$");
    evalSplineDfCompArgs.push_back("$Windv_Spline_length$");
    evalSplineDfCompArgs.push_back("$Windv_Spline_accel$");
    evalSplineDfCompArgs.push_back("$Windv_Spline_x_vals$");
    evalSplineDfCompArgs.push_back("$Windv_Spline_f_vals$");
    evalSplineDfCompArgs.push_back("$Windv_Spline_slopes$");


    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult evalSplineDfCompRes = vDiffComputation->appendComputation(evalSplineDfComputation, "{[0]}", "{[0]->[2]}", evalSplineDfCompArgs);
    unsigned int newTuplePos = evalSplineDfCompRes.tuplePosition+1;

    //Creating statement2
    //double eval_Spline_df_return = Eval_Spline_df(r_eval,Windv_Spline);
    vDiffComputation->addDataSpace("$eval_Spline_df_return$");
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
    //cDiffComputation->addParameter("$Temp_Spline$","NaturalCubicSpline_1D &");
    cDiffComputation->addParameter("$Temp_Spline_length$", "int");
    cDiffComputation->addParameter("$Temp_Spline_accel$", "int");
    cDiffComputation->addParameter("$Temp_Spline_x_vals$", "double*");
    cDiffComputation->addParameter("$Temp_Spline_f_vals$", "double*");
    cDiffComputation->addParameter("$Temp_Spline_slopes$", "double*");

    //Creating statement1
    //double r_eval = min(r, r_max);  r_eval = max(r_eval, r_min);
    cDiffComputation->addDataSpace("$r_eval$");
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
    //cCompArgs.push_back("$Temp_Spline$");
    cCompArgs.push_back("$Temp_Spline_length$");
    cCompArgs.push_back("$Temp_Spline_accel$");
    cCompArgs.push_back("$Temp_Spline_x_vals$");
    cCompArgs.push_back("$Temp_Spline_f_vals$");
    cCompArgs.push_back("$Temp_Spline_slopes$");

    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult cCompRes = cDiffComputation->appendComputation(cComputation, "{[0]}", "{[0]->[2]}", cCompArgs);
    unsigned int newTuplePos = cCompRes.tuplePosition+1;

    //double c_return = c(r,theta,phi, Temp_Spline);
    cDiffComputation->addDataSpace("$c_return$");
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

    //Args to the eval_Spline_df_Computation
    vector<std::string> evalSplineDfCompArgs;
    evalSplineDfCompArgs.push_back("$r_eval$");
    //evalSplineDfCompArgs.push_back("$Temp_Spline$");
    evalSplineDfCompArgs.push_back("$Temp_Spline_length$");
    evalSplineDfCompArgs.push_back("$Temp_Spline_accel$");
    evalSplineDfCompArgs.push_back("$Temp_Spline_x_vals$");
    evalSplineDfCompArgs.push_back("$Temp_Spline_f_vals$");
    evalSplineDfCompArgs.push_back("$Temp_Spline_slopes$");

    AppendComputationResult evalSplineDfCompRes = cDiffComputation->appendComputation(evalSplineDfComputation, "{[0]}", "{[0]->["+std::to_string(newTuplePos+1)+"]}", evalSplineDfCompArgs);
    newTuplePos = evalSplineDfCompRes.tuplePosition+1;

    //double eval_spline_df_return = Eval_Spline_df(r_eval,Temp_Spline);
    cDiffComputation->addDataSpace("$eval_spline_df_return$");
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
    //uComputation->addParameter("$Windu_Spline$","NaturalCubicSpline_1D &");
    uComputation->addParameter("$Windu_Spline_length$", "int");
    uComputation->addParameter("$Windu_Spline_accel$", "int");
    uComputation->addParameter("$Windu_Spline_x_vals$", "double*");
    uComputation->addParameter("$Windu_Spline_f_vals$", "double*");
    uComputation->addParameter("$Windu_Spline_slopes$", "double*");

    //Creating statement1
    //double r_eval = min(r, r_max);  r_eval = max(r_eval, r_min);
    uComputation->addDataSpace("$r_eval$");
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
    //eSpFArgs.push_back("$Windu_Spline$");
    eSpFArgs.push_back("$Windu_Spline_length$");
    eSpFArgs.push_back("$Windu_Spline_accel$");
    eSpFArgs.push_back("$Windu_Spline_x_vals$");
    eSpFArgs.push_back("$Windu_Spline_f_vals$");
    eSpFArgs.push_back("$Windu_Spline_slopes$");

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
    //vComputation->addParameter("$Windv_Spline$","NaturalCubicSpline_1D &");
    vComputation->addParameter("$Windv_Spline_length$", "int");
    vComputation->addParameter("$Windv_Spline_accel$", "int");
    vComputation->addParameter("$Windv_Spline_x_vals$", "double*");
    vComputation->addParameter("$Windv_Spline_f_vals$", "double*");
    vComputation->addParameter("$Windv_Spline_slopes$", "double*");

    //Creating statement1
    //double r_eval = min(r, r_max);  r_eval = max(r_eval, r_min);
    vComputation->addDataSpace("$r_eval$");
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
    //eSpFArgs.push_back("$Windv_Spline$");
    eSpFArgs.push_back("$Windv_Spline_length$");
    eSpFArgs.push_back("$Windv_Spline_accel$");
    eSpFArgs.push_back("$Windv_Spline_x_vals$");
    eSpFArgs.push_back("$Windv_Spline_f_vals$");
    eSpFArgs.push_back("$Windv_Spline_slopes$");


    //Call Eval_Spline_f_Computation() returen values is stored in result
    Computation* EvalSplineFComputation = Eval_Spline_f_Computation();
    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult eSpFCompRes = vComputation->appendComputation(EvalSplineFComputation, "{[0]}","{[0]->[1]}",eSpFArgs);

    unsigned int newTuplePos = eSpFCompRes.tuplePosition+1;

    //double v_return = Eval_Spline_f(r_eval, Windv_Spline);
    vComputation->addDataSpace("$v_return$");
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
    //cComputation->addParameter("$Temp_Spline$","NaturalCubicSpline_1D &");
    cComputation->addParameter("$Temp_Spline_length$", "int");
    cComputation->addParameter("$Temp_Spline_accel$", "int");
    cComputation->addParameter("$Temp_Spline_x_vals$", "double*");
    cComputation->addParameter("$Temp_Spline_f_vals$", "double*");
    cComputation->addParameter("$Temp_Spline_slopes$", "double*");

    //Creating statement1
    //double r_eval = min(r, r_max);  r_eval = max(r_eval, r_min);
    cComputation->addDataSpace("$r_eval$");
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
    //eSpFArgs.push_back("$Temp_Spline$");
    eSpFArgs.push_back("$Temp_Spline_length$");
    eSpFArgs.push_back("$Temp_Spline_accel$");
    eSpFArgs.push_back("$Temp_Spline_x_vals$");
    eSpFArgs.push_back("$Temp_Spline_f_vals$");
    eSpFArgs.push_back("$Temp_Spline_slopes$");

    //Call Eval_Spline_f_Computation()
    //Call this computation only once and reuse when needed
    Computation* EvalSplineFComputation = Eval_Spline_f_Computation();

    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult eSpFCompRes = cComputation->appendComputation(EvalSplineFComputation, "{[0]}","{[0]->[2]}",eSpFArgs);

    unsigned int newTuplePos = eSpFCompRes.tuplePosition+1;

    //Creating statement3
    //double c_result = sqrt(gamR * Eval_Spline_f(r_eval,Temp_Spline));
    cComputation->addDataSpace("$c_result$");
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
      int k = Find_Segment(x, Spline_x_vals, Spline_length, Spline_accel);

      //cout << "Eval: k = " << k << " spline length = " << Spline_length << endl;
      //cout << "Passed Find_Segment()" << endl;

      if(k < Spline_length){
          double X = (x - Spline_x_vals[k])/(Spline_x_vals[k+1] - Spline_x_vals[k]);
          double A = Spline_slopes[k] * (Spline_x_vals[k+1] - Spline_x_vals[k]) - (Spline_f_vals[k+1] - Spline_f_vals[k]);
          double B = -Spline_slopes[k+1] * (Spline_x_vals[k+1] - Spline_x_vals[k]) + (Spline_f_vals[k+1] - Spline_f_vals[k]);
          //cout << "Eval: X = " << X << " A = " << A << " B = " << B << endl;

          //cout << "Ready to return" << endl;
          double result = (1.0 - X) * Spline_f_vals[k] + X * Spline_f_vals[k+1] + X * (1.0 - X) * (A * (1.0 - X ) + B * X);
          //cout << "Eval: result = " << result << endl;
          return result;
      } else { return 0.0;}
  }
*/
Computation* Eval_Spline_f_Computation(){

    Computation* evalSplineFComputation = new Computation();
    evalSplineFComputation->addParameter("$x$", "double");
    //evalSplineFComputation->addParameter("$Spline$", "struct NaturalCubicSpline_1D &");
    evalSplineFComputation->addParameter("$Spline_length$", "int");
    evalSplineFComputation->addParameter("$Spline_accel$", "int");
    evalSplineFComputation->addParameter("$Spline_x_vals$", "double*");
    evalSplineFComputation->addParameter("$Spline_f_vals$", "double*");
    evalSplineFComputation->addParameter("$Spline_slopes$", "double*");

    Computation* FindSegmentComputation= Find_Segment_Computation();

    //Args to the Find_Segment_Computation
    vector<std::string> findSegArgs;
    findSegArgs.push_back("$x$");
    findSegArgs.push_back("$Spline_x_vals$");
    findSegArgs.push_back("$Spline_length$");
    findSegArgs.push_back("$Spline_accel$");

    // Return values are stored in a struct of the AppendComputationResult data structure
    AppendComputationResult fSegCompRes = evalSplineFComputation->appendComputation(FindSegmentComputation, "{[0]}", "{[0]->[0]}", findSegArgs);

    unsigned int newTuplePos = fSegCompRes.tuplePosition+1;
    
    //Creating s1
    evalSplineFComputation->addDataSpace("$k$");
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

    //Creating s2
    evalSplineFComputation->addDataSpace("$eval_spline_f_return$");
    Stmt* s2 = new Stmt("double $eval_spline_f_return$ = 0.0;",
        "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
        "{[0]->["+std::to_string(newTuplePos+1)+"]}", //Execution schedule - scheduling statement to be first (scheduling function)
        {},
        {{"$eval_spline_f_return$", "{[0]->[0]}"}}
        );
    cout << "Source statement : " << s2->getStmtSourceCode() << "\n\t"
      <<"- Iteration Space : "<< s2->getIterationSpace()->prettyPrintString() << "\n\t"
      << "- Execution Schedule : "<< s2->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

    //Adding s2
    evalSplineFComputation->addStmt(s2);

    //Creating s3 --> the entire if block, slightly modified
    evalSplineFComputation->addDataSpace("$X$");
    evalSplineFComputation->addDataSpace("$A$");
    evalSplineFComputation->addDataSpace("$B$");
    Stmt* s3 = new Stmt("if($k$ < $Spline_length$) { double $X$ = ($x$ - $Spline_x_vals$[$k$])/($Spline_x_vals$[$k$+1] - $Spline_x_vals$[$k$]); double $A$ = $Spline_slopes$[$k$] * ($Spline_x_vals$[$k$+1] - $Spline_x_vals$[$k$]) - ($Spline_f_vals$[$k$+1] - $Spline_f_vals$[$k$]); double $B$ = -$Spline_slopes$[$k$+1] * ($Spline_x_vals$[$k$+1] - $Spline_x_vals$[$k$]) + ($Spline_f_vals$[$k$+1] - $Spline_f_vals$[$k$]); $eval_spline_f_return$ = (1.0 - $X$) * $Spline_f_vals$[$k$] + $X$ * $Spline_f_vals$[$k$+1] + $X$ * (1.0 - $X$) * ($A$ * (1.0 - $X$ ) + $B$ * $X$);}",
        "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
        "{[0]->["+std::to_string(newTuplePos+2)+"]}", //Execution schedule - scheduling statement to be first (scheduling function)
        {
            {"$k$","{[0]->[0]}"},
            {"$Spline_length$", "{[0]->[0]}"},
            {"$x$","{[0]->[0]}"},
            {"$Spline_x_vals$","{[0]->[k1]: k1 = $k$}"},
            {"$Spline_x_vals$","{[0]->[kp1]: kp1 = $k$+1}"},
            {"$Spline_slopes$","{[0]->[x1]: x1 = $k$}"},
            {"$Spline_f_vals$","{[0]->[k1]: k1 = $k$}"},
            {"$Spline_f_vals$","{[0]->[kp1]: kp1 = $k$+1}"},
            {"$Spline_slopes$","{[0]->[kp1]: kp1 = $k$+1}"},
            {"$X$","{[0]->[0]}"},
            {"$A$","{[0]->[0]}"},
            {"$B$","{[0]->[0]}"}
        },//Data reads
        {
            {"$X$", "{[0]->[0]}"},
            {"$A$", "{[0]->[0]}"},
            {"$B$", "{[0]->[0]}"},
            {"$eval_spline_f_return$", "{[0]->[0]}"},
        } //Data writes
        );
    cout << "Source statement : " << s3->getStmtSourceCode() << "\n\t"
      <<"- Iteration Space : "<< s3->getIterationSpace()->prettyPrintString() << "\n\t"
      << "- Execution Schedule : "<< s3->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

    //Adding s3
    evalSplineFComputation->addStmt(s3);
    evalSplineFComputation->addReturnValue("$eval_spline_f_return$", true);


    return evalSplineFComputation;
}


/*
   double Eval_Spline_df(double x, struct NaturalCubicSpline_1D & Spline){
       int k = Find_Segment(x, Spline_x_vals, Spline_length, Spline_accel);
       double eval_spline_df_return = 0.0;

       if(k < Spline_length){
           double X = (x - Spline_x_vals[k])/(Spline_x_vals[k+1] - Spline_x_vals[k]);
           double A = Spline_slopes[k] * (Spline_x_vals[k+1] - Spline_x_vals[k]) - (Spline_f_vals[k+1] - Spline_f_vals[k]);
           double B = -Spline_slopes[k+1] * (Spline_x_vals[k+1] - Spline_x_vals[k]) + (Spline_f_vals[k+1] - Spline_f_vals[k]);

           eval_spline_df_return = (Spline_f_vals[k+1] - Spline_f_vals[k])/(Spline_x_vals[k+1] - Spline_x_vals[k])
           + (1.0 - 2.0 * X) * (A * (1.0 - X) + B * X)/(Spline_x_vals[k+1] - Spline_x_vals[k])
           + X * (1.0 - X) * (B - A)/(Spline_x_vals[k+1] - Spline_x_vals[k]);
        }
   }
 */
Computation* Eval_Spline_df_Computation(){
    Computation* evalSplineDfComputation = new Computation();
    evalSplineDfComputation->addParameter("$x$", "double");
    //evalSplineDfComputation->addParameter("$Spline$", "NaturalCubicSpline_1D &");
    evalSplineDfComputation->addParameter("$Spline_length$", "int");
    evalSplineDfComputation->addParameter("$Spline_accel$", "int");
    evalSplineDfComputation->addParameter("$Spline_x_vals$", "double*");
    evalSplineDfComputation->addParameter("$Spline_f_vals$", "double*");
    evalSplineDfComputation->addParameter("$Spline_slopes$", "double*");

    Computation* findSegmentComputation= Find_Segment_Computation();
    
    vector<std::string> findSegArgs;
    findSegArgs.push_back("$x$");
    findSegArgs.push_back("$Spline_x_vals$");
    findSegArgs.push_back("$Spline_length$");
    findSegArgs.push_back("$Spline_accel$");
    
    //Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult fSegCompRes = evalSplineDfComputation->appendComputation(findSegmentComputation, "{[0]}", "{[0]->[0]}", findSegArgs);

    unsigned int newTuplePos = fSegCompRes.tuplePosition+1;
    
    //Creating s0
    //int k = Find_Segment(x, Spline_x_vals, Spline_length, Spline_accel);
    evalSplineDfComputation->addDataSpace("$k$");
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

    //Creating s1
    evalSplineDfComputation->addDataSpace("$eval_spline_df_return$");
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

    //Creating s2
    evalSplineDfComputation->addDataSpace("$X$");
    evalSplineDfComputation->addDataSpace("$A$");
    evalSplineDfComputation->addDataSpace("$B$");
    Stmt* s2 = new Stmt("if($k$ < $Spline_length$){double $X$ = ($x$ - $Spline_x_vals$[$k$])/($Spline_x_vals$[$k$+1] - $Spline_x_vals$[$k$]); double $A$ = $Spline_slopes$[$k$] * ($Spline_x_vals$[$k$+1] -$Spline_x_vals$[$k$]) - ($Spline_f_vals$[$k$+1] - $Spline_f_vals$[$k$]); double $B$ = -$Spline_slopes$[$k$+1] * ($Spline_x_vals$[$k$+1] - $Spline_x_vals$[$k$]) + ($Spline_f_vals$[$k$+1] - $Spline_f_vals$[$k$]); $eval_spline_df_return$ = ($Spline_f_vals$[$k$+1] - $Spline_f_vals$[$k$])/($Spline_x_vals$[$k$+1] - $Spline_x_vals$[$k$]) + (1.0 - 2.0 * $X$) * ($A$ * (1.0 - $X$) + $B$ * $X$)/($Spline_x_vals$[$k$+1] - $Spline_x_vals$[$k$])+ $X$ * (1.0 - $X$) * ($B$ - $A$)/($Spline_x_vals$[$k$+1] - $Spline_x_vals$[$k$]);}",
        "{[0]}", //Iteration Schedule
        "{[0]->["+std::to_string(newTuplePos+2)+"]}", //Execution schedule
        {
            {"$k$","{[0]->[0]}"},
            {"$Spline_length$", "{[0]->[0]}"},
            {"$x$","{[0]->[0]}"},
            {"$Spline_x_vals$","{[0]->[k1]: k1 = $k$}"},
            {"$Spline_x_vals$","{[0]->[kp1]: kp1 = $k$+1}"},
            {"$Spline_slopes$","{[0]->[k1]: k1 = $k$}"},
            {"$Spline_f_vals$","{[0]->[k1]: k1 = $k$}"},
            {"$Spline_slopes$","{[0]->[kp1]: kp1 = $k$+1}"},
            {"$Spline_f_vals$","{[0]->[kp1]: kp1 = $k$+1}"},
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
    int k = Find_Segment(x, Spline_x_vals, Spline_length, Spline_accel);
    double result = 0.0;
    if(k < Spline_length){
        double X = (x - Spline_x_vals[k])/(Spline_x_vals[k+1] - Spline_x_vals[k]);
        double A = Spline_slopes[k] * (Spline_x_vals[k+1] - Spline_x_vals[k]) - (Spline_f_vals[k+1] - Spline_f_vals[k]);
        double B = -Spline_slopes[k+1] * (Spline_x_vals[k+1] - Spline_x_vals[k]) + (Spline_f_vals[k+1] - Spline_f_vals[k]);

        result =  2.0 * (B - 2.0 * A + (A - B) * 3.0 * X)/pow(Spline_x_vals[k+1] - Spline_x_vals[k],2);
    }
}
*/
Computation* Eval_Spline_ddf_Computation(){
    Computation* evalSplineDdfComputation = new Computation();
    evalSplineDdfComputation->addParameter("$x$", "double");
    //evalSplineDdfComputation->addParameter("$Spline$", "struct NaturalCubicSpline_1D &");
    evalSplineDdfComputation->addParameter("$Spline_length$", "int");
    evalSplineDdfComputation->addParameter("$Spline_accel$", "int");
    evalSplineDdfComputation->addParameter("$Spline_x_vals$", "double*");
    evalSplineDdfComputation->addParameter("$Spline_f_vals$", "double*");
    evalSplineDdfComputation->addParameter("$Spline_slopes$", "double*");

    Computation* findSegmentComputation= Find_Segment_Computation();
    
    //Args to the Find_Segment_Computation
    vector<std::string> findSegArgs;
    findSegArgs.push_back("$x$");
    findSegArgs.push_back("$Spline_x_vals$");
    findSegArgs.push_back("$Spline_length$");
    findSegArgs.push_back("$Spline_accel$");

    //Return values are stored in a struct of the AppendComputationResult data structure
    AppendComputationResult fSegCompRes = evalSplineDdfComputation->appendComputation(findSegmentComputation, "{[0]}", "{[0]->[0]}", findSegArgs);

    unsigned int newTuplePos = fSegCompRes.tuplePosition + 1;
    
    //Creating s1
    //int k = Find_Segment(x, Spline_x_vals, Spline_length, Spline_accel);
    evalSplineDdfComputation->addDataSpace("$k$");
    Stmt* s1 = new Stmt("int $k$ = "+fSegCompRes.returnValues.back()+";",
        "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
        "{[0]->["+std::to_string(newTuplePos)+"]}", //Execution schedule - scheduling statement to be first (scheduling function)
        {{fSegCompRes.returnValues.back(), "{[0]->[0]}"}},
        {{"$k$", "{[0]->[0]}"}}
    );
    //Adding s1
    evalSplineDdfComputation->addStmt(s1);

    //Creating s2
    evalSplineDdfComputation->addDataSpace("$eval_Spline_ddf_return$");
    Stmt* s2 = new Stmt(
        "double $eval_Spline_ddf_return$ = 0.0;",
        "{[0]}",  //Iteration schedule
        "{[0]->["+std::to_string(newTuplePos+1)+"]}", //Execution schedule
        {},
        {{"$eval_Spline_ddf_return$", "{[0]->[0]}"}}
    );
    //Adding s2
    evalSplineDdfComputation->addStmt(s2);
    
    //Creating s3 --> the entire if block, slightly modified
    evalSplineDdfComputation->addDataSpace("$X$");
    evalSplineDdfComputation->addDataSpace("$A$");
    evalSplineDdfComputation->addDataSpace("$B$");
    Stmt* s3 = new Stmt(
    "if($k$ < $Spline_length$){ double $X$ = ($x$ - $Spline_x_vals$[$k$])/($Spline_x_vals$[$k$+1] - $Spline_x_vals$[$k$]);double $A$ = $Spline_slopes$[$k$] * ($Spline_x_vals$[$k$+1] - $Spline_x_vals$[$k$]) - ($Spline_f_vals$[$k$+1] - $Spline_f_vals$[$k$]);double $B$ = -$Spline_slopes$[$k$+1] * ($Spline_x_vals$[$k$+1] - $Spline_x_vals$[$k$]) + ($Spline_f_vals$[$k$+1] - $Spline_f_vals$[$k$]); $eval_Spline_ddf_return$ = 2.0 * ($B$ - 2.0 * $A$ + ($A$ - $B$) * 3.0 * $X$) / pow($Spline_x_vals$[$k$+1] - $Spline_x_vals$[$k$], 2);}",
        "{[0]}",  //Iteration schedule
        "{[0]->["+std::to_string(newTuplePos+2)+"]}", //Execution schedule
        {
            {"$k$","{[0]->[0]}"},
            {"$x$","{[0]->[0]}"},
            {"$Spline_length$", "{[0]->[0]}"},
            {"$Spline_x_vals$","{[0]->[k1]: k1 = $k$}"},
            {"$Spline_x_vals$","{[0]->[kp1]: kp1 = $k$+1}"},
            {"$Spline_slopes$","{[0]->[x1]: x1 = $k$}"},
            {"$Spline_f_vals$","{[0]->[kp1]: kp1 = $k$+1}"},
            {"$Spline_f_vals$","{[0]->[k1]: k1 = $k$}"},
            {"$Spline_slopes$","{[0]->[kp1]: kp1 = $k$+1}"},
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
        "{[i]: i>=0 && i<$length$-1}",  //Iteration schedule
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


