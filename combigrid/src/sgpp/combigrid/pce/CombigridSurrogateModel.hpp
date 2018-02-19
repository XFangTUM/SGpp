// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#pragma once

#include <sgpp/base/datatypes/DataMatrix.hpp>
#include <sgpp/base/datatypes/DataVector.hpp>
#include <sgpp/combigrid/functions/OrthogonalBasisFunctionsCollection.hpp>
#include <sgpp/combigrid/functions/OrthogonalPolynomialBasis1D.hpp>
#include <sgpp/combigrid/functions/WeightFunctionsCollection.hpp>
#include <sgpp/combigrid/operation/CombigridOperation.hpp>
#include <sgpp/combigrid/operation/CombigridMultiOperation.hpp>
#include <sgpp/combigrid/operation/CombigridTensorOperation.hpp>

#include <vector>

namespace sgpp {
namespace combigrid {

enum class CombigridSurrogateModelsType {
  POLYNOMIAL_CHAOS_EXPANSION,
  POLYNOMIAL_STOCHASTIC_COLLOCATION,
  BSPLINE_STOCHASTIC_COLLOCATION
};

class CombigridSurrogateModelConfiguration {
 public:
  CombigridSurrogateModelConfiguration();
  virtual ~CombigridSurrogateModelConfiguration();

  // type
  CombigridSurrogateModelsType type;

  // structure
  std::vector<std::shared_ptr<AbstractPointHierarchy>> pointHierarchies;
  std::shared_ptr<AbstractCombigridStorage> storage;
  std::shared_ptr<sgpp::combigrid::LevelManager> levelManager;
  std::shared_ptr<sgpp::combigrid::TreeStorage<uint8_t>> levelStructure;

  // basis function for tensor operation
  std::shared_ptr<sgpp::combigrid::OrthogonalPolynomialBasis1D> basisFunction;
  sgpp::combigrid::OrthogonalBasisFunctionsCollection basisFunctions;

  // bounds for stochastic collocation
  sgpp::base::DataVector bounds;

  // Bspline degree
  size_t degree;
  //  Bspline coefficients
  std::shared_ptr<AbstractCombigridStorage> coefficientStorage;

  // weight functions
  sgpp::combigrid::WeightFunctionsCollection weightFunctions;

  bool enableLevelManagerStatsCollection;

  void loadFromCombigridOperation(std::shared_ptr<CombigridOperation> op);
  void loadFromCombigridOperation(std::shared_ptr<CombigridMultiOperation> op);
  void loadFromCombigridOperation(std::shared_ptr<CombigridTensorOperation> op);
};

// --------------------------------------------------------------------------

class CombigridSurrogateModel {
 public:
  CombigridSurrogateModel(sgpp::combigrid::CombigridSurrogateModelConfiguration& config);
  virtual ~CombigridSurrogateModel();

  virtual double eval(sgpp::base::DataVector& x) = 0;
  virtual void eval(sgpp::base::DataMatrix& xs, sgpp::base::DataVector& res) = 0;

  virtual double mean() = 0;
  virtual double variance() = 0;
  virtual void getComponentSobolIndices(sgpp::base::DataVector& componentSsobolIndices,
                                        bool normalized = true) = 0;
  virtual void getTotalSobolIndices(sgpp::base::DataVector& totalSobolIndices,
                                    bool normalized = true) = 0;

  virtual void updateConfig(sgpp::combigrid::CombigridSurrogateModelConfiguration config) = 0;

  virtual size_t numGridPoints() = 0;
  virtual std::shared_ptr<LevelInfos> getInfoOnAddedLevels() = 0;

  sgpp::combigrid::CombigridSurrogateModelConfiguration& getConfig();

 protected:
  sgpp::combigrid::CombigridSurrogateModelConfiguration config;

  size_t numDims;
};

} /* namespace combigrid */
} /* namespace sgpp */
