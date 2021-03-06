#include "AircraftSimple.h"

namespace test_absyn { 

using namespace swift::absyn;

AircraftSimple::AircraftSimple() :root(NULL){}

AircraftSimple::~AircraftSimple() {
  if (root != NULL) delete root;
}

BlogProgram* AircraftSimple::getRoot() {
  return root;
}

void AircraftSimple::build(){
  BlogProgram *blog = new BlogProgram(0, 0);
  root = blog;
  /*
  type Aircraft;
  type Blip;
  */
  {
    blog->add(new TypDecl(0, 0, Symbol("Aircraft")));
    blog->add(new TypDecl(0, 0, Symbol("Blip")));
  }
  /*
  origin Aircraft Source(Blip);
  */
  {
    OriginDecl* origin = new OriginDecl(0,0,Symbol("Aircraft"),Symbol("Source"),Symbol("Blip"));
    blog->add(origin);
  }
  /*
  #Aircraft ~ Poisson(5);
  */
  {
    FuncApp*dis;
    dis = new FuncApp(0, 0, Symbol("Poisson"));
    dis->add(new IntLiteral(0, 0, 5));
    NumStDecl*num;
    num = new NumStDecl(0, 0, Symbol("Aircraft"), dis);
    blog->add(num);
  }
  /*
  #Blip(Source=a) ~ Poisson(4);
  */
  {
    FuncApp*dis;
    dis = new FuncApp(0, 0, Symbol("Poisson"));
    dis->add(new IntLiteral(0, 0, 4));
    NumStDecl*num;
    num = new NumStDecl(0, 0, Symbol("Blip"), dis);
    num->add(Symbol("Source"), Symbol("a"));
    blog->add(num);
  }
  /*
  obs {b for Blip b} = {b1, b2, b3};
  */
  {
    TupleSetExpr* st = new TupleSetExpr(0, 0, std::vector<Expr*>({new FuncApp(0,0,Symbol("b"))}),
      std::vector<VarDecl>({ VarDecl(0, 0, Symbol("Blip"),Symbol("b")) }), NULL);
    ListSet* lst = new ListSet(0,0);
    lst->add(new FuncApp(0, 0, Symbol("b1")));
    lst->add(new FuncApp(0, 0, Symbol("b2")));
    lst->add(new FuncApp(0, 0, Symbol("b3")));
//    CardinalityExpr* num = new CardinalityExpr(0, 0, new CondSet(0, 0, VarDecl(0, 0, Symbol("Blip"))));
//    Evidence* e = new Evidence(0, 0, num, new IntLiteral(0,0,3));
    Evidence* e = new Evidence(0,0,st,lst); // Set Evidence Here!
    blog->add(e);
  }

  /*
  query size({a for Aircraft a});
  */
  {
    TupleSetExpr* st = new TupleSetExpr(0, 0, std::vector<Expr*>({ new FuncApp(0, 0, Symbol("a")) }),
      std::vector<VarDecl>({VarDecl(0,0,Symbol("Aircraft"),Symbol("a"))}),NULL);
    FuncApp* fun = new FuncApp(0,0,Symbol("size"));
    fun->add(st);
    Query* query = new Query(0,0,fun);
    blog->add(query);
  }
}

void AircraftSimple::test(FILE *file)
{
  build();
  if(file != NULL) root->print(file, 0);
}

}
