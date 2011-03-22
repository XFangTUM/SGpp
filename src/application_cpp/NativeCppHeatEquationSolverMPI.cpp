/******************************************************************************
* Copyright (C) 2011 Technische Universitaet Muenchen                         *
* This file is part of the SG++ project. For conditions of distribution and   *
* use, please see the copyright notice at http://www5.in.tum.de/SGpp          *
******************************************************************************/
// @author Alexander Heinecke (Alexander.Heinecke@mytum.de)

/// default number of Implicit Euler steps when using Crank Nicolson
#define CRNIC_IMEUL_STEPS 3
#define GUNPLOT_RESOLUTION 51
#define SOLUTION_FRAMES 100

#define DIV_SIGMA 4.0
#define DISTRI_FACTOR 5.0

#include <mpi.h>

#include <cstdlib>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <string>

#include "sgpp_mpi.hpp"


/**
 * Calls the writeHelp method in the BlackScholesSolver Object
 * after creating a screen.
 */
void writeHelp()
{
	sg::HeatEquationSolver* myHESolver = new sg::HeatEquationSolver();

	myHESolver->initScreen();

	delete myHESolver;

	std::stringstream mySStream;

	mySStream << "Some instructions for the use of Poisson/ Heat Equation Solver:" << std::endl;
	mySStream << "---------------------------------------------------------------" << std::endl << std::endl;
	mySStream << "Available execution modes are:" << std::endl;
	mySStream << "  HeatEquation        Solves Heat Equation on a quadratic" << std::endl;
	mySStream << "                      d-dimensional domain" << std::endl << std::endl;
	mySStream << "  PoissonEquation     Solves Poisson Equation on a quadratic" << std::endl;
	mySStream << "                      d-dimensional domain" << std::endl << std::endl << std::endl;

	mySStream << "Execution modes descriptions:" << std::endl;
	mySStream << "-----------------------------------------------------" << std::endl;
	mySStream << "HeatEquation" << std::endl << "------" << std::endl;
	mySStream << "the following options must be specified:" << std::endl;
	mySStream << "	dim: the number of dimensions of Sparse Grid" << std::endl;
	mySStream << "	level: number of levels within the Sparse Grid" << std::endl;
	mySStream << "	left_bound: x_i of left boundary" << std::endl;
	mySStream << "	right_bound: x_i of right boundary" << std::endl;
	mySStream << "	a: thermal diffusivity" << std::endl;
	mySStream << "	initHeat: initial heat distribution" << std::endl;
	mySStream << "	T: time to solve" << std::endl;
	mySStream << "	dT: timestep size" << std::endl;
	mySStream << "	Solver: the solver to use: ExEul, ImEul, CrNic" << std::endl;
	mySStream << "	CGEpsilon: Epsilon used in CG" << std::endl;
	mySStream << "	CGIterations: Maxmimum number of iterations used in CG mehtod" << std::endl;
	mySStream << std::endl;
	mySStream << "Example:" << std::endl;
	mySStream << "HESolver HeatEquation 3 5 0.0 3.0 1.0 smooth 1.0 0.1 ImEul 0.00001 400" << std::endl;
	mySStream << std::endl;
	mySStream << "Remark: This test generates following files (gnuplot):" << std::endl;
	mySStream << "	heatStart.gnuplot: the start condition" << std::endl;
	mySStream << "	heatSolved.gnuplot: the numerical solution" << std::endl;
	mySStream << std::endl << std::endl;

	mySStream << "PoissonEquation" << std::endl << "------" << std::endl;
	mySStream << "the following options must be specified:" << std::endl;
	mySStream << "	dim: the number of dimensions of Sparse Grid" << std::endl;
	mySStream << "	level: number of levels within the Sparse Grid" << std::endl;
	mySStream << "	left_bound: x_i of left boundary" << std::endl;
	mySStream << "	right_bound: x_i of right boundary" << std::endl;
	mySStream << "	initHeat: initial heat distribution" << std::endl;
	mySStream << "	CGEpsilon: Epsilon used in CG" << std::endl;
	mySStream << "	CGIterations: Maxmimum number of iterations used in CG mehtod" << std::endl;
	mySStream << std::endl;
	mySStream << "Example:" << std::endl;
	mySStream << "HESolver PoissonEquation 3 5 0.0 3.0 smooth 0.00001 400" << std::endl;
	mySStream << std::endl;
	mySStream << "Remark: This test generates following files (gnuplot):" << std::endl;
	mySStream << "	poissonStart.gnuplot: the start condition" << std::endl;
	mySStream << "	poissonSolved.gnuplot: the numerical solution" << std::endl;
	mySStream << std::endl << std::endl;

	std::cout << mySStream.str() << std::endl;
}

void testPoissonEquation(size_t dim, size_t level, double bound_left, double bound_right,
						std::string initFunc, double cg_eps, size_t cg_its)
{
	sg::PoissonEquationSolverMPI* myPoisSolver = new sg::PoissonEquationSolverMPI();
	DataVector* alpha = NULL;

	if (sg::myGlobalMPIComm->getMyRank() == 0)
	{
		sg::DimensionBoundary* myBoundaries = new sg::DimensionBoundary[dim];
		// set the bounding box
		for (size_t i = 0; i < dim; i++)
		{
			myBoundaries[i].leftBoundary = bound_left;
			myBoundaries[i].rightBoundary = bound_right;
			myBoundaries[i].bDirichletLeft = true;
			myBoundaries[i].bDirichletRight = true;
		}
		sg::BoundingBox* myBoundingBox = new sg::BoundingBox(dim, myBoundaries);
		delete[] myBoundaries;

		// init Screen Object
		myPoisSolver->initScreen();

		// Construct a grid
		myPoisSolver->constructGrid(*myBoundingBox, level);

		// init the basis functions' coefficient vector (start solution)
		alpha = new DataVector(myPoisSolver->getNumberGridPoints());
		if (initFunc == "smooth")
		{
			myPoisSolver->initGridWithSmoothHeat(*alpha, bound_right, bound_right/DIV_SIGMA, DISTRI_FACTOR);
		}
		else
		{
			if (sg::myGlobalMPIComm->getMyRank() == 0)
			{
				writeHelp();
			}
			sg::myGlobalMPIComm->Abort();
		}

		delete myBoundingBox;
	}

	// Communicate grid
	if (sg::myGlobalMPIComm->getMyRank() == 0)
	{
		std::string serialized_grid = myPoisSolver->getGrid();

		sg::myGlobalMPIComm->broadcastGrid(serialized_grid);
	}
	else
	{
		// Now receive the grid
		std::string serialized_grid = "";

		sg::myGlobalMPIComm->receiveGrid(serialized_grid);
		myPoisSolver->setGrid(serialized_grid);

		alpha = new DataVector(myPoisSolver->getNumberGridPoints());
	}

	sg::myGlobalMPIComm->Barrier();

	// Communicate coefficients
	if (sg::myGlobalMPIComm->getMyRank() == 0)
	{
		sg::myGlobalMPIComm->broadcastGridCoefficients(*alpha);
	}
	else
	{
		sg::myGlobalMPIComm->receiveGridCoefficients(*alpha);
	}

	sg::myGlobalMPIComm->Barrier();

	if (sg::myGlobalMPIComm->getMyRank() == 0)
	{
		// Print the initial heat function into a gnuplot file
		if (dim < 3)
		{
			myPoisSolver->printGrid(*alpha, GUNPLOT_RESOLUTION, "poissonStart.gnuplot");
		}
	}

	// solve Poisson Equation
	myPoisSolver->solvePDE(*alpha, *alpha, cg_its, cg_eps, true);

	if (sg::myGlobalMPIComm->getMyRank() == 0)
	{
		// Print the solved Heat Equation into a gnuplot file
		if (dim < 3)
		{
			myPoisSolver->printGrid(*alpha, GUNPLOT_RESOLUTION, "poissonSolved.gnuplot");
		}
	}

	delete myPoisSolver;
	delete alpha;
}

int main(int argc, char *argv[])
{
	std::string option;
	int mpi_myid;
	int mpi_ranks;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD,&mpi_ranks);
	MPI_Comm_rank(MPI_COMM_WORLD,&mpi_myid);
	sg::myGlobalMPIComm = new sg::MPICommunicator(mpi_myid, mpi_ranks);

	if (argc == 1)
	{
		if (mpi_myid == 0)
		{
			writeHelp();
		}
		sg::myGlobalMPIComm->Abort();
		return 0;
	}

	option.assign(argv[1]);

	if (option == "PoissonEquation")
	{
		if (argc != 9)
		{
			if (mpi_myid == 0)
			{
				writeHelp();
			}
			sg::myGlobalMPIComm->Abort();
			return 0;
		}

		size_t dim;
		size_t level;
		double bound_left;
		double bound_right;
		std::string initFunc;
		double cg_eps;
		size_t cg_its;

		dim = atoi(argv[2]);
		level = atoi(argv[3]);
		bound_left = atof(argv[4]);
		bound_right = atof(argv[5]);
		initFunc.assign(argv[6]);
		cg_eps = atof(argv[7]);
		cg_its = atoi(argv[8]);
		testPoissonEquation(dim, level, bound_left, bound_right, initFunc, cg_eps, cg_its);
	}
	else
	{
		if (mpi_myid == 0)
		{
			writeHelp();
		}
	}

	delete sg::myGlobalMPIComm;
	MPI_Finalize();
}
