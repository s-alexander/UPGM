#!/bin/bash

# Important for float values
LC_NUMERIC=C

if [ "$#" -ne "7" ]
then
	echo "Usage: $0 ca_cert client_сert key password receipt reason reestr"
	exit 1;
fi

cacert=$1
cert=$2
key=$3
password=$4
receipt=$5
reason=$6
reestr=$7

#echo "<response><code>9</code><date>2011-03-25T11:58:19</date></response>" | sed 's/.*<code>\([0-9]*\)<\/code>.*/\1/'
RESULT=`curl -vvv -k --cacert ${cacert} --cert ${cert}:${password} --key ${key} "https://pgate.centertelecom.ru:9001/adapter/intercapital/tm/proxy?action=cancel&receipt=${receipt}&mes=${reason}" | iconv -f CP1251 -t UTF-8`
echo "RESULT: [$RESULT]"
CODE=`echo $RESULT | sed 's/.*<code>\([0-9]*\)<\/code>.*/\1/'`
MESSAGE=`echo $RESULT | sed 's/.*<message>\(.*\)<\/message>.*/\1/'` 
echo "CODE: [$CODE]"
echo "MESSAGE: [$MESSAGE]"

tmpOut=${reestr}.cancel.tmp

if [ "$CODE" == "9" ];
then

	while read line; do
		echo "line=[$line]"
		r=`echo $line | sed -e 's/\(.*\)\t\(.*\)\t\(.*\)\t\(.*\)\t\(.*\)/\5/'`
		echo "[${r}]"
		if [ "$r" == $receipt ];
		then
			echo "Record found"
		else
			echo $line >> $tmpOut 
		fi	
	done < $reestr

fi
#
#if [ "$CODE" == "9" ]
#then
#fi
