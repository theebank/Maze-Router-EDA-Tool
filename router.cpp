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
#include "router.hpp"
using namespace ::std;

// int main()
// {
//     Grid g;

//     g.processFile("/Users/theebankumaresan/Documents/School/ECE1387/lab1/tests/cct1.txt");

//     g.initializeGraph();

//     g.printMatrix();
//     // g.mazeRouter();
//     g.si_mazeRouter();
//     cout << "----------" << endl;
//     // g.printMatrix();

//     return 0;
// }

void Grid::initializeGraph()
{
    // Initialize empty adjacency matrix with no connections between nodes
    int numSwitchBoxes = ((N + 1)) * ((N + 1));
    adjacencyMatrix.resize(numSwitchBoxes, vector<string>(numSwitchBoxes, "X"));
    costadjacencyMatrix = vector<vector<vector<int>>>((N + 1) * (N + 1), vector<vector<int>>((N + 1) * (N + 1), vector<int>(W, 2147483647)));
    // connect switch boxes in square pattern
    for (int i = 0; i <= N; i++)
    {
        for (int j = 0; j <= N; j++)
        {
            int curr = ((N - i) * (N + 1)) + j;
            // left connection
            if (j > 0)
            {
                int left = curr - 1;
                adjacencyMatrix[curr][left] = string(W, '$');
                adjacencyMatrix[left][curr] = string(W, '$');
            }
            // right connection
            if (j < N)
            {
                int right = curr + 1;
                adjacencyMatrix[curr][right] = string(W, '$');
                adjacencyMatrix[right][curr] = string(W, '$');
            }
            // bottom connection
            if (i < N)
            {
                int bot = curr - N - 1;
                adjacencyMatrix[curr][bot] = string(W, '$');
                adjacencyMatrix[bot][curr] = string(W, '$');
            }
            // top connection
            if (i > 0)
            {
                int top = curr + N + 1;
                adjacencyMatrix[curr][top] = string(W, '$');
                adjacencyMatrix[top][curr] = string(W, '$');
            }
        }
    }
}

void Grid::takeTrackSegment(int block1x, int block1y, int block2x, int block2y, int tracknum)
{
    int block1 = ((block1x) * (N + 1)) + block1y;
    int block2 = ((block2x) * (N + 1)) + block2y;
    if (abs(block1 - block2) > 1)
    {
        cerr << "No existing track segment" << endl;
        return;
    }
    string temp = adjacencyMatrix[block1][block2];
    temp[tracknum - 1] = 'U';

    adjacencyMatrix[block1][block2] = temp;
    adjacencyMatrix[block2][block1] = temp;
}

void Grid::takeFirstTrackSegment(int block1, int block2, int connectionNum, int segmentNum)
{
    string temp = adjacencyMatrix[block1][block2];
    temp[segmentNum] = '0' + connectionNum;
    Grid::adjacencyMatrix[block1][block2] = temp;
    Grid::adjacencyMatrix[block2][block1] = temp;
}

void Grid::processFile(const string &path)
{

    ifstream infile(path);
    if (!infile)
    {
        cerr << "Error: could not open file" << endl;
        return;
    }
    infile >> N;
    infile >> W;
    string line;
    getline(infile, line);

    while (getline(infile, line))
    {
        stringstream ss(line);
        vector<int> values;
        int value;

        while (ss >> value)
        {
            values.push_back(value);
        }
        if (values.size() == 6 && values[0] == -1 && values[1] == -1)
        {
            break;
        }
        i_connections.push_back(values);
    }
    infile.close();
}

int Grid::processConnection(vector<int> connection, int connectionnum)
{

    vector<int> driverPin = {connection[0], connection[1], connection[2]};
    vector<int> loadPin = {connection[3], connection[4], connection[5]};
    queue<vector<int>> expansionList = queue<vector<int>>(); // {Block1, Block2, cost}
    set<vector<int>> visited = set<vector<int>>();
    vector<vector<pair<int, int>>> parents((N + 1) * (N + 1), vector<pair<int, int>>((N + 1) * (N + 1), {-1, -1}));
    costadjacencyMatrix = vector<vector<vector<int>>>((N + 1) * (N + 1), vector<vector<int>>((N + 1) * (N + 1), vector<int>(W, 2147483647)));
    vector<vector<int>>
        mincostadjacencyMatrix = vector<vector<int>>((N + 1) * (N + 1), vector<int>((N + 1) * (N + 1), 2147483647));
    string track;

    int shortestcost = 2147483647;
    int tracknum = 0;

    // mark available track segments adjacent to load as 'T'
    /*
        Find connected group of track

        Switch box to the bottom left of block would have same x,y coords as the connection block

    */

    int Lblock1, Lblock2;
    int Dblock1, Dblock2;
    if (loadPin[2] == 1)
    { // Check down track
        Lblock1 = ((loadPin[1]) * (N + 1)) + loadPin[0];
        Lblock2 = ((loadPin[1]) * (N + 1)) + loadPin[0] + 1;
        track = adjacencyMatrix[Lblock1][Lblock2];
    }
    else if (loadPin[2] == 2)
    { // Check left track
        Lblock1 = ((loadPin[1]) * (N + 1)) + loadPin[0];
        Lblock2 = ((loadPin[1] + 1) * (N + 1)) + loadPin[0];
        track = adjacencyMatrix[Lblock1][Lblock2];
    }
    else if (loadPin[2] == 3)
    { // Check up track
        Lblock1 = ((loadPin[1] + 1) * (N + 1)) + loadPin[0];
        Lblock2 = ((loadPin[1] + 1) * (N + 1)) + loadPin[0] + 1;
        track = adjacencyMatrix[Lblock1][Lblock2];
    }
    if (loadPin[2] == 2)
    { // Pin can only connect to W/2 odd tracks(1,3,5,etc);
        for (int i = 1; i < track.length(); i += 2)
        {
            if (track[i] == '$')
            {
                track[i] = 'T';
            }
        }
        adjacencyMatrix[Lblock1][Lblock2] = track;
        adjacencyMatrix[Lblock2][Lblock1] = track;
    }
    else if (loadPin[2] == 1 || loadPin[2] == 3)
    { // Pins 1 and 3 can only connect to W/2 even tracks(0,2,4,etc);
        for (int i = 0; i < track.length(); i += 2)
        {
            if (track[i] == '$')
            {
                track[i] = 'T';
            }
        }
        adjacencyMatrix[Lblock1][Lblock2] = track;
        adjacencyMatrix[Lblock2][Lblock1] = track;
    }

    // Mark available track segments adjacent to driver as O
    Dblock1 = ((driverPin[1]) * (N + 1)) + driverPin[0] + 1;
    Dblock2 = ((driverPin[1] + 1) * (N + 1)) + driverPin[0] + 1;

    track = adjacencyMatrix[Dblock1][Dblock2];
    for (int i = 0; i < track.length(); i++)
    {
        if (track[i] == '$')
        {
            track[i] = 'O';
            adjacencyMatrix[Dblock1][Dblock2] = track;
            adjacencyMatrix[Dblock2][Dblock1] = track;
            // then put these track segments on FIFO Q
            expansionList.push({Dblock1, Dblock2, i});
            costadjacencyMatrix[Dblock1][Dblock2][i] = 1;
            costadjacencyMatrix[Dblock2][Dblock1][i] = 1;
        }
        if (track[i] == 'T')
        {

            takeFirstTrackSegment(Dblock1, Dblock2, connectionnum + 1, i);
            cout << "(" << Dblock1 << "," << Dblock2 << ")" << endl;
            return 1;
        }
    }

    int jBlock1, jBlock2, jcurrBlock;
    int jLabel;
    bool breakExpansionList = false;
    vector<int> segmentJ;
    breakExpansionList = false;
    while (!expansionList.empty())
    {

        // 2 track num
        // 3 cost
        if (breakExpansionList)
        {
            if (costadjacencyMatrix[Lblock1][Lblock2][segmentJ[2]] != 2147483647 || costadjacencyMatrix[Lblock2][Lblock1][segmentJ[2]] != 2147483647)
            {
                int temp = min(costadjacencyMatrix[Lblock1][Lblock2][segmentJ[2]], costadjacencyMatrix[Lblock2][Lblock1][segmentJ[2]]);
                shortestcost = temp;
                tracknum = segmentJ[2];
            }
            break;
        }
        // pop seg j from fifo
        segmentJ = expansionList.front();
        // for each seg k that connects to j thru switch
        vector<vector<int>> ksegs;
        // expansion phase

        for (int i = 0; i < 2; i++)
        {
            jcurrBlock = segmentJ[i];
            if ((jcurrBlock % (N + 1)) != 0)
            { // check if along left column
                if ((visited.find({jcurrBlock, jcurrBlock - 1, segmentJ[2]}) != visited.end()) || visited.find({jcurrBlock - 1, jcurrBlock, segmentJ[2]}) != visited.end())
                {
                }
                else
                {
                    if (adjacencyMatrix[jcurrBlock][jcurrBlock - 1][segmentJ[2]] != 'X')
                    {
                        ksegs.push_back({jcurrBlock, jcurrBlock - 1, segmentJ[2]});
                        visited.insert({jcurrBlock, jcurrBlock - 1, segmentJ[2]});
                    }
                }
            }
            if (((jcurrBlock + 1) % (N + 1)) != 0)
            { // check if along right column
                if ((visited.find({jcurrBlock, jcurrBlock + 1, segmentJ[2]}) != visited.end()) || visited.find({jcurrBlock + 1, jcurrBlock, segmentJ[2]}) != visited.end())
                {
                }
                else
                {
                    if (adjacencyMatrix[jcurrBlock][jcurrBlock + 1][segmentJ[2]] != 'X')
                    {
                        ksegs.push_back({jcurrBlock, jcurrBlock + 1, segmentJ[2]});
                        visited.insert({jcurrBlock, jcurrBlock + 1, segmentJ[2]});
                    }
                }
            }
            if (jcurrBlock > (N + 1))
            { // check if along bottom row
                if ((visited.find({jcurrBlock, jcurrBlock - (N + 1), segmentJ[2]}) != visited.end()) || visited.find({jcurrBlock - (N + 1), jcurrBlock, segmentJ[2]}) != visited.end())
                {
                }
                else
                {
                    if (adjacencyMatrix[jcurrBlock][jcurrBlock - (N + 1)][segmentJ[2]] != 'X')
                    {
                        ksegs.push_back({jcurrBlock, jcurrBlock - (N + 1), segmentJ[2]});
                        visited.insert({jcurrBlock, jcurrBlock - (N + 1), segmentJ[2]});
                    }
                }
            }
            if (jcurrBlock < ((N + 1) * N))
            { // check if along top row
                if ((visited.find({jcurrBlock, jcurrBlock + (N + 1), segmentJ[2]}) != visited.end()) || visited.find({jcurrBlock + (N + 1), jcurrBlock, segmentJ[2]}) != visited.end())
                {
                }
                else
                {
                    if (adjacencyMatrix[jcurrBlock][jcurrBlock + (N + 1)][segmentJ[2]] != 'X')
                    {
                        ksegs.push_back({jcurrBlock, jcurrBlock + (N + 1), segmentJ[2]});
                        visited.insert({jcurrBlock, jcurrBlock + (N + 1), segmentJ[2]});
                    }
                }
            }
        }
        for (auto k : ksegs)
        {
            if (!((k[0] == segmentJ[0] && k[1] == segmentJ[1]) || (k[0] == segmentJ[1] && k[1] == segmentJ[0])))
            {
                if (adjacencyMatrix[k[0]][k[1]][segmentJ[2]] == 'T' || adjacencyMatrix[k[1]][k[0]][segmentJ[2]] == 'T')
                {
                    // if k is target, exit while loop
                    costadjacencyMatrix[k[0]][k[1]][segmentJ[2]] = costadjacencyMatrix[segmentJ[0]][segmentJ[1]][segmentJ[2]] + 1;
                    costadjacencyMatrix[k[1]][k[0]][segmentJ[2]] = costadjacencyMatrix[segmentJ[1]][segmentJ[0]][segmentJ[2]] + 1;
                    breakExpansionList = true;
                    tracknum = segmentJ[2];
                    break;
                }
                else if (adjacencyMatrix[k[0]][k[1]][segmentJ[2]] != '$' || adjacencyMatrix[k[1]][k[0]][segmentJ[2]] != '$')
                {
                    // else if k unavailable or already landed ignore
                }
                else
                {
                    // else
                    // label k with label (j) +=1
                    costadjacencyMatrix[k[0]][k[1]][segmentJ[2]] = costadjacencyMatrix[segmentJ[0]][segmentJ[1]][segmentJ[2]] + 1;
                    costadjacencyMatrix[k[1]][k[0]][segmentJ[2]] = costadjacencyMatrix[segmentJ[1]][segmentJ[0]][segmentJ[2]] + 1;
                    // put k on expansion list
                    expansionList.push({k[0], k[1], segmentJ[2]});
                }
            }
        }
        expansionList.pop();
    }
    if (shortestcost != 2147483647)
    {
        vector<vector<int>> shortestpath;
        int cost = shortestcost;
        vector<int> currBlock = {Lblock1, Lblock2};
        shortestpath.push_back({Lblock1, Lblock2});
        while (cost != 1)
        {
            for (int i = 0; i < 2; i++)
            {
                jcurrBlock = currBlock[i];
                if ((jcurrBlock % (N + 1)) != 0)
                { // check if along left column

                    if (costadjacencyMatrix[jcurrBlock][jcurrBlock - 1][tracknum] == (cost - 1) || costadjacencyMatrix[jcurrBlock - 1][jcurrBlock][tracknum] == (cost - 1))
                    {

                        shortestpath.push_back({jcurrBlock, jcurrBlock - 1});
                        currBlock[i] -= 1;
                        cost--;
                    }
                }
                if (((jcurrBlock + 1) % (N + 1)) != 0)
                { // check if along right column
                    if (costadjacencyMatrix[jcurrBlock][jcurrBlock + 1][tracknum] == (cost - 1) || costadjacencyMatrix[jcurrBlock][jcurrBlock + 1][tracknum] == (cost - 1))
                    {
                        shortestpath.push_back({jcurrBlock, jcurrBlock + 1});
                        currBlock[i] += 1;
                        cost--;
                    }
                }
                if (jcurrBlock > (N + 1))
                { // check if along bottom row
                    if (costadjacencyMatrix[jcurrBlock][jcurrBlock - (N + 1)][tracknum] == (cost - 1) || costadjacencyMatrix[jcurrBlock - (N - 1)][jcurrBlock][tracknum] == (cost - 1))
                    {
                        shortestpath.push_back({jcurrBlock, jcurrBlock - (N + 1)});
                        currBlock[i] -= (N + 1);
                        cost--;
                    }
                }

                if (jcurrBlock < ((N + 1) * N))
                { // check if along top row
                    if (costadjacencyMatrix[jcurrBlock][jcurrBlock + (N + 1)][tracknum] == (cost - 1) || costadjacencyMatrix[jcurrBlock + (N + 1)][jcurrBlock][tracknum] == (cost - 1))
                    {

                        shortestpath.push_back({jcurrBlock, jcurrBlock + N + 1});
                        currBlock[i] += (N + 1);
                        cost--;
                    }
                }
            }
        }
        for (auto i : shortestpath)
        {
            takeFirstTrackSegment(i[0], i[1], connectionnum, tracknum);
        }
        return shortestcost;
    }
    else
    {
        return -1;
    }
}

/*
    If while loop ends without target hit:
        - some connection could not route -> can try again with problem connection moved to front of list
        - rip up and re-route
*/
int Grid::si_processConnectionWrapper(std::vector<int> connection, int connectionnum)
{

    // {1,2,3},{1,3,2},{2,1,3},{2,3,1},{3,1,2},{3,2,1}
    vector<vector<int>> possibleorientations = {{1, 2, 3}, {1, 3, 2}, {2, 3, 1}, {2, 1, 3}, {3, 1, 2}, {3, 2, 1}};
    std::vector<int> newconnection = connection;
    int lowest = 2147483647;
    vector<int> res;
    int track;
    int determinedslot = -1;
    if (i_orientations[(connection[3] * (N + 1)) + connection[4]] != vector<int>(3, -1))
    {
        if (newconnection[5] == 3)
        {
            newconnection[5] = i_orientations[(connection[3] * (N + 1)) + connection[4]][2];
        }
        else if (newconnection[5] == 2)
        {
            newconnection[5] = i_orientations[(connection[3] * (N + 1)) + connection[4]][1];
        }
        else if (newconnection[5] == 1)
        {
            newconnection[5] = i_orientations[(connection[3] * (N + 1)) + connection[4]][0];
        }
        res = si_processConnection(newconnection, connectionnum);
    }
    else
    {

        for (int i = 0; i < 3; i++)
        {
            if (newconnection[5] == 3)
            {
                newconnection[5] = possibleorientations[i][2];
            }
            else if (newconnection[5] == 2)
            {
                newconnection[5] = possibleorientations[i][1];
            }
            else if (newconnection[5] == 1)
            {
                newconnection[5] = possibleorientations[i][0];
            }

            si_resetConnection(connectionnum);
            res = si_processConnection(newconnection, connectionnum);
            if (res[0] == -1)
            {
                if (newconnection[5] == 3)
                {
                    newconnection[5] = possibleorientations[i + 3][2];
                }
                else if (newconnection[5] == 2)
                {
                    newconnection[5] = possibleorientations[i + 3][1];
                }
                else if (newconnection[5] == 1)
                {
                    newconnection[5] = possibleorientations[i + 3][0];
                }
                si_resetConnection(connectionnum);
                res = si_processConnection(newconnection, connectionnum);
            }
            if (res[0] < lowest)
            {
                i_orientations[(connection[3] * (N + 1)) + connection[4]] = possibleorientations[i];
                lowest = res[0];
            };
        }
    }
    if (res[0] == -1)
    {
        return res[0];
    }
    vector<vector<int>> shortestpath;
    int jcurrBlock;
    int cost = res[0];
    vector<int> currBlock = {res[2], res[3]};
    shortestpath.push_back({res[2], res[3]});
    while (cost != 1)
    {
        for (int i = 0; i < 2; i++)
        {
            jcurrBlock = currBlock[i];
            if ((jcurrBlock % (N + 1)) != 0)
            { // check if along left column

                if (costadjacencyMatrix[jcurrBlock][jcurrBlock - 1][res[1]] == (cost - 1) || costadjacencyMatrix[jcurrBlock - 1][jcurrBlock][res[1]] == (cost - 1))
                {

                    shortestpath.push_back({jcurrBlock, jcurrBlock - 1});
                    currBlock[i] -= 1;
                    cost--;
                }
            }
            if (((jcurrBlock + 1) % (N + 1)) != 0)
            { // check if along right column
                if (costadjacencyMatrix[jcurrBlock][jcurrBlock + 1][res[1]] == (cost - 1) || costadjacencyMatrix[jcurrBlock][jcurrBlock + 1][res[1]] == (cost - 1))
                {
                    shortestpath.push_back({jcurrBlock, jcurrBlock + 1});
                    currBlock[i] += 1;
                    cost--;
                }
            }
            if (jcurrBlock > (N + 1))
            { // check if along bottom row
                if (costadjacencyMatrix[jcurrBlock][jcurrBlock - (N + 1)][res[1]] == (cost - 1) || costadjacencyMatrix[jcurrBlock - (N - 1)][jcurrBlock][res[1]] == (cost - 1))
                {
                    shortestpath.push_back({jcurrBlock, jcurrBlock - (N + 1)});
                    currBlock[i] -= (N + 1);
                    cost--;
                }
            }

            if (jcurrBlock < ((N + 1) * N))
            { // check if along top row
                if (costadjacencyMatrix[jcurrBlock][jcurrBlock + (N + 1)][res[1]] == (cost - 1) || costadjacencyMatrix[jcurrBlock + (N + 1)][jcurrBlock][res[1]] == (cost - 1))
                {

                    shortestpath.push_back({jcurrBlock, jcurrBlock + N + 1});
                    currBlock[i] += (N + 1);
                    cost--;
                }
            }
        }
    }

    for (auto i : shortestpath)
    {
        cout << "(" << i[0] << "," << i[1] << ")" << endl;
        takeFirstTrackSegment(i[0], i[1], connectionnum, res[1]);
    }
    i_connections[connectionnum] = newconnection;
    return res[0];
}
vector<int> Grid::si_processConnection(std::vector<int> connection, int connectionnum)
{

    vector<int> driverPin = {connection[0], connection[1], connection[2]};
    vector<int> loadPin = {connection[3], connection[4], connection[5]};
    queue<vector<int>> expansionList = queue<vector<int>>(); // {Block1, Block2, cost}
    set<vector<int>> visited = set<vector<int>>();
    vector<vector<pair<int, int>>> parents((N + 1) * (N + 1), vector<pair<int, int>>((N + 1) * (N + 1), {-1, -1}));
    costadjacencyMatrix = vector<vector<vector<int>>>((N + 1) * (N + 1), vector<vector<int>>((N + 1) * (N + 1), vector<int>(W, 2147483647)));
    vector<vector<int>>
        mincostadjacencyMatrix = vector<vector<int>>((N + 1) * (N + 1), vector<int>((N + 1) * (N + 1), 2147483647));
    string track;

    int shortestcost = 2147483647;
    int tracknum = 0;

    // mark available track segments adjacent to load as 'T'
    /*
        Find connected group of track

        Switch box to the bottom left of block would have same x,y coords as the connection block

    */

    int Lblock1, Lblock2;
    int Dblock1, Dblock2;
    if (loadPin[2] == 1)
    { // Check down track
        Lblock1 = ((loadPin[1]) * (N + 1)) + loadPin[0];
        Lblock2 = ((loadPin[1]) * (N + 1)) + loadPin[0] + 1;
        track = adjacencyMatrix[Lblock1][Lblock2];
    }
    else if (loadPin[2] == 2)
    { // Check left track
        Lblock1 = ((loadPin[1]) * (N + 1)) + loadPin[0];
        Lblock2 = ((loadPin[1] + 1) * (N + 1)) + loadPin[0];
        track = adjacencyMatrix[Lblock1][Lblock2];
    }
    else if (loadPin[2] == 3)
    { // Check up track
        Lblock1 = ((loadPin[1] + 1) * (N + 1)) + loadPin[0];
        Lblock2 = ((loadPin[1] + 1) * (N + 1)) + loadPin[0] + 1;
        track = adjacencyMatrix[Lblock1][Lblock2];
    }
    if (loadPin[2] == 2)
    { // Pin can only connect to W/2 odd tracks(1,3,5,etc);
        for (int i = 1; i < track.length(); i += 2)
        {
            if (track[i] == '$')
            {
                track[i] = 'T';
            }
        }
        adjacencyMatrix[Lblock1][Lblock2] = track;
        adjacencyMatrix[Lblock2][Lblock1] = track;
    }
    else if (loadPin[2] == 1 || loadPin[2] == 3)
    { // Pins 1 and 3 can only connect to W/2 even tracks(0,2,4,etc);
        for (int i = 0; i < track.length(); i += 2)
        {
            if (track[i] == '$')
            {
                track[i] = 'T';
            }
        }
        adjacencyMatrix[Lblock1][Lblock2] = track;
        adjacencyMatrix[Lblock2][Lblock1] = track;
    }

    // Mark available track segments adjacent to driver as O
    Dblock1 = ((driverPin[1]) * (N + 1)) + driverPin[0] + 1;
    Dblock2 = ((driverPin[1] + 1) * (N + 1)) + driverPin[0] + 1;

    track = adjacencyMatrix[Dblock1][Dblock2];
    for (int i = 0; i < track.length(); i++)
    {
        if (track[i] == '$')
        {
            track[i] = 'O';
            adjacencyMatrix[Dblock1][Dblock2] = track;
            adjacencyMatrix[Dblock2][Dblock1] = track;
            // then put these track segments on FIFO Q
            expansionList.push({Dblock1, Dblock2, i});
            costadjacencyMatrix[Dblock1][Dblock2][i] = 1;
            costadjacencyMatrix[Dblock2][Dblock1][i] = 1;
        }
        if (track[i] == 'T')
        {

            takeFirstTrackSegment(Dblock1, Dblock2, connectionnum + 1, i);
            cout << "(" << Dblock1 << "," << Dblock2 << ")" << endl;
            return {1};
        }
    }

    int jBlock1, jBlock2, jcurrBlock;
    int jLabel;
    bool breakExpansionList = false;
    vector<int> segmentJ;
    breakExpansionList = false;
    while (!expansionList.empty())
    {

        // 2 track num
        // 3 cost
        if (breakExpansionList)
        {
            if (costadjacencyMatrix[Lblock1][Lblock2][segmentJ[2]] != 2147483647 || costadjacencyMatrix[Lblock2][Lblock1][segmentJ[2]] != 2147483647)
            {
                int temp = min(costadjacencyMatrix[Lblock1][Lblock2][segmentJ[2]], costadjacencyMatrix[Lblock2][Lblock1][segmentJ[2]]);
                shortestcost = temp;
                tracknum = segmentJ[2];
                return {shortestcost, tracknum, Lblock1, Lblock2};
            }
            break;
        }
        // pop seg j from fifo
        segmentJ = expansionList.front();
        // for each seg k that connects to j thru switch
        vector<vector<int>> ksegs;
        // expansion phase

        for (int i = 0; i < 2; i++)
        {
            jcurrBlock = segmentJ[i];
            if ((jcurrBlock % (N + 1)) != 0)
            { // check if along left column
                if ((visited.find({jcurrBlock, jcurrBlock - 1, segmentJ[2]}) != visited.end()) || visited.find({jcurrBlock - 1, jcurrBlock, segmentJ[2]}) != visited.end())
                {
                }
                else
                {
                    if (adjacencyMatrix[jcurrBlock][jcurrBlock - 1][segmentJ[2]] != 'X')
                    {
                        ksegs.push_back({jcurrBlock, jcurrBlock - 1, segmentJ[2]});
                        visited.insert({jcurrBlock, jcurrBlock - 1, segmentJ[2]});
                    }
                }
            }
            if (((jcurrBlock + 1) % (N + 1)) != 0)
            { // check if along right column
                if ((visited.find({jcurrBlock, jcurrBlock + 1, segmentJ[2]}) != visited.end()) || visited.find({jcurrBlock + 1, jcurrBlock, segmentJ[2]}) != visited.end())
                {
                }
                else
                {
                    if (adjacencyMatrix[jcurrBlock][jcurrBlock + 1][segmentJ[2]] != 'X')
                    {
                        ksegs.push_back({jcurrBlock, jcurrBlock + 1, segmentJ[2]});
                        visited.insert({jcurrBlock, jcurrBlock + 1, segmentJ[2]});
                    }
                }
            }
            if (jcurrBlock > (N + 1))
            { // check if along bottom row
                if ((visited.find({jcurrBlock, jcurrBlock - (N + 1), segmentJ[2]}) != visited.end()) || visited.find({jcurrBlock - (N + 1), jcurrBlock, segmentJ[2]}) != visited.end())
                {
                }
                else
                {
                    if (adjacencyMatrix[jcurrBlock][jcurrBlock - (N + 1)][segmentJ[2]] != 'X')
                    {
                        ksegs.push_back({jcurrBlock, jcurrBlock - (N + 1), segmentJ[2]});
                        visited.insert({jcurrBlock, jcurrBlock - (N + 1), segmentJ[2]});
                    }
                }
            }
            if (jcurrBlock < ((N + 1) * N))
            { // check if along top row
                if ((visited.find({jcurrBlock, jcurrBlock + (N + 1), segmentJ[2]}) != visited.end()) || visited.find({jcurrBlock + (N + 1), jcurrBlock, segmentJ[2]}) != visited.end())
                {
                }
                else
                {
                    if (adjacencyMatrix[jcurrBlock][jcurrBlock + (N + 1)][segmentJ[2]] != 'X')
                    {
                        ksegs.push_back({jcurrBlock, jcurrBlock + (N + 1), segmentJ[2]});
                        visited.insert({jcurrBlock, jcurrBlock + (N + 1), segmentJ[2]});
                    }
                }
            }
        }
        for (auto k : ksegs)
        {
            if (!((k[0] == segmentJ[0] && k[1] == segmentJ[1]) || (k[0] == segmentJ[1] && k[1] == segmentJ[0])))
            {
                if (adjacencyMatrix[k[0]][k[1]][segmentJ[2]] == 'T' || adjacencyMatrix[k[1]][k[0]][segmentJ[2]] == 'T')
                {
                    // if k is target, exit while loop
                    costadjacencyMatrix[k[0]][k[1]][segmentJ[2]] = costadjacencyMatrix[segmentJ[0]][segmentJ[1]][segmentJ[2]] + 1;
                    costadjacencyMatrix[k[1]][k[0]][segmentJ[2]] = costadjacencyMatrix[segmentJ[1]][segmentJ[0]][segmentJ[2]] + 1;
                    breakExpansionList = true;
                    tracknum = segmentJ[2];
                    break;
                }
                else if (adjacencyMatrix[k[0]][k[1]][segmentJ[2]] != '$' || adjacencyMatrix[k[1]][k[0]][segmentJ[2]] != '$')
                {
                    // else if k unavailable or already landed ignore
                }
                else
                {
                    // else
                    // label k with label (j) +=1
                    costadjacencyMatrix[k[0]][k[1]][segmentJ[2]] = costadjacencyMatrix[segmentJ[0]][segmentJ[1]][segmentJ[2]] + 1;
                    costadjacencyMatrix[k[1]][k[0]][segmentJ[2]] = costadjacencyMatrix[segmentJ[1]][segmentJ[0]][segmentJ[2]] + 1;
                    // put k on expansion list
                    expansionList.push({k[0], k[1], segmentJ[2]});
                }
            }
        }
        expansionList.pop();
    }

    return {-1};
}

void Grid::si_resetConnection(int connectionnum)
{
    for (int i = 0; i < adjacencyMatrix.size(); i++)
    {
        for (int j = 0; j < adjacencyMatrix[i].size(); j++)
        {
            for (int x = 0; x < adjacencyMatrix[i][j].size(); x++)
            {
                if (adjacencyMatrix[i][j][x] == 'T' || adjacencyMatrix[i][j][x] == 'O' || adjacencyMatrix[i][j][x] == (connectionnum + '0'))
                {
                    adjacencyMatrix[i][j][x] = '$';
                }
            }
        }
    }
}

void Grid::mazeRouter()
{
    int i = 0;
    int totalsegments = 0;
    while (i < i_connections.size())
    {
        try
        {
            int success = processConnection(i_connections[i], i);
            if (success == -1)
            {
                throw runtime_error("Connection could not route");
            }
            totalsegments += success;
            cout << "connection " << i << ", (" << i_connections[i][0] << i_connections[i][1] << "|" << success << ", was successfully made." << endl;
            i++;
            resetMatrix();
        }
        catch (runtime_error e)
        {
            cout << "connection " << i << ", (" << i_connections[i][0] << i_connections[i][1] << ") was unsuccessfully made." << endl;
            vector<int> failedConn = i_connections[i];
            i_connections.erase(i_connections.begin() + i);
            i_connections.insert(i_connections.begin(), failedConn);
            i = 0;
            totalsegments = 0;
            completeResetMatrix();
        }
    }
    cout << to_string(totalsegments) << " total segments were made." << endl;
    resetMatrix();
}

void Grid::si_mazeRouter()
{
    si_initializeOrientations();
    int i = 0;
    int totalsegments = 0;
    while (i < i_connections.size())
    {
        try
        {
            int success = si_processConnectionWrapper(i_connections[i], i);

            if (success == -1)
            {
                throw runtime_error("Connection could not route");
            }
            totalsegments += success;
            cout << "connection " << i << ", (" << i_connections[i][0] << i_connections[i][1] << "|" << success << ", was successfully made." << endl;
            i++;
            resetMatrix();
        }
        catch (runtime_error e)
        {
            cout << "connection " << i << ", (" << i_connections[i][0] << i_connections[i][1] << ") was unsuccessfully made." << endl;
            vector<int> failedConn = i_connections[i];
            i_connections.erase(i_connections.begin() + i);
            i_connections.insert(i_connections.begin(), failedConn);
            i = 0;
            totalsegments = 0;
            completeResetMatrix();
            si_initializeOrientations();
        }
    }
    cout << to_string(totalsegments) << " total segments were made." << endl;
    resetMatrix();
}
void Grid::si_initializeOrientations()
{
    i_orientations = vector<vector<int>>(((N + 1) * (N + 1)), vector<int>(3, -1));
}
void Grid::resetMatrix()
{
    for (int i = 0; i < adjacencyMatrix.size(); i++)
    {
        for (int j = 0; j < adjacencyMatrix[i].size(); j++)
        {
            for (int x = 0; x < adjacencyMatrix[i][j].size(); x++)
            {
                if (adjacencyMatrix[i][j][x] == 'T' || adjacencyMatrix[i][j][x] == 'O')
                {
                    adjacencyMatrix[i][j][x] = '$';
                }
            }
        }
    }
}

void Grid::completeResetMatrix()
{
    for (int i = 0; i < adjacencyMatrix.size(); i++)
    {
        for (int j = 0; j < adjacencyMatrix[i].size(); j++)
        {
            for (int x = 0; x < adjacencyMatrix[i][j].size(); x++)
            {
                if (adjacencyMatrix[i][j][x] != 'X')
                {
                    Grid::adjacencyMatrix[i][j] = string(W, '$');
                }
            }
        }
    }
}

void Grid::printInputFile()
{
    cout << "Total Connections: " << i_connections.size() << endl;
    cout << N << " x " << N << endl;
    cout << W << " tracks per channel" << endl;
    for (auto i = 0; i < i_connections.size(); i++)
    {
        for (auto j : i_connections[i])
        {
            cout << j;
        }
        cout << endl;
    }
}
void Grid::printMatrix()
{
    for (auto row : adjacencyMatrix)
    {
        for (auto val : row)
        {
            cout << val << " ";
        }
        cout << endl;
    }
}
