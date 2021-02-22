#include "iegenlib.h"
#include <utility>
#include <fstream>
#include <iostream>

using iegenlib::Computation;
using namespace std;

Computation Find_Segment_Computation();
Computation Eval_Spline_f_Computation();


/**
 * Main function
 */
int main(int argc, char **argv){


  Computation FindSegment = Find_Segment_Computation();
  Computation EvalSplineF = Find_Segment_Computation();
  
  // vector < pair<string, string> > dataReads;
  // vector < pair<string, string> > dataWrites;
  //Statement 1 = {"C","[k]->[k]"};
  //dataReads.push_back(make_pair(" "," "));
  //dataWrites.push_back(make_pair("C","{[k]->[k]}"));


  //cout << FindSegment.codeGen() <<endl;

  //Calling toDot() on the Computation structure
  fstream dotFileStream;
  cout << "Entering toDot()" << "\n";
  FindSegment.toDot(dotFileStream,"find_segment_dot.txt");

  //Write codegen to a file
  ofstream outStream;
  outStream.open("find_segment_codegen.cpp");
  outStream << FindSegment.codeGen();
  outStream.close();

  return 0;
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
Computation Find_Segment_Computation(){

  Computation FindSegmentComputation;

  //Creating statement1
  //if(x >= x_vals[i] && x <= x_vals[i+1]) prev = i;
  Stmt statement1("if(x >= x_vals[i] && x <= x_vals[i+1]) prev = i",
      "{[i]: i>=0 && i<length}",  //Iteration schedule - Only happening one time (not iterating)
      "{[i]->[0, i, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {},
      {}
      );
  cout << "Source statement : " << statement1.getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< statement1.getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< statement1.getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding statement2
  FindSegmentComputation.addStmt(statement1);

  //Creating statement2
  //return prev; 
  Stmt statement2("return prev",
      "{[]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[]->[1, 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {},
      {}
      );
  cout << "Source statement : " << statement2.getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< statement2.getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< statement2.getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding statement2
  FindSegmentComputation.addStmt(statement2);


  return FindSegmentComputation;
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
Computation Eval_Spline_f_Computation(){

  Computation EvalSplineFComputation;

  //Creating statement1
  //int k = Find_Segment(x, Spline.x_vals, Spline.length, Spline.accel);
  Stmt statement1("int k = Find_Segment(x, Spline.x_vals, Spline.length, Spline.accel)",
      "{[]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[]->[0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {},
      {}
      );
  cout << "Source statement : " << statement1.getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< statement1.getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< statement1.getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding statement2
  EvalSplineFComputation.addStmt(statement1);

  //Creating statement2
  //if (k >= Spline.length) return 0.0; 
  Stmt statement2("if (k >= Spline.length) return 0.0",
      "{[]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[]->[1]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {},
      {}
      );
  cout << "Source statement : " << statement2.getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< statement2.getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< statement2.getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding statement2
  EvalSplineFComputation.addStmt(statement2);

  //Creating statement3
  //double X = (x - Spline.x_vals[k])/(Spline.x_vals[k+1] - Spline.x_vals[k]);
  Stmt statement3("double X = (x - Spline.x_vals[k])/(Spline.x_vals[k+1] - Spline.x_vals[k])",
      "{[]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[]->[2]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {{"Spline.x_vals","{[k]->[k]}"}, {"Spline.x_vals","{[k+1]->[k+1]}"}}, //Data reads
      {}  //Data writes
      );
  cout << "Source statement : " << statement3.getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< statement3.getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< statement3.getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding statement3
  EvalSplineFComputation.addStmt(statement3);

  //Creating statement4
  //double A = Spline.slopes[k] * (Spline.x_vals[k+1] - Spline.x_vals[k]) - (Spline.f_vals[k+1] - Spline.f_vals[k]);
  Stmt statement4("double A = Spline.slopes[k] * (Spline.x_vals[k+1] - Spline.x_vals[k]) - (Spline.f_vals[k+1] - Spline.f_vals[k])",
      "{[]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[]->[3]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {{"Spline.slopes","{[k]->[k]}"}, {"Spline.x_vals","{[k]->[k]}"}, {"Spline.x_vals","{[k+1]->[k+1]}"}, {"Spline.f_vals","{[k]->[k]}"}, {"Spline.f_vals","{[k+1]->[k+1]}"}}, //Data reads
      {}  //Data writes
      );
  cout << "Source statement : " << statement4.getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< statement4.getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< statement4.getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding statement4
  EvalSplineFComputation.addStmt(statement4);

  //Creating statement5
  //double B = -Spline.slopes[k+1] * (Spline.x_vals[k+1] - Spline.x_vals[k]) + (Spline.f_vals[k+1] - Spline.f_vals[k]);
  Stmt statement5("double B = -Spline.slopes[k+1] * (Spline.x_vals[k+1] - Spline.x_vals[k]) + (Spline.f_vals[k+1] - Spline.f_vals[k])",
      "{[]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[]->[4]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {{"Spline.slopes","{[k+1]->[k+1]}"}, {"Spline.x_vals","{[k]->[k]}"}, {"Spline.x_vals","{[k+1]->[k+1]}"}, {"Spline.f_vals","{[k]->[k]}"}, {"Spline.f_vals","{[k+1]->[k+1]}"}}, //Data reads
      {}  //Data writes
      );
  cout << "Source statement : " << statement5.getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< statement5.getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< statement5.getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding statement5
  EvalSplineFComputation.addStmt(statement5);

  //Creating statement6
  //double result = (1.0 - X) * Spline.f_vals[k] + X * Spline.f_vals[k+1] + X * (1.0 - X) * (A * (1.0 - X ) + B * X);
  Stmt statement6("double result = (1.0 - X) * Spline.f_vals[k] + X * Spline.f_vals[k+1] + X * (1.0 - X) * (A * (1.0 - X ) + B * X)",
      "{[]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[]->[5]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {{"Spline.f_vals","{[k]->[k]}"}, {"Spline.f_vals","{[k+1]->[k+1]}"}}, //Data reads
      {}  //Data writes
      );
  cout << "Source statement : " << statement6.getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< statement6.getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< statement6.getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding statement6
  EvalSplineFComputation.addStmt(statement6);

  //Creating statement7
  //return result; 
  Stmt statement7("return result",
      "{[]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[]->[6]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {},
      {}
      );
  cout << "Source statement : " << statement7.getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< statement7.getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< statement7.getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding statement7
  EvalSplineFComputation.addStmt(statement7);

  return EvalSplineFComputation;
}

