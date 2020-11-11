#pragma once

/* ---------------------------------------------------------------------------
omp mode:
uses OpenMP to utilize parallelism.

needed another datastructure as seqMode, because was not parallelizable due to 
internal constraints because of diff update instead of full state.

--------------------------------------------------------------------------- */

#include "common.h"
#include "omp.h" // need to have project settings C/C++ openMP enabled

inline int sumNeighbours(unsigned char* ptr_cell, int yOffTop, int yOffBot, int xOffLeft, int xOffRight)
{
    // just return the sum of states for all neighbours
    return
        *(ptr_cell + yOffTop + xOffLeft) +
        *(ptr_cell + yOffTop) +
        *(ptr_cell + yOffTop + xOffRight) +
        *(ptr_cell + xOffLeft) +
        *(ptr_cell + xOffRight) +
        *(ptr_cell + yOffBot + xOffLeft) +
        *(ptr_cell + yOffBot) +
        *(ptr_cell + yOffBot + xOffRight);
}

void ompReadFromFile(const char* filePath)
{
    std::cout << "read file: " << filePath << "..." << std::endl;
    std::ifstream in(filePath);
    if (in.is_open())
    {
        std::string line;
        std::getline(in, line); // get first line for explizit w / h

        // TODO: better split
        size_t pos = line.find(',');
        if (pos != std::string::npos)
        {
            w = std::stoi(line.substr(0, pos));
            h = std::stoi(line.substr(pos + 1));
        }
        else // use default values
        {
            w = 1000;
            h = 10000;
        }

        total_elem_count = w * h;
        col_right = w - 1;
        col_bot = total_elem_count - w;
        row_bot = h - 1;
        std::cout << "total: " << total_elem_count << ", w: " << w << ", h: " << h << std::endl;
        std::cout << "col_right: " << col_right << ", col_bot: " << col_bot << ", row_bot: " << row_bot << std::endl;

        cells = new unsigned char[total_elem_count];
        memset(cells, 0, total_elem_count);

        unsigned int idx = 0;
        char c;
        for (unsigned int y = 0; y < h; y++)
        {
            getline(in, line);
            for (unsigned int x = 0; x < w; x++)
            {
                c = line[x];
                //std::cout << "read c: " << c << " for index: " << idx << std::endl;
                if (c == 'x')  // only need to set alive cells, otherwise stay 0
                {
                    *(cells + idx) = STATE_ALIVE;
                }

                if (++idx >= total_elem_count) break;
            }
        }
    }
    else std::cout << "Error opening " << filePath << std::endl;

    if (!in.eof() && in.fail())
        std::cout << "error reading " << filePath << std::endl;

    in.close();
}

void ompWriteToFile(const char* filePath, bool drawNeighbours = false)
{
    std::cout << "write file: " << filePath << "..." << std::endl;
    std::ofstream out(filePath);
    if (out.is_open())
    {
        out << w << "," << h << std::endl;
        for (unsigned int i = 0; i < total_elem_count; ++i)
        {
            if (drawNeighbours) out << (neighbours[i] + 0);
            else out << ((cells[i] & STATE_ALIVE) ? "x" : ".");

            if (i % w == w - 1) out << std::endl;
        }
    }
    else std::cout << "Error opening " << filePath << std::endl;

    out.close();
}

void runOMP(const char* fileI, const char* fileO, unsigned int generations, int threads)
{
#ifdef _DEBUG
    std::cout << "DEBUG" << std::endl;
#endif
    std::cout << "running mode: omp" << std::endl;

    // init grid from file
    Timing::getInstance()->startSetup();
    ompReadFromFile(fileI);

    // make an array for saving neighbour count for each cell
    neighbours = new int[total_elem_count];

    // OpenMP initializations
    // OMP_NUM_THREADS (environment variable) specifies initially the number of threads
    // calls to omp_set_num_threads() override the value of OMP_NUM_THREADS
    // the presence of the num_threads clause overrides both other values.

    //{200203,"2.0"},{200505,"2.5"},{200805,"3.0"},{201107,"3.1"},{201307,"4.0"},{201511,"4.5"},{201811,"5.0"}
    std::cout << "OpenMP version: " << _OPENMP << std::endl;
    int num_threads = omp_get_num_threads();
    if (threads != num_threads) omp_set_num_threads(threads);
    std::cout << "number of processors: " << omp_get_num_procs() << std::endl;
    std::cout << "number of threads: " << omp_get_num_threads() << " for param: " << threads << std::endl;

    int row = 0;
    int col = 0;
    int idx = 0;
    unsigned int gen = 0;
    int value = 0;
    int countNeighbours = 0;

    int yOffTop = -(int)w;
    int yOffBot = +(int)w;
    int xOffLeft = -1;
    int xOffRight = +1;
    Timing::getInstance()->stopSetup();

    /*
    #pragma omp parallel for

    #pragma omp parallel private(tid) // arguments are shared variables
    {
        int tid = omp_get_thread_num();
//#pragma omp single
#pragma omp master // = if (tid == 0), master thread
        {
            int num_threads = omp_get_num_threads();
            std::cout << "Number of threads: " << num_threads << std::endl;
            omp_set_num_threads(threads);
        }
        int tid = omp_get_thread_num();
        std::cout << "starting thread " << tid << std::endl;
    } // threads join master thread and disband
    */

    Timing::getInstance()->startComputation();
    for (gen = 0; gen < generations; gen++)
    {
        yOffTop = -(int)w;
        yOffBot = +(int)w;
        xOffLeft = -1;
        xOffRight = +1;

        // need to get current neighbour count, because other than seqMode updates are not diffs but full states
        // first handle all cells without border mapping, afterwards special handling
        // some timing measures bevor using omp or special handling for y == 0 or row_bot
        // only handling left/right x
        // Windows: 9112ms, 9123ms, 9180ms, 9253.18ms
        // Linux:  10513.3ms, 10319.2ms, 10323.1ms
        // also handling top/bot y
        // Windows: 9308.48ms, 9353.79ms, 9310.43ms
        // Linux:   10223.9ms, 10055.8ms, 10039.1ms
#pragma omp parallel for shared(neighbours) private(row, col)
        for (row = 0; row < h; row++)
        {
            yOffTop = (row == 0) ? col_bot : -(int)w;
            yOffBot = (row == (h - 1)) ? -col_bot : w;

//#pragma omp parallel for shared(neighbours) private(row, col)
            for (col = 1; col < col_right; col++)
            {
                idx = col + (row * w); // fastest way
                *(neighbours + idx) = sumNeighbours(cells + idx, yOffTop, yOffBot, xOffLeft, xOffRight);
            }

            // handle border for x == 0
            idx = (row * w);
            *(neighbours + idx) = sumNeighbours(cells + idx, yOffTop, yOffBot, col_right, xOffRight);

            // handle border for x == col_right
            idx = col_right + (row * w);
            *(neighbours + idx) = sumNeighbours(cells + idx, yOffTop, yOffBot, xOffLeft, -col_right);
        }

        // change cells dependent on oldCells
#pragma omp parallel for shared(cells)
        for (idx = 0; idx < total_elem_count; ++idx)
        {
            value = *(cells + idx);
            countNeighbours = *(neighbours + idx);
            if (value == STATE_ALIVE)
            {
                // cell is alive -> check diese if less than 2 or more than 3 neighbours
                if (countNeighbours < 2 || countNeighbours > 3)
                {
                    *(cells + idx) = STATE_DEAD;
                }
                // else // Ah, ha, ha, ha, stayin' alive, stayin' alive!
            }
            else if (countNeighbours == 3) // cell was dead and has enough neighbours -> newborn <3
            {
                *(cells + idx) = STATE_ALIVE;
            }
        }
    }

    Timing::getInstance()->stopComputation();

    // write out result
    Timing::getInstance()->startFinalization();
    ompWriteToFile(fileO);
    //writeToFile(fileO, true); // writes count of neighbours instead of just x / .
    Timing::getInstance()->stopFinalization();
}