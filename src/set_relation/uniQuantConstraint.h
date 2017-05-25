/*!
 * \file uniQuantConstraint.h
 *
 * \brief Interface of uniQuantConstraint class
 *
 * \date Started: 2017-04-21
 *
 * \authors Michelle Strout, Mahdi Soltan Mohammadi
 *
 * Copyright (c) 2017-, University of Arizona <br>
 * All rights reserved. 
 * See ../../COPYING for details. <br>
 */
 
#ifndef UNIQUANTCONSTRAINT_H_
#define UNIQUANTCONSTRAINT_H_

#include <vector>
#include <string>
#include <sstream>

namespace iegenlib{

/*!
** This is the basic structure for storing different domain information
** about uninterpreted function symbols in the environment, these 
** information can be of the types:

    Forall e1, e2, if e1 <> e2 then f1(e1) <> f2(e2)
    Forall e1, e2, if f1(e1) <> f2(e2) then e1 <> e2
    <>, ... can be: = < <= >=

 * \class uniQuantConstraint
 */
class uniQuantConstraint {
public:
  uniQuantConstraint(){}
  ~uniQuantConstraint(){}

  //! Copy constructor.
  uniQuantConstraint( const uniQuantConstraint& other ){ *this = other; }

  //! Assignment operator.
  uniQuantConstraint& operator=( const uniQuantConstraint& other ){   
    type = other.type;
    expCompOp = other.expCompOp;
    ufCompOp = other.ufCompOp;
    ufSymbol1 = other.ufSymbol1;
    ufSymbol2 = other.ufSymbol2;
    return *this;
  }

  //! prints the content of the map into a std::string, and returns it
  std::string toString(){
    std::stringstream ss;

    if( type =="1" ){
      ss <<"Forall e1, e2: if e1 "<<expCompOp<<" e2 -> "
         <<ufSymbol1<<"(e1) "<<ufCompOp<<" "<<ufSymbol2<<"(e2)";
    } else {
      ss <<"Forall e1, e2: if "<<ufSymbol1<<"(e1) "<<ufCompOp<<" "<<ufSymbol2
         <<"(e2) -> e1 "<<expCompOp<<" e2";
    }
    return ss.str();
  }

  void setType(std::string f){ type = f; }
  void setExpCompOp(std::string f){ expCompOp = f; }
  void setUfCompOp(std::string f){ ufCompOp = f; }
  void setUfSymbol1(std::string f){ ufSymbol1 = f; }
  void setUfSymbol2(std::string f){ ufSymbol2 = f; }

  std::string getType(){return type;}
  std::string getExpCompOp(){ return expCompOp; }
  std::string getUfCompOp(){ return ufCompOp; }
  std::string getUfSymbol1(){ return ufSymbol1; }
  std::string getUfSymbol2(){ return ufSymbol2; }

private:
  std::string type;
  std::string expCompOp;
  std::string ufCompOp;
  std::string ufSymbol1;
  std::string ufSymbol2;
};

}

#endif
