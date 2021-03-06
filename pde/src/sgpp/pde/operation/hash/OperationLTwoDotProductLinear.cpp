// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#include <sgpp/pde/operation/hash/OperationLTwoDotProductLinear.hpp>

#include <sgpp/pde/basis/linear/noboundary/algorithm_sweep/PhiPhiDownBBLinear.hpp>
#include <sgpp/pde/basis/linear/noboundary/algorithm_sweep/PhiPhiUpBBLinear.hpp>

#include <sgpp/base/algorithm/sweep.hpp>

#include <sgpp/globaldef.hpp>

namespace sgpp {
namespace pde {

OperationLTwoDotProductLinear::OperationLTwoDotProductLinear(sgpp::base::GridStorage* storage)
    : StdUpDown(storage) {}

OperationLTwoDotProductLinear::~OperationLTwoDotProductLinear() {}

void OperationLTwoDotProductLinear::up(sgpp::base::DataVector& alpha,
                                       sgpp::base::DataVector& result, size_t dim) {
  // phi * phi
  PhiPhiUpBBLinear func(this->storage);
  sgpp::base::sweep<PhiPhiUpBBLinear> s(func, *this->storage);

  s.sweep1D(alpha, result, dim);
}

void OperationLTwoDotProductLinear::down(sgpp::base::DataVector& alpha,
                                         sgpp::base::DataVector& result, size_t dim) {
  // phi * phi
  PhiPhiDownBBLinear func(this->storage);
  sgpp::base::sweep<PhiPhiDownBBLinear> s(func, *this->storage);

  s.sweep1D(alpha, result, dim);
}
}  // namespace pde
}  // namespace sgpp
