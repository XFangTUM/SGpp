// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#include <sgpp/finance/basis/linear/noboundary/algorithm_sweep/XPhidPhiDownBBLinear.hpp>

#include <sgpp/globaldef.hpp>


namespace SGPP {
namespace finance {



XPhidPhiDownBBLinear::XPhidPhiDownBBLinear(SGPP::base::GridStorage* storage) :
  storage(storage), boundingBox(storage->getBoundingBox()) {
}

XPhidPhiDownBBLinear::~XPhidPhiDownBBLinear() {
}

void XPhidPhiDownBBLinear::operator()(SGPP::base::DataVector& source,
                                      SGPP::base::DataVector& result, grid_iterator& index, size_t dim) {
  float_t q = boundingBox->getIntervalWidth(dim);
  float_t t = boundingBox->getIntervalOffset(dim);

  bool useBB = false;

  if (q != 1.0 || t != 0.0) {
    useBB = true;
  }

  if (useBB) {
    recBB(source, result, index, dim, 0.0, 0.0, q, t);
  } else {
    rec(source, result, index, dim, 0.0, 0.0);
  }
}

void XPhidPhiDownBBLinear::rec(SGPP::base::DataVector& source,
                               SGPP::base::DataVector& result, grid_iterator& index, size_t dim, float_t fl,
                               float_t fr) {
  size_t seq = index.seq();

  float_t alpha_value = source[seq];

  SGPP::base::GridStorage::index_type::level_type l;
  SGPP::base::GridStorage::index_type::index_type i;

  index.get(dim, l, i);

  float_t hhalf = 1.0 / static_cast<float_t>(1 << (l + 1));
  float_t i_dbl = static_cast<float_t>(i);

  // integration
  result[seq] = (  ( (fl * ((hhalf * i_dbl) - hhalf)) + (fr * (((-1.0) *
                     (hhalf * i_dbl)) - hhalf)) ) - ((1.0 / 3.0) * (((1.0 / static_cast<float_t>
                         (1 << l))) * alpha_value)) ); // diagonal entry

  // dehierarchisation
  float_t fm = ((fl + fr) / 2.0) + alpha_value;

  if (!index.hint()) {
    index.leftChild(dim);

    if (!storage->end(index.seq())) {
      rec(source, result, index, dim, fl, fm);
    }

    index.stepRight(dim);

    if (!storage->end(index.seq())) {
      rec(source, result, index, dim, fm, fr);
    }

    index.up(dim);
  }
}

void XPhidPhiDownBBLinear::recBB(SGPP::base::DataVector& source,
                                 SGPP::base::DataVector& result, grid_iterator& index, size_t dim, float_t fl,
                                 float_t fr, float_t q, float_t t) {
  size_t seq = index.seq();

  float_t alpha_value = source[seq];

  SGPP::base::GridStorage::index_type::level_type l;
  SGPP::base::GridStorage::index_type::index_type i;

  index.get(dim, l, i);

  float_t hhalf = 1.0 / static_cast<float_t>(1 << (l + 1));
  float_t i_dbl = static_cast<float_t>(i);

  // integration
  result[seq] = (  ( (fl * ((q * ((hhalf * i_dbl) - hhalf)) + (0.5 * t))) +
                     (fr * ((q * (((-1.0) * (hhalf * i_dbl)) - hhalf)) - (0.5 * t))) )
                   - ((1.0 / 3.0) * (((1.0 / static_cast<float_t>(1 << l)) * q) *
                                     alpha_value)) ); // diagonal entry

  // dehierarchisation
  float_t fm = ((fl + fr) / 2.0) + alpha_value;

  if (!index.hint()) {
    index.leftChild(dim);

    if (!storage->end(index.seq())) {
      recBB(source, result, index, dim, fl, fm, q, t);
    }

    index.stepRight(dim);

    if (!storage->end(index.seq())) {
      recBB(source, result, index, dim, fm, fr, q, t);
    }

    index.up(dim);
  }
}

// namespace detail

} // namespace SGPP
}