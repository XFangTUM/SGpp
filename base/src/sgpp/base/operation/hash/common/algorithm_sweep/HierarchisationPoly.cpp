// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#include "HierarchisationPoly.hpp"
#include <sgpp/base/grid/GridStorage.hpp>
#include <sgpp/base/datatypes/DataVector.hpp>

#include <sgpp/globaldef.hpp>

#include <cmath>

namespace SGPP {

namespace base {

HierarchisationPoly::HierarchisationPoly(GridStorage* storage,
    SPolyBase* base) :
  storage(storage), base(base) {
}

HierarchisationPoly::~HierarchisationPoly() {
}

void HierarchisationPoly::operator()(DataVector& source, DataVector& result,
                                     grid_iterator& index, size_t dim) {
  DataVector coeffs(index.getGridDepth(dim) + 1);
  coeffs.setAll(0.0);
  rec(source, result, index, dim, coeffs);
}

void HierarchisationPoly::rec(DataVector& source, DataVector& result,
                              grid_iterator& index, size_t dim,
                              DataVector& coeffs) {
  // current position on the grid
  size_t seq = index.seq();

  level_type cur_lev;
  index_type cur_ind;

  // get current level and index from grid
  index.get(dim, cur_lev, cur_ind);

  // hierarchisation
  float_t x = static_cast<float_t>(cur_ind) /
              static_cast<float_t>(1 << cur_lev);
  result[seq] = source[seq]
                - base->evalHierToTop(cur_lev, cur_ind, coeffs, x);

  // recursive calls for the right and left side of the current node
  if (index.hint() == false) {
    coeffs[cur_lev] = result[seq];

    // descend left
    index.leftChild(dim);

    if (!storage->end(index.seq())) {
      rec(source, result, index, dim, coeffs);
    }

    // descend right
    index.stepRight(dim);

    if (!storage->end(index.seq())) {
      rec(source, result, index, dim, coeffs);
    }

    // ascend
    index.up(dim);

    coeffs[cur_lev] = 0.0;
  }
}

}  // namespace base
}  // namespace SGPP
