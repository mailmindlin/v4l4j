#!/bin/bash

find . -name "*.html" -print | xargs svn propset -R svn:mime-type text/html 

