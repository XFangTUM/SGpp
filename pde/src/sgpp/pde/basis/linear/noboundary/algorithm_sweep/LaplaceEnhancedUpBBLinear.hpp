// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#ifndef LAPLACEENHANCEDUPBBLINEAR_HPP
#define LAPLACEENHANCEDUPBBLINEAR_HPP

#include <sgpp/base/grid/GridStorage.hpp>
#include <sgpp/base/datatypes/DataVector.hpp>
#include <sgpp/base/datatypes/DataMatrix.hpp>

#ifdef __SSE3__
#include "immintrin.h"
#endif

#include <sgpp/globaldef.hpp>


namespace SGPP {
namespace pde {

/**
 * Implementation of sweep operator () for
 * enhanced Laplace operator, up operation.
 *
 * This sweep operator calculates all ups (L2 scalar products and
 * gradient) for a given dimension.
 */
class LaplaceEnhancedUpBBLinear {
 protected:
  typedef SGPP::base::GridStorage::grid_iterator grid_iterator;

  /// Pointer to SGPP::base::GridStorage object
  SGPP::base::GridStorage* storage;
  /// Pointer to the bounding box Obejct
  SGPP::base::BoundingBox* boundingBox;
  /// algorithmic dimensions, operator is applied in this dimensions
  const std::vector<size_t> algoDims;
  /// number of algorithmic dimensions
  const size_t numAlgoDims_;
  /// pointer to DataMatrix containing source coefficients
  float_t* ptr_source_;
  /// pointer to DataMatrix containing result coefficients
  float_t* ptr_result_;
  /// current algorithmic dimension for the overall operator
  size_t cur_algo_dim_;
  /// stretching of basis functions in current algorithmic domain
  float_t q_;
  /// translation of basis function in current algorithmic domain
  float_t t_;
#if 1
#if defined(__SSE3__) && USE_DOUBLE_PRECISION==1
  /// const. vector holding 1/2 in both components
  const __m128d half_in_;
#endif
#else
#ifdef __SSE3__
  /// const. vector holding 1/2 in both components
  const __m128d half_in_;
#endif
#endif
 public:
  /**
   * Constructor
   *
   * @param storage the grid's SGPP::base::GridStorage object
   */
  LaplaceEnhancedUpBBLinear(SGPP::base::GridStorage* storage);

  /**
   * Destructor
   */
  virtual ~LaplaceEnhancedUpBBLinear();

  /**
   * This operations performs the calculation of up in the direction of dimension <i>dim</i>
   * on a grid with fix Dirichlet 0 boundary conditions
   *
   * @param source SGPP::base::DataVector that contains the gridpoint's coefficients (values from the vector of the laplace operation)
   * @param result SGPP::base::DataVector that contains the result of the up operation
   * @param index a iterator object of the grid
   * @param dim current fixed dimension of the 'execution direction'
   */
  virtual void operator()(SGPP::base::DataMatrix& source,
                          SGPP::base::DataMatrix& result, grid_iterator& index, size_t dim);

 protected:

  /**
   * recursive function for the calculation of Up (L2 scalar product) without Bounding Box
   *
   * @param fl function value on the left boundary
   * @param fr function value on the right boundary
   * @param dim current fixed dimension of the 'execution direction', here all downs are calculated
   * @param index an iterator object of the grid
   */
  void rec(float_t& fl, float_t& fr, size_t dim, grid_iterator& index);

#if 1
#if defined(__SSE3__) && USE_DOUBLE_PRECISION==1
  /**
   * recursive function for the calculation of merged-Down (L2 scalar products) without Bounding Box
   *
   * @param fl 2 function value on the left boundary, stored in xmm registers
   * @param fr 2 function value on the right boundary, stored in xmm registers
   * @param dim current fixed dimension of the 'execution direction', here all downs are calculated
   * @param index an iterator object of the grid
   */
  void rec_LL(__m128d& fl, __m128d& fr, size_t dim, grid_iterator& index);
#else
  /**
   * recursive function for the calculation of merged-Up (L2 scalar products) without Bounding Box
   *
   * @param fl first function value on the left boundary
   * @param fr first function value on the right boundary
   * @param fl2 second function value on the left boundary
   * @param fr2 second function value on the right boundary
   * @param dim current fixed dimension of the 'execution direction', here all downs are calculated
   * @param index an iterator object of the grid
   */
  void rec_LL(float_t& fl, float_t& fr, float_t& fl2, float_t& fr2, size_t dim,
              grid_iterator& index);
#endif
#else
#ifdef __SSE3__
  /**
   * recursive function for the calculation of merged-Down (L2 scalar products) without Bounding Box
   *
   * @param fl 2 function value on the left boundary, stored in xmm registers
   * @param fr 2 function value on the right boundary, stored in xmm registers
   * @param dim current fixed dimension of the 'execution direction', here all downs are calculated
   * @param index an iterator object of the grid
   */
  void rec_LL(__m128d& fl, __m128d& fr, size_t dim, grid_iterator& index);
#else
  /**
   * recursive function for the calculation of merged-Up (L2 scalar products) without Bounding Box
   *
   * @param fl first function value on the left boundary
   * @param fr first function value on the right boundary
   * @param fl2 second function value on the left boundary
   * @param fr2 second function value on the right boundary
   * @param dim current fixed dimension of the 'execution direction', here all downs are calculated
   * @param index an iterator object of the grid
   */
  void rec_LL(float_t& fl, float_t& fr, float_t& fl2, float_t& fr2, size_t dim,
              grid_iterator& index);
#endif
#endif

  /**
   * recursive function for the calculation of merged-Up (gradient and L2 scalar product) without Bounding Box
   *
   * @param fl first function value on the left boundary, L2 scalar product
   * @param fr first function value on the right boundary, L2 scalar product
   * @param dim current fixed dimension of the 'execution direction', here all downs are calculated
   * @param index an iterator object of the grid
   */
  void rec_LG(float_t& fl, float_t& fr, size_t dim, grid_iterator& index);

  /**
   * recursive function for the calculation of merged-Up (L2 scalar product and gradient) without Bounding Box
   *
   * @param fl first function value on the left boundary, L2 scalar product
   * @param fr first function value on the right boundary, L2 scalar product
   * @param dim current fixed dimension of the 'execution direction', here all downs are calculated
   * @param index an iterator object of the grid
   */
  void rec_GL(float_t& fl, float_t& fr, size_t dim, grid_iterator& index);

  /**
   * recursive function for the calculation of Up (gradient) without Bounding Box
   * @param dim current fixed dimension of the 'execution direction', here all downs are calculated
   * @param index an iterator object of the grid
   */
  void rec_grad(size_t dim, grid_iterator& index);

  /**
   * recursive function for the calculation of Up (L2 scalar product) with Bounding Box
   *
   * @param fl function value on the left boundary
   * @param fr function value on the right boundary
   * @param dim current fixed dimension of the 'execution direction', here all downs are calculated
   * @param index an iterator object of the grid
   */
  void recBB(float_t& fl, float_t& fr, size_t dim, grid_iterator& index);

  /**
   * recursive function for the calculation of merged-Up (L2 scalar products) with Bounding Box
   *
   * @param fl first function value on the left boundary
   * @param fr first function value on the right boundary
   * @param fl2 second function value on the left boundary
   * @param fr2 second function value on the right boundary
   * @param dim current fixed dimension of the 'execution direction', here all downs are calculated
   * @param index an iterator object of the grid
   */
  void recBB_LL(float_t& fl, float_t& fr, float_t& fl2, float_t& fr2, size_t dim,
                grid_iterator& index);

  /**
   * recursive function for the calculation of merged-Up (L2 scalar product and gradient) with Bounding Box
   *
   * @param fl first function value on the left boundary, L2 scalar product
   * @param fr first function value on the right boundary, L2 scalar product
   * @param dim current fixed dimension of the 'execution direction', here all downs are calculated
   * @param index an iterator object of the grid
   */
  void recBB_LG(float_t& fl, float_t& fr, size_t dim, grid_iterator& index);

  /**
   * recursive function for the calculation of merged-Up (gradient and L2 scalar product) with Bounding Box
   *
   * @param fl first function value on the left boundary, L2 scalar product
   * @param fr first function value on the right boundary, L2 scalar product
   * @param dim current fixed dimension of the 'execution direction', here all downs are calculated
   * @param index an iterator object of the grid
   */
  void recBB_GL(float_t& fl, float_t& fr, size_t dim, grid_iterator& index);

  /**
   * recursive function for the calculation of Up (gradient) with Bounding Box
   * @param dim current fixed dimension of the 'execution direction', here all downs are calculated
   * @param index an iterator object of the grid
   */
  void recBB_grad(size_t dim, grid_iterator& index);
};

// namespace detail
}
// namespace SGPP
}

#endif /* LAPLACEENHANCEDUPBBLINEAR_HPP */
