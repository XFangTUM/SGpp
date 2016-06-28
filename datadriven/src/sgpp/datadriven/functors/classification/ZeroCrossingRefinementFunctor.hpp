// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org


#ifndef ZEROCROSSINGREFINEMENTFUNCTOR_HPP
#define ZEROCROSSINGREFINEMENTFUNCTOR_HPP

#include <sgpp/base/datatypes/DataVector.hpp>
#include <sgpp/base/grid/GridStorage.hpp>
#include <sgpp/base/grid/Grid.hpp>
#include <sgpp/datadriven/functors/MultiGridRefinementFunctor.hpp>

#include <vector>
#include <string>
#include <map>


namespace sgpp {
namespace datadriven {

/**
 * Zero-crossing-based refinement uses zero crossings of the difference
 * PDFS f_1 - f_2 to determine areas of interest for the refinement
 * process. The signs of a grid point pair evaluated at two PDFs
 * are compared and only if the signs differ (a zero crossing lies
 * between them) are they considered for the scoring.
 *
 * The pairs of grid points are geometric neighbors, determined for each
 * dimension separately.
 */
class ZeroCrossingRefinementFunctor : public MultiGridRefinementFunctor {
 public:
  /**
   * Constructor.
   *
   * @param grids Vector of grids. current_grid_index specifies the grid to be refined
   * @param alphas Vector of surpluses related to the grids
   * @param refinements_num Maximum number of refinements done
   * @param level_penalize If a level penalizing is multiplied to the score (2^{|l|_1})
   * @param pre_compute Flag for precomputation of necessary grid evals. If true preComputeEvaluations needs to be called before each refinement step
   * @param threshold Threshold for refinement scores
   */
  ZeroCrossingRefinementFunctor(std::vector<base::Grid*> grids,
                                std::vector<base::DataVector*> alphas,
                                size_t refinements_num = 1,
                                bool level_penalize = false,
                                bool pre_compute = false,
                                double threshold = 0.0);

  double operator()(base::GridStorage& storage,
                    size_t seq) const override;
  double start() const override;
  size_t getRefinementsNum() const override;
  double getRefinementThreshold() const override;
  virtual ~ZeroCrossingRefinementFunctor() {}

  void setGridIndex(size_t grid_index) override;
  size_t getNumGrids() override;
  void preComputeEvaluations() override;

 protected:
  std::vector<base::Grid*> grids;
  std::vector<base::DataVector*> alphas;

  size_t current_grid_index;
  size_t refinements_num;
  double threshold;
  bool level_penalize;

  bool pre_compute;
  std::vector<std::map<std::string, double>> pre_comp_evals;

  /**
   * Gets the evaluations of all grids at the coords of seq
   */
  std::vector<double> getEvalVector(size_t ind, size_t seq) const;

  int sgn(double d) const;

  // Utility for finding geometric neighbors
  void goUp(base::HashGridPoint& gp, base::HashGridPoint& up, size_t d,
            bool left) const;
  void goDown(base::HashGridPoint& gp,
              base::HashGridPoint& down,
              size_t d,
              bool left) const;
  bool hasChild(base::HashGridPoint& gp, size_t d, bool left) const;
  bool isLeftChild(base::HashGridPoint& gp, size_t d) const;
  void getChild(base::HashGridPoint& gp, base::HashGridPoint& child,
                size_t d,
                bool left) const;
  void getParent(base::HashGridPoint& gp, base::HashGridPoint& par,
                 size_t d) const;
};
}  // namespace datadriven
}  // namespace sgpp

#endif /* ZEROCROSSINGREFINEMENTFUNCTOR_HPP */
