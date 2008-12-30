# -*- python -*-
# $Header: /nfs/slac/g/glast/ground/cvs/embed_python/SConscript,v 1.7 2008/10/30 14:30:49 glastrm Exp $
# Authors: T. Burnett <tburnett@u.washington.edu>
# Version: embed_python-01-05-02

Import('baseEnv')
Import('listFiles')
progEnv = baseEnv.Clone()
libEnv = baseEnv.Clone()

libEnv.Tool('embed_pythonLib', depsOnly = 1)
embed_pythonLib = libEnv.StaticLibrary('embed_python', listFiles(['src/*.cxx']))

progEnv.Tool('embed_pythonLib')
test_embed_pythonBin = progEnv.Program('test_embed_python', listFiles(['src/test/*.cxx']))

progEnv.Tool('registerObjects', package = 'embed_python', libraries = [embed_pythonLib], testApps = [test_embed_pythonBin], includes = listFiles(['embed_python/*.h']))
