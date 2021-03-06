/*
 * Ty.cpp
 *
 *  Created on: Nov 2, 2013
 *      Author: leili
 */
#include "Ty.h"

#include "IRConst.h"

namespace swift {
namespace ir {
Ty::Ty(IRConstant typ) :
    typ(typ) {
}

Ty::~Ty() {
}

IRConstant Ty::getTyp() const {
  return typ;
}

std::string Ty::toString() const {
  // Type: for class Ty
  switch (typ) {
  case IRConstant::BOOL:
    return IRConstString::BOOL;
  case IRConstant::INT:
    return IRConstString::INT;
  case IRConstant::DOUBLE:
    return IRConstString::DOUBLE;
  case IRConstant::STRING:
    return IRConstString::STRING;
  case IRConstant::TIMESTEP:
    // TODO: To specify the type for timestamp in C++ code
    return IRConstString::TIMESTEP;
  case IRConstant::MATRIX:
    return IRConstString::MATRIX;
  default:
    return IRConstString::NA;
  }
}

}
}
