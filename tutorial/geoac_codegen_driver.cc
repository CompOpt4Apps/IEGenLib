#include "iegenlib.h"
#include <utility>
#include <fstream>
#include <iostream>
#include <sstream>

using iegenlib::Computation;
using namespace std;

Computation* Find_Segment_Computation();
Computation* Eval_Spline_f_Computation();

Computation* c_Computation();
Computation* v_Computation();

/**
 * Main function
 */
int main(int argc, char **argv){

  Computation* temp = new Computation();
  Computation* cComputation = c_Computation();

  //Add the args of the c function (computation to be appended) as data spaces to temp
  temp->addDataSpace("$r$");
  temp->addDataSpace("$theta$");
  temp->addDataSpace("$phi$");
  temp->addDataSpace("$spl.Temp_Spline$");

  //Args to the c_Computation
  vector<std::string> cCompArgs;
  cCompArgs.push_back("$r$");
  cCompArgs.push_back("$theta$");
  cCompArgs.push_back("$phi$");
  cCompArgs.push_back("$spl.Temp_Spline$");



  // Return values are stored in a struct of the computation: AppendComputationResult
  AppendComputationResult cCompRes = temp->appendComputation(cComputation, "{[0]}", "{[0]->[0]}", cCompArgs);

  Stmt*s1 = new Stmt("$sources.c$ = "+cCompRes.returnValues.back()+";", 
      "{[0]}",
      "{[0]->["+std::to_string(cCompRes.tuplePosition+1)+", 0, 0]}",
      {{cCompRes.returnValues.back(), "{[0]->[0]}"}},
      {{"$sources.c$", "{[0]->[0]}"}}
      );
  
  temp->addStmt(s1);


  // Computation* EvalSplineF = Eval_Spline_f_Computation();
  
  // vector < pair<string, string> > dataReads;
  // vector < pair<string, string> > dataWrites;
  //Statement 1 = {"C","[k]->[k]"};
  //dataReads.push_back(make_pair(" "," "));
  //dataWrites.push_back(make_pair("C","{[k]->[k]}"));


  //cout << FindSegment.codeGen() <<endl;

  //Calling toDot() on the Computation structure
  /*
  fstream dotFileStream;
  cout << "Entering toDot()" << "\n";
  temp->toDot(dotFileStream,"c_dot.txt");
  */

  //Write codegen to a file
  ofstream outStream;
  outStream.open("c_codegen.cpp");
  outStream << temp->codeGen();
  outStream.close();

  return 0;
}


/*
  double v(double r, double theta, double phi, NaturalCubicSpline_1D &Windv_Spline){
    double r_eval = min(r, r_max);  r_eval = max(r_eval, r_min);    // Check that r_min <= r_eval <= r_max
    
    return Eval_Spline_f(r_eval, Windv_Spline);
}
*/
Computation* v_Computation(){
  
  Computation* VComputation;
  VComputation->addParameter("$r$","double");
  VComputation->addParameter("$theta$","double");
  VComputation->addParameter("$phi$","double");
  VComputation->addParameter("$Windv_Spline$","NaturalCubicSpline_1D &");

  //Creating statement1
  //double r_eval = min(r, r_max);
  Stmt* s1 = new Stmt("double $r_eval$ = min($r$, r_max); $r_eval$ = max($r_eval$, r_min)",
      "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[0]->[0, 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {{"$r$","{[0]->[0]}"},{"$r_eval$","{[0]->[0]}"}}, //Data reads
      {{"$r_eval$","{[0]->[0]}"}} //Data writes
      );
  cout << "Source statement : " << s1->getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s1->getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s1->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding statement2
  VComputation->addStmt(s1);

  //Args to the Eval_Spline_f_Computation
  vector<std::string> eSpFArgs;
  eSpFArgs.push_back("$r_eval$");
  eSpFArgs.push_back("$Windv_Spline$");


  //Call Eval_Spline_f_Computation() returen values is stored in result
  Computation* EvalSplineFComputation = Eval_Spline_f_Computation();
  // Return values are stored in a struct of the computation: AppendComputationResult
  AppendComputationResult eSpFCompRes = VComputation->appendComputation(EvalSplineFComputation, "{[0]}","{[0]->[1, 0, 0]}",eSpFArgs);

  //Add return value to the current computation
  VComputation->addReturnValue(eSpFCompRes.returnValues.back(), true);


  return VComputation;
}




/*
  double c(double r, double theta, double phi, NaturalCubicSpline_1D &Temp_Spline){
    //cout << "c: r_max = " << r_max << ", r_min = " << r_min << ", gamR = " << gamR << endl;
    double r_eval = min(r, r_max);  r_eval = max(r_eval, r_min);    // Check that r_min <= r_eval <= r_max
    //cout << "c: r_eval = " << r_eval << endl;
    double result = sqrt(gamR * Eval_Spline_f(r_eval,Temp_Spline));
    //cout << "c: result = " << result << endl;
    return result;
  }
*/
Computation* c_Computation(){
  
  Computation* CComputation = new Computation();
  CComputation->addParameter("$r$","double");
  CComputation->addParameter("$theta$","double");
  CComputation->addParameter("$phi$","double");
  CComputation->addParameter("$Temp_Spline$","NaturalCubicSpline_1D &");

  //Creating statement1
  //double r_eval = min(r, r_max);
  Stmt* s1 = new Stmt("double $r_eval$ = min($r$, r_max);",
      "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[0]->[0, 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {{"$r$","{[0]->[0]}"}}, //Data reads
      {{"$r_eval$","{[0]->[0]}"}} //Data writes
      );
  cout << "Source statement : " << s1->getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s1->getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s1->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding statement1
  CComputation->addStmt(s1);

  //Creating statement2
  //r_eval = max(r_eval, r_min);
  Stmt* s2 = new Stmt("$r_eval$ = max($r_eval$, r_min);",
      "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[0]->[1, 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {{"$r_eval$","{[0]->[0]}"}}, //Data reads
      {{"$r_eval$","{[0]->[0]}"}} //Data writes
      );
  cout << "Source statement : " << s2->getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s2->getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s2->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding statement2
  CComputation->addStmt(s2);

  //Args to the Eval_Spline_f_Computation
  vector<std::string> eSpFArgs;
  eSpFArgs.push_back("$r_eval$");
  eSpFArgs.push_back("$Temp_Spline$");

  //Call Eval_Spline_f_Computation() 
  //Call this computation only once and reuse when needed
  Computation* EvalSplineFComputation = Eval_Spline_f_Computation();

  // Return values are stored in a struct of the computation: AppendComputationResult
  AppendComputationResult eSpFCompRes = CComputation->appendComputation(EvalSplineFComputation, "{[0]}","{[0]->[2, 0, 0]}",eSpFArgs);


  //Creating statement3
  //double result = sqrt(gamR * Eval_Spline_f(r_eval,Temp_Spline));
  Stmt* s3 = new Stmt("double $result$ = sqrt(gamR * "+eSpFCompRes.returnValues.back()+");",
      "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[0]->["+std::to_string(eSpFCompRes.tuplePosition+1)+", 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {{eSpFCompRes.returnValues.back(), "{[0]->[0]}"}}, //Data reads
      {{"$result$", "{[0]->[0]}"}} //Data writes
      );
  cout << "Source statement : " << s3->getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s3->getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s3->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding statement3b
  CComputation->addStmt(s3);
  CComputation->addReturnValue("$result$", true);

  
  return CComputation;
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

  Computation* EvalSplineFComputation = new Computation();
  EvalSplineFComputation->addParameter("$x$", "double");
  EvalSplineFComputation->addParameter("$Spline$", "struct NaturalCubicSpline_1D &");
  

  Computation* FindSegmentComputation= Find_Segment_Computation();

  //Add the args of the FindSegment function (computation to be appended) as data spaces to EvalSplineFComputation
  EvalSplineFComputation->addDataSpace("$x$");
  EvalSplineFComputation->addDataSpace("$Spline.x_vals$");
  EvalSplineFComputation->addDataSpace("$Spline.length$");
  EvalSplineFComputation->addDataSpace("$Spline.accel$");

  //Args to the Find_Segment_Computation
  vector<std::string> findSegArgs;
  findSegArgs.push_back("$x$");
  findSegArgs.push_back("$Spline.x_vals$");
  findSegArgs.push_back("$Spline.length$");
  findSegArgs.push_back("$Spline.accel$");

  // Return values are stored in a struct of the AppendComputationResult data structure
  AppendComputationResult fSegCompRes = EvalSplineFComputation->appendComputation(FindSegmentComputation, "{[0]}", "{[0]->[0, 0, 0]}", findSegArgs);

  //Creating s1
  Stmt* s1a = new Stmt("int $k$ = "+fSegCompRes.returnValues.back()+";",
      "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[0]->["+std::to_string(fSegCompRes.tuplePosition+1)+", 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {{fSegCompRes.returnValues.back(), "{[0]->[0]}"}}, 
      {{"$k$", "{[0]->[0]}"}}  
      );  
  cout << "Source statement : " << s1a->getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s1a->getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s1a->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding s1
  EvalSplineFComputation->addStmt(s1a);

  //Creating s2
  Stmt* s2 = new Stmt("double $result$ = 0.0;",
      "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[0]->["+std::to_string(fSegCompRes.tuplePosition+2)+", 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {}, 
      {{"$result$", "{[0]->[0]}"}}  
      );  
  cout << "Source statement : " << s2->getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s2->getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s2->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding s2
  EvalSplineFComputation->addStmt(s2);  

  //Creating s3 --> the entire if block, slightly modified
  Stmt* s3 = new Stmt("if($k$ < $Spline.length$) { double $X$ = ($x$ - $Spline.x_vals$[k])/($Spline.x_vals$[k+1] - $Spline.x_vals$[k]); double $A$ = $Spline.slopes$[k] * ($Spline.x_vals$[k+1] - $Spline.x_vals$[k]) - ($Spline.f_vals$[k+1] - $Spline.f_vals$[k]); double $B$ = -$Spline.slopes$[k+1] * ($Spline.x_vals$[k+1] - $Spline.x_vals$[k]) + ($Spline.f_vals$[k+1] - $Spline.f_vals$[k]); $result$ = (1.0 - $X$) * $Spline.f_vals$[k] + $X$ * $Spline.f_vals$[k+1] + $X$ * (1.0 - $X$) * ($A$ * (1.0 - $X$ ) + $B$ * $X$);}",
      "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[0]->["+std::to_string(fSegCompRes.tuplePosition+3)+", 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {{"$x$","{[0]->[0]}"}, {"Spline.x_vals","{[k]->[k]}"}, {"$Spline.x_vals$","{[k]->[kp1]: kp1 = k+1}"}, 
       {"$Spline.slopes$","{[k]->[k]}"}, {"$Spline.f_vals$","{[k]->[k]}"}, {"$Spline.f_vals$","{[k]->[kp1]: kp1 = k+1}"}, 
       {"$Spline.slopes$","{[k]->[kp1]: kp1 = k+1}"}, 
       {"$X$","{[0]->[0]}"}, {"$A$","{[0]->[0]}"}, {"$B$","{[0]->[0]}"}},//Data reads
      {{"$X$", "{[0]->[0]}"}, {"$A$", "{[0]->[0]}"}, {"$B$", "{[0]->[0]}"}, {"$result$", "{[0]->[0]}"}, } //Data writes
      );
  cout << "Source statement : " << s3->getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s3->getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s3->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding s3
  EvalSplineFComputation->addStmt(s3);
  EvalSplineFComputation->addReturnValue("$result$", true);


  return EvalSplineFComputation;
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

  Computation* FindSegmentComputation = new Computation();
  FindSegmentComputation->addParameter("$x$", "double");
  FindSegmentComputation->addParameter("$x_vals$", "double*");
  FindSegmentComputation->addParameter("$length$", "int");
  FindSegmentComputation->addParameter("$prev$", "int&");

  //Creating s0
  //if(x >= x_vals[i] && x <= x_vals[i+1]) prev = i;
  Stmt*  s0 = new Stmt("if($x$ >= $x_vals$[i] && $x$ <= $x_vals$[i+1]) $prev$ = i;",
      "{[i]: i>=0 && i<length}",  //Iteration schedule - Only happening one time (not iterating)
      "{[i]->[0, i, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {{"$x$","{[0]->[0]}"}, {"$x_vals$","{[i]->[i]}"}, {"$x_vals$","{[i]->[ip1]: ip1 = i+1}"}}, //Reads
      {{"$prev$","{[0]->[0]}"}}   //writes
      );  
  cout << "Source statement : " << s0->getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s0->getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s0->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding s0
  FindSegmentComputation->addStmt(s0);
  FindSegmentComputation->addReturnValue("$prev$", true);

  return FindSegmentComputation;
}



