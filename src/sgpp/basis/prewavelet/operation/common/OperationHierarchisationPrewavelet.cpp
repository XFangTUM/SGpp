/*****************************************************************************/
/* This file is part of sgpp, a program package making use of spatially      */
/* adaptive sparse grids to solve numerical problems                         */
/*                                                                           */
/* Copyright (C) 2009 Alexander Heinecke (Alexander.Heinecke@mytum.de)       */
/*                                                                           */
/* sgpp is free software; you can redistribute it and/or modify              */
/* it under the terms of the GNU General Public License as published by      */
/* the Free Software Foundation; either version 3 of the License, or         */
/* (at your option) any later version.                                       */
/*                                                                           */
/* sgpp is distributed in the hope that it will be useful,                   */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU Lesser General Public License for more details.                       */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with sgpp; if not, write to the Free Software                       */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/* or see <http://www.gnu.org/licenses/>.                                    */
/*****************************************************************************/

#include "basis/prewavelet/operation/common/OperationHierarchisationPrewavelet.hpp"
#include "basis/prewavelet/algorithm_sweep/ConvertLinearToPrewavelet.hpp"
#include "basis/prewavelet/algorithm_sweep/ConvertPrewaveletToLinear.hpp"
#include "basis/linear/noboundary/algorithm_sweep/HierarchisationLinear.hpp"
#include "basis/linear/noboundary/algorithm_sweep/DehierarchisationLinear.hpp"

#include "sgpp.hpp"

#include "algorithm/common/sweep.hpp"

#include "basis/basis.hpp"
#include "data/DataVector.hpp"

#include <iostream>

namespace sg
{

void OperationHierarchisationPrewavelet::doHierarchisation(
		DataVector& node_values)
{
	/*
	 * Hierarchisation on prewavelets require a hierarchisation on a normal
	 * linear grid, afterwards they are converted into a prewavelet basis
	 */

	detail::HierarchisationLinear func(this->storage);
	sweep<detail::HierarchisationLinear> s(func, this->storage);

	for (size_t i = 0; i < this->storage->dim(); i++)
	{
		s.sweep1D(node_values, node_values, i);
	}

	detail::ConvertLinearToPrewavelet func2(this->storage, this->shadowStorage);
	sweep<detail::ConvertLinearToPrewavelet> s2(func2, this->storage);

	for (size_t i = 0; i < this->storage->dim(); i++)
	{
		s2.sweep1D(node_values, node_values, i);
	}

}

void OperationHierarchisationPrewavelet::doDehierarchisation(DataVector& alpha)
{
	detail::ConvertPrewaveletToLinear func(this->storage);
	sweep<detail::ConvertPrewaveletToLinear> s(func, this->storage);

	for (size_t i = 0; i < this->storage->dim(); i++)
	{
		s.sweep1D(alpha, alpha, i);
	}

	detail::DehierarchisationLinear func2(this->storage);
	sweep<detail::DehierarchisationLinear> s2(func2, this->storage);

	for (size_t i = 0; i < this->storage->dim(); i++)
	{
		s2.sweep1D(alpha, alpha, (this->storage->dim() - (i + 1)));
	}

}

void OperationHierarchisationPrewavelet::expandGrid()
{
	for (size_t i = 0; i < shadowStorage->size(); i++)
	{
		(*shadowStorage->get(i)).toString(std::cout);
		this->storage->insert(*shadowStorage->get(i));
		if ((*shadowStorage->get(i)).isLeaf())
			std::cout << "is Leaf : " << std::endl;
		else
			std::cout << "nööö" << std::endl;
	}
}

void OperationHierarchisationPrewavelet::shrinkGrid()
{
	for (size_t i = 0; i < shadowStorage->size(); i++)
	{
		this->storage->deleteLast();
	}
}

}