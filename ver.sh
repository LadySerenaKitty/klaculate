#!/bin/sh
desc=`git describe 2>/dev/null`

if [ "${desc}" ]; then
	bch=`git branch --show-current`
	echo "${desc}-${bch}"
else
	git info | grep -v Type | grep commit | awk '{ print $2 }' | grep -o '........$'
fi

