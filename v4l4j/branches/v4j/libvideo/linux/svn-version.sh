#!/bin/bash
branch="$(svn info 2>/dev/null | grep 'URL')" 
branch="${branch##*/}"
rev="$(svn info 2>/dev/null | grep 'Revision' | cut -f 2 -d ' ')"

if [ "x${rev}" != "x" ]; then
	echo "#define VER_REV \"${branch}_r${rev}\"" > version.h
else
	echo "#define VER_REV \"UNKNOWN\"" > version.h
fi

