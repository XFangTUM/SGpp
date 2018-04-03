// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#include <sgpp/pde/operation/hash/OperationMatrixLTwoDotModLinear.hpp>
#include <sgpp/base/grid/type/ModLinearGrid.hpp>
#include <sgpp/base/exception/data_exception.hpp>
#include <sgpp/base/operation/hash/common/basis/LinearModifiedBasis.hpp>

#include <sgpp/globaldef.hpp>

#include <algorithm>

namespace sgpp {
namespace pde {

OperationMatrixLTwoDotModLinear::OperationMatrixLTwoDotModLinear(
    sgpp::base::Grid* grid) {
  this->grid = grid;
}

OperationMatrixLTwoDotModLinear::~OperationMatrixLTwoDotModLinear() {}

void OperationMatrixLTwoDotModLinear::mult(sgpp::base::DataVector& alpha,
                                      sgpp::base::DataVector& result) {
  // basis of the non-abstract type because of the getIntegral function
  base::SLinearModifiedBase& basis =
    const_cast<base::SLinearModifiedBase&>(
      dynamic_cast<const base::SLinearModifiedBase&>(grid->getBasis()));
  base::GridStorage& storage = grid->getStorage();
  base::DataVector coordinates;
  base::DataVector weights;

  size_t nrows = storage.getSize();
  size_t ncols = storage.getSize();

  if (alpha.getSize() != ncols || result.getSize() != nrows) {
    throw sgpp::base::data_exception("Dimensions do not match!");
  }

  size_t gridSize = storage.getSize();
  size_t gridDim = storage.getDimension();

  for (size_t i = 0; i < gridSize; i++) {
    result[i] = 0;
  }

  for (size_t i = 0; i < gridSize; i++) {
    for (size_t j = i; j < gridSize; j++) {
      double temp_ij = 1;

      for (size_t k = 0; k < gridDim; k++) {
        const base::level_t lik = storage[i].getLevel(k);
        const base::level_t ljk = storage[j].getLevel(k);
        const base::index_t iik = storage[i].getIndex(k);
        const base::index_t ijk = storage[j].getIndex(k);
        base::index_t hInvi = (1 << lik);
        base::index_t hInvj = (1 << ljk);
        double hInviDbl = static_cast<double>(hInvi);
        double hInvjDbl = static_cast<double>(hInvj);
        double temp_res;

        if (lik == ljk) {
          if (lik == 1) {
            continue;
          } else if (iik == ijk) {
            if (iik == 1 || iik == hInvi - 1) {
              // Use formula for identical modified ansatz functions:
              temp_res = 8 / (hInviDbl * 3);
            } else {
              // Use formula for identical ansatz functions:
              temp_res = 2 / (hInviDbl * 3);
            }
          } else {
            // Different index, but same level => ansatz functions do not overlap:
            temp_ij = 0.;
            break;
          }
        } else {
          // if one of the basis functions is from level 1 it's easy
          if (lik == 1) {
            temp_res = basis.getIntegral(ljk, ijk);
          } else if (ljk == 1) {
            temp_res = basis.getIntegral(lik, iik);
          } else if ((iik - 1) / hInviDbl >= (ijk + 1) / hInvjDbl ||
                     (iik + 1) / hInviDbl <= (ijk - 1) / hInvjDbl) {
            // Ansatz functions do not not overlap:
            temp_ij = 0.;
            break;
          } else {
            // use formula for different overlapping ansatz functions:
            if (lik > ljk) {                             // Phi_i_k is the "smaller" ansatz function
              if ((iik == 1 && ijk == 1) || (iik == hInvi - 1 && ijk == hInvj - 1)) {
                // integrate modified basis prdouct from 0 to 2^(-lik + 1)
                temp_res = 4 * ((1 / hInviDbl) - (hInvjDbl / 3 / (hInviDbl * hInviDbl)));
              } else if (ijk == 1) {
                // integrate product of modified Phi_i_k with
                // regular Phi_j_k from (ijk-1)/2^(ljk) to  (ijk+1)/2^(ljk)
                temp_res = (1 / hInviDbl) * (1 / hInviDbl) * (2 * hInviDbl - iik * hInvjDbl);
              } else if (ijk == hInvj - 1) {
                // symmetric to ijk == 1
                temp_res = (1 / hInviDbl) * (1 / hInviDbl) *
                           (2 * hInviDbl - (hInvi - iik) * hInvjDbl);
              } else {
                double diff = (iik / hInviDbl) - (ijk / hInvjDbl);  // x_i_k - x_j_k
                temp_res = fabs(diff - (1 / hInviDbl)) + fabs(diff + (1 / hInviDbl)) - fabs(diff);
                temp_res *= hInvjDbl;
                temp_res = (1 - temp_res) / hInviDbl;
              }
            } else {                                     // Phi_j_k is the "smaller" ansatz function
              // symmetric to case above
              if ((iik == 1 && ijk == 1) || (iik == hInvi - 1 && ijk == hInvj - 1)) {
                // both basis functions are modified
                // integrate modified basis prdouct from 0 to 2^(-ljk + 1)
                temp_res = 4 * ((1 / hInvjDbl) - (hInviDbl / (3 * hInvjDbl * hInvjDbl)));
              } else if (iik == 1) {
                // integrate product of modified Phi_i_k with
                // regular Phi_j_k from (ijk-1)/2^(ljk) to  (ijk+1)/2^(ljk)
                temp_res = (1 / hInvjDbl) * (1 / hInvjDbl) * (2 * hInvjDbl - ijk * hInviDbl);
              } else if (iik == hInvi - 1) {
                // symmetric to iik == 1
                temp_res = (1 / hInvjDbl) * (1 / hInvjDbl) *
                           (2 * hInvjDbl - (hInvj - ijk) * hInviDbl);
              } else {
                double diff = (ijk / hInvjDbl) - (iik / hInviDbl);  // x_j_k - x_i_k
                temp_res = fabs(diff - (1 / hInvjDbl)) + fabs(diff + (1 / hInvjDbl)) - fabs(diff);
                temp_res *= hInviDbl;
                temp_res = (1 - temp_res) / hInvjDbl;
              }
            }
          }
        }
        temp_ij *= temp_res;
      }
      result[i] += temp_ij * alpha[j];
      if (i != j)
        result[j] += temp_ij * alpha[i];
    }
  }
}
}  // namespace pde
}  // namespace sgpp
