/*
* a kernel for game of life generation
*/
void kernel gol_generation(global unsigned char* board, global unsigned char* cache, unsigned int width, unsigned int height)
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

    int livingNeighbors =
        board[id] + // v1 -> adding current value
        board[x_ + width * y_] +
        board[x + width * y_] +
        board[x1 + width * y_] +
        board[x_ + width * y] +
        board[x1 + width * y] +
        board[x_ + width * y1] +
        board[x + width * y1] +
        board[x1 + width * y1];

    cache[id] = (livingNeighbors == 3) + board[id] * (livingNeighbors == 4); // v1 -> ok 87 - 99
    //cache[id] = (livingNeighbors == 3) + board[id] * (livingNeighbors == 2); // v2 -> dont add curVal: 97 - 103
}