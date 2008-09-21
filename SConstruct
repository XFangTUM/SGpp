# This file is part of sg++, a program package making use of spatially adaptive sparse grids to solve numerical problems.
#
# Copyright (C) 2007  Joerg Blank (blankj@in.tum.de)
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public License
# along with pyclass. If not, see <http://www.gnu.org/licenses/>.
#


import os
import distutils.sysconfig

opts = Options('custom.py')

opts.Add('CPPFLAGS','Set additional Flags','')
opts.Add('LINKFLAGS','Set additional Linker-flags','')

opts.Add('MARCH','Set processor specific MARCH',None)

opts.Add('ICC', 'Uses Intels Optimizing Compiler', False)

opts.Add('JSGPP', 'Build jsgpp if set to True', False)
opts.Add('JNI_CPPPATH', 'Path to JNI includes', None)
opts.Add('JNI_LIBPATH', 'Path to JNI libs', None)
opts.Add('JNI_OS', 'JNI os path', None)

env = Environment(options = opts, ENV = os.environ)

env.Append(CPPFLAGS=['-pthread'])
env.Append(LINKFLAGS=['-pthread'])

if not env['ICC'] and env.has_key('MARCH'):
	env.Append(CPPFLAGS=('-march=' + env['MARCH']))

if not env.GetOption('clean'):	
    config = env.Configure()
	
    if env['ICC']:
        if not config.CheckLib('guide'):
            Exit(1)

    if not config.CheckLibWithHeader('m', 'math.h', 'c++'):
        Exit(1)

    if env['ICC']:
        if not config.CheckLib('svml'):
            print "SVML should be available when using intelc. Consider runnning scons --config=force!"

    env = config.Finish()


python_lib='python2.4'

env.Append(PYTHON_LIBPATH=[distutils.sysconfig.PREFIX+"/libs"])
env.Append(PYTHON_LIB=[python_lib])
env.Append(PYTHON_CPPPATH=[distutils.sysconfig.get_python_inc()])

Export('env')

SConscript('src/SConscript', build_dir='build_sg', duplicate=0)
SConscript('pysgpp/SConscript', build_dir='build_pysgpp', duplicate=0)

if env['JSGPP']:
	SConscript('jsgpp/SConscript', build_dir='build_jsgpp', duplicate=0)

SConscript('tests/SConscript')

cpy = []
cpy += Command("#bin/_pysgpp.so", "#build_pysgpp/_pysgpp.so", Copy("$TARGET", "$SOURCE"))
cpy += Command("#bin/pysgpp.py", "#build_pysgpp/pysgpp.py", Copy("$TARGET", "$SOURCE"))

Help(opts.GenerateHelpText(env))
