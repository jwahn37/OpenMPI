#!/bin/bash

DIR="./ppm_example"
NUM_CORE=$1

sh compile.sh hw_imgprocess_2

for file in $DIR/*.ppm
do
    echo $file | sh run.sh $NUM_CORE hw_imgprocess_2
done    

for file in $DIR/*/*.ppm
do
    echo $file | sh run.sh $NUM_CORE hw_imgprocess_2
done

