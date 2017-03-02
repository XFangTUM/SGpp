// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#ifdef USE_GSL

#ifndef DBMATOFFLINE_H_
#define DBMATOFFLINE_H_

#include <sgpp/base/grid/Grid.hpp>
#include <sgpp/datadriven/algorithm/DBMatDensityConfiguration.hpp>

#include <gsl/gsl_permutation.h>

#include <list>
#include <string>
#include <vector>

namespace sgpp {
namespace datadriven {

using sgpp::base::Grid;
using sgpp::base::DataVector;
using sgpp::base::DataMatrix;

/**
 * Class that is used to decompose and store the left-hand-side
 * matrix for the density based classification approach
 * (The classification is divided into two parts: the offline step that does not
 * depend on the actual data and the online step that depends on the data).
 * Uses Gnu Scientific Library (GSL).
 */

class DBMatOffline {
 public:
  /**
   * Constructor
   *
   * @param oc configuration for this offline object
   */
  explicit DBMatOffline(DBMatDensityConfiguration& oc);
  /**
   * Constructor
   *
   * @param fname name of the file that stores the matrix + configuration
   */
  explicit DBMatOffline(const std::string& fname);

  /**
   * Copy Constructor
   *
   *
   * The matrix needs to be already decomposed.
   *
   * @param old Object to copy
   *
   *TODO(lettrich): there is a bug. grid objects cannot be copied, so refined grids will not be
   *copied properly.
   */
  DBMatOffline(const DBMatOffline& old);

  /**
   * Returns a pointer to the configuration
   */
  DBMatDensityConfiguration& getConfig();

  /**
   * Returns a pointer to the decomposed matrix
   */
  DataMatrix& getDecomposedMatrix();

  /**
   * Returns a reference to the sparse grid
   * (if this offline object uses a sparse grid, otherwise NULL)
   */
  Grid& getGrid();

  /**
   * Builds the right hand side matrix with or without the regularization term
   * depending
   * on the type of decomposition
   */
  void buildMatrix();

  /**
   * Decomposes the matrix according to the chosen decomposition type.
   * The number of rows of the stored result depends on the decomposition type.
   */
  void decomposeMatrix();

  /**
   * Applies the permutation that might be done during the matrix decomposition
   * to a vector
   * (has to be applied to the right hand side vector if the system of equation
   * should be solved with a decomposed matrix)
   *
   * @param b the vector that has to be permuted
   */
  void permuteVector(DataVector& b);

  /**
   * Updates offline cholesky factorization based on coarsed (deletedPoints)
   * and refined (newPoints) gridPoints
   *
   * @param deletedPoints list of indices of last coarsed points
   * @param newPoints amount of refined points
   */
  void choleskyModification(size_t newPoints, std::list<size_t> deletedPoints, double lambda);

  /**
   * Updates the cholesky factor when a new grid point is added (e.g. refine)
   *
   * @param newCol DataVector with column to add to the system matrix
   * @param size columns/rows of current Cholesky factor, necessary since the
            allocated memory is increased before the Cholesky factor is modified
   */
  void choleskyAddPoint(DataVector* newCol, size_t size);

  /**
   * Permutes the rows of the cholesky factor based on permutations
   * of the system matrix (e.g. coarsening)
   *
   * @param k "left" column to permutate
   * @param l "right" column to permutate
   * @param job = 2        => left circular shift
   *	  1,...,k-1,k,k+1, ..., l-1,l,l+1, ..,size  => 1,...,k-1,k+1, ...,
   *l-1,l,k,l+1,..., size
   * 	  job = 1       => right circular shift
   * 	  1,...,k-1,k,k+1, ..., l-1,l,l+1,...size  => 1,...,k-1,l,k,k+1, ...,
   *l-1,l+1,...size
   */
  void choleskyPermutation(size_t k, size_t l, size_t job);

  /**
   * Prints the matrix onto standard output
   */
  void printMatrix();

  /**
   * Stores the matrix + configuration
   *
   * @param fname the file name
   */
  void store(const std::string& fname);

  /**
   * Loads matrix and configuration from a file
   *
   * @param fname the file name
   */
  /*void load(std::string fname);*/

  /**
   * Destructor
   */
  virtual ~DBMatOffline();

 protected:
  DBMatOffline();

  DBMatDensityConfiguration config;  // configuration for this offline object
  DataMatrix lhsMatrix;              // stores the (decomposed) matrix
  bool isConstructed;                // If the matrix was built
  bool isDecomposed;                 // If the matrix was decomposed

  gsl_permutation* permutation;  // Stores the permutation that was
  // applied on the matrix during decomposition

  // An offline object either works on a
  // hierarchical basis grid!
  Grid* grid;

  /**
   * Method to initialize a sparse grid
   */
  void InitializeGrid();

 private:
  /**
   * Used for reading input files
   */
  void Tokenize(std::string&, std::vector<std::string>&, std::string& delimiters);
};

}  // namespace datadriven
}  // namespace sgpp

#endif /* DBMATOFFLINE_H_ */

#endif /* USE_GSL */
