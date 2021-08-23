#!/bin/sh

if [ -x build/bin/klaculate ]; then
	echo Running \"make clean\"
	make clean
fi

CMAKEFILES=`find * -type d -name CMakeFiles`
CMAKECACHE=`find * -type f -name CMakeCache.txt`
MAKEFILES=`find * -type f -name Makefile`
DOXYFILE=`find * -type f -name Doxyfile`
INSTALL=`find * -type f -name cmake_install.cmake`
PKGLIST=`find * -type f -name install_manifest.txt`
BUILDS=`find * -type d -name build`
CORES=`find * -type f -name "*.core"`
VIMS=`find * -type f -name "*~"`

FILES="src/klaculate.h compile_commands.json"

if [ -n "${CMAKEFILES}" ]; then
	echo Removing CMakeFiles
	rm -r ${CMAKEFILES}
fi

if [ -n "${CMAKECACHE}" ]; then
	echo Removing CMakeCache
	rm ${CMAKECACHE}
fi

if [ -n "${MAKEFILES}" ]; then
	echo Removing Makefiles
	rm ${MAKEFILES}
fi

if [ -n "${DOXYFILE}" ]; then
	echo Removing Doxyfiles
	rm ${DOXYFILE}
fi

if [ -n "${INSTALL}" ]; then
	echo Removing cmake_install.cmake
	rm ${INSTALL}
fi

if [ -n "${PKGLIST}" ]; then
	echo Removing install manifsts
	rm -f ${PKGLIST}
fi

if [ -n "${BUILDS}" ]; then
	echo Removing artifacts
	rm -rf ${BUILDS}
fi

if [ -n "${CORES}" ]; then
	echo Removing core dumps
	rm ${CORES}
fi

if [ -n "${VIMS}" ]; then
	echo Removing vim backups
	rm ${VIMS}
fi

for F in ${FILES}
do
	if [ -f $F ]; then
		rm $F
	fi
done
