#!/bin/bash

# Important for float values
LC_NUMERIC=C

if [ "$#" -ne "2" ]
then
	echo "Usage: $0 reestr.txt reestr.out"
	exit 1;
fi

infile=$1
outfile=$2
outTmp=${outfile}.tmp
outWin=${outfile}.tmp2

# checking for bc
which bc > /dev/null 2>&1

if [ ! $? -eq "0" ]
then
	echo Install bc first
	exit 1
fi	

# checking permissions
if [ ! -r $infile ]
then
	echo "File $infile doesnt exist or has no read permission"
	exit
fi

# generating unique file
awk '!x[$0]++' $infile >> $outTmp

# calculating total amount
COUNT=0
IFS=$'\r\n'
SUMM=0
#PENALTY=0
while read line; do
	regexpAmount='s/\(.*\)\t\(.*\)\t\(.*\)\t\(.*\)/\3/'
	amount=`echo $line | sed -e $regexpAmount`
#	regexpPenalty='s/\(.*\)\t\(.*\)\t\(.*\)\t\(.*\)\t\(.*\)/\4/'
#	penalty=`echo $line | sed -e $regexpPenalty`
	if [ "$amount" != "" ]
	then
		SUMM=`echo $amount+$SUMM | bc`
		COUNT=$(($COUNT+1))
	fi
#	if [ "$penalty" != "" ]
#	then
#		PENALTY=`echo $penalty+$PENALTY | bc`
#	fi
done < $outTmp

# writing result
#echo -ne "Agent_ID:$AgentID\r\n" > $outfile
#echo -ne "Agent_name:$AgentName\r\n" >> $outfile
#echo -ne "Contract_number:$ContractNumber\r\n" >> $outfile
#echo -ne "Total:$total\r\n" >> $outfile
#echo -ne "Table:Date\tTime\tTransactiont\tAccount\tAmount\r\n" >> $outfile

#printf "%.7i%014.2f%011.2f\\n" $COUNT $SUMM $PENALTY > $outfile
printf "%.7i%014.2f\\n" $COUNT $SUMM > $outfile
cat $outTmp >> $outfile

# replacing \n by \r\n
sed ':a;N;$!ba;s/(^\r)\n/\r\n/g' $outfile > $outWin
mv $outWin $outfile


# removing tmp file
rm $outTmp
