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
    std::vector<std::vector<std::vector<int>>> costadjacencyMatrix;

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

    // Swappable Input specifics
    std::vector<std::vector<int>> i_orientations;
    /*
        i_orientations[0] = 1 connection on a1 paper or down direction
        i_orientations[1] = 2 connection on a1 paper or left direction
        i_orientations[2] = 3 connection on a1 paper or up direction
    */

    // Methods
    void initializeGraph();
    void takeTrackSegment(int block1x, int block1y, int block2x, int block2y, int tracknum);
    void takeFirstTrackSegment(int block1, int block2, int connectionNum, int segmentNum);
    void processFile(const std::string &path);
    int processConnection(std::vector<int> connection, int connectionnum);
    std::vector<int> si_processConnection(std::vector<int> connection, int connectionnum);
    int si_processConnectionWrapper(std::vector<int> connection, int connectionnum);
    void si_resetConnection(int connectionnum);
    void mazeRouter();
    void si_mazeRouter();
    void si_initializeOrientations();
    void resetMatrix();
    void completeResetMatrix();
    void printInputFile();
    void printMatrix();
};

#endif // GRID_HPP
