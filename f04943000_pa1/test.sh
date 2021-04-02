#!/bin/bash

input="../input_pa1/input_1.dat"
output="./myOutput/output_1.dat"
#input="testInput.dat"
#output="testOutput.dat"

IPATH="../input_pa1"
OPATH="./myOutput"
make clean
make

./bin/fm $input $output
