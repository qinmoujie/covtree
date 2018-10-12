#!/bin/bash
set -e

##
absProject=""
abs_CodeCoverage_info_cleaned=""
abs_CodeCoverageReport=""
outhtml="covtree.html"
title="Coverage Report"
##

CC=clang++
if ![ $CC --help > /dev/null 2>&1 ];then
    CC=g++
fi

$CC -std=c++11 dirtree.cpp -o dirtree
./dirtree $absProject $abs_CodeCoverageReport $abs_CodeCoverageReport tree.html
cat head.html | sed "s|Coverage Report|$title|g" > $outhtml
cat tree.html >> $outhtml
open $outhtml
