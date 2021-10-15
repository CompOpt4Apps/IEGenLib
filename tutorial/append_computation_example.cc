#include "iegenlib.h"
#include <utility>
#include <fstream>
#include <iostream>
#include <sstream>

Computation* func1();
Computation* func2();

Computation* func1() {
    Computation* comp = new Computation();

    comp->addParameter("$var$", "int&");
    comp->addParameter("$baz$", "int*");

    comp->addStmt(new Stmt (
        "*$baz$ = (*$baz$) + 1;",
        "{[0]}",
        "{[0]->[0]}",
        {{"$baz$", "{[0]->[0]}"}},
        {{"$baz$", "{[0]->[0]}"}}
    ));

    comp->addStmt(new Stmt (
        "$var$ = (*$baz$) * 3;",
        "{[0]}",
        "{[0]->[1]}",
        {{"$baz$", "{[0]->[0]}"}},
        {{"$var$", "{[0]->[0]}"}}
    ));

    std::vector<std::string> args = {"$var$"};
    Computation* func2Comp = func2();
    AppendComputationResult func2Res = comp->appendComputation(func2Comp, "{[0]}", "{[0]->[2]}", args);
    delete func2Comp;
    
    unsigned int i = func2Res.tuplePosition + 1;

    std::string ret = func2Res.returnValues.back();
    comp->addStmt(new Stmt (
        "*$baz$ = "+ret+";",
        "{[0]}",
        "{[0]->["+std::to_string(i)+"]}",
        {{ret, "{[0]->[0]}"}},
        {{"$baz$", "{[0]->[0]}"}}
    ));

    return comp;
}

Computation* func2() {
    Computation* comp = new Computation();

    comp->addParameter("$val$", "int&");

    comp->addStmt(new Stmt (
        "$val$ = $val$ - 5;",
        "{[0]}",
        "{[0]->[0]}",
        {{"$val$", "{[0]->[0]}"}},
        {{"$val$", "{[0]->[0]}"}}
    ));

    comp->addDataSpace("$valReturn$", "int"); 
    comp->addReturnValue("$valReturn$", true);
    comp->addStmt(new Stmt (
        "$valReturn$ = $val$ + 8;",
        "{[0]}",
        "{[0]->[1]}",
        {{"$val$", "{[0]->[0]}"}},
        {{"$valReturn$", "{[0]->[0]}"}}
    ));

    return comp;
}

/*
 * Instructions to view the codegen file.
 * From root make tutorial. 
 * Run the executable from root to generate codegen.
*/
using iegenlib::Computation;
using namespace std;

int main(int argc, char** argv) {

 /* int foo = 6;
 *  int bar = foo * 2;
 *  func1(foo, &bar);
 *  bar = foo * 2;
 *  func1(foo, bar);
 *
 *
 *  func1(int& var, int* baz) {
 *    (*baz)++;
 *    var = (*baz) * 3;
 *    *baz = func2(var);
 *  }
 *
 *  func2(int& val) {
 *    val -= 5;
 *    return val + 8;
 *  }
 * */

    Computation* comp = new Computation();

    comp->addDataSpace("$foo$", "int");
    comp->addDataSpace("$bar$", "int");

    comp->addStmt(new Stmt (
        "$foo$ = 6;",
        "{[0]}",
        "{[0]->[0]}",
        {},
        {{"$foo$", "{[0]->[0]}"}}
    ));

    comp->addStmt(new Stmt (
        "$bar$ = $foo$ * 2;",
        "{[0]}",
        "{[0]->[1]}",
        {{"$foo$", "{[0]->[0]}"}},
        {{"$bar$", "{[0]->[0]}"}}
    ));

    std::vector<std::string> args = {"$foo$", "$bar$"};
    Computation* func1Comp = func1();
    AppendComputationResult func1Res = comp->appendComputation(func1Comp, "{[0]}", "{[0]->[2]}", args);
    
    unsigned int i = func1Res.tuplePosition + 1;

    comp->addStmt(new Stmt (
        "$bar$ = $foo$ * 2;",
        "{[0]}",
        "{[0]->["+std::to_string(i++)+"]}",
        {{"$foo$", "{[0]->[0]}"}},
        {{"$bar$", "{[0]->[0]}"}}
    ));
  
    func1Res = comp->appendComputation(func1Comp, "{[0]}", "{[0]->["+std::to_string(i++)+"]}", args);
    delete func1Comp;

    comp->finalize();

    ofstream outStream;
    outStream.open("append_test.c");
    outStream << comp->codeGen();
    outStream.close();


    delete comp;


return 0;
}


