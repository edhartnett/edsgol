#!/bin/sh

# This script used to submit jobs to frost to do the homework in Lab 8.
# $Id: sub.sh,v 1.1 2007/11/11 20:26:07 edhartnett Exp $

# checkerboard distribution - small arrays
cqsub -n 400 -O output/perf_async -t 00:10:00 ./gol -n 400 -k -s 900 -ph
cqsub -n 324 -O output/perf_async -t 00:10:00 ./gol -n 324 -k -s 900 -p
cqsub -n 484 -O output/perf_async -t 00:10:00 ./gol -n 484 -k -s 900 -p

# checkerboard distribution - large arrays
cqsub -n 484 -O output/perf_async -t 00:10:00 ./gol -n 484 -k -s 9000 -p

# row distribution - small arrays
cqsub -n 1 -O output/perf_async -t 00:10:00 ./gol -n 1 -s 900 -p
cqsub -n 4 -O output/perf_async -t 00:10:00 ./gol -n 4 -s 900 -p
cqsub -n 9 -O output/perf_async -t 00:10:00 ./gol -n 9 -s 900 -p
cqsub -n 18 -O output/perf_async -t 00:10:00 ./gol -n 18 -s 900 -p
cqsub -n 25 -O output/perf_async -t 00:10:00 ./gol -n 25 -s 900 -p
cqsub -n 36 -O output/perf_async -t 00:10:00 ./gol -n 36 -s 900 -p
cqsub -n 64 -O output/perf_async -t 00:10:00 ./gol -n 64 -s 900 -p
cqsub -n 100 -O output/perf_async -t 00:10:00 ./gol -n 100 -s 900 -p
cqsub -n 150 -O output/perf_async -t 00:10:00 ./gol -n 150 -s 900 -p
cqsub -n 300 -O output/perf_async -t 00:10:00 ./gol -n 300 -s 900 -p
cqsub -n 450 -O output/perf_async -t 00:10:00 ./gol -n 450 -s 900 -p

# row distribution - large arrays
cqsub -n 450 -O output/perf_async -t 00:50:00 ./gol -n 450 -s 9000 -p
