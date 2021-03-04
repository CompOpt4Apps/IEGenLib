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
Computation* w_Computation();
Computation* v_Computation();

/**
 * Main function
 */
int main(int argc, char **argv){


  Computation* CComputation = c_Computation();
  // Computation* EvalSplineF = Eval_Spline_f_Computation();
  
  // vector < pair<string, string> > dataReads;
  // vector < pair<string, string> > dataWrites;
  //Statement 1 = {"C","[k]->[k]"};
  //dataReads.push_back(make_pair(" "," "));
  //dataWrites.push_back(make_pair("C","{[k]->[k]}"));


  //cout << FindSegment.codeGen() <<endl;

  //Calling toDot() on the Computation structure
  fstream dotFileStream;
  cout << "Entering toDot()" << "\n";
  CComputation->toDot(dotFileStream,"eval_spline_f_dot.txt");

  //Write codegen to a file
  ofstream outStream;
  outStream.open("eval_spline_f_codegen.cpp");
  outStream << CComputation->codeGen();
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

  //Creating statement1
  //double r_eval = min(r, r_max);
  Stmt* s1 = new Stmt("double r_eval = min(r, r_max)",
      "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[0]->[0, 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {},
      {}
      );
  cout << "Source statement : " << s1->getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s1->getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s1->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding statement2
  VComputation->addStmt(*s1);

  //Creating statement1
  //r_eval = max(r_eval, r_min); 
  Stmt* s2 = new Stmt("r_eval = max(r_eval, r_min)",
      "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[0]->[1, 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {},
      {}
      );
  cout << "Source statement : " << s2->getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s2->getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s2->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding statement1
  VComputation->addStmt(*s2);

  /*
    Rename the args of c to be the same as the signature
    Function call ==> return Eval_Spline_f(r_eval, Windv_Spline);
    statement3 ==> double x = r_eval;
  */
  Stmt* s3 = new Stmt("double x = r_eval",
      "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[0]->[2, 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {},
      {}
      );
  cout << "Source statement : " << s3->getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s3->getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s3->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding statement
  VComputation->addStmt(*s3);

  /*
    Rename the args of c to be the same as the signature
    Function call ==> return Eval_Spline_f(r_eval, Windv_Spline);
    statement4 ==> struct NaturalCubicSpline_1D & Spline = Windv_Spline;
  */
  Stmt* s4 = new Stmt("struct NaturalCubicSpline_1D & Spline = Windv_Spline",
      "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[0]->[3, 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {},
      {}
      );
  cout << "Source statement : " << s4->getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s4->getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s4->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding statement1
  VComputation->addStmt(*s4);


  /*
    //TODO: ASK!?
    //Return varibale  // not know yet
    return Eval_Spline_f(r_eval, Windv_Spline);
    We now call the Eval_Spline_f_Computation function and
    append it to the existing computation VComputation
  */
  // Computation evalSplineFComputation = Eval_Spline_f_Computation();
  // VComputation.appendComputation(evalSplineFComputation);

  return VComputation;
}



/*
  double w(double r, double theta, double phi){
    return 0.0;
  }
*/
Computation* w_Computation(){
  
  Computation* WComputation;

  //Creating statement1
  Stmt* s0 = new Stmt("return 0.0",
      "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[0]->[0, 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {},
      {}
      );
  cout << "Source statement : " << s0->getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s0->getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s0->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding statement1
  WComputation->addStmt(*s0);

  return WComputation;
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
  
  Computation* CComputation = new Computation;

  //Creating statement1
  //double r_eval = min(r, r_max);
  Stmt* s1 = new Stmt("double r_eval = min(r, r_max)",
      "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[0]->[0, 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {},
      {}
      );
  cout << "Source statement : " << s1->getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s1->getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s1->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding statement1
  CComputation->addStmt(*s1);

  //Creating statement2
  //r_eval = max(r_eval, r_min);
  Stmt* s2 = new Stmt("r_eval = max(r_eval, r_min)",
      "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[0]->[1, 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {},
      {}
      );
  cout << "Source statement : " << s2->getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s2->getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s2->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding statement2
  CComputation->addStmt(*s2);

  // Eval_Spline_f(double x, struct NaturalCubicSpline_1D & Spline

  //Creating statement3a
  //Making sure the function parameters and arguments are the same
  //double x = r_eval; struct NaturalCubicSpline_1D & Spline = Temp_Spline
  Stmt* s3a = new Stmt("double x = r_eval; struct NaturalCubicSpline_1D & Spline = Temp_Spline",
      "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[0]->[2, 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {},
      {}
      );
  cout << "Source statement : " << s3a->getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s3a->getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s3a->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding statement3a
  CComputation->addStmt(*s3a);

  //Call Eval_Spline_f_Computation() returen values is stored in result
  Computation* EvalSplineFComputation = Eval_Spline_f_Computation();

  //Append EvalSplineFComputation to CComputation
  int retVal = CComputation->appendComputation(EvalSplineFComputation);

  //Creating statement3b
  //double result = sqrt(gamR * Eval_Spline_f(r_eval,Temp_Spline));
  Stmt* s3b = new Stmt("double res = sqrt(gamR * Eval_Spline_f(r_eval,Temp_Spline))",
      "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[0]->["+std::to_string(retVal+1)+", 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {},
      {}
      );
  cout << "Source statement : " << s3b->getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s3b->getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s3b->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding statement3b
  CComputation->addStmt(*s3b);

  //Creating statement4
  //return result;
  Stmt* s4 = new Stmt("return res",
      "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[0]->["+std::to_string(retVal+2)+", 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {},
      {}
      );
  cout << "Source statement : " << s4->getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s4->getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s4->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding statement3
  CComputation->addStmt(*s4);
  
  return CComputation;
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
  Stmt* s0 = new Stmt("int k; double result = 0.0",
      "{[0]}",  
      "{[0]->[0, 0, 0]}",
      {},
      {}
      );
  cout << "Source statement : " << s0->getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s0->getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s0->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding s0
  EvalSplineFComputation->addStmt(*s0);

  // Creating s1
  // int k = Find_Segment(x, Spline.x_vals, Spline.length, Spline.accel);
  // Find_Segment(double x, double* x_vals, int length, int & prev){
  Stmt* s1 = new Stmt("double* x_vals = Spline.x_vals; int length = Spline.length; int& prev = Spline.accel",
    "{[0]}",
    "{[0]->[1, 0, 0]}",
    {},
    {}
    );

  cout << "Source statement : " << s1->getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s1->getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s1->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding s1
  EvalSplineFComputation->addStmt(*s1);

  //Call Find_Segment_Computation() 
  Computation* FindSegmentComputation = Find_Segment_Computation();

  //Append FindSegmentComputation to EvalSplineFComputation
  int retVal = EvalSplineFComputation->appendComputation(FindSegmentComputation);

  //Creating s1
  //return prev; 
  Stmt* s1a = new Stmt("k = prev",
      "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[0]->["+std::to_string(retVal+1)+", 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {}, 
      {}  
      );  
  cout << "Source statement : " << s1a->getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s1a->getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s1a->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding s1
  EvalSplineFComputation->addStmt(*s1a);


  // //Creating s2
  // //if (k >= Spline.length) return 0.0; 
  // Stmt* s2= new Stmt("if (k >= Spline.length) return 0.0",
  //     "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
  //     //"{[0]->["+static_cast<ostringstream*>( &(ostringstream() << (retVal+1)) )->str()+", 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
  //     "{[0]->["+std::to_string(retVal+1)+", 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
  //     {},
  //     {}
  //     );
  // cout << "Source statement : " << s2->getStmtSourceCode() << "\n\t"
  //   <<"- Iteration Space : "<< s2->getIterationSpace()->prettyPrintString() << "\n\t"
  //   << "- Execution Schedule : "<< s2->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  // //Adding s2
  // EvalSplineFComputation->addStmt(*s2);

  //Creating s3
  //double X 
  Stmt* s3 = new Stmt("double X, A, B",
      "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
      "{[0]->["+std::to_string(retVal+2)+", 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      //{{"Spline.x_vals","{[k]->[k]}"}, {"Spline.x_vals","{[k+1]->[k+1]}"}}, //Data reads
      {}, //Data reads
      {}  //Data writes
      );
  cout << "Source statement : " << s3->getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s3->getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s3->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding s3
  EvalSplineFComputation->addStmt(*s3);

  //Creating s3
  //X = (x - Spline.x_vals[k])/(Spline.x_vals[k+1] - Spline.x_vals[k]);
  Stmt* s3a = new Stmt("X = (x - Spline.x_vals[k])/(Spline.x_vals[k+1] - Spline.x_vals[k])",
      "{[0]: k<length}",  //Iteration schedule - Only happening one time (not iterating)
      "{[0]->["+std::to_string(retVal+3)+", 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      //{{"Spline.x_vals","{[k]->[k]}"}, {"Spline.x_vals","{[k+1]->[k+1]}"}}, //Data reads
      {}, //Data reads
      {}  //Data writes
      );
  cout << "Source statement : " << s3a->getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s3a->getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s3a->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding s3
  EvalSplineFComputation->addStmt(*s3a);

  //Creating s4
  //A = Spline.slopes[k] * (Spline.x_vals[k+1] - Spline.x_vals[k]) - (Spline.f_vals[k+1] - Spline.f_vals[k]);
  Stmt* s4 = new Stmt("A = Spline.slopes[k] * (Spline.x_vals[k+1] - Spline.x_vals[k]) - (Spline.f_vals[k+1] - Spline.f_vals[k])",
      "{[0]: k<length}",  //Iteration schedule - Only happening one time (not iterating)
      "{[0]->["+std::to_string(retVal+4)+", 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      //{{"Spline.slopes","{[k]->[k]}"}, {"Spline.x_vals","{[k]->[k]}"}, {"Spline.x_vals","{[k+1]->[k+1]}"}, {"Spline.f_vals","{[k]->[k]}"}, {"Spline.f_vals","{[k+1]->[k+1]}"}}, //Data reads
      {}, //Data reads
      {}  //Data writes
      );
  cout << "Source statement : " << s4->getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s4->getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s4->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding s4
  EvalSplineFComputation->addStmt(*s4);

  //Creating s5
  //double B = -Spline.slopes[k+1] * (Spline.x_vals[k+1] - Spline.x_vals[k]) + (Spline.f_vals[k+1] - Spline.f_vals[k]);
  Stmt* s5 = new Stmt("B = -Spline.slopes[k+1] * (Spline.x_vals[k+1] - Spline.x_vals[k]) + (Spline.f_vals[k+1] - Spline.f_vals[k])",
      "{[0]: k<length}",  //Iteration schedule - Only happening one time (not iterating)
      "{[0]->["+std::to_string(retVal+5)+", 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      //{{"Spline.slopes","{[k+1]->[k+1]}"}, {"Spline.x_vals","{[k]->[k]}"}, {"Spline.x_vals","{[k+1]->[k+1]}"}, {"Spline.f_vals","{[k]->[k]}"}, {"Spline.f_vals","{[k+1]->[k+1]}"}}, //Data reads
      {}, // Data reads
      {}  //Data writes
      );
  cout << "Source statement : " << s5->getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s5->getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s5->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding s5
  EvalSplineFComputation->addStmt(*s5);

  //Creating s6
  //double result = (1.0 - X) * Spline.f_vals[k] + X * Spline.f_vals[k+1] + X * (1.0 - X) * (A * (1.0 - X ) + B * X);
  Stmt* s6 = new Stmt("result = (1.0 - X) * Spline.f_vals[k] + X * Spline.f_vals[k+1] + X * (1.0 - X) * (A * (1.0 - X ) + B * X)",
      "{[0]: k<length}",  //Iteration schedule - Only happening one time (not iterating)
      "{[0]->["+std::to_string(retVal+6)+", 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      //{{"Spline.f_vals","{[k]->[k]}"}, {"Spline.f_vals","{[k+1]->[k+1]}"}}, //Data reads
      {}, // Data reads
      {}  //Data writes
      );
  cout << "Source statement : " << s6->getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s6->getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s6->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding s6
  EvalSplineFComputation->addStmt(*s6);

  // //Creating s7
  // //return result; 
  // Stmt* s7 = new Stmt("return result",
  //     "{[0]}",  //Iteration schedule - Only happening one time (not iterating)
  //     "{[0]->["+std::to_string(retVal+1)+", 0, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
  //     {},
  //     {}
  //     );
  // cout << "Source statement : " << s7->getStmtSourceCode() << "\n\t"
  //   <<"- Iteration Space : "<< s7->getIterationSpace()->prettyPrintString() << "\n\t"
  //   << "- Execution Schedule : "<< s7->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  // //Adding s7
  // EvalSplineFComputation->addStmt(*s7);

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

  //Creating s0
  //if(x >= x_vals[i] && x <= x_vals[i+1]) prev = i;
  Stmt*  s0 = new Stmt("if(x >= x_vals[i] && x <= x_vals[i+1]) prev = i",
      "{[i]: i>=0 && i<length}",  //Iteration schedule - Only happening one time (not iterating)
      "{[i]->[0, i, 0]}", //Execution schedule - scheduling statement to be first (scheduling function)
      {}, 
      {}  
      );  
  cout << "Source statement : " << s0->getStmtSourceCode() << "\n\t"
    <<"- Iteration Space : "<< s0->getIterationSpace()->prettyPrintString() << "\n\t"
    << "- Execution Schedule : "<< s0->getExecutionSchedule()->prettyPrintString() << "\n\t" ;

  //Adding s0
  FindSegmentComputation->addStmt(*s0);

  return FindSegmentComputation;
}


