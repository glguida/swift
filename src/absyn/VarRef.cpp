/*
 * VarRef.cpp
 *
 *  Created on: Oct 29, 2013
 *      Author: yiwu
 */

#include "VarRef.h"

namespace swift {
namespace absyn {

VarRef::VarRef(int l, int c, Symbol var) :
    Expr(l, c), var(var) {
}

VarRef::~VarRef() {
}

const Symbol& VarRef::getVar() {
  return var;
}

// For Debugging Use
void VarRef::print(FILE* file, int indent) {
  fprintf(file, "%*s(VarRef: %s )\n", indent, "", var.getValue().c_str());
}

std::string VarRef::toString() {
  return var.getValue();
}

Expr* VarRef::clone() {
  return new VarRef(line, col, var);
}

}
}
