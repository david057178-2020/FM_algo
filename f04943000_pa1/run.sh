#!/bin/bash

#input="../input_pa1/input_1.dat"
input="testInput.dat"
output="testOutput.dat"

make clean
make
./bin/fm $input $output
