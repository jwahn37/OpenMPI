#!/bin/bash

DIR="./ppm_example"

for file in $DIR/*.ppm
do
    echo $file | ./hw_imgprocess_1
done    

for file in $DIR/*/*.ppm
do
    echo $file | ./hw_imgprocess_1
done

