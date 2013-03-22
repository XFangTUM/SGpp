/******************************************************************************
* Copyright (C) 2010 Technische Universitaet Muenchen                         *
* This file is part of the SG++ project. For conditions of distribution and   *
* use, please see the copyright notice at http://www5.in.tum.de/SGpp          *
******************************************************************************/
// @author Alexander Heinecke (Alexander.Heinecke@mytum.de)
// @author Roman Karlstetter (karlstetter@mytum.de)

#include "parallel/datadriven/basis/linear/noboundary/operation/OperationMultipleEvalIterativeSPX86SimdLinear.hpp"
#include "parallel/datadriven/basis/linear/noboundary/operation/impl/SPX86SimdLinearMult.hpp"
#include "parallel/datadriven/basis/linear/noboundary/operation/impl/SPX86SimdLinearMultTranspose.hpp"
#include "parallel/tools/PartitioningTool.hpp"

namespace sg
{
namespace parallel
{

OperationMultipleEvalIterativeSPX86SimdLinear::OperationMultipleEvalIterativeSPX86SimdLinear(
		sg::base::GridStorage* storage, sg::base::DataMatrixSP* dataset,
		int gridFrom, int gridTo, int datasetFrom, int datasetTo) : sg::parallel::OperationMultipleEvalVectorizedSP(dataset)
{
	m_gridFrom = gridFrom;
	m_gridTo = gridTo;
	m_datasetFrom = datasetFrom;
	m_datasetTo = datasetTo;

	this->storage = storage;

	this->level_ = new sg::base::DataMatrixSP(storage->size(), storage->dim());
	this->index_ = new sg::base::DataMatrixSP(storage->size(), storage->dim());

	storage->getLevelIndexArraysForEval(*(this->level_), *(this->index_));

	myTimer = new sg::base::SGppStopwatch();
}

OperationMultipleEvalIterativeSPX86SimdLinear::~OperationMultipleEvalIterativeSPX86SimdLinear()
{
	delete myTimer;
}

void OperationMultipleEvalIterativeSPX86SimdLinear::rebuildLevelAndIndex()
{
	delete this->level_;
	delete this->index_;

	this->level_ = new sg::base::DataMatrixSP(storage->size(), storage->dim());
	this->index_ = new sg::base::DataMatrixSP(storage->size(), storage->dim());

	storage->getLevelIndexArraysForEval(*(this->level_), *(this->index_));
}

void OperationMultipleEvalIterativeSPX86SimdLinear::updateGridComputeBoundaries(int gridFrom, int gridTo)
{
	m_gridFrom = gridFrom;
	m_gridTo = gridTo;
}

double OperationMultipleEvalIterativeSPX86SimdLinear::multTransposeVectorized(sg::base::DataVectorSP& source, sg::base::DataVectorSP& result)
{
    myTimer->start();
    result.setAll(0.0f);

	#pragma omp parallel
	{
		size_t start;
		size_t end;
		sg::parallel::PartitioningTool::getOpenMPPartitionSegment(m_gridFrom, m_gridTo, &start, &end, 1);

		sg::parallel::SPX86SimdLinearMultTranspose::multTranspose(level_, index_, NULL, NULL, dataset_, source, result, start, end, 0, dataset_->getNcols());
	}

	return myTimer->stop();
}

double OperationMultipleEvalIterativeSPX86SimdLinear::multVectorized(sg::base::DataVectorSP& alpha, sg::base::DataVectorSP& result)
{
    myTimer->start();
	result.setAll(0.0f);

	#pragma omp parallel
	{
		size_t start;
		size_t end;
		sg::parallel::PartitioningTool::getOpenMPPartitionSegment(m_datasetFrom, m_datasetTo, &start, &end, sg::parallel::SPX86SimdLinearMult::getChunkDataPoints());

		sg::parallel::SPX86SimdLinearMult::mult(level_, index_, NULL, NULL, dataset_, alpha, result, 0, alpha.getSize(), start, end);
	}

	return myTimer->stop();
}

}
}
