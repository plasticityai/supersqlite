from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import argparse
import gc
import os
import sys
import tempfile
import unittest

from supersqlite import pysqlite, apsw, SQLITE_LIB, APSW_LIB, SuperSQLite


def _clear_mmap():
    os.system("rm -rf " + os.path.join(tempfile.gettempdir(), '*.supersqlmmap'))
    os.system(
        "rm -rf " +
        os.path.join(
            tempfile.gettempdir(),
            '*.supersqlmmap*'))


class MagnitudeTest(unittest.TestCase):

    def setUp(self):
        pass

    def tearDown(self):
        pass
        gc.collect()

    def test_sqlite_lib(self):
        self.assertEqual(SQLITE_LIB, 'internal')

    def test_apsw_lib(self):
        self.assertEqual(APSW_LIB, 'internal')

    def test_pysqlite(self):
        self.assertTrue(pysqlite)

    def test_apsw(self):
        self.assertTrue(apsw)

    def test_ssqlite_subclass(self):
        self.assertTrue(issubclass(SuperSQLite, apsw))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('unittest_args', nargs='*')
    args = parser.parse_args()
    _clear_mmap()
    unittest.main(argv=[sys.argv[0]] + args.unittest_args)
