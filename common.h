#pragma once

#define STATE_DEAD  0x00
#define STATE_ALIVE 0x01

unsigned int w;
unsigned int h;
unsigned int total_elem_count;
int col_right;      // defines col index of right border (w - 1)
int col_bot;        // defines col index of bot border
int row_bot;        // defines last row (h - 1)

// in seqMode each cell is represented as a byte:
//      LSB is state 0 = dead, 1... alive
//      other bits are number of neighbours
unsigned char* cells;
unsigned char* oldCells;    // used in seqMode as buffer
int* neighbours;  // ompMode data centric design

bool debugOutput = false; // flag for console output