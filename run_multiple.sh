#!/bin/bash

# script file to generate output for multiple runs, needed for hand in files
# TODO: defines for unix / windows

# takes the following arguments:
#   $1 executable to run
#   $2 MGSUID -> gs20m012 (because UID is a readonly variable)
#   $3 input folder -> defaults to ../data if not present
#   $4 output folder -> defaults to ../data/out
#   $5 number of threads for omp

# run_multiple.sh SimOfLife gs20m012 data data/out 8

#APP=SimOfLife
APP=SimOfLife.exe
MGS_UID=gs20m012
DATA=../data
DIR_IN=$DATA # should already exist
DIR_OUT=$DATA/out
THREADS=4
GENS=250

# read input params to override default values
if [ -n "$1" ]; then APP=$1; fi
if [ -n "$2" ]; then MGS_UID=$2; fi
if [ -n "$3" ]; then DIR_IN=$3; fi
if [ -n "$4" ]; then DIR_OUT=$4; fi
if [ -n "$5" ]; then THREADS=$5; fi

echo "creating Folder '$DIR_OUT'"
mkdir -p ${DIR_OUT}

PATH=${DIR_OUT}/${MGS_UID}

for i in 1000 2000 3000 4000 5000 6000 7000 8000 9000 10000
do
    echo "Running File '$i', Mode=SEQ"
    ./$APP --load $DIR_IN/random${i}_in.gol --save ${PATH}_${i}_cpu_out.gol --generations ${GENS} --measure --mode seq >> ${PATH}_cpu_time.csv

    echo "Running File '$i', Mode=OMP, Threads=$THREADS"
    ./$APP --load $DIR_IN/random${i}_in.gol --save ${PATH}_${i}_openmp_out.gol --generations ${GENS} --measure --mode omp --threads ${THREADS} >> ${PATH}_openmp_time.csv

    echo "Running File '$i', Mode=OCL, Device=CPU"
    ./$APP --load $DIR_IN/random${i}_in.gol --save ${PATH}_${i}_opencl_cpu_out.gol --generations ${GENS} --measure --mode ocl --device cpu >> ${PATH}_opencl_cpu_time.csv

    echo "Running File '$i', Mode=OCL, Device=GPU"
    ./$APP --load $DIR_IN/random${i}_in.gol --save ${PATH}_${i}_opencl_gpu_out.gol --generations ${GENS} --measure --mode ocl --device gpu >> ${PATH}_opencl_gpu_time.csv
done

#$SHELL # to not close the window