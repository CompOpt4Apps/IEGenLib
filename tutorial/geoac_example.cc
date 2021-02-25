#include "iegenlib.h"
#include <utility>
#include <fstream>
#include <iostream>

using iegenlib::Computation;
using namespace std;

int main(void){
  return 0;
}

////void GeoAc_UpdateSources(double ray_length, double* current_values, 
////                         GeoAc_Sources_Struct &sources, 
////                         SplineStruct &spl)
//Computation GeoAc_UpdateSources_Computation(){
//  Computation MyComp;
//
//
//  //Creating statement1
//  //double r = current_values[0],   theta = current_values[1],  phi = current_values[2];
//  Stmt statement1("double r = current_values[0],   theta = current_values[1],  phi = current_values[2]",
//      "{[]}",  //Iteration schedule - Only happening one time (not iterating)
//      "{[]->[0, 0, 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
//      dataReads,
//      dataWrites
//      );
//  cout << "Source statement : " << statement1.getStmtSourceCode() << "\n\t"
//    <<"- Iteration Space : "<< statement1.getIterationSpace()->prettyPrintString() << "\n\t"
//    << "- Execution Schedule : "<< statement1.getExecutionSchedule()->prettyPrintString() << "\n\t" ;
//
//  //Adding statement1
//  MyComp.addStmt(statement1);
//  
//  //Creating statement2
//  //double nu[3] = {current_values[3],  current_values[4],    current_values[5]};
//  Stmt statement2("double nu[3] = {current_values[3],  current_values[4],    current_values[5]}",
//      "{[]}",  //Iteration schedule - Only happening one time (not iterating)
//      "{[]->[0, 0, 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
//      dataReads,
//      dataWrites
//      );
//  cout << "Source statement : " << statement2.getStmtSourceCode() << "\n\t"
//    <<"- Iteration Space : "<< statement2.getIterationSpace()->prettyPrintString() << "\n\t"
//    << "- Execution Schedule : "<< statement2.getExecutionSchedule()->prettyPrintString() << "\n\t" ;
//
//  //Adding statement2
//  MyComp.addStmt(statement2);
//
//  return MyComp;
//}//
//
//
///*double c(double r, double theta, double phi, NaturalCubicSpline_1D &Temp_Spline){
//    //cout << "c: r_max = " << r_max << ", r_min = " << r_min << ", gamR = " << gamR << endl;
//    double r_eval = min(r, r_max);  r_eval = max(r_eval, r_min);    // Check that r_min <= r_eval <= r_max
//    //cout << "c: r_eval = " << r_eval << endl;
//    double result = sqrt(gamR * Eval_Spline_f(r_eval,Temp_Spline));
//    //cout << "c: result = " << result << endl;
//    return result;
//}
//*/
//Computation c(){
//  Computation MyComp;
//
//
//  //Creating statement1
//  Stmt statement1("cout << "c: r_max = " << r_max << ", r_min = " << r_min << ", gamR = " << gamR << endl",
//      "{[]}",  //Iteration schedule - Only happening one time (not iterating)
//      "{[]->[0, 0, 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
//      dataReads,
//      dataWrites
//      );
//  cout << "Source statement : " << statement1.getStmtSourceCode() << "\n\t"
//    <<"- Iteration Space : "<< statement1.getIterationSpace()->prettyPrintString() << "\n\t"
//    << "- Execution Schedule : "<< statement1.getExecutionSchedule()->prettyPrintString() << "\n\t" ;
//
//  //Adding statement1
//  MyComp.addStmt(statement1);
//  
//  return MyComp;
//}
//
///*
//double c_diff(double r, double theta, double phi, int n, NaturalCubicSpline_1D &Temp_Spline){
//    double r_eval = min(r, r_max);  r_eval = max(r_eval, r_min);    // Check that r_min <= r_eval <= r_max
//    
//    if(n==0){   return gamR / (2.0 * c(r,theta,phi, Temp_Spline)) * Eval_Spline_df(r_eval,Temp_Spline);}
//    else {      return 0.0;}
//}
//*/
//Computation c_diff(){
//  Computation MyComp;
//
//
//  //Creating statement1
//  Stmt statement1("double r_eval = min(r, r_max);  r_eval = max(r_eval, r_min)",
//      "{[]}",  //Iteration schedule - Only happening one time (not iterating)
//      "{[]->[0, 0, 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
//      dataReads,
//      dataWrites
//      );
//  cout << "Source statement : " << statement1.getStmtSourceCode() << "\n\t"
//    <<"- Iteration Space : "<< statement1.getIterationSpace()->prettyPrintString() << "\n\t"
//    << "- Execution Schedule : "<< statement1.getExecutionSchedule()->prettyPrintString() << "\n\t" ;
//
//  //Adding statement1
//  MyComp.addStmt(statement1);
//
//  return MyComp;
//}
//
//
//
//
//int main(int argc, char **argv){
//
//  /*
//     double* GeoAc_Propagate_RK4_2(double* & solution, double& r_max, 
//     double& travel_time, double& attenuation, 
//     double GeoAc_theta, double GeoAc_phi, 
//     double freq, bool CalcAmp, 
//     GeoAc_Sources_Struct &sources, 
//     SplineStruct &splines, 
//     ofstream* raypaths, ofstream* caustics){
//
//     int k = 0;  // Integer to track ending index of solution
//
//    // Limiting number of steps, 
//    // defined by limiting ray length divided by step size (length/step)
//    int step_limit = GeoAc_ray_limit * int(1.0/(GeoAc_ds_min*10)); 
//
//  */
//
//  Computation MyComp;
//  vector < pair<string, string> > dataReads;
//  vector < pair<string, string> > dataWrites;
//
//  //Statement 1 = {"C","[k]->[k]"};
//  //dataReads.push_back(make_pair(" "," "));
//  //dataWrites.push_back(make_pair("C","{[k]->[k]}"));
//
//
//  //Creating statement1
//  Stmt statement1("int k = 0",
//      "{[]}",  //Iteration schedule - Only happening one time (not iterating)
//      "{[]->[0, 0, 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
//      dataReads,
//      dataWrites
//      );
//  cout << "Source statement : " << statement1.getStmtSourceCode() << "\n\t"
//    <<"- Iteration Space : "<< statement1.getIterationSpace()->prettyPrintString() << "\n\t"
//    << "- Execution Schedule : "<< statement1.getExecutionSchedule()->prettyPrintString() << "\n\t" ;
//
//  //Adding statement1
//  MyComp.addStmt(statement1);
//
//  //Creating statement2
//  //int step_limit = GeoAc_ray_limit * int(1.0/(GeoAc_ds_min*10));
//  Stmt statement2("int step_limit = GeoAc_ray_limit * int(1.0/(GeoAc_ds_min*10))",
//      "{[]}",             //Iteration schedule
//      "{[]->[1,0,0,0]}",  //Execuion schedule
//      {},                 //Data reads (empty vector becauser no support for scalars yet)
//      {}                  //Data writes (same reason)
//      );
//  cout << "Source statement : " << statement2.getStmtSourceCode() << "\n\t"
//    <<"- Iteration Space : "<< statement2.getIterationSpace()->prettyPrintString() << "\n\t"
//    << "- Execution Schedule : "<< statement2.getExecutionSchedule()->prettyPrintString() << "\n\t" ;
//
//  //Adding statement2
//  MyComp.addStmt(statement2);
//
//
//  //Creating statement3
//  //double s = 0, ds = GeoAc_ds_min;
//  Stmt statement3("double s = 0, ds = GeoAc_ds_min",
//      "{[]}",
//      "{[]->[2,0,0,0]}",
//      {},
//      {}
//      );
//  cout << "Source statement : " << statement3.getStmtSourceCode() << "\n\t"
//    <<"- Iteration Space : "<< statement3.getIterationSpace()->prettyPrintString() << "\n\t"
//    << "- Execution Schedule : "<< statement3.getExecutionSchedule()->prettyPrintString() << "\n\t" ;
//
//  MyComp.addStmt(statement3);
//
//  /*
//     Creating statement4
//
//     double *temp0 = new double [GeoAc_EqCnt], 
//            *temp1 = new double [GeoAc_EqCnt], 
//            *temp2 = new double [GeoAc_EqCnt], 
//            *temp3 = new double [GeoAc_EqCnt], 
//            *temp4 = new double [GeoAc_EqCnt];
//   */
//  Stmt statement4("double *temp0 = new double [GeoAc_EqCnt], *temp1 = new double [GeoAc_EqCnt], *temp2 = new double [GeoAc_EqCnt], *temp3 = new double [GeoAc_EqCnt], *temp4 = new double [GeoAc_EqCnt]",
//      "{[]}",
//      "{[]->[3,0,0,0]}",
//      {},
//      {}
//      );
//  cout << "Source statement : " << statement4.getStmtSourceCode() << "\n\t"
//    <<"- Iteration Space : "<< statement4.getIterationSpace()->prettyPrintString() << "\n\t"
//    << "- Execution Schedule : "<< statement4.getExecutionSchedule()->prettyPrintString() << "\n\t" ;
//
//  MyComp.addStmt(statement4);
//
//
//  /*
//     Creating statement5
//
//     double *partial1 = new double [GeoAc_EqCnt], 
//            *partial2 = new double [GeoAc_EqCnt],
//            *partial3 = new double [GeoAc_EqCnt];
//   */
//  Stmt statement5("double *partial1 = new double [GeoAc_EqCnt], *partial2 = new double [GeoAc_EqCnt], *partial3 = new double [GeoAc_EqCnt]",
//      "{[]}",
//      "{[]->[4,0,0,0]}",
//      {},
//      {}
//      );
//  cout << "Source statement : " << statement5.getStmtSourceCode() << "\n\t"
//    <<"- Iteration Space : "<< statement5.getIterationSpace()->prettyPrintString() << "\n\t"
//    << "- Execution Schedule : "<< statement5.getExecutionSchedule()->prettyPrintString() << "\n\t" ;
//
//  MyComp.addStmt(statement5);
//
//  //Creating statement6
//  //double* next = new double[GeoAc_EqCnt];
//  Stmt statement6("double* next = new double[GeoAc_EqCnt]",
//      "{[]}",
//      "{[]->[5,0,0,0]}",
//      {},
//      {}
//      );
//  cout << "Source statement : " << statement6.getStmtSourceCode() << "\n\t"
//    <<"- Iteration Space : "<< statement6.getIterationSpace()->prettyPrintString() << "\n\t"
//    << "- Execution Schedule : "<< statement6.getExecutionSchedule()->prettyPrintString() << "\n\t" ;
//
//  MyComp.addStmt(statement6);
//
//  //Creating statement7
//  //double* current = new double[GeoAc_EqCnt];
//  Stmt statement7("double* current = new double[GeoAc_EqCnt]",
//      "{[]}",
//      "{[]->[6,0,0,0]}",
//      {},
//      {}
//      );
//  cout << "Source statement : " << statement7.getStmtSourceCode() << "\n\t"
//    <<"- Iteration Space : "<< statement7.getIterationSpace()->prettyPrintString() << "\n\t"
//    << "- Execution Schedule : "<< statement7.getExecutionSchedule()->prettyPrintString() << "\n\t" ;
//
//  MyComp.addStmt(statement7);
//
//  //Creating statement8
//  //for (int i = 0; i < GeoAc_EqCnt; i++) current[i] = solution[i];
//  Stmt statement8("current[i] = solution[i]",
//      "{[i]: i<GeoAc_EqCnt && i>=0}",
//      "{[i]->[7,i,0,0]}",
//      {{"solution","{[i]->[i]}"}},
//      {{"current","{[i]->[i]}"}}
//      );
//  cout << "Source statement : " << statement8.getStmtSourceCode() << "\n\t"
//    <<"- Iteration Space : "<< statement8.getIterationSpace()->prettyPrintString() << "\n\t"
//    << "- Execution Schedule : "<< statement8.getExecutionSchedule()->prettyPrintString() << "\n\t" ;
//
//  MyComp.addStmt(statement8);
//
//  //Creating statement9
//  //double dr, dt, dp, ds_, r, t, p, nu[3], nu_mag, c_prop[3], c_prop_mag;
//  Stmt statement9("double dr, dt, dp, ds_, r, t, p, nu[3], nu_mag, c_prop[3], c_prop_mag",
//      "{[]}",
//      "{[i]->[8,0,0,0]}",
//      {},
//      {}
//      );
//  cout << "Source statement : " << statement9.getStmtSourceCode() << "\n\t"
//    <<"- Iteration Space : "<< statement9.getIterationSpace()->prettyPrintString() << "\n\t"
//    << "- Execution Schedule : "<< statement9.getExecutionSchedule()->prettyPrintString() << "\n\t" ;
//
//  MyComp.addStmt(statement9);
//
//  //Creating statement10
//  //double r_max_cpy = max(r_max, solution[0] - r_earth);
//  Stmt statement10("double r_max_cpy = max(r_max, solution[0] - r_earth)",
//      "{[]}",
//      "{[i]->[9,0,0,0]}",
//      {{"solution","{[0]->[0]}"}},
//      {}
//      );
//  cout << "Source statement : " << statement10.getStmtSourceCode() << "\n\t"
//    <<"- Iteration Space : "<< statement10.getIterationSpace()->prettyPrintString() << "\n\t"
//    << "- Execution Schedule : "<< statement10.getExecutionSchedule()->prettyPrintString() << "\n\t" ;
//
//  MyComp.addStmt(statement10);
//
//  //Creating statement11
//  //double D, D_prev;
//  Stmt statement11("double D, D_prev",
//      "{[]}",
//      "{[i]->[10,0,0,0]}",
//      {},
//      {}
//      );
//  cout << "Source statement : " << statement11.getStmtSourceCode() << "\n\t"
//    <<"- Iteration Space : "<< statement11.getIterationSpace()->prettyPrintString() << "\n\t"
//    << "- Execution Schedule : "<< statement11.getExecutionSchedule()->prettyPrintString() << "\n\t" ;
//
//  MyComp.addStmt(statement11);
//
//  //Creating statement12
//  //bool WriteRays = raypaths != nullptr;
//  Stmt statement12("bool WriteRays = raypaths != nullptr",
//      "{[]}",
//      "{[i]->[11,0,0,0]}",
//      {},
//      {}
//      );
//  cout << "Source statement : " << statement12.getStmtSourceCode() << "\n\t"
//    <<"- Iteration Space : "<< statement12.getIterationSpace()->prettyPrintString() << "\n\t"
//    << "- Execution Schedule : "<< statement12.getExecutionSchedule()->prettyPrintString() << "\n\t" ;
//
//  MyComp.addStmt(statement12);
//
//  //Creating statement13
//  //bool WriteCaustics = caustics != nullptr;
//  Stmt statement13("bool WriteCaustics = caustics != nullptr",
//      "{[]}",
//      "{[i]->[12,0,0,0]}",
//      {},
//      {}
//      );
//  cout << "Source statement : " << statement13.getStmtSourceCode() << "\n\t"
//    <<"- Iteration Space : "<< statement13.getIterationSpace()->prettyPrintString() << "\n\t"
//    << "- Execution Schedule : "<< statement13.getExecutionSchedule()->prettyPrintString() << "\n\t" ;
//
//  MyComp.addStmt(statement13);
//
//  //Creating statement14
//  //bool check = false;
//  Stmt statement14("bool check = false",
//      "{[]}",
//      "{[i]->[13,0,0,0]}",
//      {},
//      {}
//      );
//  cout << "Source statement : " << statement14.getStmtSourceCode() << "\n\t"
//    <<"- Iteration Space : "<< statement14.getIterationSpace()->prettyPrintString() << "\n\t"
//    << "- Execution Schedule : "<< statement14.getExecutionSchedule()->prettyPrintString() << "\n\t" ;
//
//  MyComp.addStmt(statement14);
//
//  /*
//     Creating statement15
//    
//     for (k = 0; k < step_limit - 1; k++) {
//       for (int i = 0; i < GeoAc_EqCnt; i++){
//         temp0[i] = current[i];
//   */
//  Stmt statement15("temp0[i] = current[i]",
//      "{[k,i] : 0<=k<step_limit-1 && 0<=i<GeoAc_EqCnt}",
//      "{[k,i]->[14,k,0,i,0]}",
//      {{"current[i]","{[k,i]->[i]}"}},
//      {{"temp0","{[k,i]->[i]}"}}
//      );
//  cout << "Source statement : " << statement15.getStmtSourceCode() << "\n\t"
//    <<"- Iteration Space : "<< statement15.getIterationSpace()->prettyPrintString() << "\n\t"
//    << "- Execution Schedule : "<< statement15.getExecutionSchedule()->prettyPrintString() << "\n\t" ;
//
//  MyComp.addStmt(statement15);
//
//  //TODO: We are now in the outer for loop so the execution schedule
//  //will remain 14(14th grouping) and the loop variable will be k
//  //until we exit this loop: [k,other-loop]->[14,k,stmt-position,other-loop,stmt-position]
//  //Creating statement16
//  //goto funciton and fndout the names of the params:
//  //double ray_length, double* current_values, GeoAc_Sources_Struct &sources,
//  //        SplineStruct &spl
//  //GeoAc_UpdateSources(s, temp0, sources, splines);
//  Statement newStatement("double ray_length = s),
//            {}
//            {k 15 ....}
//            {}//reads
//            {}//writes
//           _; // DO this  times for each of the params 
//  //TODO Call the function and append the returned computation
//  /*
//    Computation updateSourcesComputation = GeoAc_UpdateSources_Computation();
//    MyComp.appendComputation(updateSourcesComputation);
//  */
//  Stmt statement16("GeoAc_UpdateSources(s, temp0, sources, splines);",
//      "{[k] : 0<=k<step_limit-1}",
//      "{[k]->[14,k,1,0,0]}",
//      {},
//      {}
//      );
//  cout << "Source statement : " << statement16.getStmtSourceCode() << "\n\t"
//    <<"- Iteration Space : "<< statement16.getIterationSpace()->prettyPrintString() << "\n\t"
//    << "- Execution Schedule : "<< statement16.getExecutionSchedule()->prettyPrintString() << "\n\t" ;
//
//  MyComp.addStmt(statement16);
//
//
//  //TODO
//  //Creating statement17
//  Stmt statement17("temp0[i] = current[i]",
//      "{[k,i] : 0<=k<step_limit-1 && 0<=i<GeoAc_EqCnt}",
//      "{[k,i]->[14,k,0,i,0]}",
//      {{"current[i]","{[k,i]->[i]}"}},
//      {{"temp0","{[k,i]->[i]}"}}
//      );
//  cout << "Source statement : " << statement17.getStmtSourceCode() << "\n\t"
//    <<"- Iteration Space : "<< statement17.getIterationSpace()->prettyPrintString() << "\n\t"
//    << "- Execution Schedule : "<< statement17.getExecutionSchedule()->prettyPrintString() << "\n\t" ;
//
//  MyComp.addStmt(statement17);
//
//  //TODO
//  //Creating statement18
//  Stmt statement18("temp0[i] = current[i]",
//      "{[k,i] : 0<=k<step_limit-1 && 0<=i<GeoAc_EqCnt}",
//      "{[k,i]->[14,k,0,i,0]}",
//      {{"current[i]","{[k,i]->[i]}"}},
//      {{"temp0","{[k,i]->[i]}"}}
//      );
//  cout << "Source statement : " << statement18.getStmtSourceCode() << "\n\t"
//    <<"- Iteration Space : "<< statement18.getIterationSpace()->prettyPrintString() << "\n\t"
//    << "- Execution Schedule : "<< statement18.getExecutionSchedule()->prettyPrintString() << "\n\t" ;
//
//  MyComp.addStmt(statement18);
//
//  //TODO
//  //Creating statement19
//  Stmt statement19("temp0[i] = current[i]",
//      "{[k,i] : 0<=k<step_limit-1 && 0<=i<GeoAc_EqCnt}",
//      "{[k,i]->[14,k,0,i,0]}",
//      {{"current[i]","{[k,i]->[i]}"}},
//      {{"temp0","{[k,i]->[i]}"}}
//      );
//  cout << "Source statement : " << statement19.getStmtSourceCode() << "\n\t"
//    <<"- Iteration Space : "<< statement19.getIterationSpace()->prettyPrintString() << "\n\t"
//    << "- Execution Schedule : "<< statement19.getExecutionSchedule()->prettyPrintString() << "\n\t" ;
//
//  MyComp.addStmt(statement19);
//
//  //TODO
//  //Creating statement20
//  Stmt statement20("temp0[i] = current[i]",
//      "{[k,i] : 0<=k<step_limit-1 && 0<=i<GeoAc_EqCnt}",
//      "{[k,i]->[14,k,0,i,0]}",
//      {{"current[i]","{[k,i]->[i]}"}},
//      {{"temp0","{[k,i]->[i]}"}}
//      );
//  cout << "Source statement : " << statement20.getStmtSourceCode() << "\n\t"
//    <<"- Iteration Space : "<< statement20.getIterationSpace()->prettyPrintString() << "\n\t"
//    << "- Execution Schedule : "<< statement20.getExecutionSchedule()->prettyPrintString() << "\n\t" ;
//
//  MyComp.addStmt(statement20);
//
//  //TODO
//  //Creating statement21
//  Stmt statement21("temp0[i] = current[i]",
//      "{[k,i] : 0<=k<step_limit-1 && 0<=i<GeoAc_EqCnt}",
//      "{[k,i]->[14,k,0,i,0]}",
//      {{"current[i]","{[k,i]->[i]}"}},
//      {{"temp0","{[k,i]->[i]}"}}
//      );
//  cout << "Source statement : " << statement21.getStmtSourceCode() << "\n\t"
//    <<"- Iteration Space : "<< statement21.getIterationSpace()->prettyPrintString() << "\n\t"
//    << "- Execution Schedule : "<< statement21.getExecutionSchedule()->prettyPrintString() << "\n\t" ;
//
//  MyComp.addStmt(statement21);
//
//  //TODO
//  //Creating statement22
//  Stmt statement22("temp0[i] = current[i]",
//      "{[k,i] : 0<=k<step_limit-1 && 0<=i<GeoAc_EqCnt}",
//      "{[k,i]->[14,k,0,i,0]}",
//      {{"current[i]","{[k,i]->[i]}"}},
//      {{"temp0","{[k,i]->[i]}"}}
//      );
//  cout << "Source statement : " << statement22.getStmtSourceCode() << "\n\t"
//    <<"- Iteration Space : "<< statement22.getIterationSpace()->prettyPrintString() << "\n\t"
//    << "- Execution Schedule : "<< statement22.getExecutionSchedule()->prettyPrintString() << "\n\t" ;
//
//  MyComp.addStmt(statement22);
//
//
//  //cout << MyComp.codeGen() <<endl;
//
//  //Calling toDot() on the Computation structure
//  fstream dotFileStream;
//  cout << "Entering toDot()" << "\n";
//  MyComp.toDot(dotFileStream,"geoac.txt");
//
//  //Write codegen to a file
//  ofstream outStream;
//  outStream.open("geoac_codegen.cpp");
//  outStream << MyComp.codeGen();
//  outStream.close();
//
//  return 0;
//
//
//  /*
//  //Statement 2 = C[i+j] += A[i] * B[j];
//  dataReads.push_back(make_pair("A","{[i,j]->[i]}"));
//  dataReads.push_back(make_pair("B","{[i,j]->[j]}"));
//  dataReads.push_back(make_pair("C","{[i,j]->[k]:k=i+j}"));
//  dataWrites.push_back(make_pair("C","{[i,j]->[k]:k=i+j}"));
//
//  //Creating statement2
//  Stmt statement2("C[i+j] = C[i+j] + A[i] * B[j]",
//  "{[i,j] : 0<=i<n && 0<=j<n}",
//  "{[i,j] -> [1, i, j]}",
//  dataReads,
//  dataWrites
//  );
//  cout << "Source statement : " << statement2.getStmtSourceCode() << "\n\t"
//  <<"- Iteration Space : "<< statement2.getIterationSpace()->prettyPrintString() << "\n\t"
//  << "- Execution Schedule : "<< statement2.getExecutionSchedule()->prettyPrintString() << "\n\t" ;
//
//  //Adding statement2
//  MyComp.addStmt(statement2);
//  cout << "Stmt2 initialised" << "\n";
//
//  //Calling toDot() on the Computation structure
//  fstream dotFileStream;
//  cout << "Entering toDot()" << "\n";
//  MyComp.toDot(dotFileStream,"Example.txt");
//  */
//
//}
