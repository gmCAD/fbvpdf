#!/bin/bash
set -x

export G=`git rev-list HEAD --count`
echo $G
make -f Makefile.general GIT_BUILD='${G}'  $1
MR=$?
if [ $MR -eq 0 ]; then
	mv build/release/mupdf-gl build/release/mupdf-linux
	mkdir fbvpdf
	cp build/release/mupdf-linux fbvpdf/fbvpdf-linux
	tar zcvf fbvpdf.tar.gz fbvpdf
fi
exit $MR
