/*
 * Gaussian.h
 *
 *  Created on: Feb 6, 2014
 *      Author: yiwu
 */
 
#include <cmath>
#include "Gaussian.h"

namespace swift {
namespace random {

Gaussian::Gaussian() :
    mean(0.0), stddev(1.0) {
  coef = 1.0 / (stddev * sqrt(PI * 2));
  log_coef = - log(stddev * sqrt(PI * 2));
  scale = - 1.0 / (2 * stddev * stddev);
}

Gaussian::~Gaussian() {
}

void Gaussian::init(double mean, double stddev) {
  this->mean = mean;
  this->stddev = stddev;
  dist = std::normal_distribution<double>(mean, stddev);
}

double Gaussian::gen() {
  return dist(engine);
}

double Gaussian::likeli(double x) {
  return coef * exp(scale * (x - mean) * (x - mean));
}

double Gaussian::loglikeli(double x) {
  return - (scale * (x - mean) * (x - mean) + log_coef);
}

} /* namespace random */
} /* namespace swift */
