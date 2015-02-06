// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#ifndef SGPP_OPTIMIZATION_FUNCTION_TEST_EASOM_HPP
#define SGPP_OPTIMIZATION_FUNCTION_TEST_EASOM_HPP

#include <sgpp/globaldef.hpp>

#include <sgpp/optimization/function/test/Test.hpp>

#include <cmath>

namespace SGPP {
  namespace optimization {
    namespace function {
      namespace test {

        /**
         * Easom test function.
         *
         * Definition:
         * \f$f(\vec{x}) := -\cos x_1 \cos x_2 \exp(-(x_1 - \pi)^2 - (x_2 - \pi)^2)\f$,
         * \f$\vec{x} \in [-100, 100]^2\f$,
         * \f$\vec{x}_{\text{opt}} = (\pi, \pi)^{\mathrm{T}}\f$,
         * \f$f_{\text{opt}} = -1\f$
         * (domain scaled to \f$[0, 1]^2\f$)
         */
        class Easom : public Test {
          public:
            /**
             * Constructor.
             */
            Easom() : Test(2) {
            }

            /**
             * Evaluates the test function.
             *
             * @param x     point \f$\vec{x} \in [0, 1]^2\f$
             * @return      \f$f(\vec{x})\f$
             */
            float_t evalUndisplaced(const std::vector<float_t>& x) {
              const float_t x1 = 200.0 * x[0] - 100.0;
              const float_t x2 = 200.0 * x[1] - 100.0;

              return -std::cos(x1) * std::cos(x2) *
                     std::exp(-((x1-M_PI)*(x1-M_PI) + (x2-M_PI)*(x2-M_PI)));
            }

            /**
             * Returns minimal point and function value of the test function.
             *
             * @param[out] x    minimal point \f$\vec{x}_{\text{opt}} \in [0, 1]^2\f$
             * @return          minimal function value \f$f_{\text{opt}} = f(\vec{x}_{\text{opt}})\f$
             */
            float_t getOptimalPointUndisplaced(std::vector<float_t>& x) {
              x = std::vector<float_t>(2, 0.51570796326794896619231);
              return -1.0;
            }

            /**
             * @param[out] clone pointer to cloned object
             */
            virtual void clone(Objective*& clone) const {
              clone = new Easom(*this);
            }
        };

      }
    }
  }
}

#endif
