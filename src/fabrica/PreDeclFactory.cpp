#include "PreDeclFactory.h"

#include "../predecl/PreDeclDistrList.h"

namespace swift {
namespace fabrica {

PreDeclFactory::PreDeclFactory() {
  distr[predecl::PreDeclDistrList::bernoulliDistr.getName()] =
    &predecl::PreDeclDistrList::bernoulliDistr;
  distr[predecl::PreDeclDistrList::betaDistr.getName()] =
    &predecl::PreDeclDistrList::betaDistr;
  distr[predecl::PreDeclDistrList::booleanDistr.getName()] =
    &predecl::PreDeclDistrList::booleanDistr;
  distr[predecl::PreDeclDistrList::categoricalDistr.getName()] =
    &predecl::PreDeclDistrList::categoricalDistr;
  distr[predecl::PreDeclDistrList::gammaDistr.getName()] =
    &predecl::PreDeclDistrList::gammaDistr;
  distr[predecl::PreDeclDistrList::gaussianDistr.getName()] =
    &predecl::PreDeclDistrList::gaussianDistr;
  distr[predecl::PreDeclDistrList::poissonDistr.getName()] =
      &predecl::PreDeclDistrList::poissonDistr;
  distr[predecl::PreDeclDistrList::uniformChoiceDistr.getName()] =
      &predecl::PreDeclDistrList::uniformChoiceDistr;
}

PreDeclFactory::~PreDeclFactory() {
}

const predecl::PreDeclDistr* PreDeclFactory::getDistr(std::string name) {
  if (distr.count(name) == 0)
    return NULL;
  return distr[name];
}

}
}
