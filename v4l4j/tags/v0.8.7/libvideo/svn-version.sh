#!/bin/bash
branch="$(svn info 2>/dev/null | grep 'URL')" 
branch="${branch%/*}"
branch="${branch##*/}"
rev="${branch}_r$(svn info 2>/dev/null | grep 'Revision' | cut -f 2 -d ' ')"

if [ "x${rev}" != "x_r" ]; then
	echo "#define VER_REV \"${rev}\"" > version.h
else
        echo "#define VER_REV \"UNKNOWN\"" > version.h
fi

