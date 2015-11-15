/*!
 * \file Visitor.h
 *
 * \brief Visitor base class for expressions.h and set_relation.h.
 *
 * Usage:
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
    virtual void visitTerm(iegenlib::Term * t)=0;
    virtual void visitUFCallTerm(iegenlib::UFCallTerm * t)=0;
    virtual void visitTupleVarTerm(iegenlib::TupleVarTerm * t)=0;
    virtual void visitVarTerm(iegenlib::VarTerm * t)=0;
    virtual void visitTupleExpTerm(iegenlib::TupleExpTerm * t)=0;
    virtual void visitExp(iegenlib::Exp * e)=0;
   
    // Classes in set_relation.h
    virtual void visitConjunction(iegenlib::Conjunction * c)=0;
    virtual void visitSparseConstraints(iegenlib::SparseConstraints * sc)=0;
    virtual void visitSet(iegenlib::Set * s)=0;
    virtual void visitRelation(iegenlib::Relation * r)=0;
   
};


#endif
