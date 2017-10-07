// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#ifndef SGPP_OPTIMIZATION_OPTIMIZER_CONSTRAINED_CONSTRAINEDOPTIMIZER_HPP
#define SGPP_OPTIMIZATION_OPTIMIZER_CONSTRAINED_CONSTRAINEDOPTIMIZER_HPP

#include <sgpp/globaldef.hpp>

#include <sgpp/optimization/function/vector/VectorFunction.hpp>
#include <sgpp/optimization/function/vector/VectorFunctionGradient.hpp>
#include <sgpp/optimization/optimizer/unconstrained/UnconstrainedOptimizer.hpp>
#include <sgpp/base/datatypes/DataVector.hpp>

#include <cstddef>
#include <memory>

namespace sgpp {
namespace optimization {
namespace optimizer {

/**
 * Abstract class for solving constrained optimization problems.
 */
class ConstrainedOptimizer : public UnconstrainedOptimizer {
 public:
  /**
   * Constructor.
   * The starting point is set to
   * \f$(0.5, \dotsc, 0.5)^{\mathrm{T}}\f$.
   * Depending on the implementation $g$ and/or $h$ may be ignored
   * (if only equality or inequality constraints can be handled
   * by the underlying algorithm).
   *
   * @param f           function to optimize
   * @param fGradient   gradient of f (nullptr to omit)
   * @param g           inequality constraint function
   *                    (\f$g(\vec{x}) \le 0\f$)
   * @param gGradient   gradient of g (nullptr to omit)
   * @param h           equality constraint function
   *                    (\f$h(\vec{x}) = 0\f$)
   * @param hGradient   gradient of h (nullptr to omit)
   * @param N           maximal number of iterations or
   *                    objective function evaluations
   *                    (depending on the implementation)
   */
  ConstrainedOptimizer(const ScalarFunction& f,
                       const ScalarFunctionGradient* fGradient,
                       const VectorFunction& g,
                       const VectorFunctionGradient* gGradient,
                       const VectorFunction& h,
                       const VectorFunctionGradient* hGradient,
                       size_t N = DEFAULT_N)
      : UnconstrainedOptimizer(f, fGradient, nullptr, N) {
    g.clone(this->g);

    if (gGradient != nullptr) {
      gGradient->clone(this->gGradient);
    }

    h.clone(this->h);

    if (hGradient != nullptr) {
      hGradient->clone(this->hGradient);
    }
  }

  /**
   * Copy constructor.
   *
   * @param other optimizer to be copied
   */
  ConstrainedOptimizer(const ConstrainedOptimizer& other)
      : ConstrainedOptimizer(*other.f, other.fGradient.get(),
                             *other.g, other.gGradient.get(),
                             *other.h, other.hGradient.get(), N) {
  }

  /**
   * Destructor.
   */
  ~ConstrainedOptimizer() override {}

  /**
   * @return inequality constraint function
   */
  VectorFunction& getInequalityConstraintFunction() const { return *g; }

  /**
   * @param g  inequality constraint function
   */
  void setInequalityConstraintFunction(const VectorFunction& g) {
    g.clone(this->g);
  }

  /**
   * @return inequality constraint function gradient
   */
  VectorFunctionGradient* getInequalityConstraintGradient() const { return gGradient.get(); }

  /**
   * @param gGradient  inequality constraint function gradient
   */
  void setInequalityConstraintGradient(const VectorFunctionGradient* gGradient) {
    if (gGradient != nullptr) {
      gGradient->clone(this->gGradient);
    } else {
      this->gGradient = nullptr;
    }
  }

  /**
   * @return equality constraint function
   */
  VectorFunction& getEqualityConstraintFunction() const { return *h; }

  /**
   * @param h  equality constraint function
   */
  void setEqualityConstraintFunction(const VectorFunction& h) { h.clone(this->h); }

  /**
   * @return equality constraint function gradient
   */
  VectorFunctionGradient* getEqualityConstraintGradient() const { return hGradient.get(); }

  /**
   * @param hGradient  equality constraint function gradient
   */
  void setEqualityConstraintGradient(const VectorFunctionGradient* hGradient) {
    if (hGradient != nullptr) {
      hGradient->clone(this->hGradient);
    } else {
      this->hGradient = nullptr;
    }
  }

 protected:
  /// inequality constraint function
  std::unique_ptr<VectorFunction> g;
  /// inequality constraint function gradient
  std::unique_ptr<VectorFunctionGradient> gGradient;
  /// equality constraint function
  std::unique_ptr<VectorFunction> h;
  /// equality constraint function gradient
  std::unique_ptr<VectorFunctionGradient> hGradient;
};
}  // namespace optimizer
}  // namespace optimization
}  // namespace sgpp

#endif /* SGPP_OPTIMIZATION_OPTIMIZER_CONSTRAINED_CONSTRAINEDOPTIMIZER_HPP */
