#!/bin/sh

find XPostFacto \(  -name '*.cpp' -or -name '*.c' -or -name '*.h' -or -name '*.r' \) \
	-exec /Developer/Tools/SetFile -t TEXT -c CWIE {} \;

find XPostFacto -name '*.mcp' \
	-exec /Developer/Tools/SetFile -t MMPr -c CWIE {} \;

find XPostFacto -name '*.Lib' \
	-exec /Developer/Tools/SetFile -t MPLF -c CWIE {} \;
