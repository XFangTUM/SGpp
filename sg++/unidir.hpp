/*
This file is part of sg++, a program package making use of spatially adaptive sparse grids to solve numerical problems

Copyright (C) 2007  Jörg Blank (blankj@in.tum.de)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef UNIDIR_HPP_
#define UNIDIR_HPP_

#include "GridStorage.hpp"
#include "DataVector.h"

#include <vector>

namespace sg
{

/**
 * Unidirectional scheme with gradient
 */
class UnidirGradient
{
public:
	UnidirGradient(GridStorage* storage) : storage(storage) {}
	virtual ~UnidirGradient() {}
	
	virtual void updown(DataVector& alpha, DataVector& result)
	{
		DataVector beta(result.getSize());
		result.setAll(0.0);
		
		for(size_t i = 0; i < storage->dim(); i++)
		{
			this->updown(alpha, beta, storage->dim() - 1, i);
			result.add(beta);
		}
	}

protected:
	typedef GridStorage::grid_iterator grid_iterator;
	
	GridStorage* storage;

/**
 * Recursive procedure for updown(). In dimension <i>gradient_dim</i> the L2 scalar product of the
 * gradients is used. In all other dimensions only the L2 scalar product.
 * @param dim the current dimension
 * @param gradient_dim the dimension in which to use the gradient
 * @param alpha vector of coefficients
 * @param result vector to store the results in
 */
	virtual void updown(DataVector& alpha, DataVector& result, size_t dim, size_t gradient_dim)
	{
		if(dim == gradient_dim)
		{
			// this got its own function so we can use partial template specialization
			// if more than just down is needed
			gradient(alpha, result, dim, gradient_dim);
		}
		else
		{
			//Unidirectional scheme
			if(dim > 0)
			{
				// Reordering ups and downs
				// Use previously calculated ups for all future calculations
				// U* -> UU* and UD*
				
				DataVector temp(alpha.getSize());
				up(alpha, temp, dim);
				updown(temp, result, dim-1, gradient_dim);


				// Same from the other direction:
				// *D -> *UD and *DD

				DataVector result_temp(alpha.getSize());
				updown(alpha, temp, dim-1, gradient_dim);
				down(temp, result_temp, dim);
	
				
				//Overall memory use: 2*|alpha|*(d-1)
				
				result.add(result_temp);
			}
			else
			{
				// Terminates dimension recursion
				up(alpha, result, dim);

				DataVector temp(alpha.getSize());
				down(alpha, temp, dim);

				result.add(temp);					
			}
			
		}
	}
	
/**
 * Up-step in dimension <i>dim</i> for \f$(\phi_i(x),\phi_j(x))_{L_2}\f$.
 * Applies the up-part of the one-dimensional mass matrix in one dimension.
 * Computes \f[\int_{x=0}^1  \phi_i(x) \sum_{j, l_i < l_j} \alpha_j \phi_j(x) dx.\f]
 * @param dim dimension in which to apply the up-part
 * @param alpha vector of coefficients
 * @param result vector to store the results in
 */
	virtual void up(DataVector& alpha, DataVector& result, size_t dim) = 0;
	
/**
 * Down-step in dimension <i>dim</i> for \f$(\phi_i(x),\phi_j(x))_{L_2}\f$.
 * Applies the down-part of the one-dimensional mass matrix in one dimension.
 * Computes \f[\int_{x=0}^1  \phi_i(x) \sum_{j, l_i\geq l_j} \alpha_j \phi_j(x) dx.\f]
 * @param dim dimension in which to apply the down-part
 * @param alpha vector of coefficients
 * @param result vector to store the results in
 */
	virtual void down(DataVector& alpha, DataVector& result, size_t dim) = 0;

/**
 * All calculations for gradient_dim. 
 */
	virtual void gradient(DataVector& alpha, DataVector& result, size_t dim, size_t gradient_dim)
	{
		//Unidirectional scheme
		if(dim > 0)
		{
			// Reordering ups and downs
			// Use previously calculated ups for all future calculations
			// U* -> UU* and UD*
			
			DataVector temp(alpha.getSize());
			upGradient(alpha, temp, dim);
			updown(temp, result, dim-1, gradient_dim);


			// Same from the other direction:
			// *D -> *UD and *DD

			DataVector result_temp(alpha.getSize());
			updown(alpha, temp, dim-1, gradient_dim);
			downGradient(temp, result_temp, dim);

			
			//Overall memory use: 2*|alpha|*(d-1)
			
			result.add(result_temp);
		}
		else
		{
			// Terminates dimension recursion
			upGradient(alpha, result, dim);

			DataVector temp(alpha.getSize());
			downGradient(alpha, temp, dim);

			result.add(temp);					
		}
		
	}
	
	
	virtual void downGradient(DataVector& alpha, DataVector& result, size_t dim) = 0;
	virtual void upGradient(DataVector& alpha, DataVector& result, size_t dim) = 0;

};

}

#endif /*UNIDIR_HPP_*/
