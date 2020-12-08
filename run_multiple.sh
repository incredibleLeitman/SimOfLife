#!/bin/bash

# script file to generate output for multiple runs, needed for hand in files
# takes the following arguments:
#$1 executable to run
#$2 MGSUID -> gs20m012 (because UID is a readonly variable)
#$3 input folder -> defaults to ../data/in if not present
#$4 output folder -> defaults to ../data/out
#$5 threads

# run_multiple.sh SimOfLife gs20m012 data data/out 8

#APP=SimOfLife
APP=SimOfLife.exe
MGS_UID=gs20m012
DATA=../data
DIR_IN=$DATA # should already exist
DIR_OUT=$DATA/out
THREADS=4

if [ -n "$1" ]; then
  APP=$1
fi

if [ -n "$2" ]; then
  MGS_UID=$2
fi

if [ -n "$3" ]; then # arg is set
  DIR_IN=$3
fi

if [ -n "$4" ]; then
  DIR_OUT=$3
fi

if [ -n "$5" ]; then
  THREADS=$3
fi

echo "APP:		'$APP'"
echo "MGS_UID:	'$MGS_UID'"
echo "creating Folder '$DIR_OUT'"
mkdir -p ${DIR_OUT}

for i in 1000 2000 3000 4000 5000 6000 7000 8000 9000 10000
do
    echo "Running File '$i', Mode=SEQ"
    ./$APP --load $DIR_IN/random${i}_in.gol --save ${DIR_OUT}/${MGS_UID}_${i}_cpu_out.gol --generations 250 --measure  --mode seq >> ${DIR_OUT}/${MGS_UID}_cpu_time.csv

	echo "Running File '$i', Mode=OMP, Threads=$THREADS"
	./$APP --load $DIR_IN/random${i}_in.gol --save ${DIR_OUT}/${MGS_UID}_${i}_openmp_out.gol --generations 250 --measure  --mode omp --threads ${THREADS} >> ${DIR_OUT}/${MGS_UID}_openmp_time.csv

	echo "Running File '$i', Mode=OCL, Device=CPU"
	./$APP --load $DIR_IN/random${i}_in.gol --save ${DIR_OUT}/${MGS_UID}_${i}_opencl_cpu_out.gol --generations 250 --measure --mode ocl --device cpu >> ${DIR_OUT}/${MGS_UID}_opencl_cpu_time.csv

	echo "Running File '$i', Mode=OCL, Device=GPU"
	./$APP --load $DIR_IN/random${i}_in.gol --save ${DIR_OUT}/${MGS_UID}_${i}_opencl_gpu_out.gol --generations 250 --measure --mode ocl --device gpu >> ${DIR_OUT}/${MGS_UID}_opencl_gpu_time.csv
done

$SHELL # to not close the window