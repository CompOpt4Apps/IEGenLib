/*!
 * \file Visitor.h
 *
 * \brief Visitor base class for expressions.h and set_relation.h.
 *
 * Usage: See VisitorDebugTest in set_relation_test for an example.
 *        You do not have to overload every visit method below.
 *        Just overload those that your feature needs to do something at.
 *
 * \authors Michelle Strout
 *
 * Copyright (c) 2015, University of Arizona <br>
 * All rights reserved. <br>
 * See ../../COPYING for details. <br>
 */
 
#ifndef VISITOR_H_
#define VISITOR_H_

#include "set_relation.h"
#include "expression.h"

class Visitor {

  public:
    // Classes in expression.h
    virtual void visitTerm(iegenlib::Term * t) {}
    virtual void visitUFCallTerm(iegenlib::UFCallTerm * t) {}
    virtual void visitTupleVarTerm(iegenlib::TupleVarTerm * t) {}
    virtual void visitVarTerm(iegenlib::VarTerm * t) {}
    virtual void visitTupleExpTerm(iegenlib::TupleExpTerm * t) {}
    virtual void visitExp(iegenlib::Exp * e) {}
   
    // Classes in set_relation.h
    virtual void visitConjunction(iegenlib::Conjunction * c) {}
    virtual void visitSparseConstraints(iegenlib::SparseConstraints * sc) {}
    virtual void visitSet(iegenlib::Set * s) {}
    virtual void visitRelation(iegenlib::Relation * r) {}
   
};


#endif
