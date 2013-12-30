#pragma once
#include "Distribution.h"
#include <memory>

namespace swift { namespace ir {

class UniformChoiceDistr :
  public swift::ir::Distribution {
public:
  UniformChoiceDistr(std::shared_ptr<SetExpr> b);
  virtual ~UniformChoiceDistr();

  std::shared_ptr<SetExpr> getBody() const ;

private:
  std::shared_ptr<SetExpr> body;
};

}
}