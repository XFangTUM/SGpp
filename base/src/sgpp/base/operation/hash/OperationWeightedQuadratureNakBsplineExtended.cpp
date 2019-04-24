// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#include "OperationWeightedQuadratureNakBsplineExtended.hpp"

namespace sgpp {
namespace base {

double OperationWeightedQuadratureNakBsplineExtended::doWeightedQuadrature(
    DataVector& alpha, sgpp::base::DistributionsVector pdfs, size_t quadOrder) {
  double mean = 0;
  for (size_t i = 0; i < storage.getSize(); i++) {
    double mean1D = 1;

    for (size_t d = 0; d < storage.getDimension(); d++) {
      mean1D *= base.getMean(storage.getPointLevel(i, d), storage.getPointIndex(i, d), pdfs.get(d),
                             quadOrder);
    }
    mean += alpha[i] * mean1D;
  }

  return mean;
}

}  // namespace base
}  // namespace sgpp
