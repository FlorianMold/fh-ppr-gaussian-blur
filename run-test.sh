#!/bin/bash

images=("balls" "delta" "mandelbrot" "xl")
maxThreads=8
maxIterations=30
radius=(1 4 7 10)

# Clear directory
rm -rf results/*

for r in ${radius[@]}; do
  mkdir results/radius-${r}
  for i in ${images[@]}; do
    mkdir results/radius-${r}/image-${i}
  done
done

echo "Running tests with 30 max-iterations"
for j in $(seq 1 $maxThreads);
do
  for i in ${images[@]}; do
    for r in ${radius[@]}; do
      ./make-test.sh -i $maxIterations -t ${j} -n cmake-build-debug/images/${i}.bmp -r ${r} -f results/radius-${r}/image-${i}/result-t${j}.txt
    done
  done
done
