#include "MapExpr.h"
#include "Ty.h"

namespace swift {
namespace ir {

MapExpr::MapExpr() :
    fromTyp(NULL), toTyp(NULL) {
}

MapExpr::~MapExpr() {
}

size_t MapExpr::mapSize() const {
  return argSize() / 2;
}

void MapExpr::addMap(std::shared_ptr<Expr> a, std::shared_ptr<Expr> b) {
  addArg(a);
  addArg(b);
}

std::shared_ptr<Expr> MapExpr::getFrom(size_t k) const {
  return get(2 * k);
}

std::shared_ptr<Expr> MapExpr::getTo(size_t k) const {
  return get(2 * k + 1);
}

const Ty* MapExpr::getFromTyp() const {
  return fromTyp;
}

const Ty* MapExpr::getToTyp() const {
  return toTyp;
}

void MapExpr::setFromTyp(const Ty * t) {
  fromTyp = t;
}

void MapExpr::setToTyp(const Ty * t) {
  toTyp = t;
}

void MapExpr::print(FILE* file, int indent) const {
  fprintf(file, "%*sMapExpr:\n", indent, "");
  fprintf(file, "%*sfromTyp: %s", indent + 2, "",
      getFromTyp()->toString().c_str());
  fprintf(file, "%*stoTyp: %s", indent + 2, "",
      getToTyp()->toString().c_str());
  for (size_t i = 0; i < mapSize(); i++) {
    fprintf(file, "%*sfrom#%d: \n", indent + 2, "", (int)i);
    getFrom(i)->print(file, indent + 4);
    fprintf(file, "%*sto#%d: \n", indent + 2, "", (int)i);
    getTo(i)->print(file, indent + 4);
  }
}

std::string MapExpr::toString() {
  std::string ret = "{";
  for (size_t i = 0; i < mapSize(); ++i) {
    if (i > 0) ret.push_back(',');
    ret.append(getFrom(i)->toString()+"->"+getTo(i)->toString());
  }
  ret.push_back('}');
  return ret;
}

}
}
