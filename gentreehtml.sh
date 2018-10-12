#!/bin/bash
set -e

sourcedir=$(cd $(dirname $0);pwd)

if [ ${0##*/} = "gentreehtml.sh" ];then
    echo "Use $sourcedir/gentreehtml to parase cov info"
    echo "if not exists this file,run $sourcedir/install.sh"
    exit 1
fi

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

if [ ! -x dirtree ];then
    $CC -std=c++11 dirtree.cpp -o dirtree
fi
./dirtree $absProject $abs_CodeCoverage_info_cleaned $abs_CodeCoverageReport tree.html
cat head.html | sed "s|Coverage Report|$title|g" > $outhtml
cat tree.html >> $outhtml
open $outhtml
