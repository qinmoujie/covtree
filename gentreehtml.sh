#!/bin/bash
set -e

sourcedir=$(cd $(dirname $0);pwd)

print_help()
{
    echo -e "usage: ./gentreehtml.sh [-h] [-i] -s <project> -f <CodeCoverage.info.cleaned> -c <CodeCoverageReport> [-o <covtree.html>] [-t <title>]"
    echo -e "Path of project is needed"
    echo -e "-h\t Print help"
    echo -e "-i\t Ignore parent path of file index html in CodeCoverageReport"
    echo -e "-f\t Path of CodeCoverage.info.cleaned"
    echo -e "-c\t Path of CodeCoverageReport"
    echo -e "-o\t Name of out html file,option,default:covtree.html"
    echo -e "-t\t Title of out html,option,default:Coverage Report"
}

error_meg()
{
    echo "error: $@"
    exit 1
}

##
absProject=""
abs_CodeCoverage_info_cleaned=""
abs_CodeCoverageReport=""
outhtml="covtree.html"
title="CovReport"
ignorePar="notignorePar"
##
if [[ -z $@ ]];then
    print_help
    exit 1;
fi

while getopts his:f:c:o:t: opt
do
    case "$opt" in
    h)
        print_help
        exit 0
        ;;
    i)
        ignorePar="ignorePar" ;;
    s)
        absProject=$OPTARG ;;
    f)
        abs_CodeCoverage_info_cleaned=$OPTARG ;;
    c)
        abs_CodeCoverageReport=$OPTARG ;;
    o)
        outhtml=$OPTARG ;;
    t)
        title=$OPTARG ;;
    *)
        print_help
        exit 1
        ;;
    esac
done

if [[ -z $absProject ]];then
    error_meg "use -s to specify path of project"
fi
if [ ! -d $absProject ];then
    error_meg "$absProject is not a directory!!!"
else
    absProject=$(cd $absProject;pwd)
fi

if [[ -z $abs_CodeCoverage_info_cleaned ]];then
    error_meg "use -f to specify path of CodeCoverage.info.cleaned"
fi
if [ ! -f $abs_CodeCoverage_info_cleaned ];then
    error_meg "$abs_CodeCoverage_info_cleaned is not a file!!!"
else
    abs_CodeCoverage_info_cleaned="$(echo $(cd $(dirname $abs_CodeCoverage_info_cleaned);pwd))/${abs_CodeCoverage_info_cleaned##*/}"
fi

if [[ -z $abs_CodeCoverageReport ]];then
    error_meg "use -c to specify path of CodeCoverageReport"
fi
if [ ! -d $abs_CodeCoverageReport ];then
    error_meg "$abs_CodeCoverageReport is not a directory!!!"
else
    abs_CodeCoverageReport=$(cd $abs_CodeCoverageReport;pwd)
fi

cd $sourcedir

CC=clang++
if ![ $CC --help > /dev/null 2>&1 ];then
    CC=g++
fi

if [ ! -x dirtree ];then
    $CC -std=c++11 dirtree.cpp -o dirtree
fi
./dirtree $absProject $abs_CodeCoverage_info_cleaned $abs_CodeCoverageReport $ignorePar tree.html
cat head.html | sed "s|Coverage Report|$title|g" > $outhtml
cat tree.html >> $outhtml
open $outhtml
