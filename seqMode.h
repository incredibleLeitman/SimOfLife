#pragma once

/* ---------------------------------------------------------------------------
sequential mode:
run on only one thread sequential; to minimize footprint, actual cell value
and count of alive neighbours are stored in char.

initial idea was, that in each step only cells with alive state have an impact
on the generation, but calculaton and store was very imperformant.

--------------------------------------------------------------------------- */

#include "common.h"

void printCells()
{
#ifdef _WIN32
    system("CLS"); // TODO: remove windows specific
#endif

    for (unsigned int i = 0; i < total_elem_count; ++i)
    {
        if (i % w == 0) std::cout << std::endl;

#ifdef DEBUG_OUT
        char val = cells[i];
        if (val == 0) std::cout << ".";
        else if (val & STATE_ALIVE) std::cout << "x"; // cell is alive
        else std::cout << ((val >> 1) + 0); // cell is dead and has x neighbours
#else
        std::cout << ((cells[i] & STATE_ALIVE) ? "x" : ".");
#endif
    }
}

inline void setCellState(unsigned char* ptr_cell, unsigned int x, unsigned int y, bool alive = true)
{
    // set cell value
    *(ptr_cell) ^= STATE_ALIVE; // just toggle -> no if, saves about 500 ms

    // calculate neighbours -> wrap-around at borders: { 0, 0 } is a neighbor of { m, n } on a m x n sized grid
    // offsets in x,y direction
#ifdef NO_IFS
    int xOffLeft = col_right * (x == 0) + -1 * (x != 0);
    int xOffRight = (-col_right * (x == col_right)) + 1 * (x != col_right);
    int yOffTop = col_bot * (y == 0) + (-(int)w * (y != 0));
    int yOffBot = (y == (h - 1)) ? -col_bot : w; // need to cast to int because of negative sign
#else
    int xOffLeft = (x == 0) ? col_right : -1;
    int xOffRight = (x == col_right) ? -col_right : 1;
    int yOffTop = (y == 0) ? col_bot : -(int)w;
    int yOffBot = (y == (h - 1)) ? -col_bot : w; // need to cast to int because of negative sign
#endif

    // add bits for neighbour counts -> performs a diff!
    int val = (alive) * 0x02 + (!alive) * -0x02;
    *(ptr_cell + yOffTop + xOffLeft) += val;
    *(ptr_cell + yOffTop) += val;
    *(ptr_cell + yOffTop + xOffRight) += val;
    *(ptr_cell + xOffLeft) += val;
    *(ptr_cell + xOffRight) += val;
    *(ptr_cell + yOffBot + xOffLeft) += val;
    *(ptr_cell + yOffBot) += val;
    *(ptr_cell + yOffBot + xOffRight) += val;
}

void readFromFile(const char* filePath)
{
    std::cout << "read file: " << filePath << "..." << std::endl;
    std::ifstream in(filePath);
    if (in.is_open())
    {
        std::string line;
        std::getline(in, line); // get first line for explizit w / h definition

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
        std::cout << "total: " << total_elem_count << ", w: " << w << ", h: " << h << std::endl;

        cells = new unsigned char[total_elem_count];
        memset(cells, 0, total_elem_count);

        unsigned int idx = 0;
        char c;
        for (unsigned int y = 0; y < h; y++)
        {
            std::getline(in, line);
            for (unsigned int x = 0; x < w; x++)
            {
                c = line[x];
                //std::cout << "read c: " << c << " for index: " << idx << std::endl;
                if (c == 'x')  // only need to set alive cells, otherwise stay 0
                {
                    setCellState(cells + idx, x, y, 1);
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

void writeToFile(const char* filePath, bool drawNeighbours = false)
{
    std::cout << "write file: " << filePath << "..." << std::endl;
    std::ofstream out(filePath);
    if (out.is_open())
    {
        out << w << "," << h << std::endl;
        for (unsigned int i = 0; i < total_elem_count; ++i)
        {
            if (drawNeighbours) out << ((cells[i] >> 1) + 0);
            else out << ((cells[i] & STATE_ALIVE) ? "x" : ".");

            if (i % w == w - 1) out << std::endl;
        }
    }
    else std::cout << "Error opening " << filePath << std::endl;

    out.close();
}

void runSeq(const char* fileI, const char* fileO, unsigned int generations)
{
#ifdef _DEBUG
    std::cout << "DEBUG" << std::endl;
#endif
    std::cout << "running mode: seq" << std::endl;

    // init grid from file
    Timing::getInstance()->startSetup();
    readFromFile(fileI);

    // make a copy of cells to read from without interfering with current board
    oldCells = new unsigned char[total_elem_count];

#ifdef SHOW_GENS
    printCells();
#endif

    unsigned int gen = 0;
#ifdef USE_STEPS
    std::string str;
#endif

    int row = 0;
    int col = 0;
    unsigned int idx = 0;
    char value = 0;
    unsigned int countNeighbours = 0;
    Timing::getInstance()->stopSetup();

    // actual sim loop
    Timing::getInstance()->startComputation();
    for (gen = 0; gen < generations; gen++)
    {
        // copy current state in oldstate
        memcpy(oldCells, cells, total_elem_count);

        //std::swap(oldCells, cells); // swap ptrs instead of memcpy could improve performance bc just switch working array but is (yet) not usable because setting neighbours as diff
        //*oldCells = *cells;
        //memset(oldCells, 0, total_elem_count);

        // change cells dependent on oldCells
//#pragma omp parallel for private(row, col) //shared(cells, oldCells) // with this able to reduce runtime down to 4 sec for set_threads(8)
        for (row = 0; row < h; row++)
        {
            for (col = 0; col < w; col++)
            {
                //idx++; // in every continue
                //if (i > 0 || j > 0) idx++;
                idx = col + (row * w); // fastest way

                value = *(oldCells + idx);
                //if (value == 0) continue; // this should be the main performance gain as it skips most of the cells after some time
                                            // but without this if, execution time is even faster oO
                                            // --> which means, as every cell has to be touched no need for memcpying the whole array but handling only diffs

                countNeighbours = (value >> 1);

                // refactored to not using ifs and set value to 1 (new) -1 (die) 0 (let)
                // --> is a few seconds slower than ifs
                /*bool born = !(value & STATE_ALIVE) && (countNeighbours == 3); // Ah, ha, ha, ha, stayin' alive, stayin' alive!
                bool die = (value & STATE_ALIVE) && !(countNeighbours == 2 || countNeighbours == 3); // x_x
                setCellState(cells + idx, i, j, (born * 1 + die * -1) != 0);*/

                // set new state depending on current state, only if changed
                if (value & STATE_ALIVE)
                {
                    // cell is alive -> check diese if less than 2 or more than 3 neighbours
                    if (countNeighbours < 2 || countNeighbours > 3)
                    {
                        setCellState(cells + idx, col, row, false); // x_x
                    }
                    // else // Ah, ha, ha, ha, stayin' alive, stayin' alive!
                }
                else if (countNeighbours == 3) // cell was dead and has enough neighbours -> newborn <3
                {
                    setCellState(cells + idx, col, row, true);
                }

                // alternative option: always set value (not applicable in this version because handling depending diffs)
                //setCellState(cells + idx, col, row, (countNeighbours == 3) + (value & STATE_ALIVE) * (countNeighbours == 2));
            }
        }

#ifdef SHOW_GENS
        printCells();
#endif

#ifdef USE_STEPS
        std::getline(std::cin, str);
#endif

        /*if (gen == 1 || gen == 10 || gen == 100 || gen == 250 || gen == 500 || gen == 1000)
        {
            std::string filename = "out" + std::to_string(gen) + "neighbors.out";
            writeToFile(filename.c_str(), true);
            filename = "out" + std::to_string(gen) + ".out";
            writeToFile(filename.c_str());
        }*/
    }
    Timing::getInstance()->stopComputation();

    // write out result
    Timing::getInstance()->startFinalization();
    writeToFile(fileO);
    //writeToFile(fileO, true); // writes count of neighbours instead of just x / .
    Timing::getInstance()->stopFinalization();
}