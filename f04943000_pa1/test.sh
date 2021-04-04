#!/bin/bash

input="../input_pa1/input_2.dat"
output="./myOutput/output_2.dat"
#input="testInput.dat"
#output="testOutput.dat"

IPATH="../input_pa1"
OPATH="./myOutput"
make clean
make

./bin/fm $input $output
