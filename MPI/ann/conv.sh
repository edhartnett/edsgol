#!/bin/sh

for ((i=0; i < 100; i++))
do
    convert out_1_${i}.pgm out_1_${i}.gif
done
