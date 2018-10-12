#!/bin/bash
set -e

if [ $# -lt 3 ];then
    echo "\$1: Abs path of project"
    echo "\$2: Abs path of CodeCoverage.info.cleaned"
    echo "\$3: Abs path of CodeCoverageReport"
    echo "\$4: Path of out html file,option,default:covtree.html"
    echo "\$5: Title of out html,option,default:Coverage Report"
    exit 1
fi

sedcmd="sed -i ''"
if [ $(uname) != "Darwin" ];then
    sedcmd='sed -i'
fi

cp ./gentreehtml.sh ./gentreehtml
$sedcmd "s|absProject=\"\"|absProject=\"$1\"|g" gentreehtml
$sedcmd "s|abs_CodeCoverage_info_cleaned=\"\"|abs_CodeCoverage_info_cleaned=\"$2\"|g" gentreehtml
$sedcmd "s|abs_CodeCoverageReport=\"\"|abs_CodeCoverageReport=\"$3\"|g" gentreehtml
if [ -n $4 ];then
    $sedcmd "s|covtree.html|$4|g" gentreehtml
fi

if [ -n $5 ];then
    $sedcmd "s|CovReport|$5|g" gentreehtml
fi

echo "gentreehtml install done"
