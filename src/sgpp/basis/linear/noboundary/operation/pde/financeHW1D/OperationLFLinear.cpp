/******************************************************************************
* Copyright (C) 2009 Technische Universitaet Muenchen                         *
* This file is part of the SG++ project. For conditions of distribution and   *
* use, please see the copyright notice at http://www5.in.tum.de/SGpp          *
******************************************************************************/
// @author Chao qi(qic@in.tum.de)

#include "basis/linear/noboundary/operation/pde/financeHW1D/OperationLFLinear.hpp"

#include "basis/linear/noboundary/algorithm_sweep/XdPhiPhiDownBBLinear.hpp"
#include "basis/linear/noboundary/algorithm_sweep/XdPhiPhiUpBBLinear.hpp"

#include "algorithm/common/sweep.hpp"

namespace sg
{

OperationLFLinear::OperationLFLinear(GridStorage* storage) : StdUpDown(storage)
{
}

OperationLFLinear::~OperationLFLinear()
{
}

void OperationLFLinear::up(DataVector& alpha, DataVector& result, size_t dim)
{
	// X * dphi * phi
	detail::XdPhiPhiUpBBLinear func(this->storage);
	sweep<detail::XdPhiPhiUpBBLinear> s(func, this->storage);

	s.sweep1D(alpha, result, dim);
}

void OperationLFLinear::down(DataVector& alpha, DataVector& result, size_t dim)
{
	// X * dphi * phi
	detail::XdPhiPhiDownBBLinear func(this->storage);
	sweep<detail::XdPhiPhiDownBBLinear> s(func, this->storage);

	s.sweep1D(alpha, result, dim);
}

}
