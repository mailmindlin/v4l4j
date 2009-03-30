#!/bin/bash
url="$(svn info | grep 'URL')"
url="${url##*/}"
rev="$(svn info | grep 'Revision' | cut -f 2 -d ' ')"

if [ "x${url}" != "x" -a "x${rev}" != "x" ]; then
	echo "#define SVN_BRANCH \"${url}\"" > version.h
	echo "#define SVN_REV \"r${rev}\"" >> version.h
else
        echo "#define SVN_BRANCH \"Unknown\"" > version.h
        echo "#define SVN_REV \"\"" >> version.h
fi

