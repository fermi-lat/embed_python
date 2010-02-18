# -*- python -*-
# $Header: /nfs/slac/g/glast/ground/cvs/ScienceTools-scons/embed_python/SConscript,v 1.10 2009/07/15 17:35:27 glastrm Exp $
# Authors: T. Burnett <tburnett@u.washington.edu>
# Version: embed_python-01-05-03

Import('baseEnv')
Import('listFiles')
progEnv = baseEnv.Clone()
libEnv = baseEnv.Clone()

embed_pythonLib = libEnv.StaticLibrary('embed_python', listFiles(['src/*.cxx']))

progEnv.Tool('embed_pythonLib')
test_embed_pythonBin = progEnv.Program('test_embed_python', listFiles(['src/test/*.cxx']))


progEnv.Tool('registerTargets', package = 'embed_python',
             staticLibraryCxts = [[embed_pythonLib, libEnv]],
             testAppCxts = [[test_embed_pythonBin, progEnv]],
             includes = listFiles(['embed_python/*.h']),
             python = ['python/embed_python_Test.py'])
