// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#pragma once

#include <OrthogPolyApproximation.hpp>
#include <sgpp/base/datatypes/DataVector.hpp>
#include <sgpp/base/datatypes/DataMatrix.hpp>
#include <sgpp/combigrid/operation/CombigridOperation.hpp>
#include <sgpp/combigrid/operation/CombigridTensorOperation.hpp>
#include <sgpp/combigrid/functions/AbstractInfiniteFunctionBasis1D.hpp>

namespace sgpp {
namespace combigrid {

class PolynomialChaosExpansion {
 public:
  PolynomialChaosExpansion(
      std::shared_ptr<sgpp::combigrid::CombigridOperation> combigridOperation,
      std::shared_ptr<sgpp::combigrid::AbstractInfiniteFunctionBasis1D> functionBasis);
  virtual ~PolynomialChaosExpansion();

  double value(sgpp::base::DataVector& x);
  void values(sgpp::base::DataMatrix& x, sgpp::base::DataVector& result);

  double mean();
  double variance();
  void sobol_indices(sgpp::base::DataVector& sobol_indices);

 private:
#ifdef USE_DAKOTA
  std::shared_ptr<Pecos::OrthogPolyApproximation> orthogPoly;
#endif

  size_t numDims;
  std::shared_ptr<sgpp::combigrid::AbstractInfiniteFunctionBasis1D> functionBasis;
  std::shared_ptr<sgpp::combigrid::CombigridOperation> combigridOperation;
  std::shared_ptr<sgpp::combigrid::CombigridTensorOperation> tensorOperation;

  bool expansionCoefficientsFlag;
  sgpp::combigrid::FloatTensorVector expansionCoefficients;
};

} /* namespace combigrid */
} /* namespace sgpp */
