# Copyright (C) 2008-today The SG++ project
# This file is part of the SG++ project. For conditions of distribution and
# use, please see the copyright notice provided with SG++ or at
# sgpp.sparsegrids.org

import unittest

#correct the syspath, so python.ooks for packages in the root directory of SGpp
import sys, os
pathname = os.path.dirname(__file__)
pathlocal = os.path.abspath(pathname)
if pathlocal not in sys.path: sys.path.append(pathlocal)
pathsgpp = os.path.abspath(pathname) + '/../../..'
if pathsgpp not in sys.path: sys.path.append(pathsgpp)

from pysgpp.extensions.datadriven.learner.formatter.GridFormatter import GridFormatter
from pysgpp import Grid


##
# @package tests.tbin.test_GridFormatter
# Contains class test_GridFormatter::TestGridFormatter with unittests for @link python.learner.formatter.GridFormatter.GridFormatter GridFormatter @endlink

##
# Class with unittests for @link python.learner.formatter.GridFormatter.GridFormatter GridFormatter @endlink
#
# @ingroup tests
#
# @test Unittests for @link python.learner.formatter.GridFormatter.GridFormatter GridFormatter @endlink
class TestGridFormatter(unittest.TestCase):


    ## Set up the variables
    def setUp(self,):
        self.__gridFormatter = None
        self.filename = pathlocal + "/datasets/grid.gz"
        self.savefile = pathlocal + "/datasets/savetest.grid.gz"
        self.correct_str = ""
        self.grid = None

        self.__gridFormatter = GridFormatter()
        dim = 3
        self.grid = Grid.createLinearGrid(dim)
        self.grid.getGenerator().regular(3)
        self.correct_str = self.grid.serialize()


    ##
    # Tests the functions @link python.learner.formatter.GridFormatter.GridFormatter.serialize() GridFormatter.serialize() @endlink
    # and  @link python.learner.formatter.GridFormatter.GridFormatter.deserializeFromFile() GridFormatter.deserializeFromFile() @endlink
    def testLoad(self,):
        grid = self.__gridFormatter.deserializeFromFile(self.filename)
        test_str = grid.serialize()
        self.assertEqual(test_str, self.correct_str)


    ##
    # Tests the functions @link python.learner.formatter.GridFormatter.GridFormatter.serializeToFile() GridFormatter.serializeToFile() @endlink
    # and  @link python.learner.formatter.GridFormatter.GridFormatter.deserializeFromFile() GridFormatter.deserializeFromFile() @endlink
    def testSave(self,):
        self.__gridFormatter.serializeToFile(self.grid, self.savefile)
        grid = self.__gridFormatter.deserializeFromFile(self.savefile)
        test_str = grid.serialize()
        self.assertEqual(test_str, self.correct_str)

        os.remove(self.savefile)



if __name__=="__main__":
    unittest.main()
