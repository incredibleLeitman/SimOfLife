# SimOfLife
Simulation of life (rather than game)

## build

### windows
just compile the containing Visual Studio solution, make sure paths to openMP and openCL are set correctly in Compiler and Linker settings.

### linux
run ``make``, if CUDA / openCL libs are not installed in the default folder, provide location in LIB and INC.

## run

run with following arguments or leave as default:
```
--load <filename>             input filename with the extension ’.gol’, default "random10000_in.gol"
--save <filename>             output filename with the extension ’.gol’, default "out.out"
--generations <gens>          count of generations
--measure                     if provided, print timings in stdout
--mode <mode>                 defines the mode to run, following modes are implemented:
        seq                   default, sequential implementation
        omp                   openMp implementation, parallelized on cpu
        ocl                   openCL implementation, runs on cpu / gpu
--threads <threads>           amount of threads to use in openMp implementation
--device <type>               provides default device to run ocl mode, possible values are
        gpu                   first gpu device
        gpu                   first cpu device
--platformId <id>             provides platform id for ocl mode
--deviceId <id>               provides device id for ocl mode
--debug                       if given prints debug output to stdout
```

call ``run_multiple.sh`` to start iterations for 1000 - 10.000 values with default params. Optionally you can provide them as arguments:
```
$1 executable to run
$2 MGSUID -> gs20m012 (because UID is a readonly variable)
$3 input folder -> defaults to ../data if not present
$4 output folder -> defaults to ../data/out
$5 number of threads for omp
```

## timings

example timings for different modes:
| mode | values | setup | calc | finalize |
| ---- | ---- | ---- | ---- | ---- |
| seq | 1000 | 0:00.013 | 0:00.782 | 0:00.054 |
| seq | 2000 | 0:00.027 | 0:01.561 | 0:00.121 |
| seq | 3000 | 0:00.041 | 0:02.457 | 0:00.166 |
| seq | 4000 | 0:00.059 | 0:03.444 | 0:00.210 |
| seq | 5000 | 0:00.067 | 0:03.986 | 0:00.265 |
| seq | 6000 | 0:00.078 | 0:04.925 | 0:00.326 |
| seq | 7000 | 0:00.091 | 0:05.880 | 0:00.360 |
| seq | 8000 | 0:00.165 | 0:06.589 | 0:00.413 |
| seq | 9000 | 0:00.118 | 0:07.793 | 0:00.590 |
| seq | 10000 | 0:00.134 | 0:08.492 | 0:00.557 |
| omp | 1000 | 0:00.011 | 0:00.246 | 0:00.103 |
| omp | 2000 | 0:00.090 | 0:00.611 | 0:00.172 |
| omp | 3000 | 0:00.031 | 0:00.891 | 0:00.229 |
| omp | 4000 | 0:00.041 | 0:01.291 | 0:00.249 |
| omp | 5000 | 0:00.051 | 0:01.731 | 0:00.315 |
| omp | 6000 | 0:00.062 | 0:02.057 | 0:00.347 |
| omp | 7000 | 0:00.073 | 0:02.047 | 0:00.404 |
| omp | 8000 | 0:00.083 | 0:02.343 | 0:00.593 |
| omp | 9000 | 0:00.094 | 0:03.182 | 0:00.659 |
| omp | 10000 | 0:00.109 | 0:03.057 | 0:00.581 |
| ocl cpu | 1000 | 0:00.315 | 0:00.019 | 0:00.057 |
| ocl cpu | 2000 | 0:00.330 | 0:00.028 | 0:00.118 |
| ocl cpu | 3000 | 0:00.327 | 0:00.035 | 0:00.162 |
| ocl cpu | 4000 | 0:00.360 | 0:00.042 | 0:00.221 |
| ocl cpu | 5000 | 0:00.359 | 0:00.050 | 0:00.280 |
| ocl cpu | 6000 | 0:00.386 | 0:00.058 | 0:00.351 |
| ocl cpu | 7000 | 0:00.402 | 0:00.068 | 0:00.394 |
| ocl cpu | 8000 | 0:00.416 | 0:00.075 | 0:00.439 |
| ocl cpu | 9000 | 0:00.428 | 0:00.088 | 0:00.472 |
| ocl cpu | 10000 | 0:00.441 | 0:00.092 | 0:00.509 |
| ocl gpu | 1000 | 0:00.304 | 0:00.019 | 0:00.060 |
| ocl gpu | 2000 | 0:00.322 | 0:00.028 | 0:00.128 |
| ocl gpu | 3000 | 0:00.352 | 0:00.037 | 0:00.162 |
| ocl gpu | 4000 | 0:00.344 | 0:00.042 | 0:00.206 |
| ocl gpu | 5000 | 0:00.361 | 0:00.050 | 0:00.263 |
| ocl gpu | 6000 | 0:00.376 | 0:00.058 | 0:00.391 |
| ocl gpu | 7000 | 0:00.383 | 0:00.068 | 0:00.409 |
| ocl gpu | 8000 | 0:00.388 | 0:00.074 | 0:00.432 |
| ocl gpu | 9000 | 0:00.397 | 0:00.087 | 0:00.484 |
| ocl gpu | 10000 | 0:00.412 | 0:00.095 | 0:00.626 |
