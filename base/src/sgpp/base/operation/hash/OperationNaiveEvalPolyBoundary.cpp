// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#include "OperationNaiveEvalPolyBoundary.hpp"

namespace SGPP {
  namespace base {

    float_t OperationNaiveEvalPolyBoundary::eval(DataVector& alpha,
        DataVector& point) {
      const size_t n = storage->size();
      const size_t dim = storage->dim();
      float_t result = 0.0;

      for (size_t i = 0; i < n; i++) {
        const GridIndex* gp = storage->get(i);
        float_t cur_val = 1.0;

        for (size_t idim = 0; idim < dim; idim++) {
          float_t val1d = base.evalSave(gp->getLevel(idim),
                                        gp->getIndex(idim), point[idim]);

          if (val1d == 0.0) {
            cur_val = 0.0;
            break;
          }

          cur_val *= val1d;
        }

        result += alpha[i] * cur_val;
      }

      return result;
    }

  } /* namespace base */
} /* namespace SGPP */
