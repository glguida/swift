/*
 * CPPTranslator.cpp
 *
 *  Created on: Nov 23, 2013
 *      Author: leili
 */

#include "CPPTranslator.h"

namespace swift {
namespace codegen {

const code::QualType CPPTranslator::INT_TYPE("int");
const code::QualType CPPTranslator::DOUBLE_TYPE("double");
const code::QualType CPPTranslator::STRING_TYPE("std::string");
const std::string CPPTranslator::DISTINCT_FIELDNAME = "__name_";

namespace {
/**
 * give the name of the type,
 * return the variable name corresponding to the number of objects for this type
 */
std::string getVarOfNumType(std::string name) {
  return "__num_" + name;
}

/**
 * given the name of a variable (can be number var, or random function)
 * return the function name to get the number of objects for this type
 */
std::string getGetterFunName(std::string name) {
  return "__get_" + name;
}
}

CPPTranslator::CPPTranslator() {
  // TODO Auto-generated constructor stub

}

CPPTranslator::~CPPTranslator() {
  // TODO Auto-generated destructor stub
}

void CPPTranslator::translate(swift::ir::BlogModel* model) {
  for (auto ty : model->getTypes())
    transTypeDomain(ty);
  for (auto fun : model->getRandFuncs())
    transFun(fun);
}

code::Code* CPPTranslator::getResult() const {
  return prog;
}

void CPPTranslator::transTypeDomain(std::shared_ptr<ir::TypeDomain> td) {
  const std::string& name = td->getName();
  code::ClassDecl* cd = code::ClassDecl::createClassDecl(coreNs, name);
  code::FieldDecl::createFieldDecl(cd, DISTINCT_FIELDNAME, STRING_TYPE);
  int len = td->getPreLen();
  std::string numvar = getVarOfNumType(name);
  // create a field in the main class:::    int numvar;
  code::FieldDecl* fd = code::FieldDecl::createFieldDecl(coreCls, numvar,
      INT_TYPE);
  // add in the init function:::            numvar = len;
  coreClsInit->addStmt(
      new code::BinaryOperator(new code::VarRef(numvar),
          new code::IntegerLiteral(len), code::OpKind::BO_ASSIGN));
  if (len > 0) {
    // create the function for getting number of objects in this instance, i.e. numvar
    code::FunctionDecl* fun = code::FunctionDecl::createFunctionDecl(coreCls, getGetterFunName(numvar), INT_TYPE, true);
    fun->addStmt(new code::ReturnStmt( new code::VarRef(numvar) ) );

    // TODO please add the corresponding distinct name

  }
  // handle number statement
  int numstlen = td->getNumberStmtSize();
  if (numstlen > 0) {
    // TODO create functions for number statement and their likelihood
  }
}

void CPPTranslator::transFun(std::shared_ptr<ir::FuncDefn> td) {
  // TODO add sampleing function and likelihood calculation function
}

} /* namespace codegen */
} /* namespace swift */

