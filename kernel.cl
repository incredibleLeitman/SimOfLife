
/*
* a kernel that simply prints the (2-dimensional) thread id
* processes no data
*
__kernel void hello(void)
{
	int tidX = get_global_id(0);
	int tidY = get_global_id(1);
	printf("hi from thread (%d,%d) \n", tidX, tidY);
}*/

/*
* a kernel that add the elements of two vectors pairwise
*
__kernel void vector_add(
	__global const int *A,
	__global const int *B,
	__global int *C)
{
	int i = get_global_id(0);
	C[i] = A[i] + B[i];
}*/

/*
* a kernel for game of life generation
*/
void kernel gol_generation(global int* board, global int* tempBoard, int useTemp, int width, int height)
{
    int id = get_global_id(0);
    int x = id % width;
    int y = id / width;

    int y_ = y - 1;
    y_ += (y_ < 0) * height;
    int x_ = x - 1;
    x_ += (x_ < 0) * width;
    int y1 = y + 1;
    y1 -= (y1 >= height) * height;
    int x1 = x + 1;
    x1 -= (x1 >= width) * width;

    if (useTemp > 0)
    {
        int livingNeighbors = tempBoard[id];
        livingNeighbors += tempBoard[x_ + width * y_];
        livingNeighbors += tempBoard[x + width * y_];
        livingNeighbors += tempBoard[x1 + width * y_];

        livingNeighbors += tempBoard[x_ + width * y];
        livingNeighbors += tempBoard[x1 + width * y];

        livingNeighbors += tempBoard[x_ + width * y1];
        livingNeighbors += tempBoard[x + width * y1];
        livingNeighbors += tempBoard[x1 + width * y1];

        board[id] = (livingNeighbors == 3 + tempBoard[id] * (livingNeighbors == 4)) > 0;
    }
    else
    {
        int livingNeighbors = board[id];
        livingNeighbors += board[x_ + width * y_];
        livingNeighbors += board[x + width * y_];
        livingNeighbors += board[x1 + width * y_];

        livingNeighbors += board[x_ + width * y];
        livingNeighbors += board[x1 + width * y];

        livingNeighbors += board[x_ + width * y1];
        livingNeighbors += board[x + width * y1];
        livingNeighbors += board[x1 + width * y1];

        tempBoard[id] = (livingNeighbors == 3 + board[id] * (livingNeighbors == 4)) > 0;
    }

    /*
    // need to get current neighbour count, because other than seqMode updates are not diffs but full states
    // first handle all cells without border mapping, afterwards special handling
    for (row = 0; row < height; row++)
    {
        yOffTop = (row == 0) ? col_bot : -(int)w;
        yOffBot = (row == (h - 1)) ? -col_bot : w;

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
    for (idx = 0; idx < elems; ++idx)
    {
        value = *(cells + idx);
        countNeighbours = *(neighbours + idx);

        // alternative option: always set value
        * (cells + idx) = (countNeighbours == 3) + value * (countNeighbours == 2);
    }
    */
}