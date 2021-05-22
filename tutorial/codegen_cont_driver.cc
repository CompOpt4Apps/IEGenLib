#include "iegenlib.h"
#include <utility>
#include <fstream>
#include <iostream>
#include <sstream>

using iegenlib::Computation;
using namespace std;

int main(int argc, char** argv) {

	Computation* temp = new Computation();

    //sources.nu_mag = 	 sqrt( nu[0]*nu[0] + nu[1]*nu[1] + nu[2]*nu[2]);
    temp->addDataSpace("$nu$");
    temp->addDataSpace("$sources.nu_mag$");
    Stmt* s2 = new Stmt("$sources.nu_mag$ = sqrt( $nu$(0)*$nu$(0) + $nu$(1)*$nu$(1) + $nu$(2)*$nu$(2));",
        "{[0]}", //Iteration space
        "{[0]->[0]}", //execution schedule
        { 
          {"$nu$", "{[0]->[0]}"},
          {"$nu$", "{[0]->[1]}"}, //Data reads
          {"$nu$", "{[0]->[2]}"} 
        },
        { {"$sources.nu_mag$"," {[0]->[0]}"} } //Data writes
    );
    cout << "Source statement : " << s2->getStmtSourceCode() << "\n\t"
        << "- Iteration Space : " << s2->getIterationSpace()->prettyPrintString() << "\n\t"
        << "- Execution Schedule : " << s2->getExecutionSchedule()->prettyPrintString() << "\n\t";

    temp->addStmt(s2);

    //    sources.c_gr[0] =  sources.c*nu[0]/sources.nu_mag + sources.w;
    temp->addDataSpace("$sources.c_gr$");
    temp->addDataSpace("$sources.w$");
    Stmt* s3 = new Stmt("$sources.c_gr$(0) =  $sources.c$*$nu$(0)/$sources.nu_mag$ + $sources.w$;",
        "{[0]}",
        "{[0]->[1]}",
        { 
          {"$source.c$"," {[0]->[0]}"},
          {"$nu$"," {[0]->[0]}"},
          {"$source.nu_mag$"," {[0]->[0]}"},
          {"$source.w$"," {[0]->[0]}"} 
        },
        { {"$source.c_gr$"," {[0]->[0]}"} }
    );
    cout << "Source statement : " << s3->getStmtSourceCode() << "\n\t"
        << "- Iteration Space : " << s3->getIterationSpace()->prettyPrintString() << "\n\t"
        << "- Execution Schedule : " << s3->getExecutionSchedule()->prettyPrintString() << "\n\t";

    temp->addStmt(s3);


    //    sources.c_gr[1] =  sources.c*nu[1]/sources.nu_mag + sources.v;
    temp->addDataSpace("$sources.c_gr$");
    temp->addDataSpace("$sources.nu_mag$");
    temp->addDataSpace("$sources.v$");
    Stmt* s4 = new Stmt("$sources.c_gr$(1) =  $sources.c$*$nu$(1)/$sources.nu_mag$ + $sources.v$;",
        "{[0]}",
        "{[0]->[2]}",
        { 
            {"$source.c$"," {[0]->[0]}"},
            {"$nu$"," {[0]->[1]}"},
            {"$source.nu_mag$"," {[0]->[0]}"},
            {"$source.v$"," {[0]->[0]}"} 
        },
        { {"$source.c_gr$"," {[0]->[1]}"} }
    );
    cout << "Source statement : " << s4->getStmtSourceCode() << "\n\t"
        << "- Iteration Space : " << s4->getIterationSpace()->prettyPrintString() << "\n\t"
        << "- Execution Schedule : " << s4->getExecutionSchedule()->prettyPrintString() << "\n\t";

    temp->addStmt(s4);


    //       sources.c_gr[2] =  sources.c*nu[2]/sources.nu_mag + sources.u;
    temp->addDataSpace("$sources.c_gr$");
    temp->addDataSpace("$sources.nu_mag$");
    temp->addDataSpace("$sources.u$");
    Stmt* s5 = new Stmt("$sources.c_gr$(2) =  $sources.c$*$nu$(2)/$sources.nu_mag$ + $sources.u$;",
        "{[0]}",
        "{[0]->[3]}",
        { 
            {"$source.c$"," {[0]->[0]}"},
            {"$nu$"," {[0]->[2]}"},
            {"$source.nu_mag$"," {[0]->[0]}"},
            {"$source.u$"," {[0]->[0]}"} 
        },
        { {"$source.c_gr$"," {[0]->[2]}"} }
    );
    cout << "Source statement : " << s5->getStmtSourceCode() << "\n\t"
        << "- Iteration Space : " << s5->getIterationSpace()->prettyPrintString() << "\n\t"
        << "- Execution Schedule : " << s5->getExecutionSchedule()->prettyPrintString() << "\n\t";

    temp->addStmt(s5);


    //    sources.GeoCoeff[0] = 1.0;
    temp->addDataSpace("$sources.GeoCoeff$");
    Stmt* s6 = new Stmt("$sources.GeoCoeff$(0) = 1.0;",
        "{[0]}",
        "{[0]->[4]}",
        {},
        { {"$sources.GeoCoeff$"," {[0]->[0]}"} }
    );
    cout << "Source statement : " << s6->getStmtSourceCode() << "\n\t"
        << "- Iteration Space : " << s6->getIterationSpace()->prettyPrintString() << "\n\t"
        << "- Execution Schedule : " << s6->getExecutionSchedule()->prettyPrintString() << "\n\t";

    temp->addStmt(s6);


    // sources.GeoCoeff[1] = 1.0 / r;
    temp->addDataSpace("$r$");
    Stmt* s7 = new Stmt("$sources.GeoCoeff$(1) = 1.0/$r$;",
        "{[0]}",
        "{[0]->[5]}",
        { {"$r$"," {[0]->[0]}"} },
        { {"$sources.GeoCoeff$"," {[0]->[1]}"} }
    );
    cout << "Source statement : " << s7->getStmtSourceCode() << "\n\t"
        << "- Iteration Space : " << s7->getIterationSpace()->prettyPrintString() << "\n\t"
        << "- Execution Schedule : " << s7->getExecutionSchedule()->prettyPrintString() << "\n\t";

    temp->addStmt(s7);


    //sources.GeoCoeff[2] = 1.0/(r*cos(theta));
    temp->addDataSpace("$theta$");
    Stmt* s8 = new Stmt("$sources.GeoCoeff$(2) = 1.0/($r$*cos($theta$);",
        "{[0]}",
        "{[0]->[6]}",
        { 
            {"$r$"," {[0]->[0]}"},
            {"$theta$"," {[0]->[0]}"} 
        },
        { {"$sources.GeoCoeff$"," {[0]->[2]}"} }
    );
    cout << "Source statement : " << s8->getStmtSourceCode() << "\n\t"
        << "- Iteration Space : " << s8->getIterationSpace()->prettyPrintString() << "\n\t"
        << "- Execution Schedule : " << s8->getExecutionSchedule()->prettyPrintString() << "\n\t";

    temp->addStmt(s8);


    //sources.GeoTerms[0] = 0.0;
    temp->addDataSpace("$sources.GeoTerms$");
    Stmt* s9 = new Stmt("$sources.GeoTerms$(0) = 0.0;",
        "{[0]}",
        "{[0]->[7]}",
        {},
        { {"$sources.GeoTerms$"," {[0]->[0]}"} }
    );
    cout << "Source statement : " << s9->getStmtSourceCode() << "\n\t"
        << "- Iteration Space : " << s9->getIterationSpace()->prettyPrintString() << "\n\t"
        << "- Execution Schedule : " << s9->getExecutionSchedule()->prettyPrintString() << "\n\t";

    temp->addStmt(s9);


    //sources.GeoTerms[1] = (nu[0]*sources.v - nu[1]*sources.w);
    Stmt* s10 = new Stmt("$sources.GeoTerms$(1) = $nu$(0)*$sources.v$ - $nu$(1)*$sources.w$",
        "{[0]}",
        "{[0]->[8]}",
        { 
            {"$nu$"," {[0]->[0]}"},
            {"$source.v$"," {[0]->[0]}"},
            {"$nu$"," {[0]->[1]}"},
            {"$source.w$"," {[0]->[0]}"} 
        },
        { {"$sources.GeoTerms$"," {[0]->[1]}"} }
    );
    cout << "Source statement : " << s10->getStmtSourceCode() << "\n\t"
        << "- Iteration Space : " << s10->getIterationSpace()->prettyPrintString() << "\n\t"
        << "- Execution Schedule : " << s10->getExecutionSchedule()->prettyPrintString() << "\n\t";

    temp->addStmt(s10);


    //    sources.GeoTerms[2] = (nu[0]*sources.u - nu[2]*sources.w)*cos(theta) + (nu[1]*sources.u - nu[2]*sources.v)*sin(theta);
    Stmt* s11 = new Stmt("$sources.GeoTerms$(2) = ($nu$(0)*$sources.u$ - $nu$(2)*$sources.w$*cos($theta$)) \
        + ($nu$(1) * $sources.u$ - $nu$(2) * $sources.v$ * sin($theta$))",
        "{[0]}",
        "{[0]->[9]}",
        { 
            {"$nu$"," {[0]->[0]}"},
            {"$source.u$"," {[0]->[0]}"},
            {"$nu$"," {[0]->[2]}"},
            {"$source.w$"," {[0]->[0]}"},
            {"$theta$"," {[0]->[0]}"},
            {"$nu$"," {[0]->[1]}"},
            {"$source.v$"," {[0]->[0]}"},
        },
        { {"$sources.GeoTerms$"," {[0]->[2]}"} }
    );
    cout << "Source statement : " << s11->getStmtSourceCode() << "\n\t"
        << "- Iteration Space : " << s11->getIterationSpace()->prettyPrintString() << "\n\t"
        << "- Execution Schedule : " << s11->getExecutionSchedule()->prettyPrintString() << "\n\t";

    temp->addStmt(s11);


    //sources.GeoTerms[0] += 1.0/r * (nu[1]*sources.c_gr[1] + nu[2]*sources.c_gr[2]);
    Stmt* s12 = new Stmt("$sources.GeoTerms$(0) += 1.0/$r$ * ($nu$(1)*$sources.c_gr$(1) + $nu$(2)*$sources.c_gr$(2));",
        "{[0]}",
        "{[0]->[10]}",
        { 
            {"$r$"," {[0]->[0]}"},
            {"$nu$"," {[0]->[1]}"},
            {"$source.c_gr$"," {[0]->[1]}"},
            {"$nu$"," {[0]->[2]}"},
            {"$source.c_gr$"," {[0]->[2]}"} 
        },
        { {"$sources.GeoTerms$"," {[0]->[0]}"} }
    );
    cout << "Source statement : " << s12->getStmtSourceCode() << "\n\t"
        << "- Iteration Space : " << s12->getIterationSpace()->prettyPrintString() << "\n\t"
        << "- Execution Schedule : " << s12->getExecutionSchedule()->prettyPrintString() << "\n\t";

    temp->addStmt(s12);



    //    sources.GeoTerms[1] += -nu[0]*sources.c_gr[1] + nu[2]*sources.c_gr[2]*tan(theta);
    Stmt* s13 = new Stmt("$sources.GeoTerms$(1) +=-$nu$(0)*$sources.c_gr$(1) + $nu$(2)*$sources.c_gr$(2)*tan($theta$);",
        "{[0]}",
        "{[0]->[11]}",
        { 
            {"$nu$"," {[0]->[0]}"},
            {"$sources.c_gr$"," {[0]->[1]}"},
            {"$nu$"," {[0]->[2]}"},
            {"$sources.c_gr$"," {[0]->[2]}"},
            {"$theta$"," {[0]->[0]}"} 
        },
        { {"$sources.GeoTerms$"," {[0]->[1]}"} }
    );
    cout << "Source statement : " << s13->getStmtSourceCode() << "\n\t"
        << "- Iteration Space : " << s13->getIterationSpace()->prettyPrintString() << "\n\t"
        << "- Execution Schedule : " << s13->getExecutionSchedule()->prettyPrintString() << "\n\t";

    temp->addStmt(s12);


    //sources.GeoTerms[2] += -sources.c_gr[2]*(nu[0]*cos(theta) + nu[1]*sin(theta));
    Stmt* s14 = new Stmt("$sources.GeoTerms$(2) += -$sources.c_gr$(2)*($nu$(0)*cos($theta$) + $nu$(1)*sin($theta$));",
        "{[0]}",
        "{[0]->[12]}",
        { 
            {"$sources.c_gr$"," {[0]->[2]}"},
            {"$nu$"," {[0]->[0]}"},
            {"$theta$"," {[0]->[0]}"},
            {"$nu$"," {[0]->[1]}"},
        },
        { {"$sources.GeoTerms$"," {[0]->[2]}"} }
    );
    cout << "Source statement : " << s14->getStmtSourceCode() << "\n\t"
        << "- Iteration Space : " << s14->getIterationSpace()->prettyPrintString() << "\n\t"
        << "- Execution Schedule : " << s14->getExecutionSchedule()->prettyPrintString() << "\n\t";

    temp->addStmt(s14);

    //Write codegen to a file
    ofstream outStream;
    outStream.open("codegen_cont.cpp");
    outStream << temp->codeGen();
    outStream.close();

    return 0;
}