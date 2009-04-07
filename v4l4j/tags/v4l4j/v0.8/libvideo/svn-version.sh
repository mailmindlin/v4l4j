#!/bin/bash
branch="$(svn info | grep 'URL')"
branch="${branch%/*}"
branch="${branch##*/}"
rev="${branch}_r$(svn info | grep 'Revision' | cut -f 2 -d ' ')"

if [ "x${rev}" != "x" ]; then
	echo "#define VER_REV \"${rev}\"" > version.h
else
        echo "#define VER_REV \"UNKNOWN\"" > version.h
fi

