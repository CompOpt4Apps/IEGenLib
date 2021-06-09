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
    /* 
    // double r = current_values[0],             theta = current_values[1],      phi = current_values[2];
    Stmt * s01 = new Stmt("double $r$ = $current_values$[0], $theta$=$current_values$[1], $phi$=$current_values$[2];",
         "{[0]}",
         "{[0]->[0]}",
         {{"$current_values$[0]","{[0]->[0]}"},
          {"$current_values$[1]","{[0]->[1]}"},
          {"$current_values$[2]","{[0]->[2]}"}},
         {{"$r$", "{[0]->[0]}"},
          {"$theta$", "{[0]->[0]}"},
          {"$phi$", "{[0]->[0]}"}}
         );
    updateSources->addStmt(s01);
  
    //double nu[3] = {current_values[3],        current_values[4],              current_values[5]};
    updateSources->addDataSpace("$nu$");
    Stmt * s02 = new Stmt("double $nu$[3] = {$current_values$[3], $current_values$[4], $current_values$[5]};",
         "{[0]}",
         "{[0]->[1]",
         {{"$current_values$[3]","{[0]->[3]}"},
          {"$current_values$[4]","{[0]->[4]}"},
          {"$current_values$[5]","{[0]->[5]}"}},
         {{"$nu$", "{[0]->[3]}"}}
         );
    updateSources->addStmt(s02);
    */

    Computation* cComputation = c_Computation();
  
    //Add the args of the c function (computation to be appended) as data spaces to temp
    updateSources->addDataSpace("$spl.Temp_Spline$");
  
    //Args to the c_Computation
    vector<std::string> cCompArgs;
    cCompArgs.push_back("$r$");
    cCompArgs.push_back("$theta$");
    cCompArgs.push_back("$phi$");
    cCompArgs.push_back("$spl.Temp_Spline$");

    // Return values are stored in a struct of the computation: AppendComputationResult
    AppendComputationResult cCompRes = updateSources->appendComputation(cComputation, "{[0]}", "{[0]->[2]}", cCompArgs); // set to 2
 
    unsigned int newTuplePos = cCompRes.tuplePosition+1;
    updateSources->addDataSpace("$sources.c$");
    //Creating s1
    //sources.c = c(r,theta,phi,spl.Temp_Spline);
    Stmt*s1 = new Stmt("$sources.c$ = "+cCompRes.returnValues.back()+";", 
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
    Stmt*s2 = new Stmt("$sources.w$ = 0;", 
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
    AppendComputationResult vCompRes = updateSources->appendComputation(vComputation, "{[0]}", "{[0]->["+std::to_string(cCompRes.tuplePosition+3)+"]}", vCompArgs);
    
    updateSources->addDataSpace("$sources.v$");
    //Creating s3
    //sources.v = v(r,theta,phi,spl.Temp_Spline);
    Stmt*s3 = new Stmt("$sources.v$ = "+vCompRes.returnValues.back()+";", 
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos+2)+"]}",
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
    AppendComputationResult uCompRes = updateSources->appendComputation(uComputation, "{[0]}", "{[0]->["+std::to_string(vCompRes.tuplePosition+2)+"]}", uCompArgs);
    
    newTuplePos = uCompRes.tuplePosition+1;
    updateSources->addDataSpace("$sources.u$");
    //Creating s3=4
    //sources.u = u(r,theta,phi, spl.Windu_Spline);
    Stmt*s4 = new Stmt("$sources.u$ = "+uCompRes.returnValues.back()+";", 
        "{[0]}",
        "{[0]->["+std::to_string(newTuplePos)+"]}",
        {{uCompRes.returnValues.back(), "{[0]->[0]}"}},
        {{"$sources.u$", "{[0]->[0]}"}}
        );
    
    updateSources->addStmt(s4);
  
    
    /*
    //Calling toDot() on the Computation structure
    fstream dotFileStream;
    cout << "Entering toDot()" << "\n";
    temp->toDot(dotFileStream,"c_dot.txt");
    */
  
    //Write codegen to a file
    ofstream outStream;
    outStream.open("codegen.c");
    outStream << updateSources->codeGen();
    outStream.close();
  
    return 0;
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
double c_diff(double r, double theta, double phi, int n, NaturalCubicSpline_1D &Temp_Spline){
    double r_eval = min(r, r_max);  r_eval = max(r_eval, r_min);    // Check that r_min <= r_eval <= r_max
    
    if(n==0){   return gamR / (2.0 * c(r,theta,phi, Temp_Spline)) * Eval_Spline_df(r_eval,Temp_Spline);}
    else {      return 0.0;}
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
    Stmt* s1 = new Stmt("double $r_eval$ = min($r$, r_max); $r_eval$ = max($r_eval$, r_min)",
        "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
        "{[0]->[0, 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
        {{"$r$","{[0]->[0]}"},{"$r_eval$","{[0]->[0]}"}}, //Data reads
        {{"$r_eval$","{[0]->[0]}"}} //Data writes
        );
    cout << "Source statement : " << s1->getStmtSourceCode() << "\n\t"
      <<"- Iteration Space : "<< s1->getIterationSpace()->prettyPrintString() << "\n\t"
      << "- Execution Schedule : "<< s1->getExecutionSchedule()->prettyPrintString() << "\n\t" ;
  
    //Adding statement1
    cDiffComputation->addStmt(s1);
    
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
  
    //Call Eval_Spline_f_Computation() returen values is stored in result
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
      
      return Eval_Spline_f(r_eval, Windv_Spline);
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
  
    //Add return value to the current computation
    vComputation->addReturnValue(eSpFCompRes.returnValues.back(), true);
  
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
    evalSplineFComputation->addDataSpace("$x$");
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
       double eval_Spline_df_return = 0.0;
        
       if(k < Spline.length){
           double X = (x - Spline.x_vals[k])/(Spline.x_vals[k+1] - Spline.x_vals[k]);
           double A = Spline.slopes[k] * (Spline.x_vals[k+1] - Spline.x_vals[k]) - (Spline.f_vals[k+1] - Spline.f_vals[k]);
           double B = -Spline.slopes[k+1] * (Spline.x_vals[k+1] - Spline.x_vals[k]) + (Spline.f_vals[k+1] - Spline.f_vals[k]);
   
           eval_Spline_df_return = (Spline.f_vals[k+1] - Spline.f_vals[k])/(Spline.x_vals[k+1] - Spline.x_vals[k])
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
    findSegArgs.push_back("Spline.length");
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
  
   evalSplineDfComputation->addDataSpace("$eval_Spline_df_return$");
   //Creating s1
   Stmt* s1 = new Stmt("double $eval_Spline_df_return$ = 0.0;",
       "{[0]}",  
       "{[0]->["+std::to_string(newTuplePos+1)+"]}",
       {}, 
       {{"$eval_Spline_df_return$", "{[0]->[0]}"}}  
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
   
   Stmt * s2 = new Stmt("if($k$ < $Spline.length$){double $X$ = ($x$ - $Spline.x_vals$[$k$])/($Spline.x_vals$[$k$+1] - $Spline.x_vals$[$k$]}; double $A$ = $Spline.slopes$[$k$] * ($Spline.x_vals$[$k$+1] -$Spline.x_vals$[$k$]) - ($Spline.f_vals$[$k$+1] - $Spline.f_vals$[$k$]); double $B$ = -$Spline.slopes$[$k$+1] * ($Spline.x_vals$[$k$+1] - $Spline.x_vals$[$k$]) + ($Spline.f_vals$[$k$+1] - $Spline.f_vals$[$k$]); $eval_Spline_df_return$ = ($Spline.f_vals$[$k$+1] - $Spline.f_vals$[$k$])/($Spline.x_vals$[$k$+1] - $Spline.x_vals$[$k$]) + (1.0 - 2.0 * $X$) * ($A$ * (1.0 - $X$) + $B$ * $X$)/($Spline.x_vals$[$k$+1] - $Spline.x_vals$[$k$])+ $X$ * (1.0 - $X$) * ($B$ - $A$)/($Spline.x_vals$[$k$+1] - $Spline.x_vals$[$k$]);",
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
           {"$eval_Spline_df_return$", "{[0]->[0]}"
        }} //Data writes
    );
    cout << "Source statement : " << s2->getStmtSourceCode() << "\n\t"
      <<"- Iteration Space : "<< s2->getIterationSpace()->prettyPrintString() << "\n\t"
      << "- Execution Schedule : "<< s2->getExecutionSchedule()->prettyPrintString() << "\n\t" ;
   //Adding statement S2
   evalSplineDfComputation->addStmt(s2);
   evalSplineDfComputation->addReturnValue("eval_Spline_df_return",true);
  
  
   return evalSplineDfComputation;
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



