#
# SConscript for pdbml-v1.0
# Updated: Dec 18, 2008 - Jdw
# Updated: Mar 30, 2011 - Jdw clone environment 
#
Import('env')
env=env.Clone()
#
#if (len(env.subst('$MYDEBUG')) > 0):
#	dict = env.Dictionary()
#	for k,v in dict.items():
#		print  k, " = ", str(v)
#
libName = 'pdbml'

libSrcList = ['src/XmlWriter.C',
	'src/XsdWriter.C',
	'src/PdbMlSchema.C',
	'src/PdbMlWriter.C']

libObjList = [s.replace('.C','.o') for s in libSrcList]
#
libIncList = ['include/XmlWriter.h',
	'include/XsdWriter.h',
	'include/PdbMlSchema.h',
	'include/PdbMlWriter.h']

myLib=env.Library(libName,libSrcList)
#
#
env.Install(env.subst('$MY_INCLUDE_INSTALL_PATH'),libIncList)
env.Alias('install-include',env.subst('$MY_INCLUDE_INSTALL_PATH'))
#
env.Install(env.subst('$MY_LIB_INSTALL_PATH'),myLib)
env.Alias('install-lib',env.subst('$MY_LIB_INSTALL_PATH'))
#
env.Install(env.subst('$MY_OBJ_INSTALL_PATH'),libObjList)
env.Alias('install-obj',env.subst('$MY_OBJ_INSTALL_PATH'))
#
env.Default('install-include','install-obj','install-lib')
#
