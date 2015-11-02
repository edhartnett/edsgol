#!/bin/sh

# This script used to submit jobs to frost to do the homework in Lab 8.
# $Id: run_class_perf.sh,v 1.1 2007/11/11 21:29:37 edhartnett Exp $

# From the assignment:
# Please record your performance timing data from Frost using coprocessor mode
# for the increasing runs for the 900x900 problem using the PGM initial
# conditions file provided with the first CGL lab for all logical combinations
# less than np=36.

# For checkerboard, n must be a square. For row decomposition, it must be even.
cqsub -m co -n 1 -O output/perf_matthew -t 00:10:00 ./gol -n 1 -k -s 900 -i input/life.pgm -ph
cqsub -m co -n 1 -O output/perf_matthew -t 00:10:00 ./gol -n 1 -s 900 -i input/life.pgm -p
cqsub -m co -n 4 -O output/perf_matthew -t 00:10:00 ./gol -n 4 -k -s 900 -i input/life.pgm -p
cqsub -m co -n 4 -O output/perf_matthew -t 00:10:00 ./gol -n 4 -s 900 -i input/life.pgm -p
cqsub -m co -n 16 -O output/perf_matthew -t 00:10:00 ./gol -n 16 -k -s 900 -i input/life.pgm -p
cqsub -m co -n 16 -O output/perf_matthew -t 00:10:00 ./gol -n 16 -s 900 -i input/life.pgm -p
cqsub -m co -n 36 -O output/perf_matthew -t 00:10:00 ./gol -n 36 -k -s 900 -i input/life.pgm -p
cqsub -m co -n 36 -O output/perf_matthew -t 00:10:00 ./gol -n 36 -s 900 -i input/life.pgm -p

