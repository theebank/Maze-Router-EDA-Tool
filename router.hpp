#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <queue>
#include <unordered_set>
#include <set>
#include <algorithm>

#ifndef GRID_HPP
#define GRID_HPP

#include <vector>
#include <string>

class Grid
{
public:
    // Graph D.S. to represent tracks
    std::vector<std::vector<std::string>> adjacencyMatrix; // adjacency matrix for track segments
    std::vector<std::vector<int>> costadjacencyMatrix;

    /*
        No Connection - 'X'
        Connection but unused (Available) - '$'
        Connection and used (Unavailable) - 'U'
        Target of search/expansion - 'T'
        Available and adjacent to driver - 'O'
    */
    int N;                                       // NxN grid (N Vertices)
    int W;                                       // W Channels
    std::vector<std::vector<int>> i_connections; // input connections

    int currEven = 0;
    int currOdd = 1;

    // Methods
    void initializeGraph();
    void takeTrackSegment(int block1x, int block1y, int block2x, int block2y, int tracknum);
    void takeFirstTrackSegment(int block1, int block2, int connectionNum, int segmentNum);
    void processFile(const std::string &path);
    int processConnection(std::vector<int> connection, int connectionnum);
    void mazeRouter();
    void resetMatrix();
    void completeResetMatrix();
    void printInputFile();
    void printMatrix();
};

#endif // GRID_HPP
