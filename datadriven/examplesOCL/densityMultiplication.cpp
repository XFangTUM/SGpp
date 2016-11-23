// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#include <sgpp/base/grid/Grid.hpp>
#include <sgpp/base/datatypes/DataVector.hpp>
#include <sgpp/base/grid/GridStorage.hpp>
#include <sgpp/base/grid/generation/GridGenerator.hpp>
#include <sgpp/globaldef.hpp>
#include <sgpp/solver/sle/ConjugateGradients.hpp>
#include <sgpp/base/opencl/OCLOperationConfiguration.hpp>
#include <sgpp/datadriven/operation/hash/OperationDensityOCLMultiPlatform/OpFactory.hpp>
#include <sgpp/datadriven/operation/hash/OperationDensityMultiplicationAVX/OperationDensityMultiplicationAVX.hpp>
#include <sgpp/datadriven/operation/hash/OperationCreateGraphOCL/OpFactory.hpp>
#include <sgpp/datadriven/operation/hash/OperationPruneGraphOCL/OpFactory.hpp>

#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include "sgpp/datadriven/tools/ARFFTools.hpp"

int main() {
  size_t dimension = 10, tiefe = 6;
  double lambda = 0.00001;

  // Create Grid
  sgpp::base::Grid *grid = sgpp::base::Grid::createLinearGrid(dimension);
  sgpp::base::GridGenerator& gridGen = grid->getGenerator();
  gridGen.regular(tiefe);
  size_t gridsize = grid->getStorage().getSize();
  std::cerr << "Grid created! Number of grid points:     " << gridsize << std::endl;
  sgpp::datadriven::DensityAVX::OperationDensityMultiplicationAVX operation_mult(*grid, 0.01);

  sgpp::base::DataVector alpha(gridsize);
  sgpp::base::DataVector result(gridsize);
  alpha.setAll(1.0);

  std::chrono::time_point<std::chrono::system_clock> start, end;
  start = std::chrono::system_clock::now();
  operation_mult.mult(alpha, result);
  end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;

  std::ofstream out_mult("multavx_erg_dim2_depth11.txt");
  out_mult.precision(17);
  for (size_t i = 0; i < 100; ++i) {
    out_mult << result[i] << " ";
  }

  for (size_t i = 0; i < 100; ++i)
    std::cout << result[i] << " ";
  std::cout << std::endl;
  gridsize = gridsize + 1024 - gridsize % 1024;
  std::cout << "Duration: " << elapsed_seconds.count() << "\n";
  std::cout << "GFLOPS: " << (gridsize*gridsize * dimension * 18 + gridsize * gridsize * 2) /
      (elapsed_seconds.count() * 1000 * 1000 * 1000) << "\n";
  out_mult.close();
}
