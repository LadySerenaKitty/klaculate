#!/bin/sh
if [ -e "{$1}" ]; then
	lldb -p {$1} -f build/bin/klaculate -s src
else
	lldb -c klaculate.core -f build/bin/klaculate
fi

