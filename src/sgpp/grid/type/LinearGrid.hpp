/******************************************************************************
* Copyright (C) 2009 Technische Universitaet Muenchen                         *
* This file is part of the SG++ project. For conditions of distribution and   *
* use, please see the copyright notice at http://www5.in.tum.de/SGpp          *
******************************************************************************/
// @author Jörg Blank (blankj@in.tum.de), Alexander Heinecke (Alexander.Heinecke@mytum.de)

#ifndef LINEARGRID_HPP
#define LINEARGRID_HPP

#include "grid/Grid.hpp"
#include "grid/common/BoundingBox.hpp"

#include <iostream>

namespace sg
{

/**
 * grid with linear base functions
 */
class LinearGrid : public Grid
{
protected:
	LinearGrid(std::istream& istr);

public:
	/**
	 * Constructor Linear Grid without boundaries
	 *
	 * @param dim the dimension of the grid
	 */
	LinearGrid(size_t dim);

	/**
	 * Constructor Linear Grid
	 *
	 * @param BB the BoundingBox of the grid
	 */
	LinearGrid(BoundingBox& BB);

	/**
	 * Destructor
	 */
	virtual ~LinearGrid();

	virtual const char* getType();

	virtual OperationB* createOperationB();
	virtual OperationBVectorized* createOperationBVectorized(const std::string& VecType);
	virtual OperationBVectorizedSP* createOperationBVectorizedSP(const std::string& VecType);
	virtual GridGenerator* createGridGenerator();
	virtual OperationMatrix* createOperationLaplace();
	virtual OperationEval* createOperationEval();
	virtual OperationTest* createOperationTest();
	virtual OperationHierarchisation* createOperationHierarchisation();
	virtual OperationMatrix* createOperationLTwoDotProduct();
	virtual OperationMatrix* createOperationLE();
	virtual OperationMatrix* createOperationLB();
	virtual OperationMatrix* createOperationLF();
	virtual OperationMatrix* createOperationLD();


	// @todo (heinecke) remove this when done
	virtual OperationMatrix* createOperationUpDownTest();

	// finance operations
	virtual OperationMatrix* createOperationDelta(DataVector& coef);
	virtual OperationMatrix* createOperationGamma(DataMatrix& coef);
	virtual OperationMatrix* createOperationDeltaLog(DataVector& coef);
	virtual OperationMatrix* createOperationGammaLog(DataMatrix& coef);

	static Grid* unserialize(std::istream& istr);
};

}

#endif /* LINEARGRID_HPP */
