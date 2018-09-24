#!/bin/bash

definedOnly=0
while getopts "ad" value
do
	case $value in
				a)
				binaryfile=1
				;;
				d)
				defineOnly=1
				;;
	esac
done

shift $(($OPTIND-1))

param=""
value=""

if [[ $# == 1 ]]
then
	value=$1
fi

pwd=$PWD

if [[ X$value == X ]]
then
	echo "usage:"
	echo " $0 [-ad]<string>"
	echo " -a only search the *.a "
	echo " -d only search the defined function"
	exit 0
fi

if [[ $binaryfile == 1 ]]
then
	if [[ $defineOnly == 1 ]]
	then
	value="T$value"
	fi
	echo "search in binary file ...."
	time find . -path '*thon*' -prune -o -name "*.a" -type f -print -exec nm {} -A \; | grep "$value"
	exit 0
fi

echo "searching $value....."

exclude="(symvers|depend|cmd|xtt|map|ko|o|lib|svn-base|d|mod|yin)"

if [[ $defineOnly == 1 ]]
then
	echo "defined only"
	reg="$value\(\w+[\*]*\s[\*]*\w+[,)]"
	time find . -path '*.svn' -prune -o -regextype posix-extended ! -regex ".*\.$exclude" -type f -print0 | xargs  -0 -P 0 grep -n -w "$value" | grep -E $reg
else
	echo "search all"
	time find . \( -path '*.svn' -o -path '*.deps' \) -prune -o -regextype posix-extended ! -regex " .*\.$exclude" -type f -print0 | xargs -0 -P 0 grep -n -w "$value"
fi


