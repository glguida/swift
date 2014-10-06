/*
 * Multinomial.cpp
 *
 *  Created on: Feb 28, 2014
 *      Author: yiwu
 */
 
#include <cmath>
#include <algorithm>
#include "Multinomial.h"

namespace swift {
namespace random {

Multinomial::Multinomial() : dist(0, 1.0) {
}

Multinomial::~Multinomial() {
}

// Make sure that the second argument is the number of samples to generate
void Multinomial::init(int n, ...) {
  double val;
  va_list vl;
  va_start(vl, n);
  k = va_arg(vl, int); --n;
  weight.resize(n);
  for (int i = 0; i<n; i++) {
    weight[i] = va_arg(vl, double);
    if (i>0) weight[i] += weight[i - 1];
  }
  va_end(vl);
  sum_wei = weight.back();
  is_logwei_ok.clear();
}

void Multinomial::init(const std::vector<double>& wei) {
  weight = wei;
  k = wei.size();
  for (size_t i = 1; i < weight.size(); ++ i)
    weight[i] += weight[i - 1];
  sum_wei = weight.back();
  is_logwei_ok.clear();
}

void Multinomial::init(const std::vector<double>& wei, int k) {
  weight = wei;
  this->k = k;
  for (size_t i = 1; i < weight.size(); ++i)
    weight[i] += weight[i - 1];
  sum_wei = weight.back();
  is_logwei_ok.clear();
}

void Multinomial::init(const double* wei, int n) {
  init(wei, wei + n);
}

void Multinomial::init(const double* begin, const double* end) {
  weight.clear();
  sum_wei = 0;
  for (const double* ptr = begin; ptr != end; ++ ptr)
    weight.push_back(sum_wei += *ptr);
  k = weight.size();
  is_logwei_ok.clear();
}

void Multinomial::init(const arma::mat& wei) {
  init(wei.memptr(), wei.memptr() + wei.n_elem);
}

void Multinomial::init(const arma::mat& wei, int k) {
  init(wei.memptr(), wei.memptr() + wei.n_elem);
  this->k = k;
}

std::vector<int> Multinomial::gen() {
  return gen(k);
}

std::vector<int> Multinomial::gen(int n) {
  std::vector<double> key;
  for (int i = 0; i < n; ++ i)
    key.push_back(dist(engine) * sum_wei);
  std::sort(key.begin(), key.end());
  std::vector<int> ret(weight.size());
  int ptr = 0;
  for (auto& k : key) {
    while (weight[ptr] < k) ++ ptr;
    ret[ptr] ++;
  }
  return ret;
}

double Multinomial::likeli(const std::vector<int>& x) {
  if (x.size() != weight.size()) return 0;
  double ret = 1;
  for (size_t i = 0; i < x.size(); ++i) {
    if (x[i] > 0) {
      double w = (!i ? weight[0] : weight[i] - weight[i - 1]);
      ret *= std::pow(w, x[i]);
    }
  }
  return ret;
}

double Multinomial::loglikeli(const std::vector<int>& x) {
  if (x.size() != weight.size()) return -INFINITY;
  if(is_logwei_ok.size() == 0) {
    is_logwei_ok.resize(x.size());
    log_weight.resize(x.size());
  }
  double ret = 0;
  for(size_t i = 0; i < x.size(); ++ i) {
    if (x[i]<=0) continue;
    if(is_logwei_ok[i])
      ret += log_weight[i] * x[i];
    else {
      is_logwei_ok[i] = true;
      ret += 
      x[i] * (log_weight[i] = 
      std::log(i == 0 ? weight[0] : weight[i] - weight[i-1]));
    }
  }
  return ret;
}

} /* namespace random */
} /* namespace swift */

