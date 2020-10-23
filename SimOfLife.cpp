// --------------------------------------------
// without structs:
// Results (Release x64):
// -----
// computation : 7043.44ms - 7394.76ms
// finalization : 871.228ms - 917.633ms
// setup : 330.595ms - 482.229ms

// Results (Release x86):
// -----
// computation :  8229.94ms - 8373.14ms
// finalization : 822.842ms - 876.652ms
// setup : 517.811ms - 546.773ms

// --------------------------------------------
// with structs:
// not even close...
// --------------------------------------------

// TODOs:
// - optimizations :p
// - data oriented design -> split bool value and neighbours in favor of tmp buffer to read from
// - read cmd args with getopt port

// major bottleneck optimizations:
// - faster memcpy for structs -> dod

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
//#define USE_STRUCT // using a struct which contains ptr to all neighbours so don't have to be calculated every setCell step
//#define NO_IFS // ~6800ms, with normal ifs about 200 - 300 ms faster

#include "Timing.h"
#include "omp.h" // need to have project settings C/C++ openMP enabled

// each cell is represented as a byte:
//      LSB is state 0 = dead, 1... alive
//      other bits are number of neighbours

#define STATE_ALIVE 0x01

unsigned int w;
unsigned int h;
unsigned int total_elem_count;
int right_border;
int bot_border;
unsigned int generations = 250;
unsigned char* cells;

#ifdef USE_STRUCT
struct Cell
{
    char value = 0;
    // save ptr to all neighbours
    Cell* topLeft = nullptr;
    Cell* top = nullptr;
    Cell* topRight = nullptr;
    Cell* left = nullptr;
    Cell* right = nullptr;
    Cell* botLeft = nullptr;
    Cell* bot = nullptr;
    Cell* botRight = nullptr;
};
Cell* CELLS;
#endif

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
    int xOffLeft = right_border*(x == 0) + -1*(x != 0);
    int xOffRight = (-right_border*(x == right_border)) + 1*(x != right_border);
    int yOffTop = bot_border*(y == 0) + (-(int)w*(y != 0));
    int yOffBot = (y == (h - 1)) ? -bot_border : w; // need to cast to int because of negative sign
#else
    int xOffLeft = (x == 0) ? right_border : -1;
    int xOffRight = (x == right_border) ? -right_border : 1;
    int yOffTop = (y == 0) ? bot_border : -(int)w;
    int yOffBot = (y == (h - 1)) ? -bot_border : w; // need to cast to int because of negative sign
#endif

    // add bits for neighbour counts
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

#ifdef USE_STRUCT
// minor modifications for struct: using +1 to add, -1 to sub, 0 just clears the value
inline void setCellState(Cell* ptr_cell, int alive)
{
    // set cell value
    //if (alive > 0)        ptr_cell->value |= STATE_ALIVE;
    //else if (alive < 0)   ptr_cell->value &= ~STATE_ALIVE;
    ptr_cell->value ^= STATE_ALIVE; // just toggle -> no if, saves about 500 ms

    // add bits for neighbour counts
    int val = (alive > 0) * 0x02 + (alive < 0) * -0x02;
    ptr_cell->topLeft->value += val;
    ptr_cell->top->value += val;
    ptr_cell->topRight->value += val;
    ptr_cell->left->value += val;
    ptr_cell->right->value += val;
    ptr_cell->botLeft->value += val;
    ptr_cell->bot->value += val;
    ptr_cell->botRight->value += val;
}
#endif

void readFromFile(const char* filePath)
{
    std::cout << "read file: " << filePath << "..." << std::endl;
    std::ifstream in(filePath);
    if (in.is_open())
    {
        std::string line;
        std::getline(in, line);
        std::cout << "read line " << line << std::endl;
        // TODO: better split
        unsigned int pos = line.find(',');
        if (pos != std::string::npos)
        {
            w = std::stoi(line.substr(0, pos));
            h = std::stoi(line.substr(pos + 1));
        }
        else // use default values
        {
            w = 1000;
            h = 250;
        }

        total_elem_count = w * h;
        right_border = w - 1;
        bot_border = total_elem_count - w;
        std::cout << "total: " << total_elem_count << ", w: " << w << ", h: " << h << " gen: " << generations << std::endl;

#ifdef USE_STRUCT
        CELLS = new Cell[total_elem_count];
        //memset(cells, 0, total_elem_count); // not needed if init all values
#else
        cells = new unsigned char[total_elem_count];
        memset(cells, 0, total_elem_count);
#endif

        unsigned int idx = 0;
        char c;
        for (unsigned int y = 0; y < h; y++)
        {
            getline(in, line);
            for (unsigned int x = 0; x < w; x++)
            {
                c = line[x];
#ifdef USE_STRUCT
                // for struct need to setup neighbours
                Cell* cell = CELLS + idx;

                int xOffLeft = (x == 0) ? w - 1 : -1;
                int xOffRight = (x == (w - 1)) ? -(w - 1) : 1;
                int yOffTop = (y == 0) ? total_elem_count - w : -w;
                int yOffBot = (y == (h - 1)) ? -((int)total_elem_count - w) : w; // need to cast to int because of negative sign

                cell->topLeft = (cell + yOffTop + xOffLeft);
                cell->top = (cell + yOffTop);
                cell->topRight = (cell + yOffTop + xOffRight);
                cell->left = (cell + xOffLeft);
                cell->right = (cell + xOffRight);
                cell->botLeft = (cell + yOffBot + xOffLeft);
                cell->bot = (cell + yOffBot);
                cell->botRight = (cell + yOffBot + xOffRight);

                if (c == 'x')  // only need to set alive cells, otherwise stay 0
                {
                    setCellState(cell, 1);
                }
#else
                //std::cout << "read c: " << c << " for index: " << idx << std::endl;
                if (c == 'x')  // only need to set alive cells, otherwise stay 0
                {
                    setCellState(cells + idx, x, y, 1);
                }
#endif

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
        out << w << ", " << h << std::endl;
        for (unsigned int i = 0; i < total_elem_count; ++i)
        {
#ifdef USE_STRUCT
            if (drawNeighbours) out << (((CELLS + i)->value >> 1) + 0);
            else out << (((CELLS + i)->value & STATE_ALIVE) ? "x" : ".");
#else
            if (drawNeighbours) out << ((cells[i] >> 1) + 0);
            else out << ((cells[i] & STATE_ALIVE) ? "x" : ".");
#endif

            if (i % w == w - 1) out << std::endl;
        }
    }
    else std::cout << "Error opening " << filePath << std::endl;

    out.close();
}

int main(int argc, char** argv)
{
    // parse command line options
    // --load NAME(where NAME is a filename with the extension ’.gol’)
    // --generations NUM
    // --save NAME(where NAME is a filename with the extension ’.gol’)
    // --measure (generates measurement output on stdout)
    // --mode seq (extended in Project 2)

    // TODO: use getopt (unix "only" but simple function)
    // https://github.com/skandhurkat/Getopt-for-Visual-Studio/blob/master/getopt.h
    std::cout << "args: " << argc << std::endl;
    const char* fileI = "random10000_in.gol";
    std::string filename = "out" + std::to_string(generations) + ".out";
    const char* fileO = filename.c_str();
    bool printMeasure = true;
    for (int i = 0; i < argc; ++i)
    {
        //std::cout << "\t" << argv[i] << std::endl;
        if (i < (argc + 1))
        {
            //if (argv[i] == "--load") --> no comparison between const char * and std::string
            if (strcmp(argv[i], "--load") == 0) fileI = argv[i + 1];
            else if (strcmp(argv[i], "--save") == 0) fileO = argv[i + 1];
            else if (strcmp(argv[i], "--generations") == 0) generations = std::stoi(argv[i + 1]);
            else if (strcmp(argv[i], "--measure") == 0) printMeasure = true;
            /* TODO: exercise 2
            else if (argv[i] == "--mode" && i < (argc + 1))*/
        }
    }

    // init grid from file
    Timing::getInstance()->startSetup();
    // ------------------------------------------------------
    //readFromFile("mini.gol");
    //writeToFile("mini.neighbors.out", true);
    //writeToFile("mini.out");
    // ------------------------------------------------------
    //readFromFile("random250_in.gol");
    //writeToFile("random250_afterLoad.neighbours.out", true);
    //writeToFile("random250_afterLoad.out");
    // ------------------------------------------------------
    //readFromFile("random2000_in.gol");
    // ------------------------------------------------------
    readFromFile(fileI);

    // make a temp copy of cells to read from without interfering
#ifdef USE_STRUCT
    Cell* oldCells = new Cell[total_elem_count];
    memcpy(oldCells, CELLS, total_elem_count * sizeof(Cell)); // need set neighbours
#else
    unsigned char* oldCells = new unsigned char[total_elem_count];
#endif
    Timing::getInstance()->stopSetup();

    // ------------------------------------------------------
    // test
    /*generations = 100;
    w = 10;
    h = 20;
    total_elem_count = w * h;
    std::cout << "total: " << total_elem_count << ", w: " << w << ", h: " << h << " gen: " << generations << std::endl;

    cells = new unsigned char[total_elem_count];
    memset(cells, 0, total_elem_count);
    */
    // ------------------------------------------------------
    /*
    setCellState(cells, total_elem_count, 0);
    setCellState(cells, total_elem_count, 41);
    setCellState(cells, total_elem_count, 42);
    setCellState(cells, total_elem_count, 43);
    */
    // ------------------------------------------------------
    // 4-8-12 diamond
    /*
    unsigned int idx = 14 + w;
    for (unsigned int i = 0; i < 4; ++i)  // 4
        setCellState(cells, total_elem_count, idx++);

    idx += 2 * w; // newline
    idx -= 4 + 2;
    for (unsigned int i = 0; i < 8; ++i)  // 8
        setCellState(cells, total_elem_count, idx++);

    idx += 2 * w; // newline
    idx -= 8 + 2;
    for (unsigned int i = 0; i < 12; ++i)  // 12
        setCellState(cells, total_elem_count, idx++);

    idx += 2 * w; // newline
    idx -= 12 - 2;
    for (unsigned int i = 0; i < 8; ++i)  // 8
        setCellState(cells, total_elem_count, idx++);

    idx += 2 * w; // newline
    idx -= 8 - 2;
    for (unsigned int i = 0; i < 4; ++i)  // 4
        setCellState(cells, total_elem_count, idx++);
    */
    // ------------------------------------------------------

#ifdef SHOW_GENS
    printCells();
#endif

    unsigned int gen = 0;
#ifdef USE_STEPS
    std::string str;
#endif

    int nthreads, tid;
#pragma omp parallel private(nthreads, tid) // arguments are shared variables
    {
        // ... tid =
        nthreads = omp_get_num_threads();
        std::cout << "Number of threads: " << nthreads << std::endl;
    } // threads join master thread and disband

    Timing::getInstance()->startComputation();
    while (gen < generations)
    {
        // copy current state in oldstate
#ifdef USE_STRUCT
        memcpy(oldCells, CELLS, total_elem_count * sizeof(Cell)); // for 10 Mio cells this takes about 15 seconds, on x64 almost 30 O_O
        //std::swap(oldCells, CELLS); // swap could improve performance bc just switch working array but is (yet) not usable because setting neighbours as diff
#else
        memcpy(oldCells, cells, total_elem_count);
        //std::swap(oldCells, cells); // swap could improve performance bc just switch working array but is (yet) not usable because setting neighbours as diff
#endif

        // calculate next step
        // version 1: just set a random cell
        //setCellState(cells, total_elem_count, rand() % total_elem_count);

        // version 2: set next cell each generation
        //setCellState(cells, total_elem_count, gen);

        // version 3: change cells dependent on oldCells
        unsigned int idx = 0;
        char value = 0;
        unsigned int countNeighbours = 0;
        //for (unsigned int i = 0; i < total_elem_count; i++) ~ 10 sec
        for (unsigned int j = 0; j < h; j++) // ~ 9,760 sec
        {
            for (unsigned int i = 0; i < w; i++)
            {
                // TODO: directly use char * instead of array access?

                idx = i + (j * w); // fastest way
                //idx++; // in every continue
                //if (i > 0 || j > 0) idx++;
#ifdef USE_STRUCT
                value = (oldCells + idx)->value;
#else
                value = oldCells[idx];
#endif
                //if (value == 0) continue; // this should be the main performance gain as it skips most of the cells after some time 8,253 - 8,538
                                            // but without this if, execution time is even faster oO 7,258 - 7,494
                                            // --> which means, as every cell has to be touched no need for memcpying the whole array

                countNeighbours = (value >> 1);

                // refactored to not using ifs and set value to 1 (new) -1 (die) 0 (let)
                // --> is a few seconds slower than ifs
                /*bool born = !(value & STATE_ALIVE) && (countNeighbours == 3); // Ah, ha, ha, ha, stayin' alive, stayin' alive!
                bool die = (value & STATE_ALIVE) && !(countNeighbours == 2 || countNeighbours == 3); // x_x
                setCellState(cells + idx, i, j, (born * 1 + die * -1) != 0);*/

                if (value & STATE_ALIVE) {
                    // cell is alive -> check if dies (less than 2 or more than 3 neighbours)
                    if (countNeighbours == 2 || countNeighbours == 3) continue; // Ah, ha, ha, ha, stayin' alive, stayin' alive!

#ifdef USE_STRUCT
                    setCellState((CELLS + idx), -1); // x_x
#else
                    setCellState(cells + idx, i, j, false); // x_x
#endif
                }
                else if (countNeighbours == 3) // cell was dead and has enough neighbours -> newborn <3
                {
#ifdef USE_STRUCT
                    setCellState((CELLS + idx), +1);
#else
                    setCellState(cells + idx, i, j, true);
#endif
                }
            }
        }

#ifdef SHOW_GENS
        printCells();
#endif

#ifdef USE_STEPS
        std::getline(std::cin, str);
#endif
        gen++;

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

    if (printMeasure)
    {
        Timing::getInstance()->print();
    }

    return EXIT_SUCCESS;
}