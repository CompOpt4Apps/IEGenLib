/*!
 * \file SSA.cpp
 *
 * \brief Implementation of the SSA class
 *
 * The SSA class is the class that gives us the dominance graph.
 *
 * \date Started: 08/02/22
 *
 * \authors Kalyan Bhetwal
 *
 * Copyright (c) 2020, Boise State University <br>
 * All rights reserved. <br>
 * See ../../COPYING for details. <br>
 */
#include "SSA.h"
#include <vector>
#include <tuple>
#include <stack>
#include "set_relation/set_relation.h"
#include <iostream>
#include "Computation.h"
#include <utility>
#include <string>

using namespace SSA;
using namespace iegenlib;
DominanceTree::DominanceTree() {}

DominanceTree::~DominanceTree() {
}

int DominanceTree::push_Back(std::pair<int, iegenlib::Set*> data) {
    Node* node = new Node();
    node->parent={};
    node->children = {};
    node->data = data;
    node-> dominanceFrontier = {};
    nodes.push_back(*node);
    return(nodes.size()-1);
}


void DominanceTree::add_edge(int parent, int child) {
    nodes[parent].children.push_back(child);
    nodes[child].parent = parent;
}
// j is predecessor of i
void DominanceTree::add_predecessors(int i, int j){
    nodes[i].predecessors.push_back(j);
}
void DominanceTree::add_successors(int i, int j){
    nodes[j].successors.push_back(i);
}
int DominanceTree::getVectorSize() {
    return nodes.size();
}
Set* DominanceTree::getElem(int i){
    return(nodes[i].data.second);

}
bool DominanceTree::isParent(int parent, int child) {
   for(auto v:  nodes[parent].children){
       if(v == child){
           return true;
       }
   }
    return  false;
}
std::vector<int> DominanceTree::getPredecessor(int i){
    return(nodes[i].predecessors);
}
//calculate Dominance Frontier
void DominanceTree::DFCal() {
    for(int i=0; i<this->nodes.size();i++){
        if( this->nodes[i].predecessors.size() > 0) {

            //std:: cout<< "predecessor  for node " <<i <<'\n';
            std::cout << "--------------------"<<'\n';
            for (int pred:  this->nodes[i].predecessors) {
                std:: cout<< "pred "<< pred << " for node " <<i <<'\n';
                int runner = pred;
                std::vector<int> DF_runner =  this->nodes[runner].dominanceFrontier;

                while( runner !=  this->nodes[i].parent){
                    //check if element i isn't in the vector
                    if(std::find(DF_runner.begin(), DF_runner.end(), i) == DF_runner.end()) {
                        DF_runner.push_back(i);
                        std:: cout << "DF  " << i<<'\n';
                        //looks like additional DF nodes are being added
                        // might have to remove the statement
                        this->nodes[runner].dominanceFrontier.push_back(i);
                    }
                    runner =   this->nodes[runner].parent;
                }
            }
        }
    }
    return;
}

//test if two dominator tree are equivalent
bool DominanceTree::equivalent(DominanceTree dt) {

 for(int i=0;i<dt.nodes.size();i++) {
     if (dt.nodes[i].data.second->prettyPrintString() != this->nodes[i].data.second->prettyPrintString()) return false;

     std::vector<int> v1 = dt.nodes[i].children;
     std::vector<int> v2 = this->nodes[i].children;
     std::sort(v1.begin(), v1.end());
     std::sort(v2.begin(), v2.end());
     if(v1.size()) continue;

     if (v1 != v2) {
         return false;
     }
 }
    return true;

}
bool DominanceTree::predecessorEquivalent(DominanceTree dt){
for(int i=0;i<dt.getVectorSize();i++) {
    for(int j = 0; j<dt.nodes[i].predecessors.size();j++){
        if(this->nodes[i].predecessors.size()>0 && this->nodes[i].predecessors.size()== dt.nodes[i].predecessors.size()) {
            if (dt.nodes[i].predecessors[j] != this->nodes[i].predecessors[j]) return false;
            if (dt.nodes[i].predecessors[j] != this->nodes[i].predecessors[j]) return false;
        }
        else{ return  false;}
}
}
    return true;
}


bool SSA::isReverseDominator(iegenlib::Set * s1, iegenlib::Set * s2){
    if(s2->getArity()< s1->getArity()){
        return true;
    }
    while( s2->getArity() > s1->getArity()) {
        TupleDecl tl = s2->getTupleDecl();
        s2 = s2->projectOut(tl.getSize()-1);
    }
    if (s2->isSubset(s1)) return false;
    return true;

}

// function returns list of prefixes
std::vector<Set*> DominanceTree::getPrefixes(Set*s) {

    std::vector<Set*>v;
    Set* res = new Set(*s);
    TupleDecl tl = s->getTupleDecl();
    if( tl.size()==1 ||  tl.size()==2){
        return v;
    }
    for(int i= tl.size()-1; i>0 ;i--){
        if( tl.elemIsConst(i)) {
            //std::cout<< "the const elem are " << i <<'\n';
            res = res->projectOut(i);
            v.push_back(res);
        }
        else{
            res = res->projectOut(i);
        }
    }
    //std::cout << "print "<< s->prettyPrintString() <<'\n';
    return v;
}

void DominanceTree::updatePredecessors(int i, std::vector<int>&v) {
    std::vector<int> pred = nodes[i].predecessors;
    if(i>0){
        for(int k=0;k<pred.size();k++){
            //std::cout << "--------------------"<<'\n' << pred[k]<<'\n';
            v.push_back(pred[k]);
            //nodes[i].cfg_predecessors.push_back(pred[k]);
            updatePredecessors(pred[k],v);
        }
    }
    return;
}

void DominanceTree::updateCfgPredecessors(int k,std::vector<int>v){
    for(int i=0;i<v.size();i++){
        if (std::find(nodes[k].cfg_predecessors.begin(), nodes[k].cfg_predecessors.end(), v[i]) == nodes[k].cfg_predecessors.end()) {
            nodes[k].cfg_predecessors.push_back(v[i]);
        }

    }
}
DominanceTree* SSA::findPredecessors(DominanceTree* dt) {

    std::vector<std::pair<Set*, int>> stack;
    for (int i = dt->getVectorSize() -1; i >= 0; i--) {
       // std::cout << "--------the node is---------- " << i<<'\n';
        for (int j = i-1; j >= 0; j--) {
          //  std::cout << "--------i am in  node ---------- " << j<<'\n';
            bool flag = true;
            for(int k: dt->getPredecessor(i)){
               // std:: cout << "does the node  " << k << " reverse dominates "<< j  << '\n';
                flag = SSA::isReverseDominator( dt->getElem(k),dt->getElem(j) );
            }
            if(flag){
               // std::cout <<" adding pred "<<j <<" in node " << i << '\n';
                dt->add_predecessors(i, j);
                dt->add_successors(i,j);
            }
          //  std::cout << "hello hello"<<'\n';

            // // get the prefixes for particular node
            // put them in the stack
            // move one node above


            std::vector<Set*>  v = dt->getPrefixes(dt->getElem(j) );

            std::vector<std::pair<Set*, int>> tmp_stack;
            std::vector <int> pop_elm;
            int l,m;
            bool no_match;
            for( l=stack.size()-1;l>0;l--){
                tmp_stack = {};
                for(m=0;m<v.size();m++){
                    if(stack[l].first->prettyPrintString()==v[m]->prettyPrintString()){
                       no_match = false;
                       break;
                    }
                    no_match = true;
                    tmp_stack.push_back({v[m],i});
                }
                if(!no_match)break;
            }
            if(no_match){ stack = {};}
            else{

            }
            stack.insert(stack.end(), tmp_stack.begin(), tmp_stack.end());
            std:: cout << "----------------------" <<'\n';
            if(dt->isParent(j,i)) {break;}
        }
    }

//    for(int i=dt->getVectorSize()-1;i>0;i--){
//        std::vector<int> v;
//        dt-> updatePredecessors(i,v);
//        dt->updateCfgPredecessors(i, v);
//        std::cout<< "size   "<<i << "   "<<v.size() <<'\n';
//    }
     return dt;
}


bool SSA::isDominator(iegenlib::Set * parent, iegenlib::Set * child){
    // project out constant both parent and child
    Set* childP, *parentP;

    childP = child->projectOutConst(child);
    parentP = parent->projectOutConst(parent);
    

    if( parentP->getArity() > childP->getArity()){
        return  false;
    }
    // project out until parents and child parity matches
    while( parentP->getArity() < childP->getArity()) {
        TupleDecl tl = childP->getTupleDecl();
        childP = childP->projectOut(tl.getSize()-1);
    }
    // child subset of parent;
    return (childP->isSubset(parentP));
}

// create dominator tree
DominanceTree* SSA::createDominanceTree(std::vector<iegenlib::Set *>executionS1) {
    DominanceTree* pDominanceTree = new DominanceTree();


    std::vector<std::pair<int, iegenlib::Set*>> executionS;
    for(int i = 0; i<executionS1.size();i++){
        executionS.push_back({i,executionS1[i] });
    }

    //perform the lexicographical sort
    std::sort(executionS.begin(), executionS.end(), []
            (const std::pair<int, iegenlib::Set *> &a, const std::pair<int, iegenlib::Set *> &b) {
        Set* s;
        bool status = s->Set::LexiSort(a.second, b.second);
        return(status);
        delete s;
    });

    // collect nodes into node list
    for (auto v: executionS) {
        pDominanceTree ->push_Back(v);
    }
    // set up the relations( parent and child)

    for (int i = executionS.size() - 1; i >= 0; i--) {
        for (int j = i-1; j >= 0; j--) {
            bool isDominator1 = SSA::isDominator(executionS[j].second, executionS[i].second);
            if(isDominator1){
                pDominanceTree -> add_edge(j, i);
                break;
            }
        }
    }
    return pDominanceTree;
}

void DominanceTree::insertPhiNode(std::map<string, std::vector<int>> globals, Computation* comp ){

    std:: vector<string> global_var ;

        std:: cout << "hell0 ===="<<'\n';

        std::vector<int> actual_phi;
        std::map<string, std::vector<int>> ::iterator it;

    for (it = globals.begin(); it != globals.end(); it++){

        global_var.push_back(it->first);

        std::vector<int> workList = it->second;
        for(int j=0 ;j<workList.size();j++){
           // std::cout << "variable's definition node "<< workList[j]<<'\n';
            std::vector<int> DF = this->nodes[workList[j]].dominanceFrontier;
            for(int k=0;k<DF.size();k++){
                //location of phi to insert in phi nodes;
                int df =  DF[k];
                //insert only unique
                actual_phi =  nodes[df].phis[it->first];

                if(std::find( actual_phi.begin(),  actual_phi.end(), workList[j]) ==  actual_phi.end()) {
                    actual_phi.push_back(workList[j]);
                    nodes[df].phis[it->first].push_back(workList[j]);
                }

              //  std::cout << "insert phi nodes in DF "<< this->nodes[workList[j]].dominanceFrontier[k]<<'\n';
                if (std::find(workList.begin(), workList.end(), DF[k]) == workList.end()) {
                   // actual_phi[it->first].push_back(workList[j]);
                    workList.push_back(DF[k]);
                }

            }
        }

    }

    std::map<string, int> counter;
    std::map<string, int> stack;

    for(int i=0;i<global_var.size();i++){
        std:: cout << "global vars " << global_var[i]<< '\n';
        counter[global_var[i]] = 0;
        stack[global_var[i]] = {};
    }

//    std::cout << rename(counter , stack ,"x")<<'\n';
//    std::cout << rename(counter , stack ,"x")<<'\n';

    for(int i=0;i<nodes.size();i++){
        std::vector<int>loc ;
        for (it = nodes[i].phis.begin(); it != nodes[i].phis.end(); it++){
            loc = it -> second;
            for(int e=0;e<loc.size();e++){
                std::cout << "phi reads from "<< e << "\n";
            }
            string newName = rename(counter, stack, it ->first);
            std::cout<< newName<<"  loc  " << i <<'\n';

            Stmt * s_org = comp->getStmt(nodes[i].data.first);

//                std:: cout << s_org->prettyPrintString()<<'\n';
//                std:: cout << s_org->getIterationSpace()->prettyPrintString()<<'\n';

//                for(int g=0; g<nodes[i]['x'].size();g++) {
//                    std::cout << nodes[i]['x'][g];
//                }

            //nodes[i].phis.at('x');
            string itrspace = s_org->getIterationSpace()->prettyPrintString();
            string executionSch = s_org -> getExecutionSchedule()->prettyPrintString();
            comp->addStmt(new Stmt (
                    "phi",
                    itrspace,
                    executionSch,
                    {},
                    {{newName, "{[0]->[0]}"}}
            ));


        }
    }
//    for( int a=0;a<comp->getNumStmts();a++){
//        std::cout << comp->getStmt(a)->prettyPrintString();
//    }


}

string DominanceTree::rename( std::map<string, int> &counter, std::map<string, int> &stack,string n ){
    int i = counter[n];
    counter[n] = counter[n]+1;
    stack[n] = i;
    n.erase(n.begin());
    n.erase(n.end()-1);
    //std:: cout << "    "<<n <<'\n';
    return n + std::to_string(i);
    //return n.insert( n.length()-1, "1" ) ;//+ std::to_string(i);

}
void SSA::generateSSA(Computation *  comp) {

    std::vector<Stmt*> stmts  ;
    for( int a=0;a<comp->getNumStmts();a++){
        stmts.push_back(comp->getStmt(a));
    }
    std::vector<iegenlib::Set*> executionS;
    std::map<string, std::vector<int>> globals;

    for(int i=0; i<stmts.size(); i++){
            iegenlib::Set* s1 = stmts[i]->getExecutionSchedule()->Apply( stmts[i]->getIterationSpace());
            //std::cout << s1->prettyPrintString()<<'\n';

            std::map<string, std::vector<int>> vec ;
            int numWrites = stmts[i]->getNumWrites();

            for (unsigned int j = 0; j < numWrites; ++j){
                globals[stmts[i]->getWriteDataSpace(j)].push_back(i);
                //std::cout << "variables    " << stmts[i]->getWriteDataSpace(j)<<" ,  "<< i<<'\n';
            }
            // insert only unique
            //globals.push_back(vec);
            executionS.push_back(s1);
    }



        std::map<string, std::vector<int>> ::iterator it = globals.begin();
        std::cout << it->first<< " sdfd  "<<'\n';

    DominanceTree* dt = createDominanceTree(executionS);
    DominanceTree* dt1 = findPredecessors(dt);
    //dt1->printPredecessor();
    //dt1->printCfgPred();
    dt1->printTree();
    dt1->DFCal();
   // dt1->insertPhiNode(globals, comp);
    std::cout << "print random";

}

void DominanceTree::printPredecessor() {
    for(int i=0;i<nodes.size();i++){
        std::cout << " node " << nodes[i].data.second->prettyPrintString() ;
        for( int j=0; j<nodes[i].predecessors.size();j++){
            std::cout  << " has dominator  "<<nodes[nodes[i].predecessors[j]].data.second->prettyPrintString() <<'\n';

        }
        std::cout<<'\n';
    }
}
//
//void DominanceTree::printTree(){
//
//}

void DominanceTree:: printCfgPred(){
    for(int i=0;i<nodes.size();i++){
        for( int j=0; j<nodes[i].cfg_predecessors.size();j++){
            std::cout << "node  " << i << "  "<<nodes[i].cfg_predecessors[j] <<'\n';
        }
        std:: cout << "---------------" <<'\n';
    }
}

void DominanceTree::printTree() {
    for(int i=0;i<nodes.size();i++){
        for( int j=0; j<nodes[i].children.size();j++){
            std::cout << "node  " << i << " parent  "<<nodes[i].parent<< "   children  "  << nodes[i].children[j] <<'\n';
        }
        std:: cout << "---------------" <<'\n';
    }
}