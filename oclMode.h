#pragma once

#include "common.h"

void runOCL(const char* fileI, const char* fileO, unsigned int generations, unsigned int platformId, unsigned int deviceId)
{
#ifdef _DEBUG
    std::cout << "DEBUG" << std::endl;
#endif
    std::cout << "running mode: ocl" << std::endl;

    // init grid from file
    Timing::getInstance()->startSetup();
    // TODO: fill datastructure
    Timing::getInstance()->stopSetup();

    Timing::getInstance()->startComputation();
    // TODO: do stuff
    Timing::getInstance()->stopComputation();
}
