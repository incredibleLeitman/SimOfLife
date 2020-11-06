#pragma once

void runOCL(const char* fileI, const char* fileO, unsigned int generations, unsigned int platformId, unsigned int deviceId)
{
    // init grid from file
    Timing::getInstance()->startSetup();
    // TODO: fill datastructure
    Timing::getInstance()->stopSetup();

    Timing::getInstance()->startComputation();
    // TODO: do stuff
    Timing::getInstance()->stopComputation();
}
