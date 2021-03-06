#include "StringLiteral.h"

#include "IRConst.h"

namespace swift {
namespace ir {
StringLiteral::StringLiteral(const std::string& v) :
    ConstSymbol(IRConstant::STRINGLITERAL), value(v) {
}

StringLiteral::~StringLiteral() {
}

const std::string& StringLiteral::getValue() const {
  return value;
}

void StringLiteral::print(FILE* file, int indent) const {
  fprintf(file, "%*s(StringLitera: %s)\n", indent, "", value.c_str());
}

std::string StringLiteral::toString() {
  return "\"" + value + "\"";
}

}
}
