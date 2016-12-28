
import os
import subprocess
import SCons
import glob
import re
import sys
from inspect import currentframe, getframeinfo

EXT_SUFFIX = 0

#===========================================================
# The first 4 functions provide for building a library,
# program, multiple-programs, or plugin from all the source
# in the current directory.
#
# The next section contains useful utility functions.
#
# The functions that follow in the final section add support
# for various packages (e.g. ROOT, Xerces, ...)
#===========================================================


##################################
# library
##################################
def library(env, libname=''):

	# Library name comes from directory name
	if libname=='':
		libname = os.path.split(os.getcwd())[1]

	env.PrependUnique(CPPPATH = ['.'])

	# Add C/C++, and FORTRAN targets
	env.AppendUnique(ALL_SOURCES = env.Glob('*.c'))
	env.AppendUnique(ALL_SOURCES = env.Glob('*.cc'))
	env.AppendUnique(ALL_SOURCES = env.Glob('*.cpp'))
	env.AppendUnique(ALL_SOURCES = env.Glob('*.F'))

	sources = env['ALL_SOURCES']
	objects = env['MISC_OBJECTS']
	if 'IGNORE_SOURCES' in env.Dictionary().keys():
		ignore  = env['IGNORE_SOURCES']
		sources = [s for s in env['ALL_SOURCES'] if s.name not in ignore]

	# Build static library from all source
	myobjs = env.Object(sources)
	myobjs.extend(objects)
	mylib = env.Library(target = libname, source = myobjs)

	# Cleaning and installation are restricted to the directory
	# scons was launched from or its descendents
	CurrentDir = env.Dir('.').srcnode().abspath
	if not CurrentDir.startswith(env.GetLaunchDir()):
		# Not in launch directory. Tell scons not to clean these targets
		env.NoClean([myobjs, mylib])
	else:
		# We're in launch directory (or descendent) schedule installation

		# Installation directories for library and headers
		installdir = env.subst('$INSTALLDIR')
		includedir = "%s/%s" %(env.subst('$INCDIR'), libname)
		libdir = env.subst('$LIBDIR')

		# Install targets
		env.Install(libdir, mylib)
		env.Install(includedir, env.Glob('*.h')+env.Glob('*.hh')+env.Glob('*.hpp'))


##################################
# executable
##################################
def executable(env, exename=''):

	# Executable name comes from directory name
	if exename=='':
		exename = os.path.split(os.getcwd())[1]

	env.PrependUnique(CPPPATH = ['.'])

	# Add C/C++, and FORTRAN targets
	env.AppendUnique(ALL_SOURCES = env.Glob('*.c'))
	env.AppendUnique(ALL_SOURCES = env.Glob('*.cc'))
	env.AppendUnique(ALL_SOURCES = env.Glob('*.cpp'))
	env.AppendUnique(ALL_SOURCES = env.Glob('*.F'))

	# Push commonly used libraries to end of list
	ReorderCommonLibraries(env)

	sources = env['ALL_SOURCES']
	if 'IGNORE_SOURCES' in env.Dictionary().keys():
		ignore  = env['IGNORE_SOURCES']
		sources = [s for s in env['ALL_SOURCES'] if s.name not in ignore]

	# Build program from all source
	myobjs = env.Object(sources)
	myexe = env.Program(target = exename, source = myobjs)

	# Cleaning and installation are restricted to the directory
	# scons was launched from or its descendents
	CurrentDir = env.Dir('.').srcnode().abspath
	if not CurrentDir.startswith(env.GetLaunchDir()):
		# Not in launch directory. Tell scons not to clean these targets
		env.NoClean([myobjs, myexe])
	else:
		# We're in launch directory (or descendent) schedule installation

		# Installation directories for executable and headers
		installdir = env.subst('$INSTALLDIR')
		includedir = env.subst('$INCDIR')
		bindir = env.subst('$BINDIR')

		# Install targets
		env.Install(bindir, myexe)


##################################
# python_so_module
##################################
def python_so_module(env, modname):

	# This will build a single python extension module
	# from c/c++ sources as a shared library (.so) and
	# install it either in $INSTALLDIR/python2 or else
	# $INSTALLDIR/python3 depending on which version of
	# python the user environment has installed.

	# Build the module as a shared library
	# using the distutils setup.py mechanism.
	if env['SHOWBUILD']==0:
		setup_py_action = SCons.Script.Action('python $SOURCE build -b ' +
											  'build_' +  modname +
											  ' > /dev/null',
											  'PYMODBUILD [$SOURCE]')
	else:
		setup_py_action = SCons.Script.Action('python $SOURCE build -b ' +
											  'build_' +  modname)
	setup_py_builder = SCons.Script.Builder(action = setup_py_action)
	env.Append(BUILDERS = {'PYMODBUILD' : setup_py_builder})

	# Look up EXT_SUFFIX in the distutils config variables.
	# Unfortunately scons erases the system-set python path,
	# so I cannot import distutils directly here.
	# Spawn another python shell for that.
	global EXT_SUFFIX
	if EXT_SUFFIX == 0:
		req = subprocess.Popen(['python', '-c',
						'from distutils import sysconfig;' +
						'print(sysconfig.get_config_var("EXT_SUFFIX"))'],
						stdout=subprocess.PIPE)
		EXT_SUFFIX = req.communicate()[0].rstrip()
	if EXT_SUFFIX == "None":
		mymod = modname + '.so'
		moduledir = env.subst('$PYTHON2DIR')
	else:
		mymod = modname + EXT_SUFFIX
		moduledir = env.subst('$PYTHON3DIR')
	modsource = 'setup_' + modname + '.py'
	modlib = env.PYMODBUILD(mymod, modsource)

	# Add dependencies of the shared module on the setup.py and c++ sources
	env.Depends([mymod], [modsource, "py" + modname + ".cpy"])

	# Cleaning and installation are restricted to the directory
	# scons was launched from or its descendents
	CurrentDir = env.Dir('.').srcnode().abspath
	if not CurrentDir.startswith(env.GetLaunchDir()):
		# Not in launch directory. Tell scons not to clean these targets
		env.NoClean([modlib])
	else:
		# We're in launch directory (or descendent) schedule installation

		# Install targets
		env.Install(moduledir, modlib)


##################################
# executables
##################################
def executables(env):

	# This will generate multiple executables from the
	# source in the current directory. It does this
	# by identifying source files that define "main()"
	# and linking those with all source files that do not
	# define "main()". Program names are based on the
	# filename of the source file defining "main()"
	main_sources = []
	common_sources = []
	curpath = os.getcwd()
	srcpath = env.Dir('.').srcnode().abspath
	os.chdir(srcpath)
	files = glob.glob('*.c') + glob.glob('*.cc') + glob.glob('*.cpp')
	for f in files:
		if 'main(' in open(f).read():
			main_sources.append(f)
		else:
			common_sources.append(f)

	for f in glob.glob('*.F'):
		if '      PROGRAM ' in open(f).read():
			main_sources.append(f)
		else:
			common_sources.append(f)
	os.chdir(curpath)

	env.PrependUnique(CPPPATH = ['.'])

	# Push commonly used libraries to end of list
	ReorderCommonLibraries(env)

	common_sources.extend(env['ALL_SOURCES'])

	if 'IGNORE_SOURCES' in env.Dictionary().keys():
		ignore  = env['IGNORE_SOURCES']
		main_sources   = [s for s in main_sources   if s.name not in ignore]
		common_sources = [s for s in common_sources if s.name not in ignore]

	# Build program from all source
	main_objs = env.Object(main_sources)
	common_objs = env.Object(common_sources)

	progs = []
	for obj in main_objs:
		exename = re.sub('\.o$', '', str(obj))  # strip off ".o" from object file name
		progs.append(env.Program(target = exename, source = [obj, common_objs]))

	# Cleaning and installation are restricted to the directory
	# scons was launched from or its descendents
	CurrentDir = env.Dir('.').srcnode().abspath
	if not CurrentDir.startswith(env.GetLaunchDir()):
		# Not in launch directory. Tell scons not to clean these targets
		env.NoClean([common_objs, main_objs, progs])
	else:
		# We're in launch directory (or descendent) schedule installation
		bindir = env.subst('$BINDIR')
		env.Install(bindir, progs)


##################################
# plugin
##################################
def plugin(env, pluginname=''):

	# Library name comes from directory name
	if pluginname=='':
		pluginname = os.path.split(os.getcwd())[1]

	env.PrependUnique(CPPPATH = ['.'])

	# Add C/C++ targets
	env.AppendUnique(ALL_SOURCES = env.Glob('*.c'))
	env.AppendUnique(ALL_SOURCES = env.Glob('*.cc'))
	env.AppendUnique(ALL_SOURCES = env.Glob('*.cpp'))
	env.AppendUnique(ALL_SOURCES = env.Glob('*.F'))

        #print str([x.rstr() for x in env.Glob('*.cc')])

	sources = env['ALL_SOURCES']
	if 'IGNORE_SOURCES' in env.Dictionary().keys():
		ignore  = env['IGNORE_SOURCES']
		sources = [s for s in env['ALL_SOURCES'] if s.name not in ignore]

	# Build static library from all source
	myobjs = env.SharedObject(sources)
	myplugin = env.SharedLibrary(target = pluginname, source = myobjs, SHLIBPREFIX='', SHLIBSUFFIX='.so')

	# Cleaning and installation are restricted to the directory
	# scons was launched from or its descendents
	CurrentDir = env.Dir('.').srcnode().abspath
	if not CurrentDir.startswith(env.GetLaunchDir()):
		# Not in launch directory. Tell scons not to clean these targets
		env.NoClean([myobjs, myplugin])
	else:
		# We're in launch directory (or descendent) schedule installation

		# Installation directories for plugin and headers
		installdir = env.subst('$INSTALLDIR')
		includedir = "%s/%s" %(env.subst('$INCDIR'), pluginname)
		pluginsdir = env.subst('$PLUGINSDIR')

		# Install targets
		installed = env.Install(pluginsdir, myplugin)
		env.Install(includedir, env.Glob('*.h')+env.Glob('*.hh')+env.Glob('*.hpp'))


#===========================================================
# Misc utility routines for the SBMS system
#===========================================================

##################################
# AddCompileFlags
##################################
def AddCompileFlags(env, allflags):

	# The allflags parameter should be a string containing all
	# of the link flags (e.g. what is returned by root-config --cflags)
	# It is split on white space and the parameters sorted into
	# the 2 lists: ccflags, cpppath

	ccflags = []
	cxxflags = []
	cpppath = []
	for f in allflags.split():
		if f.startswith('-I'):
			cpppath.append(f[2:])
		else:
			if f == '-g': continue
			if f.startswith('-std=c++'):
				cxxflags.append(f)
			else:
				ccflags.append(f)

	if len(ccflags)>0 :
		env.AppendUnique(CCFLAGS=ccflags)

	if len(cxxflags)>0 :
		env.AppendUnique(CXXFLAGS=cxxflags)

	if len(cpppath)>0 :
		env.AppendUnique(CPPPATH=cpppath)


##################################
# AddLinkFlags
##################################
def AddLinkFlags(env, allflags):

	# The allflags parameter should be a string containing all
	# of the link flags (e.g. what is returned by root-config --glibs)
	# It is split on white space and the parameters sorted into
	# the 3 lists: linkflags, libpath, and libs

	linkflags = []
	libpath = []
	libs = []
	for f in allflags.split():
		if f.startswith('-L'):
			libpath.append(f[2:])
		elif f.startswith('-l'):
			libs.append(f[2:])
		else:
			linkflags.append(f)

	if len(linkflags)>0 :
		env.AppendUnique(LINKFLAGS=linkflags)

	if len(libpath)>0 :
		env.AppendUnique(LIBPATH=libpath)

	if len(libs)>0 :
		env.AppendUnique(LIBS=libs)


##################################
# ReorderCommonLibraries
##################################
def ReorderCommonLibraries(env):

	# Some common libraries are often added by multiple packages
	# (e.g. libz is one that many packages use). The gcc4.8.0
	# compiler that comes with Ubuntu13.10 seems particularly
	# sensitive to the ordering of the libraries. This means if
	# one package "AppendUnique"s the "z" library, it may appear
	# too early in the link command for another library that needs
	# it, even though the second library tries appending it at the
	# end. This routine looks for some commonly used libraries
	# in the LIBS variable of the given environment and moves them
	# to the end of the list.

	# If LIBS is not defined in this environment, return now
	if 'LIBS' not in env.Dictionary(): return

	# If LIBS is not set or is a simple string, return now
	if type(env['LIBS']) is not list: return

	libs = ['z', 'bz2', 'pthread', 'm', 'dl']
	for lib in libs:
		if lib in env['LIBS']:
			env['LIBS'].remove(lib)
			env.Append(LIBS=[lib])


##################################
# ApplyPlatformSpecificSettings
##################################
def ApplyPlatformSpecificSettings(env, platform):

	# Look for SBMS file based on this platform and run the InitENV
	# function in it to allow for platform-specific settings. Normally,
	# the BMS_OSNAME will be passed in which almost certainly contains
	# "."s. The Python module loader doesn't like these and we have to
	# replace them with "-"s to appease it.

	platform = re.sub('\.', '-', platform)

	modname = "sbms_%s" % platform
	if (int(env['SHOWBUILD']) > 0):
		print "looking for %s.py" % modname
	try:
		InitENV = getattr(__import__(modname), "InitENV")

		# Run the InitENV function (if found)
		if(InitENV != None):
			print "sbms : Applying settings for platform %s" % platform
			InitENV(env)

	except ImportError,e:
		if (int(env['SHOWBUILD']) > 0): print "%s" % e
		pass



##################################
# OptionallyBuild
##################################
def OptionallyBuild(env, dirs):

	# This is used to add directories that are not built as
	# part of the standard build, but can still be added
	# to the dependency tree so that the user can build them
	# by either invoking scons from within the specific source
	# directory or by specifying it on the command line.
	#
	#

	subdirs = []
	for dir in dirs:
		add_dir = False
		if env.GetLaunchDir().endswith(dir): add_dir = True
		#if dir in env['COMMAND_LINE_TARGETS']: add_dir = True
		for target in env['COMMAND_LINE_TARGETS']:
			if target.endswith(dir): add_dir = True

		if add_dir : subdirs.extend([dir])

	if len(subdirs)>0 : env.SConscript(dirs=subdirs, exports='env', duplicate=0)

##################################
# ProgramExists
##################################
def ProgramExists(program):
	# this checks the PATH environmental variable to see if a program exists
	# it works like the "which" command in UNIX
	def is_exe(fpath):
		return os.path.isfile(fpath) and os.access(fpath, os.X_OK)

	fpath, fname = os.path.split(program)
	if fpath:
		if is_exe(program):
			return program
	else:
		for path in os.environ["PATH"].split(os.pathsep):
			path = path.strip('"')
			exe_file = os.path.join(path, program)
			if is_exe(exe_file):
				return exe_file

	return None


##################################
# IsNotSWIGWrapper
##################################
def IsNotSWIGWrapper(fileNode):
	# this function takes an input of type SCons.Node.FS.File
	# i.e., what the Environment.Glob() function returns
	f = open(fileNode.rstr(),'r')
	f.readline()  ## the first line is blank
	# check to see if the first line of the auto-generated file has some magic
	# let's check to see if it contains the string "SWIG"
	match = re.search( r'SWIG', f.readline() )
	f.close()
	if match:
		return False
	else:
		return True

#===========================================================
# Package support follows
#===========================================================


##################################
# JANA
##################################
def AddJANA(env):
	jana_home = os.getenv('JANA_HOME', '/no/default/jana/path')

	# Only run jana-config the first time through
	if "JANA_CFLAGS" not in AddJANA.__dict__:
		AddJANA.JANA_CFLAGS = subprocess.Popen(["%s/bin/jana-config" % jana_home,"--jana-only","--cflags"], stdout=subprocess.PIPE).communicate()[0]
		AddJANA.JANA_LINKFLAGS = subprocess.Popen(["%s/bin/jana-config" % jana_home,"--jana-only","--libs"], stdout=subprocess.PIPE).communicate()[0]

	AddCompileFlags(env, AddJANA.JANA_CFLAGS)
	AddLinkFlags(env, AddJANA.JANA_LINKFLAGS)


##################################
# HDDS
##################################
def AddHDDS(env):
	hdds_home = os.getenv('HDDS_HOME', 'hdds')
	env.AppendUnique(CPPPATH = ["%s/%s/src" % (hdds_home, env['OSNAME'])])
	env.AppendUnique(LIBPATH = ["%s/%s/lib" % (hdds_home, env['OSNAME'])])


##################################
# HDDM
##################################
def AddHDDM(env):
	env.AppendUnique(LIBS = 'HDDM')
	Add_xstream(env)

##################################
# MYSQL
##################################
def AddMySQL(env):

	# Only run mysql_config the first time through
	if "MYSQL_CFLAGS" not in AddMySQL.__dict__:
		AddMySQL.MYSQL_CFLAGS = subprocess.Popen(["mysql_config","--cflags"], stdout=subprocess.PIPE).communicate()[0]
		AddMySQL.MYSQL_LINKFLAGS = subprocess.Popen(["mysql_config","--libs"], stdout=subprocess.PIPE).communicate()[0]
	AddCompileFlags(env, AddMySQL.MYSQL_CFLAGS)
	AddLinkFlags(env, AddMySQL.MYSQL_LINKFLAGS)

##################################
# DANA
##################################
def AddJLEIC(env):
	AddROOT(env)
	AddJANA(env)
	JLEIC_LIBS  = "JLEIC_MC"
	#JLEIC_LIBS += " expat"
	env.PrependUnique(LIBS = JLEIC_LIBS.split())


##################################
# CCDB
##################################
def AddCCDB(env):
	ccdb_home = os.getenv('CCDB_HOME')
	if(ccdb_home != None) :
		env.AppendUnique(CXXFLAGS = ['-DHAVE_CCDB'])
		CCDB_CPPPATH = "%s/include" % (ccdb_home)
		CCDB_LIBPATH = "%s/lib" % (ccdb_home)
		CCDB_LIBS = "ccdb"
		env.AppendUnique(CPPPATH = CCDB_CPPPATH)
		env.AppendUnique(LIBPATH = CCDB_LIBPATH)
		env.AppendUnique(LIBS    = CCDB_LIBS)


##################################
# RCDB
##################################
def AddRCDB(env):
	rcdb_home = os.getenv('RCDB_HOME')
	if(rcdb_home != None) :
		env.AppendUnique(CXXFLAGS = ['-DHAVE_RCDB'])
		RCDB_CPPPATH = ["%s/cpp/include" % (rcdb_home), "%s/cpp/include/SQLite" % (rcdb_home)]
		RCDB_LIBPATH = "%s/cpp/lib" % (rcdb_home)
		RCDB_LIBS = ["rcdb"]
		env.AppendUnique(CPPPATH = RCDB_CPPPATH)
		env.AppendUnique(LIBPATH = RCDB_LIBPATH)
		env.AppendUnique(LIBS    = RCDB_LIBS)
		AddMySQL(env)


##################################
# EVIO
##################################
def AddEVIO(env):

	# Only add EVIO if EVIOROOT is set
	evioroot = os.getenv('EVIOROOT')
	if(evioroot != None) :
		env.AppendUnique(CXXFLAGS = ['-DHAVE_EVIO'])
		env.AppendUnique(CPPPATH = ['%s/include' % evioroot])
		env.AppendUnique(LIBPATH = ['%s/lib' % evioroot])
		env.AppendUnique(LIBS=['evioxx', 'evio', 'expat'])
		AddET(env)


##################################
# ET
##################################
def AddET(env):

	# Only add ET if ETROOT is set
	etroot = os.getenv('ETROOT')
	if(etroot != None) :
		env.AppendUnique(CXXFLAGS = ['-DHAVE_ET'])
		env.AppendUnique(CPPPATH = ['%s/include' % etroot])
		env.AppendUnique(LIBPATH = ['%s/lib' % etroot])
		env.AppendUnique(LIBS=['et_remote', 'et'])


##################################
# Xerces
##################################
def AddXERCES(env):
	xercescroot = os.getenv('XERCESCROOT', 'xerces')
	XERCES_CPPPATH = "%s/include" % (xercescroot)
	XERCES_LIBPATH = "%s/lib" % (xercescroot)
	XERCES_LIBS = "xerces-c"
	env.AppendUnique(CPPPATH = XERCES_CPPPATH)
	env.AppendUnique(LIBPATH = XERCES_LIBPATH)
	env.AppendUnique(LIBS    = XERCES_LIBS)


##################################
# ROOT
##################################
def AddROOT(env):
	#
	# Here we use the root-config program to give us the compiler
	# and linker options needed for ROOT. We use the AddCompileFlags()
	# and AddLinkFlags() routines (defined above) to split the arguments
	# into the categories scons wants. E.g. scons wants to know the
	# search path and basenames for libraries rather than just giving it
	# the full compiler options like "-L/path/to/lib -lmylib".
	#
	# We also create a builder for ROOT dictionaries and add targets to
	# build dictionaries for any headers with "ClassDef" in them.

	rootsys = os.getenv('ROOTSYS', '/usr/local/root/PRO')
	if not os.path.isdir(rootsys):
		print 'ROOTSYS not defined or points to a non-existant directory!'
		sys.exit(-1)

	print
	# Only root-config the first time through
	if "ROOT_CFLAGS" not in AddROOT.__dict__:
		AddROOT.ROOT_CFLAGS    = subprocess.Popen(["%s/bin/root-config" % rootsys, "--cflags"], stdout=subprocess.PIPE).communicate()[0]
		AddROOT.ROOT_LINKFLAGS = subprocess.Popen(["%s/bin/root-config" % rootsys, "--glibs" ], stdout=subprocess.PIPE).communicate()[0]

	AddCompileFlags(env, AddROOT.ROOT_CFLAGS)
	AddLinkFlags(env, AddROOT.ROOT_LINKFLAGS)
	env.AppendUnique(LIBS = "Geom")
	if os.getenv('LD_LIBRARY_PATH'  ) != None : env.Append(LD_LIBRARY_PATH   = os.environ['LD_LIBRARY_PATH'  ])
	if os.getenv('DYLD_LIBRARY_PATH') != None : env.Append(DYLD_LIBRARY_PATH = os.environ['DYLD_LIBRARY_PATH'])

	# NOTE on (DY)LD_LIBRARY_PATH :
	# Linux (and most unixes) use LD_LIBRARY_PATH while Mac OS X uses
	# DYLD_LIBRARY_PATH. Unfortunately, the "thisroot.csh" script distributed
	# with ROOT sets both of these so we can't use the presence of the
	# DYLD_LIBRARY_PATH environment variable to decide which of these to
	# work with. Thus, we just append to whichever are set, which may be both.

	# Create Builder that can convert .h file into _Dict.cc file
	if os.getenv('LD_LIBRARY_PATH'  ) != None : env.AppendENVPath('LD_LIBRARY_PATH'  , '%s/lib' % rootsys )
	if os.getenv('DYLD_LIBRARY_PATH') != None : env.AppendENVPath('DYLD_LIBRARY_PATH', '%s/lib' % rootsys )
	rootcintpath  = "%s/bin/rootcint" % (rootsys)
	rootclingpath = "%s/bin/rootcling" % (rootsys)
	if env['SHOWBUILD']==0:
		rootcintaction  = SCons.Script.Action("%s -f $TARGET -c $SOURCE" % (rootcintpath) , 'ROOTCINT   [$SOURCE]')
		rootclingaction = SCons.Script.Action("%s -f $TARGET -c $SOURCE" % (rootclingpath), 'ROOTCLING  [$SOURCE]')
	else:
		rootcintaction  = SCons.Script.Action("%s -f $TARGET -c $SOURCE" % (rootcintpath) )
		rootclingaction = SCons.Script.Action("%s -f $TARGET -c $SOURCE" % (rootclingpath))
	if os.path.exists(rootclingpath) :
		bld = SCons.Script.Builder(action = rootclingaction, suffix='_Dict.cc', src_suffix='.h')
	elif os.path.exists(rootcintpath):
		bld = SCons.Script.Builder(action = rootcintaction, suffix='_Dict.cc', src_suffix='.h')
	else:
		print 'Neither rootcint nor rootcling exists. Unable to create ROOT dictionaries if any encountered.'
		return

	env.Append(BUILDERS = {'ROOTDict' : bld})

	# Generate ROOT dictionary file targets for each header
	# containing "ClassDef"
	#
	# n.b. It seems if scons is run when the build directory doesn't exist,
	# then the cwd is set to the source directory. Otherwise, it is the
	# build directory. Since the headers will only exist in the source
	# directory, we must temporarily cd into that to look for headers that
	# we wish to generate dictionaries for. (This took a long time to figure
	# out!)
	curpath = os.getcwd()
	srcpath = env.Dir('.').srcnode().abspath
	if(int(env['SHOWBUILD'])>1):
		print "---- Scanning for headers to generate ROOT dictionaries in: %s" % srcpath
	os.chdir(srcpath)
	for f in glob.glob('*.[h|hh|hpp]'):
		if 'ClassDef' in open(f).read():
			env.AppendUnique(ALL_SOURCES = env.ROOTDict(f))
			if(int(env['SHOWBUILD'])>1):
				print "       ROOT dictionary for %s" % f
	os.chdir(curpath)


##################################
# CUDA
##################################
def AddCUDA(env):
	CUDA = os.getenv('CUDA_INSTALL_PATH')
	if CUDA != None	:

		# Create Builder that can compile .cu file into object files
		NVCC = '%s/bin/nvcc' % CUDA
#		CUDAFLAGS = '-I%s/include -arch=compute_13 -code=compute_13' % CUDA
		CUDAFLAGS = ['-g', '-I%s/include' % CUDA]
		try:
			CUDAFLAGS.extend(env['CUDAFLAGS'])
		except:
			pass

		CUDAFLAGS_STR = ''
		for flag in CUDAFLAGS: CUDAFLAGS_STR = '%s %s' %(CUDAFLAGS_STR, flag)

		if env['SHOWBUILD']==0:
			nvccaction = SCons.Script.Action('%s -c %s -o $TARGET $SOURCE' % (NVCC, CUDAFLAGS_STR), 'NVCC       [$SOURCE]')
		else:
			nvccaction = SCons.Script.Action('%s -c %s -o $TARGET $SOURCE' % (NVCC, CUDAFLAGS_STR))
		bld = SCons.Script.Builder(action = nvccaction, suffix='.o', src_suffix='.cu')
		env.Append(BUILDERS = {'CUDA' : bld})

		# Add flags to environment
		env.AppendUnique(LIBPATH=['%s/lib' % CUDA, '%s/lib64' % CUDA])
		env.AppendUnique(LIBS=['cublas', 'cudart'])
		env.AppendUnique(CPPPATH=['%s/include' % CUDA])
		env.AppendUnique(CXXFLAGS=['-DGPU_ACCELERATION'])

		# Temporarily change to source directory and add all .cu files
		curpath = os.getcwd()
		srcpath = env.Dir('.').srcnode().abspath
		os.chdir(srcpath)
		for f in glob.glob('*.cu'):
			if env['SHOWBUILD']>0 : print 'Adding %s' % f
			env.AppendUnique(MISC_OBJECTS = env.CUDA(f))
		os.chdir(curpath)


##################################
# AmpTools
##################################
def AddAmpTools(env):
	# Amptools can be downloaded from http://sourceforge.net/projects/amptools/
	# It can be built with GPU support using CUDA (add GPU=1 to command
	# line when builidng AmpTools itself). If the CUDA_INSTALL_PATH
	# environment variable is set and the $AMPTOOLS/lib/libAmpTools_GPU.a
	# file exists then this will automatically use the GPU enabled
	# library. Otherwise, the CPU only library will be used.
	# If the AMPTOOLS environment variable is not set, then a message is
	# printed and env left unchanged.
	AMPTOOLS = os.getenv('AMPTOOLS')
	if AMPTOOLS==None:
		print ''
		print 'AmpTools is being requested but the AMPTOOLS environment variable'
		print 'is not set. Expect to see an error message below....'
		print ''
	else:
		env.AppendUnique(CUDAFLAGS=['-I%s -I%s/src/libraries' % (AMPTOOLS, os.getenv('JLEIC_HOME',os.getcwd()))])
		AddCUDA(env)
		AMPTOOLS_CPPPATH = "%s" % (AMPTOOLS)
		AMPTOOLS_LIBPATH = "%s/lib" % (AMPTOOLS)
		AMPTOOLS_LIBS = 'AmpTools'
		if os.getenv('CUDA')!=None and os.path.exists('%s/lib/libAmpTools_GPU.a' % AMPTOOLS):
			AMPTOOLS_LIBS = 'AmpTools_GPU'
			print 'Using GPU enabled AMPTOOLS library'

		env.AppendUnique(CPPPATH = AMPTOOLS_CPPPATH)
		env.AppendUnique(LIBPATH = AMPTOOLS_LIBPATH)
		env.AppendUnique(LIBS    = AMPTOOLS_LIBS)


##################################
# AmpPlotter
##################################
def AddAmpPlotter(env):
	ampplotterroot = os.getenv('AMPPLOTTER', 'ampplotter')
	AMPPLOTTER_CPPPATH = "%s" % (ampplotterroot)
	AMPPLOTTER_LIBPATH = "%s/lib" % (ampplotterroot)
	AMPPLOTTER_LIBS = "AmpPlotter"
	env.AppendUnique(CPPPATH = AMPPLOTTER_CPPPATH)
	env.AppendUnique(LIBPATH = AMPPLOTTER_LIBPATH)
	env.AppendUnique(LIBS    = AMPPLOTTER_LIBS)