#include "PoissonBall.h"

namespace test_absyn { 

using namespace swift::absyn;

PoissonBall::PoissonBall() :root(NULL){}

PoissonBall::~PoissonBall() {
  if (root != NULL) delete root;
}

BlogProgram* PoissonBall::getRoot() {
  return root;
}

void PoissonBall::build(){
  BlogProgram *blog = new BlogProgram(0, 0);
  root = blog;
  /*
  type Ball;
  type Draw;
  type Color;
  */
  {
    blog->add(new TypDecl(0, 0, Symbol("Ball")));
    blog->add(new TypDecl(0, 0, Symbol("Draw")));
    blog->add(new TypDecl(0, 0, Symbol("Color")));
  }
  /*
  distinct Color Blue, Green;
  distinct Draw Draw[10];
  */
  {
    DistinctDecl*dis;
    dis = new DistinctDecl(0, 0, Symbol("Color"));
    dis->add(Symbol("Blue"), 1);
    dis->add(Symbol("Green"), 1);
    blog->add(dis);
    dis = new DistinctDecl(0, 0, Symbol("Draw"));
    dis->add(Symbol("Draw"), 10);
    blog->add(dis);
  }
  /*
  #Ball ~ Poisson(6);
  */
  {
    DistrExpr*dis;
    dis = new DistrExpr(0, 0, Symbol("Poisson"));
    dis->add(new IntLiteral(0, 0, 6));
    NumStDecl*num;
    num = new NumStDecl(0, 0, Symbol("Ball"), dis);
    blog->add(num);
  }
  /*
  random Color TrueColor(Ball b) ~ Categorical({Blue -> 0.5, Green -> 0.5});
  */
  {
    DistrExpr*cate;
    {
      MapExpr* mp;
      mp = new MapExpr(0, 0);
      mp->addMap(new VarRef(0, 0, Symbol("Blue")), new DoubleLiteral(0, 0, 0.5));
      mp->addMap(new VarRef(0, 0, Symbol("Green")), new DoubleLiteral(0, 0, 0.5));
      cate = new DistrExpr(0, 0, Symbol("Categorical"));
      cate->add(mp);
    }
    FuncDecl*fun;
    fun = new FuncDecl(0, 0, true, Symbol("Color"), Symbol("TrueColor"), cate);
    fun->addArg(VarDecl(0, 0, Symbol("Ball"), Symbol("b")));
    blog->add(fun);
  }
  /*
  random Ball BallDrawn(Draw d) ~UniformChoice({ Ball b });
  */
  {
    SetExpr*st;
    st = new CondSet(0, 0, VarDecl(0, 0, Symbol("Ball"), Symbol("b")), NULL);
    DistrExpr*uc;
    uc = new DistrExpr(0, 0, Symbol("UniformChoice"));
    uc->add(st);
    FuncDecl*fun;
    fun = new FuncDecl(0, 0, true, Symbol("Ball"), Symbol("BallDrawn"), uc);
    fun->addArg(VarDecl(0, 0, Symbol("Draw"), Symbol("d")));
    blog->add(fun);
  }
  /*
  random Color ObsColor(Draw d) {
  if (BallDrawn(d) != null)
  then ~ TabularCPD({Blue -> ~ Categorical({Blue -> 0.8, Green -> 0.2}),
  Green -> ~ Categorical({Blue -> 0.2, Green -> 0.8})},
  TrueColor(BallDrawn(d)))
  };
  */
  {
    FuncApp* ball_draw;
    ball_draw = new FuncApp(0, 0, Symbol("BallDrawn"));
    ball_draw->add(new VarRef(0, 0, Symbol("d")));
    OpExpr* cond;
    cond = new OpExpr(0,0,AbsynConstant::NEQ,ball_draw, new NullLiteral(0,0));
    DistrExpr*cate = new DistrExpr(0, 0, Symbol("TabularCPD"));;
    {
      MapExpr* mp = new MapExpr(0,0);
      {
        MapExpr*sub1 = new MapExpr(0,0);
        sub1->addMap(new VarRef(0, 0, Symbol("Blue")), new DoubleLiteral(0, 0, 0.8));
        sub1->addMap(new VarRef(0, 0, Symbol("Green")), new DoubleLiteral(0, 0, 0.2));
        DistrExpr*cate1 = new DistrExpr(0,0,Symbol("Categorical"));
        cate1->add(sub1);

        MapExpr*sub2 = new MapExpr(0, 0);
        sub2->addMap(new VarRef(0, 0, Symbol("Blue")), new DoubleLiteral(0, 0, 0.2));
        sub2->addMap(new VarRef(0, 0, Symbol("Green")), new DoubleLiteral(0, 0, 0.8));
        DistrExpr*cate2 = new DistrExpr(0, 0, Symbol("Categorical"));
        cate2->add(sub2);

        mp->addMap(new VarRef(0, 0, Symbol("Blue")), cate1);
        mp->addMap(new VarRef(0, 0, Symbol("Green")), cate2);
      }
      FuncApp* ball_draw;
      ball_draw = new FuncApp(0, 0, Symbol("BallDrawn"));
      ball_draw->add(new VarRef(0, 0, Symbol("d")));
      FuncApp* true_color;
      true_color = new FuncApp(0, 0, Symbol("TrueColor"));
      true_color->add(ball_draw);
      
      cate->add(mp);
      cate->add(true_color);
    }
    IfExpr* ift;
    ift = new IfExpr(0,0,cond,cate,NULL);
    FuncDecl*fun;
    fun = new FuncDecl(0, 0, true, Symbol("Color"), Symbol("ObsColor"), ift);
    fun->addArg(VarDecl(0, 0, Symbol("Draw"), Symbol("d")));

    blog->add(fun);
  }
  /*
  obs ObsColor(Draw[0]) = Blue;
  obs ObsColor(Draw[1]) = Green;
  obs ObsColor(Draw[2]) = Blue;
  obs ObsColor(Draw[3]) = Green;
  obs ObsColor(Draw[4]) = Blue;
  obs ObsColor(Draw[5]) = Green;
  obs ObsColor(Draw[6]) = Blue;
  obs ObsColor(Draw[7]) = Green;
  obs ObsColor(Draw[8]) = Blue;
  obs ObsColor(Draw[9]) = Green;
  */
  for (int k = 0; k < 10; ++k) {
    OpExpr* ball = new OpExpr(0, 0, AbsynConstant::SUB, new VarRef(0, 0, Symbol("Draw")), new IntLiteral(0,0,k));
    FuncApp* fun = new FuncApp(0, 0, Symbol("ObsColor"));
    fun->add(ball);
    Evidence* e = new Evidence(0,0,fun,new VarRef(0,0,Symbol(k & 1 ? "Green" : "Blue")));

    blog->add(e);
  }

  /*
  query #{Ball b};
  */
  {
    CondSet* st = new CondSet(0, 0, VarDecl(0, 0, Symbol("Ball"), Symbol("b")), NULL);
    CardinalityExpr* num = new CardinalityExpr(0, 0, st);
    Query* query = new Query(0,0,num);
    blog->add(query);
  }
}

void PoissonBall::test(FILE *file)
{
  build();
  if(file != NULL) root->print(file, 0);
}

}
