// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#ifndef OPERATIONDENSITYMARGTO1D_HPP_
#define OPERATIONDENSITYMARGTO1D_HPP_

#include <sgpp/base/grid/Grid.hpp>

#include <sgpp/globaldef.hpp>

#include <vector>

namespace sgpp {
namespace datadriven {

/**
 * Marginalize Probability Density Function
 */

class OperationDensityMargTo1D {
 public:
  explicit OperationDensityMargTo1D(base::Grid* grid) : grid(grid) {}
  virtual ~OperationDensityMargTo1D() {}

  /**
   * Keep applying marginalizes to (Density) Functions, until it's reduced to 1 dimension (dim_x)
   *
   * @param alpha Coefficient vector for current grid
   * @param grid_x output 1D grid pointer
   * @param alpha_x Coefficient vector for new grid (grid_x). Will be initialized.
   * @param dim_x Target dimension, all other dimensions will be marginalized
   */
  virtual void margToDimX(base::DataVector* alpha, base::Grid*& grid_x, base::DataVector*& alpha_x,
                          size_t dim_x);

  /**
   * Keep applying marginalizes to (Density) Functions, until it's reduced to d dimensions (dim_x)
   *
   * @param alpha Coefficient vector for current grid
   * @param grid_x output 1D grid pointer
   * @param alpha_x Coefficient vector for new grid (grid_x). Will be initialized.
   * @param dim_x Target dimension, all other dimensions will be marginalized
   */
  virtual void margToDimXs(base::DataVector* alpha, base::Grid*& grid_x, base::DataVector*& alpha_x,
                           std::vector<size_t>& dim_x);

 protected:
  base::Grid* grid;

  void computeMarginalizationIndices(std::vector<size_t>& dim_x, size_t numDims,
                                     std::vector<size_t>& margDims);

  void marg_next_dim(base::Grid* g_in, base::DataVector* a_in, base::Grid*& g_out,
                     base::DataVector*& a_out, std::vector<size_t> margDims, size_t ix);
};
}  // namespace datadriven
}  // namespace sgpp
#endif /* OPERATIONDENSITYMARGTO1D_HPP_ */
