#pragma once
#include "Expr.h"
#include "IRForwardDecl.h"

namespace swift { namespace ir {
class ArrayExpr :
  public swift::ir::Expr {
public:
  ArrayExpr();
  virtual ~ArrayExpr();
};

}
}