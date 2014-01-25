/*
 * TypeDomain.cpp
 *
 *  Created on: Nov 2, 2013
 *      Author: yiwu
 */
#include "TypeDomain.h"

#include "OriginAttr.h"
#include "Ty.h"
#include "NameTy.h"
#include "NumberStmt.h"

namespace swift {
namespace ir {

TypeDomain::TypeDomain(const std::string& name) :
    name(name), refer(NULL), prelen(0) {
}

TypeDomain::~TypeDomain() {
}

const std::string& TypeDomain::getName() const {
  return name;
}

void TypeDomain::setRefer(const NameTy* ref) {
  refer = ref;
}

const NameTy* TypeDomain::getRefer() const {
  return refer;
}

void TypeDomain::setPreLen(size_t l) {
  prelen = l;
  instName.resize(l);
}

size_t TypeDomain::getPreLen() const {
  return prelen;
}

void TypeDomain::addNumberStmt(std::shared_ptr<NumberStmt> num) {
  gen.push_back(num);
}

const std::vector<std::shared_ptr<NumberStmt> >& TypeDomain::getAllNumberStmt() const {
  return gen;
}

std::shared_ptr<NumberStmt> TypeDomain::getNumberStmt(size_t k) const {
  return gen[k];
}

size_t TypeDomain::getNumberStmtSize() const {
  return gen.size();
}

void TypeDomain::addOrigin(OriginAttr* o) {
  originID[o->getName()] = origin.size();
  o->setID(origin.size());
  origin.push_back(o);
}

const std::vector<OriginAttr*>& TypeDomain::getAllOrigin() const {
  return origin;
}

OriginAttr* TypeDomain::getOrigin(size_t k) const {
  return origin[k];
}

size_t TypeDomain::getOriginSize() const {
  return origin.size();
}

int TypeDomain::getOriginID(const std::string& str) {
  if (originID.find(str) == originID.end())
    return -1;
  return (int) originID[str];
}

OriginAttr* TypeDomain::getOrigin(const std::string& str) {
  int id = getOriginID(str);
  if (id < 0)
    return NULL;
  return origin[id];
}

void TypeDomain::setInstName(size_t k, std::string name) {
  if (k < prelen)
    instName[k] = name;
}

const std::string& TypeDomain::getInstName(size_t k) const {
  return instName[k];
}

const std::vector<std::string>& TypeDomain::getInstNames() const {
  return instName;
}

void TypeDomain::addReferFun(std::shared_ptr<FuncDefn> fn) {
  usedBy.push_back(fn);
}

std::vector<std::shared_ptr<FuncDefn> > & TypeDomain::getReferFuns() {
  return usedBy;
}

}
}
