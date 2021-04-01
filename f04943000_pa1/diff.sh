#!/bin/bash
PATH1="./myLog"
PATH2="./record"

for i in $(seq 1 5)
do
	diff $PATH1/log_$i.dat $PATH2/log_$i.dat
done
