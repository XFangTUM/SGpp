// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#ifndef SGPP_OPTIMIZATION_SLE_SOLVER_GAUSSIANELIMINATION_HPP
#define SGPP_OPTIMIZATION_SLE_SOLVER_GAUSSIANELIMINATION_HPP

#include <sgpp/globaldef.hpp>
#include <sgpp/optimization/sle/solver/SLESolver.hpp>

#include <cstddef>

namespace sgpp {
namespace optimization {
namespace sle_solver {

/**
 * Linear system solver implementing the direct Gaussian elimination.
 */
class GaussianElimination : public SLESolver {
 public:
  /**
   * Destructor.
   */
  ~GaussianElimination() override;

  /**
   * @param       system  system to be solved
   * @param       b       right-hand side
   * @param[out]  x       solution to the system
   * @return              whether all went well
   *                      (false if errors occurred)
   */
  bool solve(SLE& system, base::DataVector& b, base::DataVector& x) const override;
};
}  // namespace sle_solver
}  // namespace optimization
}  // namespace sgpp

#endif /* SGPP_OPTIMIZATION_SLE_SOLVER_GAUSSIANELIMINATION_HPP */
