#!/bin/bash
url="$(svn info | grep 'URL')"
url="${url##*/}"
rev="$(svn info | grep 'Revision' | cut -f 2 -d ' ')"

if [ "x${url}" != "x" -a "x${rev}" != "x" ]; then
	echo "#define SVN_BRANCH \"${url}\"" > libv4l/version.h
	echo "#define SVN_REV \"r${rev}\"" >> libv4l/version.h
else
        echo "#define SVN_BRANCH \"Unknown\"" > libv4l/version.h
        echo "#define SVN_REV \"\"" >> libv4l/version.h
fi

