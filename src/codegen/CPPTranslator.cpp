/*
 * CPPTranslator.cpp
 *
 *  Created on: Nov 23, 2013
 *      Author: leili
 */

#include "CPPTranslator.h"

namespace swift {
namespace codegen {
const std::string CPPTranslator::VECTOR_CLASS_NAME = "vector";
const std::string CPPTranslator::VECTOR_RESIZE_METHOD_NAME = "resize";
const code::Type CPPTranslator::INT_TYPE("int");
const code::Type CPPTranslator::INT_REF_TYPE("int", true);
const code::Type CPPTranslator::INT_VECTOR_TYPE(VECTOR_CLASS_NAME, {INT_TYPE});
const code::Type CPPTranslator::DOUBLE_TYPE("double");
const code::Type CPPTranslator::DOUBLE_VECTOR_TYPE(VECTOR_CLASS_NAME, {DOUBLE_TYPE});
const code::Type CPPTranslator::STRING_TYPE("string");
const code::Type CPPTranslator::BOOL_TYPE("bool");
const code::Type CPPTranslator::VOID_TYPE("void");
const code::Type CPPTranslator::MAP_BASE_TYPE("map");
const std::string CPPTranslator::SET_EVIDENCE_FUN_NAME = "_set_evidence";
const std::string CPPTranslator::QUERY_EVALUATE_FUN_NAME = "_evaluate_query";
const std::string CPPTranslator::DISTINCT_FIELDNAME = "_name";
const std::string CPPTranslator::DISTRIBUTION_INIT_FUN_NAME = "init";
const std::string CPPTranslator::DISTRIBUTION_GEN_FUN_NAME = "gen";
const std::string CPPTranslator::DISTRIBUTION_LIKELI_FUN_NAME = "likeli";
const std::string CPPTranslator::DISTRIBUTION_LOGLIKELI_FUN_NAME = "loglikeli";
const std::string CPPTranslator::CURRENT_SAMPLE_NUM_VARNAME = "_cur_loop";
const std::string CPPTranslator::WEIGHT_VAR_REF_NAME = "__local_weight";
const std::string CPPTranslator::GLOBAL_WEIGHT_VARNAME = "_weight";
const std::string CPPTranslator::VALUE_VAR_REF_NAME = "__value";
const std::string CPPTranslator::VALUE_ARG_NAME = "__value_arg";
const std::string CPPTranslator::MARK_VAR_REF_NAME = "__mark";
const std::string CPPTranslator::HISTOGRAM_CLASS_NAME = "Hist";
const std::string CPPTranslator::HISTOGRAM_ADD_METHOD_NAME = "add";
const std::string CPPTranslator::HISTOGRAM_PRINT_METHOD_NAME = "print";
const std::string CPPTranslator::ANSWER_VAR_NAME_PREFIX = "_answer_";
const std::string CPPTranslator::ANSWER_PRINT_METHOD_NAME = "print";
const std::string CPPTranslator::MAIN_DEBUG_METHOD_NAME = "debug";
const std::string CPPTranslator::MAIN_SAMPLING_FUN_NAME = "sample";
const std::string CPPTranslator::SAMPLER_VAR_NAME = "sampler";
const std::string CPPTranslator::MAIN_NAMESPACE_NAME = "swift";
const std::string CPPTranslator::MAIN_INIT_FUN_NAME = "init";
const std::string CPPTranslator::MAIN_FUN_NAME = "main";
const std::string CPPTranslator::LOCAL_NUM_SAMPLE_ARG_NAME = "n";
// randomness
const std::string CPPTranslator::RANDOM_DEVICE_VAR_NAME = "__random_device";
const code::Type CPPTranslator::RANDOM_ENGINE_TYPE(
    "std::default_random_engine");
const std::string CPPTranslator::RANDOM_ENGINE_VAR_NAME = "__random_engine";
const int CPPTranslator::INIT_SAMPLE_NUM = 0;
const int CPPTranslator::TOTAL_NUM_SAMPLES = 1000000;

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

/**
 * given the name of a variable (can be number var, or random function)
 * return the function name to get the likelihood of objects for this type
 */
std::string getLikeliFunName(std::string name) {
  return "__likeli_" + name;
}

/**
 * given the name of a variable (can be number var, or random function)
 * return the function name to set the value
 */
std::string getSetterFunName(std::string name) {
  return "__set_" + name;
}

/**
 * given the name of a variable (can be number var, or random function)
 * return the variable name to get the number of samples for the current variable
 */
std::string getMarkVarName(std::string name) {
  return "__mark_" + name;
}

/**
 * given the name of a variable (can be number var, or random function)
 * return the variable name to get the value of that var/fun
 */
std::string getValueVarName(std::string name) {
  return "__value_" + name;
}

CPPTranslator::CPPTranslator() {
  useTag = false;
  prog = new code::Code();
  coreNs = new code::NamespaceDecl(MAIN_NAMESPACE_NAME);
  prog->addDecl(coreNs);
  coreCls = NULL;
  coreClsInit = NULL;
  coreClsConstructor = NULL;
  coreClsPrint = NULL;
  coreClsDebug = NULL;
  mainFun = NULL;
}

CPPTranslator::~CPPTranslator() {
  if (!useTag)
    delete prog;
}

void CPPTranslator::translate(swift::ir::BlogModel* model) {
  if (coreCls == NULL) {
    coreCls = code::ClassDecl::createClassDecl(coreNs, model->getName());
    createInit();
  }

  // translate type and distinct symbols;
  for (auto ty : model->getTypes())
    transTypeDomain(ty);

  // translate random function
  for (auto fun : model->getRandFuncs())
    transFun(fun);

  // translate fixed function

  // translate evidence
  transAllEvidence(model->getEvidences());

  transSampleAlg();

  // TODO translate query
  transAllQuery(model->getQueries());

  createMain();
}

code::Code* CPPTranslator::getResult() {
  useTag = true;
  return prog;
}

code::FunctionDecl* CPPTranslator::transSampleAlg() {
  // for the moment, only supporting likelihood weighting algorithm
  // declare the sample method within coreCls
  code::FunctionDecl* fun = code::FunctionDecl::createFunctionDecl(coreCls,
      MAIN_SAMPLING_FUN_NAME, VOID_TYPE);
  std::vector<code::ParamVarDecl*> args;
  args.push_back(
      new code::ParamVarDecl(fun, LOCAL_NUM_SAMPLE_ARG_NAME, INT_TYPE));
  fun->setParams(args);
  fun->addStmt(
      new code::DeclStmt(
          new code::VarDecl(fun, WEIGHT_VAR_REF_NAME, DOUBLE_TYPE)));
  // declaring weight field in the class
  // ::: => vector<double> weight;
  code::FieldDecl::createFieldDecl(coreCls, GLOBAL_WEIGHT_VARNAME,
      DOUBLE_VECTOR_TYPE);
  // add initialization function in init()
  // :::=> weight.resize(LOCAL_NUM_SAMPLE_ARG_NAME);
  coreClsInit->addStmt(
      code::CallExpr::createMethodCall(GLOBAL_WEIGHT_VARNAME,
          VECTOR_RESIZE_METHOD_NAME, std::vector<code::Expr*>( {
              new code::Identifier(LOCAL_NUM_SAMPLE_ARG_NAME) })));
  //call the initialization function
  std::vector<code::Expr*> initArg;
  initArg.push_back(new code::Identifier(LOCAL_NUM_SAMPLE_ARG_NAME));
  fun->addStmt(
      new code::CallExpr(new code::Identifier(MAIN_INIT_FUN_NAME), initArg));
  // create for loop for the sampling
  // :::=> for (int cur_loop=0; cur_loop < number of sample; cur_loop++)
  code::Stmt* init = new code::BinaryOperator(
      new code::Identifier(CURRENT_SAMPLE_NUM_VARNAME),
      new code::IntegerLiteral(INIT_SAMPLE_NUM + 1), code::OpKind::BO_ASSIGN);
  // ::: => cur_loop < n
  code::Expr* cond = new code::BinaryOperator(
      new code::Identifier(CURRENT_SAMPLE_NUM_VARNAME),
      new code::Identifier(LOCAL_NUM_SAMPLE_ARG_NAME), code::OpKind::BO_LT);
  // ::: =>cur_loop++
  code::Expr* step = new code::BinaryOperator(
      new code::Identifier(CURRENT_SAMPLE_NUM_VARNAME), NULL,
      code::OpKind::UO_INC);
  code::CompoundStmt* body = new code::CompoundStmt();
  // :::=> weight = set_evidence();
  body->addStmt(
      new code::BinaryOperator(new code::Identifier(WEIGHT_VAR_REF_NAME),
          new code::CallExpr(new code::Identifier(SET_EVIDENCE_FUN_NAME)),
          code::OpKind::BO_ASSIGN));
  std::vector<code::Expr*> weightArg;
  weightArg.push_back(new code::Identifier(WEIGHT_VAR_REF_NAME));
  body->addStmt(
      new code::CallExpr(new code::Identifier(QUERY_EVALUATE_FUN_NAME),
          weightArg));
  // :::==> weight[current_loop] = w;
  body->addStmt(
      new code::BinaryOperator(
          new code::ArraySubscriptExpr(
              new code::Identifier(GLOBAL_WEIGHT_VARNAME),
              new code::Identifier(CURRENT_SAMPLE_NUM_VARNAME)),
          new code::Identifier(WEIGHT_VAR_REF_NAME), code::OpKind::BO_ASSIGN));
  fun->addStmt(new code::ForStmt(init, cond, step, body));
  return fun;
  //TODO adding other algorithms
}

void CPPTranslator::transTypeDomain(std::shared_ptr<ir::TypeDomain> td) {
  const std::string& name = td->getName();
  // create a class for this declared type
  code::ClassDecl* cd = code::ClassDecl::createClassDecl(coreNs, name);
  code::FieldDecl::createFieldDecl(cd, DISTINCT_FIELDNAME, STRING_TYPE);
  size_t len = td->getPreLen();
  std::string numvar = getVarOfNumType(name);
  // create a field in the main class:::    int numvar;
  code::FieldDecl::createFieldDecl(coreCls, numvar, INT_TYPE);
  // create a field in the main class:::    int mark_numvar;
  std::string marknumvar = getMarkVarName(numvar);
  code::FieldDecl::createFieldDecl(coreCls, marknumvar, INT_TYPE);
  // add in the init function:::            numvar = len;
  coreClsInit->addStmt(
      new code::BinaryOperator(new code::Identifier(numvar),
          new code::IntegerLiteral((int) len), code::OpKind::BO_ASSIGN));
  // add in the init function:::            mark_numvar = -1;
  coreClsInit->addStmt(
      new code::BinaryOperator(new code::Identifier(marknumvar),
          new code::IntegerLiteral(INIT_SAMPLE_NUM - 1),
          code::OpKind::BO_ASSIGN));
  if (len > 0) {
    // create the function for getting number of objects in this instance, i.e. numvar
    code::FunctionDecl* fun = code::FunctionDecl::createFunctionDecl(coreCls,
        getGetterFunName(numvar), INT_TYPE, true);
    fun->addStmt(new code::ReturnStmt(new code::Identifier(numvar)));
    // TODO please add the corresponding distinct name

  }
  // handle number statement
  size_t numstlen = td->getNumberStmtSize();
  if (numstlen > 0) {
    // TODO create functions for number statement and their likelihood

  }
}

void CPPTranslator::transFun(std::shared_ptr<ir::FuncDefn> fd) {
  if (fd->isRand()) {
    // translate random function
    // create gettfunname function
    transGetterFun(fd);

    // add likelihood function::: double likeli_fun(int, ...);
    transLikeliFun(fd);

    // add setter function::: double set_fun(int, ,,,);
    transSetterFun(fd);
  }
  // TODO handle fixed function
}

code::FunctionDecl* CPPTranslator::transGetterFun(
    std::shared_ptr<ir::FuncDefn> fd) {
  const std::string & name = fd->getName();
  std::string getterfunname = getGetterFunName(name);
  code::Type valuetype = mapIRTypeToCodeType(fd->getRetTyp());
  // __value__name for recording the value of the function application variable
  std::string valuevarname = getValueVarName(name);
  // adding in the main class a declaration of field for value of a random variable
  addFieldForFunVar(valuevarname, fd->getArgs());
  // __mark__name
  std::string markvarname = getMarkVarName(name);
  // adding in the main class a declaration of field for mark of a random variable
  addFieldForFunVar(markvarname, fd->getArgs());
  // define getter function :::==> __get_value()
  code::FunctionDecl* getterfun = code::FunctionDecl::createFunctionDecl(
      coreCls, getterfunname, valuetype);
  getterfun->setParams(transParamVarDecls(getterfun, fd->getArgs()));

  addFunValueRefStmt(getterfun, valuevarname, getterfun->getParams(),
      VALUE_VAR_REF_NAME);
  addFunValueRefStmt(getterfun, markvarname, getterfun->getParams(),
      MARK_VAR_REF_NAME);
  // now translating::: if (markvar == current sample num) then return value;
  code::Stmt* st = new code::IfStmt(
      new code::BinaryOperator(new code::Identifier(MARK_VAR_REF_NAME),
          new code::Identifier(CURRENT_SAMPLE_NUM_VARNAME),
          code::OpKind::BO_EQU),
      new code::ReturnStmt(new code::Identifier(VALUE_VAR_REF_NAME)), NULL);
  getterfun->addStmt(st);
  // now should sample
  // mark the variable first
  st = new code::BinaryOperator(new code::Identifier(MARK_VAR_REF_NAME),
      new code::Identifier(CURRENT_SAMPLE_NUM_VARNAME),
      code::OpKind::BO_ASSIGN);
  getterfun->addStmt(st);
  //now translate actual sampling part
  getterfun->addStmt(transClause(fd->getBody(), VALUE_VAR_REF_NAME));
  // now return the value
  getterfun->addStmt(
      new code::ReturnStmt(new code::Identifier(VALUE_VAR_REF_NAME)));
  return getterfun;
}

code::FunctionDecl* CPPTranslator::transLikeliFun(
    std::shared_ptr<ir::FuncDefn> fd) {
  const std::string & name = fd->getName();
  std::string likelifunname = getLikeliFunName(name);
  code::Type valuetype = mapIRTypeToCodeType(fd->getRetTyp());
  // __value__name for recording the value of the function application variable
  std::string valuevarname = getValueVarName(name);
  // __mark__name
  std::string markvarname = getMarkVarName(name);
  // adding method declaration in the main class
  code::FunctionDecl* likelifun = code::FunctionDecl::createFunctionDecl(
      coreCls, likelifunname, DOUBLE_TYPE);
  likelifun->setParams(transParamVarDecls(likelifun, fd->getArgs()));

  // now the value of this function app var is in VALUE_VAR_REF_NAME
  addFunValueRefStmt(likelifun, valuevarname, likelifun->getParams(),
      VALUE_VAR_REF_NAME);
  // declare the weight variable and setting its init value
  // it is recording the log likelihood
  // ::: __weight = 0
  code::VarDecl* weightvar = new code::VarDecl(likelifun, WEIGHT_VAR_REF_NAME,
      DOUBLE_TYPE, new code::FloatingLiteral(0));
  likelifun->addStmt(new code::DeclStmt(weightvar));
  // translate the Clause and calculate weight
  likelifun->addStmt(
      transClause(fd->getBody(), WEIGHT_VAR_REF_NAME, VALUE_VAR_REF_NAME));
  // now return the value
  likelifun->addStmt(
      new code::ReturnStmt(new code::Identifier(WEIGHT_VAR_REF_NAME)));
  return likelifun;
}

code::FunctionDecl* CPPTranslator::transSetterFun(
    std::shared_ptr<ir::FuncDefn> fd) {
  const std::string & name = fd->getName();
  std::string likelifunname = getLikeliFunName(name);
  code::Type valuetype = mapIRTypeToCodeType(fd->getRetTyp());
  // __value__name for recording the value of the function application variable
  std::string valuevarname = getValueVarName(name);
  // __mark__name
  std::string markvarname = getMarkVarName(name);
  std::string setterfunname = getSetterFunName(name);
  // adding setter method delcaration in the main class
  code::FunctionDecl* setterfun = code::FunctionDecl::createFunctionDecl(
      coreCls, setterfunname, DOUBLE_TYPE);
  std::vector<code::ParamVarDecl*> args_with_value = transParamVarDecls(
      setterfun, fd->getArgs());
  addFunValueRefStmt(setterfun, valuevarname, args_with_value,
      VALUE_VAR_REF_NAME);
  addFunValueRefStmt(setterfun, markvarname, args_with_value,
      MARK_VAR_REF_NAME);
  // set the argument of setter function
  args_with_value.push_back(
      new code::ParamVarDecl(setterfun, VALUE_ARG_NAME, valuetype));
  setterfun->setParams(args_with_value);
  code::Stmt* st = new code::BinaryOperator(
      new code::Identifier(MARK_VAR_REF_NAME),
      new code::Identifier(CURRENT_SAMPLE_NUM_VARNAME),
      code::OpKind::BO_ASSIGN);
  setterfun->addStmt(st);
  // :::==> value_var = value_arg
  st = new code::BinaryOperator(new code::Identifier(VALUE_VAR_REF_NAME),
      new code::Identifier(VALUE_ARG_NAME), code::OpKind::BO_ASSIGN);
  setterfun->addStmt(st);
  std::vector<code::Expr*> args_ref;
  for (auto a : fd->getArgs()) {
    args_ref.push_back(new code::Identifier(a->getVar()));
  }
  st = new code::ReturnStmt(
      new code::CallExpr(new code::Identifier(likelifunname), args_ref));
  setterfun->addStmt(st);
  return setterfun;
}

code::Stmt* CPPTranslator::transClause(std::shared_ptr<ir::Clause> clause,
    std::string retvar, std::string valuevar) {
  std::shared_ptr<ir::Branch> br = std::dynamic_pointer_cast<ir::Branch>(
      clause);
  if (br) {
    return transBranch(br, retvar, valuevar);
  }
  std::shared_ptr<ir::IfThen> ith = std::dynamic_pointer_cast<ir::IfThen>(
      clause);
  if (ith) {
    return transIfThen(ith, retvar, valuevar);
  }
  std::shared_ptr<ir::Expr> expr = std::dynamic_pointer_cast<ir::Expr>(clause);
  if (expr) {
    return new code::BinaryOperator(new code::Identifier(retvar),
        transExpr(expr, valuevar), code::OpKind::BO_ASSIGN);
    // TODO no 100% correct here why??
  }
  //todo: warning should not reach here
  return NULL;
}

code::ParamVarDecl* CPPTranslator::transParamVarDecl(code::DeclContext* context,
    const std::shared_ptr<ir::VarDecl> var) {
  return new code::ParamVarDecl(context, var->getVar(),
      mapIRTypeToCodeType(var->getTyp()));
}

std::vector<code::ParamVarDecl*> CPPTranslator::transParamVarDecls(
    code::DeclContext* context,
    const std::vector<std::shared_ptr<ir::VarDecl> > & vars) {
  std::vector<code::ParamVarDecl*> vds;
  for (auto v : vars) {
    vds.push_back(transParamVarDecl(context, v));
  }
  return vds;
}

code::Stmt* CPPTranslator::transBranch(std::shared_ptr<ir::Branch> br,
    std::string retvar, std::string valuevar) {
  code::SwitchStmt* sst = new code::SwitchStmt(transExpr(br->getVar()));
  code::CaseStmt* cst;
  for (size_t i = 0; i < br->size(); i++) {
    cst = new code::CaseStmt(transExpr(br->getCond(i)),
        transClause(br->getBranch(i), retvar, valuevar));
    sst->addStmt(cst);
    sst->addStmt(new code::BreakStmt());
  }
  return sst;
}

code::Stmt* CPPTranslator::transIfThen(std::shared_ptr<ir::IfThen> ith,
    std::string retvar, std::string valuevar) {
  code::Expr* cond = transExpr(ith->getCond());
  code::Stmt* ifcl = transClause(ith->getThen(), retvar, valuevar);
  code::Stmt* elcl = NULL;
  if (ith->getElse())
    elcl = transClause(ith->getElse(), retvar, valuevar);
  return new code::IfStmt(cond, ifcl, elcl);
}

code::Expr* CPPTranslator::transExpr(std::shared_ptr<ir::Expr> expr,
    std::string valuevar) {
  std::vector<code::Expr*> args;
  for (size_t k = 0; k < expr->argSize(); k++) {
    args.push_back(transExpr(expr->get(k)));
  }
  // warning::: better to put the above code in separate function
  // translate distribution expression
  std::shared_ptr<ir::Distribution> dist = std::dynamic_pointer_cast<
      ir::Distribution>(expr);
  if (dist) {
    return transDistribution(dist, args, valuevar);
  }

  // translate variable reference
  std::shared_ptr<ir::FunctionCall> fc = std::dynamic_pointer_cast<
      ir::FunctionCall>(expr);
  if (fc) {
    return transFunctionCall(fc, args);
  }

  std::shared_ptr<ir::ConstSymbol> cs = std::dynamic_pointer_cast<
      ir::ConstSymbol>(expr);
  if (cs) {
    return transConstSymbol(cs);
  }

  std::shared_ptr<ir::MapExpr> mex = std::dynamic_pointer_cast<ir::MapExpr>(
      expr);
  if (mex) {
    return transMapExpr(mex);
  }

  std::shared_ptr<ir::VarRefer> vref = std::dynamic_pointer_cast<ir::VarRefer>(
      expr);
  if (vref) {
    return new code::Identifier(vref->getRefer()->getVar());
  }

  std::shared_ptr<ir::OprExpr> opexp = std::dynamic_pointer_cast<ir::OprExpr>(
      expr);
  if (opexp) {
    return transOprExpr(opexp);
  }

  // TODO translate other expression
  // if valuevar is provided it should be
  return NULL;
}

code::Expr* CPPTranslator::transMapExpr(std::shared_ptr<ir::MapExpr> mex) {
  std::vector<code::Expr*> args;
  std::vector<code::Expr*> inside;
  code::Type fromType = mapIRTypeToCodeType(mex->getFromTyp());
  code::Type toType = mapIRTypeToCodeType(mex->getToTyp());
  for (size_t i = 0; i < mex->mapSize(); i++) {
    inside.clear();
    inside.push_back(transExpr(mex->getFrom(i)));
    inside.push_back(transExpr(mex->getTo(i)));
    args.push_back(new code::ListInitExpr(inside));
  }
  code::Expr* list = new code::ListInitExpr(args);
  std::vector<code::Expr*> maparg;
  maparg.push_back(list);
  // todo just a hack for the moment, need to support templated type natively
  code::Type maptype(MAP_BASE_TYPE, { fromType, toType });
  return new code::CallClassConstructor(maptype, maparg);
}

code::Expr* CPPTranslator::transOprExpr(std::shared_ptr<ir::OprExpr> opr) {
  code::OpKind kind;
  code::Expr* lhs = transExpr(opr->get(0));
  code::Expr* rhs = NULL;
  if (opr->argSize() > 1)
    rhs = transExpr(opr->get(1));
  // only need two arguments
  switch (opr->getOp()) {
  case ir::IRConstant::EQ:
    kind = code::OpKind::BO_EQU;
    break;
  case ir::IRConstant::NEQ:
    kind = code::OpKind::BO_NEQ;
    break;
  case ir::IRConstant::LE:
    kind = code::OpKind::BO_LEQ;
    break;
  case ir::IRConstant::GE:
    kind = code::OpKind::BO_GEQ;
    break;
  case ir::IRConstant::LT:
    kind = code::OpKind::BO_LT;
    break;
  case ir::IRConstant::GT:
    kind = code::OpKind::BO_GT;
    break;
  }
  return new code::BinaryOperator(lhs, rhs, kind);
  // wrong operation
  return NULL;
}

code::Expr* CPPTranslator::transDistribution(
    std::shared_ptr<ir::Distribution> dist, std::vector<code::Expr*> args,
    std::string valuevar) {
  std::string name = dist->getDistrName();
  std::string distvarname = name + std::to_string((size_t) &(dist->getArgs()));
  if (valuevar.empty()) {
    // now actual sampling a value from the distribution
    // define a field in the main class corresponding to the distribution
    code::FieldDecl::createFieldDecl(coreCls, distvarname, code::Type(name));
    //put initialization in coreClasInit
    coreClsInit->addStmt(
        code::CallExpr::createMethodCall(distvarname,
            DISTRIBUTION_INIT_FUN_NAME, args));
    // :::==> distribution.gen();
    // the following two lines of code are not used right now, just use the default engine
//    std::vector<code::Expr *> rd;
//    rd.push_back(new code::Identifier(RANDOM_ENGINE_VAR_NAME));
    return code::CallExpr::createMethodCall(distvarname,
        DISTRIBUTION_GEN_FUN_NAME);
  } else {
    // calculating likelihood
    // :::==> distribution.loglikeli
    std::vector<code::Expr *> args;
    args.push_back(new code::Identifier(valuevar));
    return code::CallExpr::createMethodCall(distvarname,
        DISTRIBUTION_LOGLIKELI_FUN_NAME, args);
  }
}

void CPPTranslator::createInit() {
  // adding setup for
  // 1. member declarations for core class, need valuearray, mark array, ...
  // 2. initialization function to initialize the values (function called in sample(n)
  // 3. print method
  // 4. debug method
  coreClsConstructor = code::ClassConstructor::createClassConstructor(coreCls);
  coreClsInit = code::FunctionDecl::createFunctionDecl(coreCls,
      MAIN_INIT_FUN_NAME, VOID_TYPE);

  // add method print() in main class to print the answers
  coreClsPrint = code::FunctionDecl::createFunctionDecl(coreCls,
      ANSWER_PRINT_METHOD_NAME, VOID_TYPE);

  // add method debug() in main class to print the current state of the possible world
  // TODO add a flag to support debug or not
  coreClsDebug = code::FunctionDecl::createFunctionDecl(coreCls,
      MAIN_DEBUG_METHOD_NAME, VOID_TYPE);

  std::vector<code::ParamVarDecl*> args;
  args.push_back(
      new code::ParamVarDecl(coreClsInit, LOCAL_NUM_SAMPLE_ARG_NAME, INT_TYPE));
  coreClsInit->setParams(args);
  code::FieldDecl::createFieldDecl(coreCls, CURRENT_SAMPLE_NUM_VARNAME,
      INT_TYPE);
  code::FieldDecl::createFieldDecl(coreCls, RANDOM_DEVICE_VAR_NAME,
      RANDOM_ENGINE_TYPE);
}

// add the in the class a field variable for a funcappvar with params
void CPPTranslator::addFieldForFunVar(std::string varname,
    const std::vector<std::shared_ptr<ir::VarDecl> >& params) {
  code::Type valueType = INT_TYPE;
  if (!params.empty()) {
    valueType = INT_VECTOR_TYPE;
    // NOTE: currently only support single argument
    // todo support more arguments!
    std::string argtypename = params[0]->toSignature();
    std::string numvarname_for_arg = getVarOfNumType(argtypename);
    // adding in the initialization function for value of a random variable
    // ::: valuevar.resize(number_of_instance);
    coreClsInit->addStmt(
        code::CallExpr::createMethodCall(varname, VECTOR_RESIZE_METHOD_NAME,
            std::vector<code::Expr*>(
                { new code::Identifier(numvarname_for_arg) })));
    // adding printing this variable in debug method
    // ::: for (int i = 0; i < length of var; i++) print( );
  }
  // adding in the main class a declaration of field for value of a random variable
  code::FieldDecl::createFieldDecl(coreCls, varname, valueType);
}

void CPPTranslator::addFunValueRefStmt(code::FunctionDecl* fun,
    std::string valuevarname, std::vector<code::ParamVarDecl*>& valueindex,
    std::string valuerefname, code::Type varType) {
  // the value of this function application variable is stored in
  // valuevarname[index1][index2]...
  // where the index are corresponding to the arguments
  code::Expr* exp = new code::Identifier(valuevarname);
  for (auto prm : valueindex) {
    exp = new code::ArraySubscriptExpr(exp, new code::Identifier(prm->getId()));
  }
//  if (valueindex.empty()) {
//    // void function does not have Parameter Variable ParamVarDecl, but we still need to access the single element
////    exp = new code::ArraySubscriptExpr(exp, new code::IntegerLiteral(0));
//  }

  // assign the function application variable value to valuerefname
  code::VarDecl* retvar = new code::VarDecl(fun, valuerefname, varType, exp);
  code::DeclStmt* dst = new code::DeclStmt(retvar);
  fun->addStmt(dst);
}

void CPPTranslator::transEvidence(code::FunctionDecl* fun,
    std::shared_ptr<ir::Evidence> evid) {
  const std::shared_ptr<ir::Expr>& left = evid->getLeft();
  // check whether left is a function application variable
  std::shared_ptr<ir::FunctionCall> leftexp = std::dynamic_pointer_cast<
      ir::FunctionCall>(left);
  if (leftexp) {
    // left side of the evidence is a function application
    std::string blogfunname = leftexp->getRefer()->getName(); // the function name in blog model
    std::string setterfunname = getSetterFunName(blogfunname); // setter function for the blog function predicate
    std::vector<code::Expr*> args;
    // now translate arguments to this setter function
    for (auto a : leftexp->getArgs()) {
      args.push_back(transExpr(a));
    }
    // assign the right side of the evidence to left function application variable
    args.push_back(transExpr(evid->getRight()));
    // call setter function and calculate likelihood
    code::Stmt* st = new code::BinaryOperator(
        new code::Identifier(WEIGHT_VAR_REF_NAME),
        new code::CallExpr(new code::Identifier(setterfunname), args),
        code::OpKind::BO_SPLUS);
    fun->addStmt(st);
  }
  // TODO adding support for other evidence
  // 1. Cardinality evidence
  // 2. Set evidence
}

void CPPTranslator::transAllEvidence(
    std::vector<std::shared_ptr<ir::Evidence> > evids) {
  code::FunctionDecl* fun = code::FunctionDecl::createFunctionDecl(coreCls,
      SET_EVIDENCE_FUN_NAME, DOUBLE_TYPE);
  code::VarDecl* weightvar = new code::VarDecl(fun, WEIGHT_VAR_REF_NAME,
      DOUBLE_TYPE, new code::FloatingLiteral(0));
  fun->addStmt(new code::DeclStmt(weightvar));
  for (auto evid : evids) {
    transEvidence(fun, evid);
  }
  fun->addStmt(new code::ReturnStmt(new code::Identifier(WEIGHT_VAR_REF_NAME)));
}

void CPPTranslator::transAllQuery(
    std::vector<std::shared_ptr<ir::Query> > queries) {
  // create evaluate function
  code::FunctionDecl* fun = code::FunctionDecl::createFunctionDecl(coreCls,
      QUERY_EVALUATE_FUN_NAME, VOID_TYPE, true);
  // setting arguments for queryfun
  std::vector<code::ParamVarDecl*> args;
  // query function has one argument of double, for weight
  args.push_back(new code::ParamVarDecl(fun, WEIGHT_VAR_REF_NAME, DOUBLE_TYPE));
  fun->setParams(args);
  int index = 0;
  for (auto qr : queries) {
    transQuery(fun, qr, index);
    ++index;
  }
}

void CPPTranslator::transQuery(code::FunctionDecl* fun,
    std::shared_ptr<ir::Query> qr, int n) {
  std::string answervarname = ANSWER_VAR_NAME_PREFIX + std::to_string(n);
  code::FieldDecl::createFieldDecl(coreCls, answervarname,
      code::Type(HISTOGRAM_CLASS_NAME, std::vector<code::Type>( {
          mapIRTypeToCodeType(qr->getVar()->getTyp()) })));
  std::vector<code::Expr*> args;
  args.push_back(transExpr(qr->getVar()));
  args.push_back(new code::Identifier(WEIGHT_VAR_REF_NAME));
  fun->addStmt(
      code::CallExpr::createMethodCall(answervarname, HISTOGRAM_ADD_METHOD_NAME,
          args));
  // add print this result in print()
  // :::=> answer_id.print();
  coreClsPrint->addStmt(
      code::CallExpr::createMethodCall(answervarname,
          HISTOGRAM_PRINT_METHOD_NAME));
}

code::Type CPPTranslator::mapIRTypeToCodeType(const ir::Ty* ty, bool isRef) {
  // todo add support for more ref type
  switch (ty->getTyp()) {
  case ir::IRConstant::BOOL:
    return BOOL_TYPE;
  case ir::IRConstant::INT:
    return INT_TYPE;
  case ir::IRConstant::DOUBLE:
    return DOUBLE_TYPE;
  case ir::IRConstant::STRING:
    return STRING_TYPE;
  default:
    return isRef ? INT_REF_TYPE : INT_TYPE; // all declared type return int type
  }
}

code::Expr* CPPTranslator::transConstSymbol(
    std::shared_ptr<ir::ConstSymbol> cs) {
  std::shared_ptr<ir::InstSymbolRef> isr = std::dynamic_pointer_cast<
      ir::InstSymbolRef>(cs);
  if (isr) {
    return new code::IntegerLiteral((int) (isr->getInst()->getID()));
  }
  std::shared_ptr<ir::BoolLiteral> bl = std::dynamic_pointer_cast<
      ir::BoolLiteral>(cs);
  if (bl) {
    return new code::BooleanLiteral(bl->getValue());
  }
  std::shared_ptr<ir::DoubleLiteral> dl = std::dynamic_pointer_cast<
      ir::DoubleLiteral>(cs);
  if (dl) {
    return new code::FloatingLiteral(dl->getValue());
  }
  std::shared_ptr<ir::IntLiteral> il =
      std::dynamic_pointer_cast<ir::IntLiteral>(cs);
  if (il) {
    return new code::IntegerLiteral(il->getValue());
  }
  std::shared_ptr<ir::NullSymbol> nl =
      std::dynamic_pointer_cast<ir::NullSymbol>(cs);
  if (bl) {
    // todo change the translation of null
    return new code::IntegerLiteral(-1);
  }
  std::shared_ptr<ir::StringLiteral> sl = std::dynamic_pointer_cast<
      ir::StringLiteral>(cs);
  if (sl) {
    return new code::StringLiteral(sl->getValue());
  }
  return NULL;
}

code::Expr* CPPTranslator::transFunctionCall(
    std::shared_ptr<ir::FunctionCall> fc, std::vector<code::Expr*> args) {
  std::string getterfunname;
  switch (fc->getKind()) {
  case ir::IRConstant::RANDOM:
    getterfunname = getGetterFunName(fc->getRefer()->getName());
    return new code::CallExpr(new code::Identifier(getterfunname), args);
  case ir::IRConstant::FIXED:
    // todo
  default:
    return NULL;
  }
}

void CPPTranslator::createMain() {
  mainFun = code::FunctionDecl::createFunctionDecl(prog, MAIN_FUN_NAME,
      INT_TYPE);
  code::Stmt* st = new code::DeclStmt(
      new code::VarDecl(mainFun, SAMPLER_VAR_NAME,
          code::Type(std::vector<std::string>( { coreNs->getName() }),
              coreCls->getName())));
  mainFun->addStmt(st);
  std::vector<code::Expr*> args;
  args.push_back(new code::IntegerLiteral(TOTAL_NUM_SAMPLES));
  st = code::CallExpr::createMethodCall(SAMPLER_VAR_NAME,
      MAIN_SAMPLING_FUN_NAME, args);
  mainFun->addStmt(st);
  st = code::CallExpr::createMethodCall(SAMPLER_VAR_NAME,
      ANSWER_PRINT_METHOD_NAME);
  mainFun->addStmt(st);
}

} /* namespace codegen */
} /* namespace swift */
