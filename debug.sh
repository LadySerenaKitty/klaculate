#!/bin/sh
if [ -e "{$1}" ]; then
	gdb --pid={$1} -se build/bin/klaculate -d src -w --tui
else
	gdb -c klaculate.core -se build/bin/klaculate -d src -w --tui
fi

