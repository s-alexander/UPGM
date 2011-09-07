#!/bin/bash

if [ "$#" -ne "3" ]
then
	echo "Usage: $0 user_id reestr.txt reestr.out"
	exit 1;
fi

userId=$1
inFile=$2
outFile=$3

gpg -e -r $userId -a --output $outFile $inFile
