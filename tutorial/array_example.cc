#include "iegenlib.h"
#include <utility>
#include <fstream>
#include <iostream>
#include <sstream>

/*
 * Instructions to view the codegen file.
 * From root make tutorial. 
 * Run the executable from root to generate codegen.
*/
using iegenlib::Computation;
using namespace std;

/*
    // Assume arr is of size 3
    void func1 (int& arr[], int foo) {
        arr[0] = 1;
        arr[0] = arr[0] + arr[1];
        func2(arr);
        arr[0] = 0;
        arr[2] = func3(arr);
    }
    void func2 (int& arr[]) {
        arr[2] = arr[0] * arr[1];
    }
    int func3 (int arr[]) {
        arr[2] = arr[0] * arr[1];
        return arr[2];
    }
    */


Computation* cFunc1();
Computation* cFunc2();
Computation* cFunc3();

Computation* cFunc1() {
    Computation* func1 = new Computation();
    Computation* func2 = cFunc2();
    Computation* func3 = cFunc3();

    func1->addParameter("arr", "int[]&");
    func1->addParameter("foo", "int");

    func1->addStmt(new Stmt(
        "arr[0] = 1;",
        "{[0]}",
        "{[0]->[0]}",
        {},
        { {"arr", "{[0]->[0]}"} }
    ));

    
    func1->addStmt(new Stmt(
        "arr[0] = arr[0] + arr[1];",
        "{[0]}",
        "{[0]->[1]}",
        { {"arr", "{[0]->[0]}"},
          {"arr", "{[0]->[1]}"} },
        { {"arr", "{[0]->[0]}"} }
    ));

    std::vector<std::string> args = { "arr" };
    AppendComputationResult func2Res = func1->appendComputation(func2, "{[0]}", "{[0]->[2]}", args);
    unsigned int i = func2Res.tuplePosition + 1;

    func1->addStmt(new Stmt(
        "arr[0] = 0;",
        "{[0]}",
        "{[0]->["+std::to_string(i++)+"]}",
        {},
        { {"arr", "{[0]->[0]}"} }
    ));

    AppendComputationResult func3Res = func1->appendComputation(func3, "{[0]}", "{[0]->["+std::to_string(i)+"]}", args);
    i = func3Res.tuplePosition + 1;

    func1->addStmt(new Stmt(
        "arr[2] = " + func3Res.returnValues.back() + ";",
        "{[0]}",
        "{[0]->["+std::to_string(i++)+"]}",
        { {func3Res.returnValues.back(), "{[0]->[0]}"} },
        { {"arr", "{[0]->[2]}"} }
    ));

    delete func2;
    delete func3;
    return func1;
}

Computation* cFunc2() {
    
    Computation* func2 = new Computation();

    func2->addParameter("arr2", "int[]&");

    func2->addStmt(new Stmt (
        "arr2[2] = arr2[0] * arr2[1];",
        "{[0]}",
        "{[0]->[0]}",
        {{"arr2", "{[0]->[0]}"},
         {"arr2", "{[0]->[1]}"}},
        {{"arr2", "{[0]->[2]}"}}
    ));

    return func2;
}

Computation* cFunc3() {
    Computation* func3 = new Computation();

    func3->addParameter("arr3", "int[]");

    func3->addStmt(new Stmt (
        "arr3[2] = arr3[0] * arr3[1];",
        "{[0]}",
        "{[0]->[0]}",
        { {"arr3", "{[0]->[0]}"},
         {"arr3", "{[0]->[1]}"} },
        { {"arr3", "{[0]->[2]}"} }
    ));

    func3->addDataSpace("arr3_return", "int");
    func3->addReturnValue("arr3_return", true);
    func3->addStmt(new Stmt (
        "arr3_return = arr3[2];",
        "{[0]}",
        "{[0]->[1]}",
        {{"arr3", "{[0]->[2]}"}},
        {{"arr3_return", "{[0]->[0]}"}}
    ));

    return func3;
}

Computation* cFunc4() {

    Computation* func3 = new Computation();

    func3->addParameter("arr", "int[]&");
    func3->addParameter("arr2", "int[]&");
    func3->addDataSpace("arr3", "int[]");
    func3->addDataSpace("foo", "int");

    func3->addStmt(new Stmt (
        "arr3 = arr;",
        "{[0]}",
        "{[0]->[0]}",
        { {"arr", "{[0]->[0]}"} },
        { {"arr3", "{[0]->[0]}"} }
    ));

    func3->addStmt(new Stmt (
        "arr3[2] = 0;",
        "{[0]}",
        "{[0]->[1]}",
        {},
        {{"arr3", "{[0]->[2]}"}}
    ));

    func3->addStmt(new Stmt (
        "foo = arr3[0];",
        "{[0]}",
        "{[0]->[2]}",
        {{"arr3", "{[0]->[0]}"}},
        {{"foo", "{[0]->[0]}"}}
    ));
    
    func3->addStmt(new Stmt (
        "arr3 = arr2;",
        "{[0]}",
        "{[0]->[3]}",
        {{"arr2", "{[0]->[0]}"}},
        {{"arr3", "{[0]->[0]}"}}
    ));
    return func3;
}
int main(int argc, char** argv) {

    Computation* comp = cFunc1();

    comp->finalize();

    ofstream dot, codegen, header;
    dot.open("array_example_dot.txt");
    dot << comp->toDotString();
    dot.close();
    codegen.open("array_example.c");
    codegen << comp->codeGen();
    codegen.close();
    header.open("array_example.h");
    header << comp->codeGenMemoryManagementString();
    header.close();

    delete comp;
    comp = cFunc4();

    comp->finalize();

    dot.open("func4_array_example_dot.txt");
    dot << comp->toDotString();
    dot.close();
    codegen.open("func4_array_example.c");
    codegen << comp->codeGen();
    codegen.close();
    header.open("func4_array_example.h");
    header << comp->codeGenMemoryManagementString();
    header.close();
    delete comp;


return 0;
}



