/*****************************************************************************/
/* This file is part of pysgpp, a program package making use of spatially    */
/* adaptive sparse grids to solve numerical problems                         */
/*                                                                           */
/* Copyright (C) 2008 Joerg Blank (blankj@in.tum.de)                         */
/* Copyright (C) 2009 Alexander Heinecke (Alexander.Heinecke@mytum.de)       */
/*                                                                           */
/* pysgpp is free software; you can redistribute it and/or modify            */
/* it under the terms of the GNU Lesser General Public License as published  */
/* by the Free Software Foundation; either version 3 of the License, or      */
/* (at your option) any later version.                                       */
/*                                                                           */
/* pysgpp is distributed in the hope that it will be useful,                 */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU Lesser General Public License for more details.                       */
/*                                                                           */
/* You should have received a copy of the GNU Lesser General Public License  */
/* along with pysgpp; if not, write to the Free Software                     */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/* or see <http://www.gnu.org/licenses/>.                                    */
/*****************************************************************************/

%module(directors="1") pysgpp

%include "stl.i"
%include "std_vector.i"
%include "std_pair.i"

%include "typemaps.i"

%include "exception.i"

%exception {
  try {
    $action
  } catch (const std::exception& e) {
    SWIG_exception(SWIG_RuntimeError, e.what());
  }
}

%include "carrays.i"
//%array_class(double, doubleArray);
%array_class(unsigned int, unsignedIntArray);


namespace std {
	%template(DoubleVector) vector<double>;
	%template(IndexValPair) pair<size_t, double>;
	%template(IndexValVector) vector<pair<size_t, double> >;
}

// This should include all necessary header files
%{
#include "sgpp.hpp"
%}

// The Good, i.e. without any modifications
%include "src/sgpp/grid/common/BoundingBox.hpp"
%include "src/sgpp/grid/storage/hashmap/SerializationVersion.hpp"
%include "src/sgpp/grid/storage/hashmap/HashGridIndex.hpp"
%include "src/sgpp/grid/storage/hashmap/HashGridIterator.hpp"
%include "src/sgpp/grid/storage/hashmap/HashGridStorage.hpp"
%include "src/sgpp/grid/GridStorage.hpp"

%include "Operations.i"

%include "src/sgpp/grid/generation/hashmap/HashGenerator.hpp"
%include "src/sgpp/grid/generation/hashmap/HashRefinement.hpp"
%include "src/sgpp/grid/generation/hashmap/HashRefinementBoundaries.hpp"
%include "src/sgpp/grid/generation/StandardGridGenerator.hpp"
%include "src/sgpp/grid/generation/BoundaryGridGenerator.hpp"
%include "src/sgpp/grid/generation/SurplusRefinementFunctor.hpp"

%include "GridFactory.i"

// the Bad

%include "DataVector.i"

// and the rest

%include "src/sgpp/sgpp.hpp"

%include "src/sgpp/algorithm/classification/AlgorithmDGEMV.hpp"
%include "src/sgpp/algorithm/classification/AlgorithmDGEMVBoundaries.hpp"
%include "src/sgpp/algorithm/classification/test_dataset.hpp"
%include "src/sgpp/algorithm/classification/test_dataset_boundary.hpp"
%include "src/sgpp/algorithm/common/GetAffectedBasisFunctions.hpp"
%include "src/sgpp/algorithm/common/GetAffectedBasisFunctionsBoundaries.hpp"
%include "src/sgpp/algorithm/common/GetAffectedBasisFunctionsBBBoundaries.hpp"
%include "src/sgpp/algorithm/common/sweep.hpp"
%include "src/sgpp/algorithm/classification/UnidirGradient.hpp"
%include "src/sgpp/algorithm/classification/DMSystemMatrix.hpp"
%include "src/sgpp/algorithm/finance/BlackScholesTimestepMatrix.hpp"

%include "src/sgpp/basis/linear/linear_base.hpp"
%include "src/sgpp/basis/linearboundary/linearboundaryBase.hpp"
%include "src/sgpp/basis/lineartrapezoidboundary/lineartrapezoidboundaryBase.hpp"
%include "src/sgpp/basis/modlinear/modified_linear_base.hpp"
%include "src/sgpp/basis/modpoly/modified_poly_base.hpp"
%include "src/sgpp/basis/poly/poly_base.hpp"
%include "src/sgpp/basis/modwavelet/modified_wavelet_base.hpp"

%include "src/sgpp/linearSystem/LinearSystem.hpp"
%include "src/sgpp/solver/SGSolver.hpp"
%feature("director") ConjugateGradients;
%include "src/sgpp/solver/cg/ConjugateGradients.hpp"


%apply std::string *INPUT { std::string& istr };

%apply unsigned int *OUTPUT { unsigned int& l, unsigned int& i };

%template(GridIndex) sg::HashGridIndex<unsigned int, unsigned int>;
%template(GridStorage) sg::HashGridStorage<sg::GridIndex>;

%template(SLinearBase) sg::linear_base<unsigned int, unsigned int>;
%template(SLinearBoundaryBase) sg::linearboundaryBase<unsigned int, unsigned int>;
%template(SLinearTrapezoidBoundaryBase) sg::lineartrapezoidboundaryBase<unsigned int, unsigned int>;
%template(SModLinearBase) sg::modified_linear_base<unsigned int, unsigned int>;
%template(SPolyBase) sg::poly_base<unsigned int, unsigned int>;
%template(SModPolyBase) sg::modified_poly_base<unsigned int, unsigned int>;

%apply std::vector<std::pair<size_t, double> > *OUTPUT { std::vector<std::pair<size_t, double> >& result };
%apply std::vector<double> *INPUT { std::vector<double>& point }; 
%template(SGetAffectedBasisFunctions) sg::GetAffectedBasisFunctions<sg::SLinearBase>;
%template(SGetAffectedBasisFunctionsBoundaries) sg::GetAffectedBasisFunctionsBoundaries<sg::SLinearBoundaryBase>;
%template(SGetAffectedBasisFunctionsTrapezoidBoundaries) sg::GetAffectedBasisFunctionsBoundaries<sg::SLinearTrapezoidBoundaryBase>;
%template(SGetAffectedBasisFunctionsBBTrapezoidBoundaries) sg::GetAffectedBasisFunctionsBBBoundaries<sg::SLinearTrapezoidBoundaryBase>;

%template(test_dataset_linear) sg::test_dataset<sg::SLinearBase>;
%template(test_dataset_linearboundary) sg::test_dataset_boundary<sg::SLinearBoundaryBase>;
%template(test_dataset_lineartrapezoidboundary) sg::test_dataset_boundary<sg::SLinearTrapezoidBoundaryBase>;
%template(test_dataset_modlin) sg::test_dataset<sg::SModLinearBase>;
