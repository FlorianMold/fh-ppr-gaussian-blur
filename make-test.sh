#!/bin/bash

while getopts i:t:n:f:r: flag
do
    case "${flag}" in
        i) iterations=${OPTARG};;
        t) threads=${OPTARG};;
        n) imageName=${OPTARG};;
        f) filename=${OPTARG};;
        r) radius=${OPTARG};;
    esac
done

echo "Running $iterations iteration(s) of the program";
echo "Gaussian Blur is run with: $radius radius on $imageName";
echo "Running: $threads threads";
echo "Saving to: $filename";

for j in $(seq 1 $iterations);
do
   ./cmake-build-debug/fh_ppr_gaussian_blur $radius $imageName $threads >> $filename
done