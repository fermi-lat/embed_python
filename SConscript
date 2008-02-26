#$Id$
Import('baseEnv')
Import('listFiles')
progEnv = baseEnv.Clone()
libEnv = baseEnv.Clone()

libEnv.Tool('embed_pythonLib', depsOnly = 1)
embed_pythonLib = libEnv.StaticLibrary('embed_python', listFiles(['src/*.cxx']))

progEnv.Tool('embed_pythonLib')
test_embed_pythonBin = progEnv.Program('test_embed_python', listFiles(['src/test/*.cxx']))

progEnv.Tool('registerObjects', package = 'embed_python', libraries = [embed_pythonLib], testApps = [test_embed_pythonBin], includes = listFiles(['embed_python/*.h']))
