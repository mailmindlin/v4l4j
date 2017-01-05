#!/bin/bash
unversionned="$(svn st | grep "?" | cut -f8 -d" ")"
svn add $unversionned
