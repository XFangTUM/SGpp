/******************************************************************************
* Copyright (C) 2009 Technische Universitaet Muenchen                         *
* This file is part of the SG++ project. For conditions of distribution and   *
* use, please see the copyright notice at http://www5.in.tum.de/SGpp          *
******************************************************************************/
// @author Chao qi (qic@in.tum.de)

#ifndef HULLWHITESOLVER_HPP
#define HULLWHITESOLVER_HPP

#include "sgpp.hpp"

#include "application/pde/ParabolicPDESolver.hpp"

#include "grid/type/LinearTrapezoidBoundaryGrid.hpp"
#include "grid/type/LinearGrid.hpp"
#include "grid/common/BoundingBox.hpp"
#include "application/common/ScreenOutput.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cmath>
#include <algorithm>

namespace sg
{

/**
 * This class provides a simple-to-use solver of the "multi" dimensional Hull
 * White Equation that uses Sparse Grids.
 *
 * The class's aim is, to hide all complex details of solving the Hull White
 * Equation with Sparse Grids!
 *
 * @version $HEAD$
 */


class HullWhiteSolver : public ParabolicPDESolver
{
private:
	///  the theta value
	double theta;
	/// the sigma value
	double sigma;
	/// the a value
	double a;
	/// the current time
	//double t;
	/// stores if the stochastic asset data was passed to the solver
    bool bStochasticDataAlloc;
	/// screen object used in this solver
	ScreenOutput* myScreen;
	/// use coarsening between timesteps in order to reduce gridsize
	bool useCoarsen;
	/// Threshold used to decide if a grid point should be deleted
	double coarsenThreshold;
	/// Threshold used to decide if a grid point should be refined
	double refineThreshold;
	/// adaptive mode during solving Black Scholes Equation: none, coarsen, refine, coarsenNrefine
	std::string adaptSolveMode;
	/// refine mode during solving Black Scholes Equation: classic or maxLevel
	std::string refineMode;
	/// number of points the are coarsened in each coarsening-step
	int numCoarsenPoints;
	/// max. level for refinement during solving
	size_t refineMaxLevel;
	/// variable to store needed solving iterations

	/**
	 * returns the option value (payoff value) for an European call option
	 *
	 * @param assetValue the current asset's value
	 * @param strike the strike price of the option
	 *
	 * @return the call premium
	 */
	//double get1DEuroCallPayoffValue(double assetValue, double r);


public:
	/**
	 * Std-Constructor of the solver
	 */
	HullWhiteSolver();

	/**
	 * Std-Destructor of the solver
	 */
	virtual ~HullWhiteSolver();

	void constructGrid(BoundingBox& myBoundingBox, size_t level);

	void setStochasticData(double theta, double sigma, double a);


	void solveImplicitEuler(size_t numTimesteps, double timestepsize, size_t maxCGIterations, double epsilonCG, DataVector& alpha, bool verbose = false, bool generateAnimation = false, size_t numEvalsAnimation = 20);

	void solveExplicitEuler(size_t numTimesteps, double timestepsize, size_t maxCGIterations, double epsilonCG, DataVector& alpha, bool verbose = false, bool generateAnimation = false, size_t numEvalsAnimation = 20);

	void solveCrankNicolson(size_t numTimesteps, double timestepsize, size_t maxCGIterations, double epsilonCG, DataVector& alpha, size_t NumImEul = 0);

	//void solveAdamsBashforth(size_t numTimesteps, double timestepsize, size_t maxCGIterations, double epsilonCG, DataVector& alpha, bool verbose = false, bool generateAnimation = false, size_t numEvalsAnimation = 20);

	//void solveVarTimestep(size_t numTimesteps, double timestepsize, size_t maxCGIterations, double epsilonCG, DataVector& alpha, bool verbose = false, bool generateAnimation = false, size_t numEvalsAnimation = 20);

	/**
	 * Solves the closed form of the Hull White equation, the Hull White
	 * formular. It evaluates the Hull White formular in a Interest Rate Range
	 * from 0.0 to 1.0, by increasing the Interest Rate in every step by
	 * a given (small) values, so the analytical solution of the PDE can
	 * be determined and compared.
	 *
	 * @param premiums the result vector, here the combinations of stock price and premium are stored
	 * @param a the a value
	 * @param maxRate the maximum rate regarded in these calculations
	 * @param RateInc the increase of the interest rate in one step
	 * @param strike the strike price of the Option
	 * @param t time to maturity
	 */
//	void solve1DAnalytic(std::vector< std::pair<double, double> >& premiums, double a, double maxRate, double RateInc, double strike, double t);

	/**
	 * Writes the premiums into a file that can be easily plot with gnuplot
	 *
	 * @param premiums the result vector, here the combinations of stock price and premium are stored
	 * @param tfilename absolute path to file into which the grid's evaluation is written
	 */
//	void print1DAnalytic(std::vector< std::pair<double, double> >& premiums, std::string tfilename);

	/**
	 * Inits the alpha vector with a payoff function of an European call option
	 *
	 * @param alpha the coefficient vector of the grid's ansatzfunctions
	 * @param strik the option's strike
	 * @param payoffType specifies the type of the combined payoff function; std_euro_call or std_euro_put are available
	 * @param sigma the sigma value in HullWhite model
	 * @param a the value of a in HullWhite model
	 * @param t the current time
	 * @param T the maturity time
	 */
	void initGridWithPayoff(DataVector& alpha, double strike, std::string payoffType,double sigma, double a, double t, double T);

	/**
	 * Inits the screen object
	 */
	void initScreen();

	/**
	 * returns the algorithmic dimensions (the dimensions in which the Up Down
	 * operations (need for space discretization) should be applied)
	 *
	 * @return the algorithmic dimensions
	 */
	std::vector<size_t> getAlgorithmicDimensions();

	/**
	 * sets the algorithmic dimensions (the dimensions in which the Up Down
	 * operations (need for space discretization) should be applied)
	 *
	 * @param algoDims std::vector containing the algorithmic dimensions
	 */
	void setAlgorithmicDimensions(std::vector<size_t> newAlgoDims);

	/**
	 *	enables coarsening of grid during solving the Black Scholes
	 *	Equation. The coarsening settings have to be specified in order to
	 *	enable coarsening.
	 *
	 *	@param coarsenThreshold Threshold needed to determine if a grid point should be removed
	 *	@param coarsenPercent Percent of removable grid points that should be tested for deletion
	 *	@param numExecCoarsen denotes the number coarsening procedures within one timestep
	 */
	//void setEnableCoarseningData(double coarsenThreshold, double coarsenPercent, size_t numExecCoarsen);

	/**
	 * Refines a grid by taking the grid's coefficients into account. This refinement method
	 * refines the grid based on the surplus by refining grid points with big surpluses
	 * first.
	 * The grid is refined to max. Level!
	 *
	 * @param alpha a DataVector containing the grids coefficients
	 * @param dThreshold Threshold for a point's surplus for refining this point
	 * @param maxLevel maxLevel of refinement
	 */
	//void refineSurplusToMaxLevel(DataVector& alpha, double dThreshold, unsigned int maxLevel);

	/**
	 * prints the 2D interpolation error @money into a file. This file is plotable via gnuplot. A bounding
	 * box [0,x] X [0,y] is assumed.
	 *
	 * @param alpha the sparse grid's coefficients
	 * @param tFilename the name of file contain the interpolation error
	 * @param numTestpoints Number of equal distribute testpoints @money
	 * @param strike the option's strike
	 */
	//void printPayoffInterpolationError2D(DataVector& alpha, std::string tFilename, size_t numTestpoints, double strike);
};

}

#endif /* BLACKSCHOLESSOLVER_HPP */
