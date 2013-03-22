/******************************************************************************
* Copyright (C) 2011 Technische Universitaet Muenchen                         *
* This file is part of the SG++ project. For conditions of distribution and   *
* use, please see the copyright notice at http://www5.in.tum.de/SGpp          *
******************************************************************************/
// @author Alexander Heinecke (Alexander.Heinecke@mytum.de)
// @author Roman Karlstetter (karlstetter@mytum.de)

#include "parallel/datadriven/basis/modlinear/operation/OperationMultipleEvalIterativeX86SimdModLinear.hpp"
#include "parallel/datadriven/basis/modlinear/operation/impl/X86SimdModLinearMult.hpp"
#include "parallel/datadriven/basis/modlinear/operation/impl/X86SimdModLinearMultTranspose.hpp"
#include "parallel/tools/PartitioningTool.hpp"


namespace sg
{
namespace parallel
{

OperationMultipleEvalIterativeX86SimdModLinear::OperationMultipleEvalIterativeX86SimdModLinear(
		sg::base::GridStorage* storage, sg::base::DataMatrix* dataset,
		int gridFrom, int gridTo, int datasetFrom, int datasetTo) : sg::parallel::OperationMultipleEvalVectorized(dataset)
{
	m_gridFrom = gridFrom;
	m_gridTo = gridTo;
	m_datasetFrom = datasetFrom;
	m_datasetTo = datasetTo;

	this->storage = storage;

	this->level_ = new sg::base::DataMatrix(storage->size(), storage->dim());
	this->index_ = new sg::base::DataMatrix(storage->size(), storage->dim());

	storage->getLevelIndexArraysForEval(*(this->level_), *(this->index_));

	myTimer = new sg::base::SGppStopwatch();
}

OperationMultipleEvalIterativeX86SimdModLinear::~OperationMultipleEvalIterativeX86SimdModLinear()
{
	delete myTimer;
}

void OperationMultipleEvalIterativeX86SimdModLinear::rebuildLevelAndIndex()
{
	delete this->level_;
	delete this->index_;

	this->level_ = new sg::base::DataMatrix(storage->size(), storage->dim());
	this->index_ = new sg::base::DataMatrix(storage->size(), storage->dim());

	storage->getLevelIndexArraysForEval(*(this->level_), *(this->index_));
}

void OperationMultipleEvalIterativeX86SimdModLinear::updateGridComputeBoundaries(int gridFrom, int gridTo)
{
	m_gridFrom = gridFrom;
	m_gridTo = gridTo;
}

double OperationMultipleEvalIterativeX86SimdModLinear::multTransposeVectorized(sg::base::DataVector& source, sg::base::DataVector& result)
{
    myTimer->start();
    result.setAll(0.0);

	#pragma omp parallel
	{
		size_t start;
		size_t end;
		sg::parallel::PartitioningTool::getOpenMPPartitionSegment(m_gridFrom, m_gridTo, &start, &end, 1);

		sg::parallel::X86SimdModLinearMultTranspose::multTranspose(level_, index_, NULL, NULL, dataset_, source, result, start, end, 0, this->dataset_->getNcols());
	}

	return myTimer->stop();
}

double OperationMultipleEvalIterativeX86SimdModLinear::multVectorized(sg::base::DataVector& alpha, sg::base::DataVector& result)
{
    myTimer->start();
	result.setAll(0.0);

	#pragma omp parallel
	{
		size_t start;
		size_t end;
		sg::parallel::PartitioningTool::getOpenMPPartitionSegment(m_datasetFrom, m_datasetTo, &start, &end, sg::parallel::X86SimdModLinearMult::getChunkDataPoints());

		sg::parallel::X86SimdModLinearMult::mult(level_, index_, NULL, NULL, dataset_, alpha, result, 0, alpha.getSize(), start, end);
	}

	return myTimer->stop();
}

}
}
