#!/bin/bash

DIR="./ppm_example"

gcc hw_imgprocess_1.c -o hw_imgprocess_1

for file in $DIR/*.ppm
do
    echo $file | ./hw_imgprocess_1
done    

for file in $DIR/*/*.ppm
do
    echo $file | ./hw_imgprocess_1
done

