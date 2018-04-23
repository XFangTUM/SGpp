/*
 * Copyright (C) 2008-today The SG++ project
 * This file is part of the SG++ project. For conditions of distribution and
 * use, please see the copyright notice provided with SG++ or at
 * sgpp.sparsegrids.org
 *
 * LeastSquaresRegressionFitterFactory.hpp
 *
 *  Created on:	17.12.2017
 *      Author: Eric Koepke
 */

#pragma once

#include <sgpp/datadriven/datamining/modules/hpo/FitterFactory.hpp>
#include <sgpp/datadriven/datamining/configuration/DataMiningConfigParser.hpp>
#include <sgpp/datadriven/datamining/modules/fitting/ModelFittingBase.hpp>
#include <sgpp/datadriven/datamining/modules/fitting/FitterConfigurationLeastSquares.hpp>
#include <sgpp/datadriven/datamining/modules/hpo/harmonica/ConfigurationBit.hpp>
#include <sgpp/datadriven/datamining/modules/hpo/parameters/ContinuousParameter.hpp>
#include <sgpp/datadriven/datamining/modules/hpo/parameters/DiscreteParameter.hpp>




namespace sgpp {
namespace datadriven {

/**
 * Concrete factory to build an instance of #sgpp::datadriven::ModelFittingBase
 */
class LeastSquaresRegressionFitterFactory : public FitterFactory {
 public:
  /**
   * Default constructor
   */
  LeastSquaresRegressionFitterFactory(DataMiningConfigParser& parser);

  /**
   * Assemble a #sgpp::datadriven::ModelFittingBase object based on the configuration
   * @param parser Instance of #sgpp::datadriven::DataMiningConfigParser that reads the required
   * data from the config file.
   * @return Fully configured instance of a  #sgpp::datadriven::ModelFittingBase object.
   */
  ModelFittingBase* buildFitter() override;

  std::string printConfig() override;


protected:
  FitterConfigurationLeastSquares baseConfig;
  std::vector<base::GridType> basisFunctions;

};

} /* namespace datadriven */
} /* namespace sgpp */
