#pragma once

#include "omp.h" // need to have project settings C/C++ openMP enabled

void runOMP(const char* fileI, const char* fileO, unsigned int generations, unsigned int threads)
{
    // init grid from file
    Timing::getInstance()->startSetup();
    // TODO: fill datastructure
    Timing::getInstance()->stopSetup();

    int tid;
    omp_set_num_threads(threads);
    Timing::getInstance()->startComputation();
#pragma omp parallel private(tid) // arguments are shared variables
    {
        tid = omp_get_thread_num();
#pragma omp master // = if (tid == 0), master thread
        {
            std::cout << "Number of threads: " << omp_get_num_threads() << std::endl;
        }
        std::cout << "starting thread " << tid << std::endl;

        // TODO: refactor data structure

    } // threads join master thread and disband

    Timing::getInstance()->stopComputation();

    // write out result
    Timing::getInstance()->startFinalization();
    writeToFile(fileO);
    //writeToFile(fileO, true); // writes count of neighbours instead of just x / .
    Timing::getInstance()->stopFinalization();
}