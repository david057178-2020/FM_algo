#!/bin/bash

IPATH="../input_pa1"
OPATH="./myOutput"
LOGPATH="./myLog"
#make clean
#make

for i in $(seq 1 5)
do
#./bin/fm $IPATH/input_$Idx.dat $OPATH/output_$Idx.dat > $LOGPATH/log_$Idx.dat
	./bin/fm $IPATH/input_$i.dat $OPATH/output_$i.dat > $LOGPATH/log_$i.dat
done
#./bin/fm $input $output
