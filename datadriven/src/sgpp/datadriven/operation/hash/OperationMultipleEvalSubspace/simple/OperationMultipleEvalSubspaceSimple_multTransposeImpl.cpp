// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#include <sgpp/datadriven/operation/hash/OperationMultipleEvalSubspace/simple/OperationMultipleEvalSubspaceSimple.hpp>

#include <sgpp/globaldef.hpp>

namespace sgpp {
namespace datadriven {

/**
 * Internal eval operator, should not be called directly.
 *
 * @see OperationMultipleEval
 *
 * @param alpha surplusses of the grid
 * @param result will contain the evaluation results for the given range.
 * @param start_index_data beginning of the range to evaluate
 * @param end_index_data end of the range to evaluate
 */
void OperationMultipleEvalSubspaceSimple::multTransposeImpl(sgpp::base::DataVector& alpha,
                                                            sgpp::base::DataVector& result,
                                                            // const size_t start_index_grid,
                                                            // const size_t end_index_grid,
                                                            const size_t start_index_data,
                                                            const size_t end_index_data) {
  size_t tid = omp_get_thread_num();

  if (tid == 0) {
    this->setCoefficients(result);
  }

#pragma omp barrier

  size_t dim = this->dataset.getNcols();

  base::DataVector dataTuple(dim);
  double* dataTuplePtr = dataTuple.getPointer();

  size_t* indexPtr = new size_t[dim];
  double* evalIndexValues = new double[dim + 1];

  // double **flatLevels = this->flatLevels;

  // for faster index flattening
  size_t* intermediates = new size_t[dim + 1];

  // double maxIndex = sizeof(allSubspaces) / (subspaceSize * sizeof(size_t));
  size_t maxIndex = subspaceCount * subspaceSize;

  for (size_t dataIndex = start_index_data; dataIndex < end_index_data; dataIndex++) {
    evalIndexValues[0] = 1.0;
    intermediates[0] = 0;
    dataset.getRow(dataIndex, dataTuple);
    size_t levelIndex = 0;
    size_t nextIterationToRecalc = 0;  // all index components are recalculated

    while (levelIndex < maxIndex) {
      size_t* hInversePtr = allSubspaces + levelIndex + dim;
      // size_t *levelPtr = allSubspaces + levelIndex;
      size_t linearSurplusIndex = *(allSubspaces + levelIndex + (2 * dim) + 1);
      // double *levelArray = flatLevels[levelFlat];
      double* levelArray = &(this->allSurplusses[linearSurplusIndex]);

// calculate index
#if X86SIMPLE_ENABLE_PARTIAL_RESULT_REUSAGE == 1

      for (size_t i = nextIterationToRecalc; i < dim; i++) {
#else

      for (size_t i = 0; i < dim; i++) {
#endif
        double unadjusted = dataTuplePtr[i] * static_cast<double>(hInversePtr[i]);
        indexPtr[i] = calculateIndexComponent(unadjusted);
      }

      size_t indexFlat =
          this->flattenIndex(intermediates, dim, hInversePtr, indexPtr, nextIterationToRecalc);
      double surplus = levelArray[indexFlat];

      if (!std::isnan(surplus)) {
// prepare the values for the individual components
#if X86SIMPLE_ENABLE_PARTIAL_RESULT_REUSAGE == 1
        double phiEval = evalIndexValues[nextIterationToRecalc];

        for (size_t i = nextIterationToRecalc; i < dim; i++) {
#else
        double phiEval = 1.0;

        for (size_t i = 0; i < dim; i++) {
#endif
          double phi1DEval = static_cast<double>(hInversePtr[i]) * dataTuplePtr[i] -
                             static_cast<double>(indexPtr[i]);
          phi1DEval = std::max(0.0, 1.0 - fabs(phi1DEval));
          phiEval *= phi1DEval;
          evalIndexValues[i + 1] = phiEval;
        }

        double partialSurplus = phiEval * alpha[dataIndex];

#pragma omp atomic
        levelArray[indexFlat] += partialSurplus;

        nextIterationToRecalc = allSubspaces[levelIndex + (2 * dim) + 2];
        levelIndex += subspaceSize;
      } else {
#if X86SIMPLE_ENABLE_SUBSPACE_SKIPPING == 1
        // skip to next relevant subspace
        nextIterationToRecalc = allSubspaces[levelIndex + (2 * dim) + 3];
        levelIndex = allSubspaces[levelIndex + 2 * dim];
#else
        nextIterationToRecalc = allSubspaces[levelIndex + (2 * dim) + 2];
        levelIndex += subspaceSize;
#endif
      }

    }  // end iterate subspaces

  }  // end iterate data points

  delete[] indexPtr;
  delete[] evalIndexValues;
  delete[] intermediates;

#pragma omp barrier

  if (tid == 0) {
    this->unflatten(result);
  }
}
}
}
