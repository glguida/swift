/*
* Decl.cpp
*
*  Created on: Oct 30, 2013
*      Author: yiwu
*/

#include "Decl.h"

namespace swift {
  namespace absyn {

Decl::Decl(int l, int c)
  :Stmt(l, c) {
}

Decl::~Decl() {
}

}}