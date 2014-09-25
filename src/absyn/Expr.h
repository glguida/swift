/*
 * Expr.h
 *
 *  Created on: Oct 28, 2013
 *      Author: leili
 */

#pragma once

#include<vector>
#include<string>
#include "Absyn.h"

namespace swift {
namespace absyn {

class Expr: public Absyn {
public:
  Expr(int l, int c);
  virtual ~Expr();

  void add(Expr* e);
  Expr* get(size_t id);
  size_t size();
  void clearArgs();

  virtual std::string toString() = 0;
  virtual Expr* clone() = 0;

protected:
  std::vector<Expr*> args;

  virtual void cloneArgs();
};

}
}
