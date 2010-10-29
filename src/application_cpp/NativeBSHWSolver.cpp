/******************************************************************************
* Copyright (C) 2009 Technische Universitaet Muenchen                         *
* This file is part of the SG++ project. For conditions of distribution and   *
* use, please see the copyright notice at http://www5.in.tum.de/SGpp          *
******************************************************************************/
// @author Chao qi (qic@in.tum.de)

#include "sgpp.hpp"
#include <iostream>
#include <string>
#include <stdlib.h>
#include <fstream>
#include <iomanip>
#include <cmath>
/// default number of Implicit Euler steps before starting with Crank Nicolson approach
#define CRNIC_IMEUL_STEPS 3
/// default value for epsilon in gridpoints @money
#define DFLT_EPS_AT_MONEY 0.0

/**
 * reads the values of mu, sigma and rho of all assets from
 * a file and stores them into three separated DataVectors
 *
 * @param tFile the file that contains the stochastic data
 * @param numAssests the of Assets stored in the file
 * @param mu DataVector for the exspected values
 * @param sigma DataVector for standard deviation
 * @param rho DataMatrix for the correlations
 *
 * @return returns 0 if the file was successfully read, otherwise -1
 */
int readStochasticData(std::string tFile, size_t numAssests, DataVector& mu, DataVector& sigma, DataMatrix& rho)
{
	std::fstream file;
	double cur_mu;
	double cur_sigma;
	double cur_rho;

	file.open(tFile.c_str());

	if(!file.is_open())
	{
		std::cout << "Error cannot read file: " << tFile << std::endl;
		return -1;
	}

	for (size_t i = 0; i < numAssests; i++)
	{
		file >> cur_mu;
		file >> cur_sigma;
		mu.set(i, cur_mu);
		sigma.set(i, cur_sigma);
		for (size_t j = 0; j < numAssests; j++)
		{
			file >> cur_rho;
			rho.set(i,j, cur_rho);
		}
	}

	file.close();

	return 0;
}

/**
 * reads the values of the Bounding Box
 *
 * @param tFile the file that contains the stochastic data
 * @param numAssests the of Assets stored in the file
 * @param BoundaryArray Pointer to the Bounding Box array
 *
 * @return returns 0 if the file was successfully read, otherwise -1
 */
int readBoudingBoxData(std::string tFile, size_t numAssests, sg::DimensionBoundary* BoundaryArray)
{
	std::fstream file;
	double cur_right;
	double cur_left;

	file.open(tFile.c_str());

	if(!file.is_open())
	{
		std::cout << "Error cannot read file: " << tFile << std::endl;
		return -1;
	}

	for (size_t i = 0; i < numAssests; i++)
	{
		file >> cur_left;
		file >> cur_right;

		BoundaryArray[i].leftBoundary = cur_left;
		BoundaryArray[i].rightBoundary = cur_right;
		BoundaryArray[i].bDirichletLeft = true;
		BoundaryArray[i].bDirichletRight = true;
	}

	file.close();

	return 0;
}

/**
 * calculate the theta value for the Hull White model
 *
 * @param a is the mean reversion rate
 * @param sigma is the volatility
 * @param T is the maturity
 * @param count*stepsize is the current time
 *
 * @return returns 0 if the file was successfully read, otherwise -1
 */

double calculatetheta(double a, double sigma, double T, int count, double stepsize)
{
	double theta=0;
	return theta=0.04*a + pow(sigma,2.0)*(1-exp(-2*a*(T-count*stepsize)))/(2*a);
}
/**
 * Combine Hull White solver and Black Scholes solver with European call option
 *
 * @param l the number of levels used in the Sparse Grid
 * @param the stochastic data (theta, sigmahw, sigmabs, a)
 * @param the grid's bounding box - domain boundary(min,max)
 * @param payoffType method that is used to determine the multidimensional payoff function
 * @param timeSt the number of timesteps that are executed during the solving process
 * @param dt the size of delta t in the ODE solver
 * @param CGIt the maximum number of Iterations that are executed by the CG/BiCGStab
 * @param CGeps the epsilon used in the CG/BiCGStab
 * @param Solver specifies the sovler that should be used, ExEul, ImEul and CrNic are the possibilities
 */
void testBSHW(size_t d,size_t l, double sigma, double a, std::string fileStoch, std::string fileBound, std::string payoffType,
		size_t timeSt, double dt, size_t CGIt, double CGeps, std::string Solver, double T,double dStrike, bool isLogSolve)
{
	    size_t dim = d;
		size_t level = l;
		size_t timesteps = timeSt;
		double stepsize = dt;
		size_t CGiterations = CGIt;
		double CGepsilon = CGeps;
		DataVector mu(dim);
		DataVector sigmabs(dim);
		DataMatrix rho(dim,dim);

		if (readStochasticData(fileStoch, dim, mu, sigmabs, rho) != 0)
			{
				return;
			}

		sg::DimensionBoundary* myBoundaries = new sg::DimensionBoundary[dim];
		if (readBoudingBoxData(fileBound, dim, myBoundaries) != 0)
		{
			return;
		}
        //std::cout<<myBoundaries[0].bDirichletLeft << std::endl;
        //std::cout<<myBoundaries[1].bDirichletLeft << std::endl;
		sg::BlackScholesHullWhiteSolver* myBSHWSolver;
		if (isLogSolve == true)
			{
				myBSHWSolver = new sg::BlackScholesHullWhiteSolver(true);
			}
			else
			{
				myBSHWSolver = new sg::BlackScholesHullWhiteSolver(false);
			}
		sg::BoundingBox* myBoundingBox = new sg::BoundingBox(dim, myBoundaries);
		delete[] myBoundaries;
		// init Screen Object
		myBSHWSolver->initScreen();

		// Construct a grid
		myBSHWSolver->constructGrid(*myBoundingBox, level);

		// init the basis functions' coefficient vector
		DataVector* alpha = new DataVector(myBSHWSolver->getNumberGridPoints());

		std::cout << "Grid has " << level << " Levels" << std::endl;
		std::cout << "Initial Grid size: " << myBSHWSolver->getNumberGridPoints() << std::endl;
		std::cout << "Initial Grid size (inner): " << myBSHWSolver->getNumberInnerGridPoints() << std::endl << std::endl << std::endl;

	// Init the grid with on payoff function
	myBSHWSolver->initGridWithPayoffBSHW(*alpha, dStrike, payoffType, a, sigma);

	// Gridpoints @Money
	std::cout << "Gridpoints @Money: " << myBSHWSolver->getGridPointsAtMoney(payoffType, dStrike, DFLT_EPS_AT_MONEY) << std::endl << std::endl << std::endl;

	// Print the payoff function into a gnuplot file
	if (dim < 3)
	{
		if (dim == 2)
		{
			sg::DimensionBoundary* myAreaBoundaries = new sg::DimensionBoundary[dim];

			for (size_t i = 0; i < 2; i++)
			{
				myAreaBoundaries[i].leftBoundary = 0.9;
				myAreaBoundaries[i].rightBoundary = 1.1;
			}
			sg::BoundingBox* myGridArea = new sg::BoundingBox(dim, myAreaBoundaries);

			myBSHWSolver->printGridDomain(*alpha, 50, *myGridArea, "payoff_area.gnuplot");

			delete[] myAreaBoundaries;
			delete myGridArea;
		}
		myBSHWSolver->printGrid(*alpha, 30, "payoffBSHW.gnuplot");
		myBSHWSolver->printSparseGrid(*alpha, "payoffBSHW_surplus.grid.gnuplot", true);
		myBSHWSolver->printSparseGrid(*alpha, "payoffBSHW_nodal.grid.gnuplot", false);
		//myBSHWSolver->printPayoffInterpolationError2D(*alpha, "payoff_interpolation_error.grid.gnuplot", 10000, dStrike);
		if (isLogSolve == true)
		{
			myBSHWSolver->printSparseGridExpTransform(*alpha, "payoffBSHW_surplus_cart.grid.gnuplot", true);
			myBSHWSolver->printSparseGridExpTransform(*alpha, "payoffBSHW_nodal_cart.grid.gnuplot", false);
		}
	}

	// Set stochastic data
	//myBSHWSolver->setStochasticData(mu, sigmabs, rho, 0.0,theta, sigma, a);

	// Start combining the Black Scholes and Hull White Equation
	if (Solver == "ExEul")
	{
		double theta=0;
		int count=0;
		for (int i=0; i<T/stepsize; i++)
		{
		theta=calculatetheta(a, sigma, T, count,stepsize);
		myBSHWSolver->setStochasticData(mu, sigmabs, rho, 0.0,theta, sigma, a);
		myBSHWSolver->solveExplicitEuler(timesteps, stepsize, CGiterations, CGepsilon, *alpha, false, false, 20);
		count=count+1;
	    }
	}
	else if (Solver == "ImEul")
	{
		double theta=0;
		int count=0;
		for (int i=0; i<T/stepsize; i++)
		{
		theta=calculatetheta(a, sigma, T, count,stepsize);
		myBSHWSolver->setStochasticData(mu, sigmabs, rho, 0.0,theta, sigma, a);
		myBSHWSolver->solveImplicitEuler(timesteps, stepsize, CGiterations, CGepsilon, *alpha, false, false, 20);
		count=count+1;
	    }
	}
	else
	{
		std::cout << "!!!! You have chosen an unsupported solver type !!!!" << std::endl;
	}

	if (dim < 3)
	{
		// Print the solved Black Scholes Equation into a gnuplot file
		myBSHWSolver->printGrid(*alpha, 42, "solvedBSHW.gnuplot");
		myBSHWSolver->printSparseGrid(*alpha, "solvedBSHW_surplus.grid.gnuplot", true);
		myBSHWSolver->printSparseGrid(*alpha, "solvedBSHW_nodal.grid.gnuplot", false);
		/*if (isLogSolve == true)
		{
			myBSSolver->printSparseGridExpTransform(*alpha, "solvedBSHW_surplus_cart.grid.gnuplot", true);
			myBSSolver->printSparseGridExpTransform(*alpha, "solvedBSHW_nodal_cart.grid.gnuplot", false);
		}*/
	}

	// Test call @ the money
	std::vector<double> point;
	for (size_t i = 0; i < d; i++)
	{
		if (isLogSolve == true)
		{
			point.push_back(log(1.0));
		}
		else
		{
			point.push_back(1.0);
		}
	}
	std::cout << "Optionprice at testpoint: " << myBSHWSolver->evaluatePoint(point, *alpha) << std::endl << std::endl;

	delete alpha;
	delete myBSHWSolver;
	delete myBoundingBox;
}







/**
 * Calls the writeHelp method in the BlackScholesHullWhiteSolver Object
 * after creating a screen.
 */
void writeHelp()
{
	std::stringstream mySStream;

	mySStream << "Some instructions for the use of combing Hull White and Black Scholes Solver:" << std::endl;
		mySStream << "------------------------------------------------------" << std::endl << std::endl;
		mySStream << "Available execution modes are:" << std::endl;
		mySStream << "  CombineBSHW" << std::endl;

		mySStream << "Execution modes descriptions:" << std::endl;
		mySStream << "-----------------------------------------------------" << std::endl;
		mySStream << "Combine Hull-White and Black-Scholes" << std::endl << "------" << std::endl;
		mySStream << "the following options must be specified:" << std::endl;
		mySStream << "	dim: the number of dimensions of Sparse Grid" << std::endl;
		mySStream << "	level: number of levels within the Sparse Grid" << std::endl;
		mySStream << "	value of sigma: sigma value-determine overall level of volatility for hull white" << std::endl;
	    mySStream << "	value of a: a" << std::endl;
	    mySStream << "	file_Stochdata: file with the asset's mu, sigma, rho" << std::endl;
	    mySStream << "	file_Boundaries: file that contains the bounding box" << std::endl;
		mySStream << "	payoff_func: function for n-d payoff: std_euro_{call|put}" << std::endl;
		mySStream << "	simeSt: number of time steps of doing HW and BS when calling allternatively, default: 1" << std::endl;
		mySStream << "	dt: timestep size" << std::endl;
		mySStream << "	CGIterations: Maxmimum number of iterations used in CG mehtod" << std::endl;
		mySStream << "	CGEpsilon: Epsilon used in CG" << std::endl;
		mySStream << "	Solver: the solver to use: ExEul, ImEul or CrNic" << std::endl;
		mySStream << "	T: time to maturity" << std::endl;
		mySStream << "	Strike: the strike" << std::endl;
		mySStream << "	Coordinates: cart: cartisian coordinates; log: log coords, this is only related to Black-Scholes" << std::endl;

		mySStream << std::endl;
		mySStream << "Example:" << std::endl;
		mySStream << "2 5 0.01 0.1 stoch.data  bound.data " << " std_euro_call "<< "1.0 " << "0.01 "<< "400 " << "0.000001 " << "ImEul " << "1.0 " <<"0.3 " << "cart "<<std::endl;
		mySStream << std::endl;

		mySStream << std::endl << std::endl;
		std::cout << mySStream.str() << std::endl;
	}

	/**
	 * main routine of the application, do some first cli
	 * correction test and branches to right solver configuration
	 *
	 * @param argc contains the number of cli arguments
	 * @param argv contains the cli arguments as C-Strings
	 */
	int main(int argc, char *argv[])
	{
		std::string option;

		if (argc == 1)
		{
			writeHelp();

			return 0;
		}

		option.assign(argv[1]);

		if (option == "CombineBSHW")
		{
			if (argc != 17)
			{
				writeHelp();
			}
			else
			{
				std::string solver;
				std::string payoff;
				double sigma;
				double a;
				double dStrike;
				std::string fileStoch;
				std::string fileBound;
				sigma = atof(argv[4]);
				a = atof(argv[5]);
				fileStoch.assign(argv[6]);
				fileBound.assign(argv[7]);
				payoff.assign(argv[8]);
				solver.assign(argv[13]);
				dStrike = atof(argv[15]);
				std::string coordsType;
				bool coords = false;
				coordsType.assign(argv[16]);
			    if (coordsType == "cart")
				    {
						coords = false;
					}
						else if (coordsType == "log")
					{
						coords = true;
					}
					    else
					{
						std::cout << "Unsupported coordinate option! cart or log are supported!" << std::endl;
						std::cout << std::endl << std::endl;
						writeHelp();
					}
				//testHullWhite(size_t l, double theta, double signma, double a, std::string fileBound, std::string payoffType,
					//	size_t timeSt, double dt, size_t CGIt, double CGeps, std::string Solver, double t, double T)
				testBSHW(atoi(argv[2]),atoi(argv[3]),sigma, a, fileStoch, fileBound, payoff, atof(argv[9]), atof(argv[10]), atoi(argv[11]), atof(argv[12]), solver,atof(argv[14]),dStrike,coords);
			}
		}

		else
		{
			writeHelp();
		}

		return 0;
	}
