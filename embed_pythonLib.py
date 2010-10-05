#$Id: embed_pythonLib.py,v 1.2 2008/02/26 03:06:47 glastrm Exp $
def generate(env, **kw):
    if not kw.get('depsOnly',0):
        env.Tool('addLibrary', library = ['embed_python'])
	if env['PLATFORM'] == 'posix':
            env.AppendUnique(LINKFLAGS	= ['-rdynamic'])	
    env.Tool('addLibrary', library = env['pythonLibs'])

def exists(env):
    return 1
