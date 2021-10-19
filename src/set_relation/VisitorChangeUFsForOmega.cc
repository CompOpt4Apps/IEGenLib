#include "VisitorChangeUFsForOmega.h"
namespace  iegenlib{
/* VisitorChangeUFsForOmega */

VisitorChangeUFsForOmega::VisitorChangeUFsForOmega() { reset(); }

VisitorChangeUFsForOmega::~VisitorChangeUFsForOmega() { reset(); }

void VisitorChangeUFsForOmega::reset() {
    ufCallDecls.clear();
    macros.clear();
    knownUFs.clear();
    nextFuncReplacementNumber = 0;
    currentTupleDecl = NULL;
    for(auto uf : ufMap) { delete uf.second;}
    ufMap.clear();
}

void VisitorChangeUFsForOmega::prepareForNext() {
    ufCallDecls.clear();
    currentTupleDecl = NULL;
    // known UFs are preserved for reference in next
    // macros are accumulated over Visitor lifetime
}

std::map<std::string, std::string>* VisitorChangeUFsForOmega::getUFMacros() {
    return &macros;
}

std::set<std::string> VisitorChangeUFsForOmega::getUFCallDecls() {
    return ufCallDecls;
}


std::map<int,std::string>& VisitorChangeUFsForOmega::getTupleAssignments(){
    return tupleAssignments;
}


std::map<std::string, UFCallTerm*>& VisitorChangeUFsForOmega::getUFMap()  {
    return ufMap;
}

void VisitorChangeUFsForOmega::preVisitSparseConstraints(
    SparseConstraints* sc) {
    if (sc->getNumConjuncts() != 1) {
        throw assert_exception(
            "Must have exactly one conjunction for Omega conversion");
    }
}
void VisitorChangeUFsForOmega::preVisitConjunction(Conjunction* c){
    bool requireChange = false;
    std::string prefix = "__x";
    std::list<Exp*> newConstraints;
    TupleDecl decl = c->getTupleDecl();
    for(unsigned int i = 0; i <decl.getSize(); i++){
        if(decl.elemIsConst(i)){
	    requireChange = true;
	    decl.setTupleElem(i,prefix+std::to_string(i));
	    // Create constraint for replacement
	    TupleVarTerm* tupleTerm = new TupleVarTerm(i);
	    Term* constTerm = new Term(-decl.elemConstVal(i));
	    Exp * eqConstraint = new Exp();
	    eqConstraint->setEquality();
            eqConstraint->addTerm(tupleTerm);
            eqConstraint->addTerm(constTerm);
	    newConstraints.push_back(eqConstraint);
	}
    }

    // Change the conjunction at this point to the
    // new conjunction.
    if (requireChange){
        Conjunction* conj = new Conjunction(decl);
        conj->setInArity(c->inarity());
	for(auto it = newConstraints.begin();
		it!=newConstraints.end(); it++){
	    conj->addEquality(*it);
	}
        for (auto it = c->equalities().begin();
		it!=c->equalities().end(); it++){
	    conj->addEquality((*it)->clone());
	}

        for (auto it = c->inequalities().begin();
		it!=c->inequalities().end(); it++){
	    conj->addInequality((*it)->clone());
	}
	if(c->isUnsat()){
            conj->setUnsat();
	}
        // Copy contents of new conjunction to current
	// conjunction being visited.
	*c = *conj;
	delete conj;
    }
    currentTupleDecl = decl;
    // Initilize all tuple variable to zero
    for (unsigned int i = 0; i < decl.size(); i++ ){
       tupleAssignments[i] = "0";
    }
}
void VisitorChangeUFsForOmega::preVisitExp(iegenlib::Exp * e){
    // The goal of this code section is to generate tuple
    // variable intiializations. The code is currently buggy
    // and will be bypassed for now.
    /*
    // Make sure the expression does not have UF
    // and is equality.
    if(!e->hasIndexedUFCall() && e->isEquality()){
        std::cerr << "Analysing: " << e->toString() << "\n";
	// Check if expression has only one tuple variable.
	TupleVarTerm * term = NULL;
	bool isValid = true;
	std::list<Term*> terms = e->getTermList();
	// Count number of tuple variable terms in expression,
	// multiple tuple term in an expression is not considered
	// as a candidate for tuple initialization.
	int tv_count = std::count_if(terms.begin(), terms.end(),
	    [](Term* t){return dynamic_cast<TupleVarTerm*>(t)!=NULL;});
	if (tv_count != 1){
	    return;
	}
	for(auto it = terms.begin();!term && it != terms.end(); it++){
	   if((*it)->isUFCall()){
	       isValid = false;
	       break;
	   }
	   term = dynamic_cast<TupleVarTerm*>(*it);
	}
	// If this expression is valid for assignment
	// we go ahead.
	if(isValid && term){
	    std::cerr << "Is Valid: " << term->toString() << "\n" ;
	    if(terms.size()==1){
	        tupleAssignments[term->tvloc()] = "0";
	        return;
	    }
	    term->setCoefficient(1);
	    Exp * solveFor = e->solveForFactor(term);
	    std::string solvedForString = solveFor->toString();
	    tupleAssignments[term->tvloc()] =  solvedForString;
	    delete solveFor;
	}
    }*/
}


void VisitorChangeUFsForOmega::postVisitUFCallTerm(UFCallTerm* callTerm) {
    if (currentTupleDecl == NULL) {
        throw assert_exception(
            "No TupleDecl collected -- is this Visitor (incorrectly) being run "
            "on something other than a Set/Relation?");
    }

    // determine which tuple variables are needed in the call (how large of a
    // prefix)
    int max_tvloc = -1;
    for (unsigned int i = 0; i < callTerm->numArgs(); ++i) {
        // loop through all terms, processing as needed
        for (const auto& term : callTerm->getParamExp(i)->getTermList()) {
            if (term->isUFCall()) {
                throw assert_exception("Nested UF calls are not yet supported");
            } else if (term->type() == "TupleVarTerm") {
                TupleVarTerm* termAsTupleVar = static_cast<TupleVarTerm*>(term);
                max_tvloc = std::max(termAsTupleVar->tvloc(), max_tvloc);
            } else if (term->type() == "TupleExpTerm") {
                throw assert_exception("TupleExpTerm unsupported");
            }
        }
    }
    // ensure presence of at least one tuple var (UF calls cannot be
    // constant-only)
    if (max_tvloc == -1) {
        throw assert_exception(
            "Cannot make UF calls with only constant arguments");
    }

    // save original coefficient, then temporarily modify for printing
    int originalCoefficient = callTerm->coefficient();
    callTerm->setCoefficient(1);
   
   
    // Original UFCall's to string will be used to 
    // uniquely identify a UFCallTerm. This is because
    // 2 uf calls can have the same name but different 
    // expressions inside of it. A good example is 
    // rowptr(i) and rowptr(i+1).

    std::string originalUFString = callTerm->toString();
    std::string originalCall = callTerm->toString();
    UFCallTerm* originalTerm =(UFCallTerm*) callTerm->clone(); 
    auto itArr = std::find(arrayAccessUFs.begin(),
		    arrayAccessUFs.end(),originalCall);
    
    // Check if UF Call is a replacement.
    auto itReplace= std::find_if(knownUFs.begin(),
		    knownUFs.end(), [originalCall](
			    std::pair<std::string,std::string> e){
		        return originalCall == e.second;   
		    });
    
    if (itArr == arrayAccessUFs.end() && itReplace == knownUFs.end()){
        // Create a ufMacro uf Array access
        std::string ufMacro = callTerm->name();
        std::string ufArrayAccess = callTerm->name();
        ufMacro+= "(";
        bool isFirst = true;
        for (int i = 0; i < callTerm->numArgs(); i++){

	    if(isFirst){
                ufMacro+= "t"+ std::to_string(i);
                isFirst = false;
	    }else{
	        ufMacro += ",t"+std::to_string(i);
	    }
            ufArrayAccess+="[t"+std::to_string(i)+"]";
        }
        ufMacro+=")";
        // Map ufs as macros to actual array aaccess.
        macros[ufMacro] = ufArrayAccess;
	arrayAccessUFs.push_back(originalCall);
    }
    
    // rewrite argument list as a prefix of input tuple
    callTerm->resetNumArgs(max_tvloc + 1);
    for (int i = 0; i < callTerm->numArgs(); ++i) {
        Exp* newParamExp = new Exp();
        TupleVarTerm* tupleVarParam = new TupleVarTerm(i);
        newParamExp->addTerm(tupleVarParam);
        callTerm->setParamExp(i, newParamExp);
    }
    


    std::string replacementName;
    auto it = knownUFs.find(originalUFString);
    // check if this particular UF invocation has already been encountered
    if (it != knownUFs.end()) {
        // use the function name from the already-existing definition
        replacementName = it->second;
        callTerm->setName(replacementName);
	// We dont need the clone we created at this point.
	delete originalTerm;
    } else {
        // assign a new name to this function and add it to our list
        replacementName =
            callTerm->name() + "_" + std::to_string(nextFuncReplacementNumber);
        nextFuncReplacementNumber++;
        knownUFs.emplace(originalUFString, replacementName);

        callTerm->setName(replacementName);
        // this is a new UF, so add a macro definition for it
        macros.emplace(callTerm->toString(), originalCall);

	ufMap.emplace(replacementName,originalTerm);
    }
    // add UF call to the list of declarations
    ufCallDecls.emplace(callTerm->name() + "(" + std::to_string(max_tvloc + 1) +
                        ")");

    // restore coefficient, which was changed temporarily for printing
    callTerm->setCoefficient(originalCoefficient);
}
}//namespace igenlib end
