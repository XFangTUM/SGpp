// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#pragma once

#include <string>

#include "sgpp/globaldef.hpp"
#include "sgpp/base/grid/Grid.hpp"
#include "sgpp/solver/TypesSolver.hpp"
#include "sgpp/base/tools/json/JSON.hpp"

namespace SGPP {
namespace datadriven {

class TestsetConfiguration {
 public:
  bool hasTestDataset;
  std::string datasetFileName;
  //  std::string valuesFileName;
  double expectedMSE;
  double expectedLargestDifference;

  TestsetConfiguration()
      : hasTestDataset(false),
        datasetFileName(""),
        //        valuesFileName(""),
        expectedMSE(0.0),
        expectedLargestDifference(0.0) {}
};

class LearnerScenario : public json::JSON {
 private:
  bool isInitialized;
  //
  //  // variables for the scenario
  //  double lambda;
  //  std::string datasetFileName;
  //  base::RegularGridConfiguration gridConfig;
  //  solver::SLESolverConfiguration solverConfigRefine;
  //  solver::SLESolverConfiguration solverConfigFinal;
  //  base::AdpativityConfiguration adaptConfig;
  //  datadriven::TestsetConfiguration testsetConfig;

 public:
  LearnerScenario();

  explicit LearnerScenario(std::string scenarioFileName);

  LearnerScenario(std::string datasetFileName, double lambda,
                  base::RegularGridConfiguration gridConfig,
                  solver::SLESolverConfiguration SLESolverConfigRefine,
                  solver::SLESolverConfiguration SLESolverConfigFinal,
                  base::AdpativityConfiguration adaptConfig);

  LearnerScenario(std::string datasetFileName, double lambda,
                  base::RegularGridConfiguration gridConfig,
                  solver::SLESolverConfiguration SLESolverConfigRefine,
                  solver::SLESolverConfiguration SLESolverConfigFinal,
                  base::AdpativityConfiguration adaptConfig,
                  datadriven::TestsetConfiguration testsetConfiguration);

  void setDatasetFileName(std::string datasetFileName);

  std::string getDatasetFileName();

  void setLambda(double lambda);

  double getLambda();

  void setGridConfig(base::RegularGridConfiguration& gridConfig);

  base::RegularGridConfiguration getGridConfig();

  void setSolverConfigurationRefine(solver::SLESolverConfiguration& solverConfigRefine);

  solver::SLESolverConfiguration getSolverConfigurationRefine();

  void setSolverConfigurationFinal(solver::SLESolverConfiguration& solverConfigFinal);

  solver::SLESolverConfiguration getSolverConfigurationFinal();

  void setAdaptivityConfiguration(base::AdpativityConfiguration& adaptConfig);

  base::AdpativityConfiguration getAdaptivityConfiguration();

  bool hasTestsetConfiguration();

  void setTestsetConfiguration(datadriven::TestsetConfiguration& testsetConfig);

  datadriven::TestsetConfiguration getTestsetConfiguration();

  //  void writeToFile(std::string fileName);
  //
  //  void readFromFile(std::string fileName);

 private:
  template <class T>
  T fromString(const std::string& s);
};
}  // namespace datadriven
}  // namespace SGPP
