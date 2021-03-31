#!/bin/bash

#input="../input_pa1/input_1.dat"
#output="./myOutput/output_1.dat"
#input="testInput.dat"
#output="testOutput.dat"

IPATH="../input_pa1"
OPATH="./myOutput"
LOGPATH="./myLog"
#make clean
#make

for file in "$IPATH"/*
do
	Idx=$(echo $file | awk -F '[_.]' '{print $5}')
#./bin/fm $IPATH/input_$Idx.dat $OPATH/output_$Idx.dat > $LOGPATH/log_$Idx.dat
	./bin/fm $IPATH/input_$Idx.dat $OPATH/output_$Idx.dat
done
#./bin/fm $input $output
