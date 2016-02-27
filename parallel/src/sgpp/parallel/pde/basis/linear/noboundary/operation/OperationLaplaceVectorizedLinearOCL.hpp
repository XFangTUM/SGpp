// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#ifndef OPERATIONLAPLACEVECTORIZEDLINEAROCL_HPP
#define OPERATIONLAPLACEVECTORIZEDLINEAROCL_HPP

#include <sgpp/base/operation/hash/OperationMatrix.hpp>
#include <sgpp/base/datatypes/DataMatrix.hpp>
#include <sgpp/base/grid/Grid.hpp>
#include <sgpp/parallel/pde/basis/common/OCLPDEKernels.hpp>

#include <sgpp/globaldef.hpp>

namespace sgpp {
namespace parallel {

/**
 * Implementation for linear functions of Laplace Operation, linear grids without boundaries
 *
 */
class OperationLaplaceVectorizedLinearOCL : public sgpp::base::OperationMatrix {
 private:
  sgpp::base::GridStorage* storage;
  sgpp::base::DataMatrix* level_;
  sgpp::base::DataMatrix* level_int_;
  sgpp::base::DataMatrix* index_;
  double* lcl_q;
  double* lcl_q_inv;
  sgpp::base::DataVector* lambda;
  OCLPDEKernels OCLPDEKernelsHandle;

 public:
  /**
   * Construtor of OperationLaplaceLinear
   *
   * @param storage Pointer to the grid's gridstorage obejct
   * @param lambda Vector which contains pre-factors for every dimension of the operator
   */
  OperationLaplaceVectorizedLinearOCL(sgpp::base::GridStorage* storage,
                                      sgpp::base::DataVector& lambda);

  /**
   * Construtor of OperationLaplaceLinear
   *
   * @param storage Pointer to the grid's gridstorage obejct
   */
  explicit OperationLaplaceVectorizedLinearOCL(sgpp::base::GridStorage* storage);

  /**
   * Destructor
   */
  virtual ~OperationLaplaceVectorizedLinearOCL();

  virtual void mult(sgpp::base::DataVector& alpha, sgpp::base::DataVector& result);
};
}  // namespace parallel
}  // namespace sgpp

#endif /* OPERATIONLAPLACEVECTORIZEDLINEAROCL_HPP */
