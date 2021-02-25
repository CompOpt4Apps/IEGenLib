#include "iegenlib.h"
#include <utility>
#include <fstream>
#include <iostream>

using iegenlib::Computation;
using namespace std;

Computation* Find_Segment_Computation();
Computation* Eval_Spline_f_Computation();


/**
 * Main function
 */
int main(int argc, char **argv){


  Computation* EvalSplineF = Eval_Spline_f_Computation();
  
  // vector < pair<string, string> > dataReads;
  // vector < pair<string, string> > dataWrites;
  //Statement 1 = {"C","[k]->[k]"};
  //dataReads.push_back(make_pair(" "," "));
  //dataWrites.push_back(make_pair("C","{[k]->[k]}"));


  //cout << FindSegment.codeGen() <<endl;

  //Calling toDot() on the Computation structure
  fstream dotFileStream;
  cout << "Entering toDot()" << "\n";
  EvalSplineF->toDot(dotFileStream,"eval_spline_f_dot.txt");

  //Write codegen to a file
  ofstream outStream;
  outStream.open("eval_spline_f_codegen.cpp");
  outStream << EvalSplineF->codeGen();
  outStream.close();

  return 0;
}


/*
  double Eval_Spline_f(double x, struct NaturalCubicSpline_1D & Spline){
    //cout << "Called Eval_Spline_f" << endl;
    //cout << "Eval: x = " << x << endl;

    int k = Find_Segment(x, Spline.x_vals, Spline.length, Spline.accel);

    //cout << "Eval: k = " << k << " spline length = " << Spline.length << endl;   
    //cout << "Passed Find_Segment()" << endl;

    if(k >= Spline.length) return 0.0;
    
    //if(k < Spline.length)
    double X = (x - Spline.x_vals[k])/(Spline.x_vals[k+1] - Spline.x_vals[k]);
    double A = Spline.slopes[k] * (Spline.x_vals[k+1] - Spline.x_vals[k]) - (Spline.f_vals[k+1] - Spline.f_vals[k]);
    double B = -Spline.slopes[k+1] * (Spline.x_vals[k+1] - Spline.x_vals[k]) + (Spline.f_vals[k+1] - Spline.f_vals[k]);

    //cout << "Eval: X = " << X << " A = " << A << " B = " << B << endl;
    //cout << "Ready to return" << endl;

    double result = (1.0 - X) * Spline.f_vals[k] + X * Spline.f_vals[k+1] + X * (1.0 - X) * (A * (1.0 - X ) + B * X);
    //cout << "Eval: result = " << result << endl;
    return result;
  }
*/
Computation* Eval_Spline_f_Computation(){

  Computation* EvalSplineFComputation = new Computation();

  //Creating s0
  //int k = Find_Segment(x, Spline.x_vals, Spline.length, Spline.accel);
  Stmt s0("int k",
      "{[]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[0]->[0, 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {},
      {}
      );
  cout << "Source statement : " << s0.getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s0.getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s0.getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding s0
  EvalSplineFComputation->addStmt(s0);

  // Creating s1
  // int k = Find_Segment(x, Spline.x_vals, Spline.length, Spline.accel);
  // Find_Segment(double x, double* x_vals, int length, int & prev){
  Stmt s1("double* x_vals = Spline.x_vals; int length = Spline.length; int& prev = Spline.accel",
    "{[0]}",
    "{[0]->[1, 0, 0]}",
    {},
    {}
    );

  cout << "Source statement : " << s1.getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s1.getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s1.getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding s1
  EvalSplineFComputation->addStmt(s1);

  //Call Find_Segment_Computation() 
  Computation* FindSegmentComputation = Find_Segment_Computation();

  //Append FindSegmentComputation to EvalSplineFComputation
  int retVal = EvalSplineFComputation->appendComputation(FindSegmentComputation);

/*
  //Creating s2
  //if (k >= Spline.length) return 0.0; 
  Stmt s2("if (k >= Spline.length) return 0.0",
      "{[]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[0]->[1, 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {},
      {}
      );
  cout << "Source statement : " << s2.getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s2.getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s2.getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding s2
  EvalSplineFComputation->addStmt(s2);

  //Creating s3
  //double X = (x - Spline.x_vals[k])/(Spline.x_vals[k+1] - Spline.x_vals[k]);
  Stmt s3("double X = (x - Spline.x_vals[k])/(Spline.x_vals[k+1] - Spline.x_vals[k])",
      "{[]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[0]->[2, 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {{"Spline.x_vals","{[k]->[k]}"}, {"Spline.x_vals","{[k+1]->[k+1]}"}}, //Data reads
      {}  //Data writes
      );
  cout << "Source statement : " << s3.getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s3.getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s3.getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding s3
  EvalSplineFComputation->addStmt(s3);

  //Creating s4
  //double A = Spline.slopes[k] * (Spline.x_vals[k+1] - Spline.x_vals[k]) - (Spline.f_vals[k+1] - Spline.f_vals[k]);
  Stmt s4("double A = Spline.slopes[k] * (Spline.x_vals[k+1] - Spline.x_vals[k]) - (Spline.f_vals[k+1] - Spline.f_vals[k])",
      "{[]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[0]->[3, 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {{"Spline.slopes","{[k]->[k]}"}, {"Spline.x_vals","{[k]->[k]}"}, {"Spline.x_vals","{[k+1]->[k+1]}"}, {"Spline.f_vals","{[k]->[k]}"}, {"Spline.f_vals","{[k+1]->[k+1]}"}}, //Data reads
      {}  //Data writes
      );
  cout << "Source statement : " << s4.getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s4.getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s4.getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding s4
  EvalSplineFComputation.addStmt(s4);

  //Creating s5
  //double B = -Spline.slopes[k+1] * (Spline.x_vals[k+1] - Spline.x_vals[k]) + (Spline.f_vals[k+1] - Spline.f_vals[k]);
  Stmt s5("double B = -Spline.slopes[k+1] * (Spline.x_vals[k+1] - Spline.x_vals[k]) + (Spline.f_vals[k+1] - Spline.f_vals[k])",
      "{[]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[0]->[4, 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {{"Spline.slopes","{[k+1]->[k+1]}"}, {"Spline.x_vals","{[k]->[k]}"}, {"Spline.x_vals","{[k+1]->[k+1]}"}, {"Spline.f_vals","{[k]->[k]}"}, {"Spline.f_vals","{[k+1]->[k+1]}"}}, //Data reads
      {}  //Data writes
      );
  cout << "Source statement : " << s5.getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s5.getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s5.getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding s5
  EvalSplineFComputation.addStmt(s5);

  //Creating s6
  //double result = (1.0 - X) * Spline.f_vals[k] + X * Spline.f_vals[k+1] + X * (1.0 - X) * (A * (1.0 - X ) + B * X);
  Stmt s6("double result = (1.0 - X) * Spline.f_vals[k] + X * Spline.f_vals[k+1] + X * (1.0 - X) * (A * (1.0 - X ) + B * X)",
      "{[]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[0]->[5, 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {{"Spline.f_vals","{[k]->[k]}"}, {"Spline.f_vals","{[k+1]->[k+1]}"}}, //Data reads
      {}  //Data writes
      );
  cout << "Source statement : " << s6.getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s6.getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s6.getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding s6
  EvalSplineFComputation.addStmt(s6);

  //Creating s7
  //return result; 
  Stmt s7("return result",
      "{[]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[0]->[6, 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {},
      {}
      );
  cout << "Source statement : " << s7.getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s7.getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s7.getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding s7
  EvalSplineFComputation.addStmt(s7);
*/
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

  Computation* FindSegmentComputation;

  //Creating s0
  //if(x >= x_vals[i] && x <= x_vals[i+1]) prev = i;
  Stmt s0("if(x >= x_vals[i] && x <= x_vals[i+1]) prev = i",
      "{[i]: i>=0 && i<length}",  //Iteration schedule - Only happening one time (not iterating)
      "{[i]->[0, i, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {}, 
      {}  
      );  
  cout << "Source statement : " << s0.getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s0.getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s0.getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding s0
  FindSegmentComputation->addStmt(s0);

  //Creating s1
  //return prev; 
  Stmt s1("return prev",
      "{[]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[0]->[1, 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {}, 
      {}  
      );  
  cout << "Source statement : " << s1.getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s1.getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s1.getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding s1
  FindSegmentComputation->addStmt(s1);


  return FindSegmentComputation;
}

