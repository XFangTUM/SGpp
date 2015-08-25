/*
 * multiEvalPerformance.cpp
 *
 *  Created on: Mar 12, 2015
 *      Author: pfandedd
 */

#if USE_OCL==1

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <random>
#include <fstream>
#include <iostream>
#include <chrono>

#include <zlib.h>

#include "testsCommon.hpp"

#include <sgpp/globaldef.hpp>

#include <sgpp/base/operation/hash/OperationMultipleEval.hpp>
#include <sgpp/datadriven/DatadrivenOpFactory.hpp>
#include <sgpp/base/operation/BaseOpFactory.hpp>
#include <sgpp/datadriven/tools/ARFFTools.hpp>
#include <sgpp/base/grid/generation/functors/SurplusRefinementFunctor.hpp>
#include <sgpp/base/tools/ConfigurationParameters.hpp>
#include <sgpp/datadriven/opencl/OCLConfigurationParameters.hpp>

#define OUT_FILENAME "results.csv"

std::vector<std::string> fileNames = { "datadriven/tests/data/friedman_4d.arff.gz",
        "datadriven/tests/data/friedman_10d.arff.gz", "datadriven/tests/data/DR5_train.arff.gz" };

std::vector<size_t> levels = { 11, 7, 9 };
std::vector<size_t> refinementSteps = { 40, 30, 40 };

struct HPCSE2015Fixture {
    HPCSE2015Fixture() {
        BOOST_TEST_MESSAGE("setup fixture");
        outFile.open(OUT_FILENAME);
        outFile << "dataset, kernel, refinedGridSize, duration" << std::endl;
    }
    ~HPCSE2015Fixture() {
        BOOST_TEST_MESSAGE("teardown fixture");
        outFile.close();
    }
    std::ofstream outFile;
} logger;

static size_t refinedGridSize = 0;

BOOST_AUTO_TEST_SUITE(HPCSE2015)

void getRuntime(const std::string &kernel, std::string &fileName, size_t level,
SGPP::base::AdpativityConfiguration adaptConfig,
SGPP::datadriven::OperationMultipleEvalConfiguration configuration) {

    std::string content = uncompressFile(fileName);

    SGPP::datadriven::ARFFTools arffTools;
    SGPP::datadriven::Dataset dataset = arffTools.readARFFFromString(content);

    SGPP::base::DataMatrix* trainingData = dataset.getTrainingData();

    size_t dim = dataset.getDimension();
    SGPP::base::Grid* grid = SGPP::base::Grid::createLinearGrid(dim);
    SGPP::base::GridStorage* gridStorage = grid->getStorage();
    BOOST_TEST_MESSAGE("dimensionality:        " << gridStorage->dim());

    SGPP::base::GridGenerator* gridGen = grid->createGridGenerator();
    gridGen->regular(level);
    BOOST_TEST_MESSAGE("number of grid points: " << gridStorage->size());
    BOOST_TEST_MESSAGE("number of data points: " << dataset.getNumberInstances());

    doRandomRefinements(adaptConfig, *grid, *gridGen);

    BOOST_TEST_MESSAGE("size of refined grid: " << gridStorage->size());
    refinedGridSize = gridStorage->size();

    SGPP::base::DataVector alpha(gridStorage->size());

    for (size_t i = 0; i < alpha.getSize(); i++) {
        alpha[i] = static_cast<double>(i);
    }

    BOOST_TEST_MESSAGE("creating operation with unrefined grid");
    SGPP::base::OperationMultipleEval* eval =
    SGPP::op_factory::createOperationMultipleEval(*grid, *trainingData, configuration);

    SGPP::base::DataVector dataSizeVectorResult(dataset.getNumberInstances());
    dataSizeVectorResult.setAll(0);

    BOOST_TEST_MESSAGE("preparing operation for refined grid");
    eval->prepare();

    BOOST_TEST_MESSAGE("calculating result");

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    eval->mult(alpha, dataSizeVectorResult);
    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;

    BOOST_TEST_MESSAGE("duration: " << elapsed_seconds.count());

    logger.outFile << fileName << "," << kernel << "," << refinedGridSize << "," << elapsed_seconds.count()
            << std::endl;

}

BOOST_AUTO_TEST_CASE(StreamingDefault) {

    SGPP::base::AdpativityConfiguration adaptConfig;
    adaptConfig.maxLevelType_ = false;
    adaptConfig.noPoints_ = 80;
    adaptConfig.percent_ = 200.0;
    adaptConfig.threshold_ = 0.0;

    SGPP::base::OCLConfigurationParameters parameters;
    parameters.set("OCL_MANAGER_VERBOSE", "false");
    parameters.set("KERNEL_USE_LOCAL_MEMORY", "false");
    parameters.set("KERNEL_DATA_BLOCKING_SIZE", "1");
    parameters.set("KERNEL_TRANS_GRID_BLOCKING_SIZE", "1");
    parameters.set("KERNEL_STORE_DATA", "array");
    parameters.set("KERNEL_MAX_DIM_UNROLL", "1");
    parameters.set("PLATFORM", "first");
    parameters.set("SELECT_SPECIFIC_DEVICE", "0");
    parameters.set("MAX_DEVICES", "1");

    SGPP::datadriven::OperationMultipleEvalConfiguration configuration(
    SGPP::datadriven::OperationMultipleEvalType::STREAMING,
    SGPP::datadriven::OperationMultipleEvalSubType::DEFAULT, parameters);

    for (size_t i = 0; i < fileNames.size(); i++) {
        adaptConfig.numRefinements_ = refinementSteps[i];
        getRuntime("STREAMING::DEFAULT", fileNames[i], levels[i], adaptConfig, configuration);
    }
}

BOOST_AUTO_TEST_CASE(StreamingSubspaceLinear) {

    SGPP::base::AdpativityConfiguration adaptConfig;
    adaptConfig.maxLevelType_ = false;
    adaptConfig.noPoints_ = 80;
    adaptConfig.percent_ = 200.0;
    adaptConfig.threshold_ = 0.0;

    SGPP::base::OCLConfigurationParameters parameters;
    parameters.set("OCL_MANAGER_VERBOSE", "false");
    parameters.set("KERNEL_USE_LOCAL_MEMORY", "false");
    parameters.set("KERNEL_DATA_BLOCKING_SIZE", "1");
    parameters.set("KERNEL_TRANS_GRID_BLOCKING_SIZE", "1");
    parameters.set("KERNEL_STORE_DATA", "array");
    parameters.set("KERNEL_MAX_DIM_UNROLL", "1");
    parameters.set("PLATFORM", "first");
    parameters.set("SELECT_SPECIFIC_DEVICE", "0");
    parameters.set("MAX_DEVICES", "1");

    SGPP::datadriven::OperationMultipleEvalConfiguration configuration(
    SGPP::datadriven::OperationMultipleEvalType::SUBSPACELINEAR,
    SGPP::datadriven::OperationMultipleEvalSubType::COMBINED, parameters);

    for (size_t i = 0; i < fileNames.size(); i++) {
        adaptConfig.numRefinements_ = refinementSteps[i];
        getRuntime("SUBSPACELINEAR::COMBINED", fileNames[i], levels[i], adaptConfig, configuration);
    }
}

BOOST_AUTO_TEST_CASE(StreamingBase) {

    SGPP::base::AdpativityConfiguration adaptConfig;
    adaptConfig.maxLevelType_ = false;
    adaptConfig.noPoints_ = 80;
    adaptConfig.percent_ = 200.0;
    adaptConfig.threshold_ = 0.0;

    SGPP::base::OCLConfigurationParameters parameters;
    parameters.set("OCL_MANAGER_VERBOSE", "false");
    parameters.set("KERNEL_USE_LOCAL_MEMORY", "false");
    parameters.set("KERNEL_DATA_BLOCKING_SIZE", "1");
    parameters.set("KERNEL_TRANS_GRID_BLOCKING_SIZE", "1");
    parameters.set("KERNEL_STORE_DATA", "array");
    parameters.set("KERNEL_MAX_DIM_UNROLL", "1");
    parameters.set("PLATFORM", "first");
    parameters.set("SELECT_SPECIFIC_DEVICE", "0");
    parameters.set("MAX_DEVICES", "1");

    SGPP::datadriven::OperationMultipleEvalConfiguration configuration(
    SGPP::datadriven::OperationMultipleEvalType::DEFAULT,
    SGPP::datadriven::OperationMultipleEvalSubType::DEFAULT, parameters);

    for (size_t i = 0; i < fileNames.size(); i++) {
        adaptConfig.numRefinements_ = refinementSteps[i];
        getRuntime("DEFAULT::DEFAULT", fileNames[i], levels[i], adaptConfig, configuration);
    }
}

BOOST_AUTO_TEST_CASE(StreamingOCL) {

    SGPP::base::AdpativityConfiguration adaptConfig;
    adaptConfig.maxLevelType_ = false;
    adaptConfig.noPoints_ = 80;
    adaptConfig.percent_ = 200.0;
    adaptConfig.threshold_ = 0.0;

    SGPP::base::OCLConfigurationParameters parameters;
    parameters.set("OCL_MANAGER_VERBOSE", "true");
    parameters.set("KERNEL_USE_LOCAL_MEMORY", "false");
    parameters.set("KERNEL_DATA_BLOCKING_SIZE", "4");
    parameters.set("KERNEL_TRANS_GRID_BLOCKING_SIZE", "4");
    parameters.set("KERNEL_STORE_DATA", "register");
    parameters.set("KERNEL_MAX_DIM_UNROLL", "10");
    parameters.set("PLATFORM", "NVIDIA CUDA");
    parameters.set("INTERNAL_PRECISION", "float");
    parameters.set("SELECT_SPECIFIC_DEVICE", "2");
    parameters.set("MAX_DEVICES", "1");
    parameters.set("VERBOSE", "true");

    SGPP::datadriven::OperationMultipleEvalConfiguration configuration(
    SGPP::datadriven::OperationMultipleEvalType::STREAMING,
    SGPP::datadriven::OperationMultipleEvalSubType::OCL, parameters);

    for (size_t i = 0; i < fileNames.size(); i++) {
        adaptConfig.numRefinements_ = refinementSteps[i];
        getRuntime("STREAMING::OCL", fileNames[i], levels[i], adaptConfig, configuration);
    }
}

BOOST_AUTO_TEST_SUITE_END()

#endif
