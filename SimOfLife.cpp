// TODOs:
// - optimizations :p
// - data oriented design -> split bool value and neighbours in favor of tmp buffer to read from
// - read cmd args with getopt port (unix "only" but simple function)
//   https://github.com/skandhurkat/Getopt-for-Visual-Studio/blob/master/getopt.h

// major bottleneck optimizations:
// - faster memcpy for structs -> dod
// - only look at alive cells -> only those can die or create new cells
// - at least check if a map neighbours for cell which contains 8 pointers to cells would be faster than recalc everytime

// minor optimizations:
// - no if()'s to better perform on GPU -> val = TRUE_COND * val + FALSE_CON * val
// - use classic C file operations?
// - using charptrs everywhere instead of array indexing
// - using array as param instead of globals for easy swap between read/work buffer
// - emulate -O3 for g++ in vc++ (saves about 500 ms)

// done:
// - input x and y instead of index to minimize calculations -> saves about 1 sec for 10k cells
// - use structs to init temp variables (offsets, ...) once rather than globals (yikes!) --> need more memory thus memcpy is much slower!
// - read whole DataBlob and set cells from there instead of ifstream.get(c);

#include <stdlib.h> // EXIT_SUCCESS
#include <iostream> // memset
#include <fstream> // ifstream
#include <string> // getline
#include <cassert> // assert

#ifdef __GNUC__
#include <cstring> // memcpy for g++
#endif

//#define DEBUG_OUT
//#define USE_STEPS // wait for user input to perform next step
//#define SHOW_GENS // prints every generation
//#define NO_IFS // ~6800ms, with normal ifs about 200 - 300 ms faster

#include "Timing.h"

#include "seqMode.h" // sequential implementation
#include "ompMode.h" // openMP implementation
#include "oclMode.h" // openCL implementation

int main(int argc, char** argv)
{
    // parse command line options
    //std::cout << "args: " << argc << std::endl;
    unsigned int generations = 250;             // --generations - count of gens
    const char* fileI = "random10000_in.gol";   // --load - input filename with the extension ’.gol’
    std::string path("out" + std::to_string(generations) + ".out");
    const char* fileO = path.c_str();           // --save - filename with the extension ’.gol’
    bool printMeasure = true;                   // --mesaure - generates measurement output on stdout
    std::string mode = "seq";                   // --mode - extended in Project 2: seq, omp, ocl
    int threads = 8;                            // --threads - amount of threads to use for omp
    int platformId = 0;                         // --platformId - ocl
    int deviceId = 0;                           // --deviceId - ocl
    for (int i = 0; i < argc; ++i)
    {
        //std::cout << "\t" << argv[i] << std::endl;
        if (i < (argc + 1))
        {
            //if (argv[i] == "--load") --> no comparison between const char * and std::string
            if (strcmp(argv[i], "--load") == 0) fileI = argv[i + 1];
            else if (strcmp(argv[i], "--save") == 0) fileO = argv[i + 1];
            else if (strcmp(argv[i], "--generations") == 0) generations = std::stoul(argv[i + 1]);
            else if (strcmp(argv[i], "--measure") == 0) printMeasure = true;
            else if (strcmp(argv[i], "--mode") == 0) mode = argv[i + 1];
            else if (strcmp(argv[i], "--threads") == 0) threads = std::stoi(argv[i + 1]);
            else if (strcmp(argv[i], "--device") == 0) ; // automatically selects platform & device -> handle as default
            else if (strcmp(argv[i], "--platformId") == 0) platformId = std::stoi(argv[i + 1]);
            else if (strcmp(argv[i], "--deviceId") == 0) deviceId = std::stoi(argv[i + 1]);
        }
    }

    if (mode == "default" || mode == "seq")
    {
        runSeq(fileI, fileO, generations);
    }
    else if (mode == "omp")
    {
        runOMP(fileI, fileO, generations, threads);
    }
    else if (mode == "ocl")
    {
        //if (platformId == 0) // TODO: auto select platform
        //if (deviceId == 0) // TODO: auto select device
        runOCL(fileI, fileO, generations, platformId, deviceId);
    }

    if (printMeasure) Timing::getInstance()->print();

    return EXIT_SUCCESS;
}