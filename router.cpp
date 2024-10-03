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

int main()
{
    Grid g;

    g.processFile("/Users/theebankumaresan/Documents/School/ECE1387/lab1/tests/cct1.txt");

    g.initializeGraph();

    g.printMatrix();
    g.mazeRouter();
    cout << "----------" << endl;
    g.printMatrix();

    return 0;
}

void Grid::initializeGraph()
{
    // Initialize empty adjacency matrix with no connections between nodes
    int numSwitchBoxes = ((N + 1)) * ((N + 1));
    adjacencyMatrix.resize(numSwitchBoxes, vector<string>(numSwitchBoxes, "X"));
    costadjacencyMatrix.resize(numSwitchBoxes, vector<int>(numSwitchBoxes, 0));
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
    // int block1 = ((N - block1x) * (N + 1)) + block1y;
    int block1 = ((block1x) * (N + 1)) + block1y;
    // int block2 = ((N - block2x) * (N + 1)) + block2y;
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
    adjacencyMatrix[block1][block2] = temp;
    adjacencyMatrix[block2][block1] = temp;
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
    vector<vector<int>> costadjacencyMatrix = vector<vector<int>>((N + 1) * (N + 1), vector<int>((N + 1) * (N + 1), INT_MAX));
    vector<vector<int>> mincostadjacencyMatrix = vector<vector<int>>((N + 1) * (N + 1), vector<int>((N + 1) * (N + 1), INT_MAX));
    string track;

    int shortestcost = INT_MAX;
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
    // visited.insert({Dblock1, Dblock2});
    track = adjacencyMatrix[Dblock1][Dblock2];
    for (int i = 0; i < track.length(); i++)
    {
        if (track[i] == '$')
        {
            track[i] = 'O';
            adjacencyMatrix[Dblock1][Dblock2] = track;
            adjacencyMatrix[Dblock2][Dblock1] = track;
            // then put these track segments on FIFO Q
            expansionList.push({Dblock1, Dblock2, loadPin[2]});
            costadjacencyMatrix[Dblock1][Dblock2] = 1;
            costadjacencyMatrix[Dblock2][Dblock1] = 1;
        }
        if (track[i] == 'T')
        {
            // cout << loadPin[2] << endl;
            takeFirstTrackSegment(Dblock1, Dblock2, connectionnum + 1, i);
            cout << "(" << Dblock1 << "," << Dblock2 << ")" << endl;
            return 1;
        }
    }

    int jBlock1, jBlock2, jcurrBlock;
    int jLabel;
    bool breakExpansionList = false;
    if (loadPin[2] == 1 || loadPin[2] == 3)
    {
        for (int currEven = 0; currEven <= W; currEven += 2)
        {
            breakExpansionList = false;
            while (!expansionList.empty())
            {
                if (breakExpansionList)
                {
                    break;
                }
                // pop seg j from fifo
                vector<int> segmentJ = expansionList.front();
                // for each seg k that connects to j thru switch
                vector<vector<int>> ksegs;
                // expansion phase
                for (int i = 0; i < 2; i++)
                {
                    jcurrBlock = segmentJ[i];
                    if ((jcurrBlock % (N + 1)) != 0)
                    { // check if along left column
                        if ((visited.find({jcurrBlock, jcurrBlock - 1}) != visited.end()) || visited.find({jcurrBlock - 1, jcurrBlock}) != visited.end())
                        {
                        }
                        else
                        {
                            if (adjacencyMatrix[jcurrBlock][jcurrBlock - 1][currEven] != 'X')
                            {
                                ksegs.push_back({jcurrBlock, jcurrBlock - 1});
                                visited.insert({jcurrBlock, jcurrBlock - 1});
                            }
                        }
                    }
                    if (((jcurrBlock + 1) % (N + 1)) != 0)
                    { // check if along right column
                        if ((visited.find({jcurrBlock, jcurrBlock + 1}) != visited.end()) || visited.find({jcurrBlock + 1, jcurrBlock}) != visited.end())
                        {
                        }
                        else
                        {
                            if (adjacencyMatrix[jcurrBlock][jcurrBlock + 1][currEven] != 'X')
                            {
                                ksegs.push_back({jcurrBlock, jcurrBlock + 1});
                                visited.insert({jcurrBlock, jcurrBlock + 1});
                            }
                        }
                    }
                    if (jcurrBlock > (N + 1))
                    { // check if along bottom row
                        if ((visited.find({jcurrBlock, jcurrBlock - (N + 1)}) != visited.end()) || visited.find({jcurrBlock - (N + 1), jcurrBlock}) != visited.end())
                        {
                        }
                        else
                        {
                            if (adjacencyMatrix[jcurrBlock][jcurrBlock - (N + 1)][currEven] != 'X')
                            {
                                ksegs.push_back({jcurrBlock, jcurrBlock - (N + 1)});
                                visited.insert({jcurrBlock, jcurrBlock - (N + 1)});
                            }
                        }
                    }
                    if (jcurrBlock < ((N + 1) * N))
                    { // check if along top row
                        if ((visited.find({jcurrBlock, jcurrBlock + (N + 1)}) != visited.end()) || visited.find({jcurrBlock + (N + 1), jcurrBlock}) != visited.end())
                        {
                        }
                        else
                        {
                            if (adjacencyMatrix[jcurrBlock][jcurrBlock + (N + 1)][currEven] != 'X')
                            {
                                ksegs.push_back({jcurrBlock, jcurrBlock + (N + 1)});
                                visited.insert({jcurrBlock, jcurrBlock + (N + 1)});
                            }
                        }
                    }
                }
                for (auto k : ksegs)
                {
                    if (!((k[0] == segmentJ[0] && k[1] == segmentJ[1]) || (k[0] == segmentJ[1] && k[1] == segmentJ[0])))
                    {
                        parents[k[0]][k[1]] = make_pair(segmentJ[0], segmentJ[1]);
                        parents[k[1]][k[0]] = make_pair(segmentJ[0], segmentJ[1]);
                        costadjacencyMatrix[k[0]][k[1]] = min(costadjacencyMatrix[k[0]][k[1]], costadjacencyMatrix[segmentJ[0]][segmentJ[1]] + 1);
                        costadjacencyMatrix[k[1]][k[0]] = min(costadjacencyMatrix[k[1]][k[0]], costadjacencyMatrix[segmentJ[1]][segmentJ[0]] + 1);
                        if (adjacencyMatrix[k[0]][k[1]][currEven] == 'T')
                        {
                            // if k is target, exit while loop
                            breakExpansionList = true;
                            break;
                            // loadpin[0], loadpin[1] = end pair Lblock1, Lblock2 = end
                        }
                        // else if (adjacencyMatrix[k[0]][k[1]][currEven] == 'U' || isdigit(adjacencyMatrix[k[0]][k[1]][currEven]))
                        else if (adjacencyMatrix[k[0]][k[1]][currEven] != '$')
                        {
                            // else if k unavailable or already landed ignore
                            continue;
                        }
                        else
                        {
                            // else
                            // label k with label (j) +=1
                            // put k on expansion list
                            parents[k[0]][k[1]] = {segmentJ[0], segmentJ[1]};
                            expansionList.push({k[0], k[1], segmentJ[2] + 1});
                        }
                    }
                }
                expansionList.pop();
            }
            if (costadjacencyMatrix[Lblock1][Lblock2] != INT_MAX || costadjacencyMatrix[Lblock2][Lblock1] != INT_MAX)
            {
                int temp = min(costadjacencyMatrix[Lblock1][Lblock2], costadjacencyMatrix[Lblock2][Lblock1]);
                if (temp < shortestcost)
                {
                    cout << currEven << endl;
                    shortestcost = temp;
                    tracknum = currEven;
                    mincostadjacencyMatrix = costadjacencyMatrix;
                    // cout << shortestcost << endl;
                }
            }
        }
    }
    else if (loadPin[2] == 2)
    {
        for (int currOdd = 1; currOdd <= W; currOdd += 2)
        {
            breakExpansionList = false;
            while (!expansionList.empty())
            {
                if (breakExpansionList)
                {
                    break;
                }
                // pop seg j from fifo
                vector<int> segmentJ = expansionList.front();
                // for each seg k that connects to j thru switch
                vector<vector<int>> ksegs;
                for (int i = 0; i < 2; i++)
                {
                    jcurrBlock = segmentJ[i];
                    if ((jcurrBlock % (N + 1)) != 0)
                    { // check if along left column
                        if ((visited.find({jcurrBlock, jcurrBlock - 1}) != visited.end()) || visited.find({jcurrBlock - 1, jcurrBlock}) != visited.end())
                        {
                        }
                        else
                        {
                            if (adjacencyMatrix[jcurrBlock][jcurrBlock - 1][currOdd] != 'X')
                            {
                                ksegs.push_back({jcurrBlock, jcurrBlock - 1});
                                visited.insert({jcurrBlock, jcurrBlock - 1});
                            }
                        }
                    }
                    if (((jcurrBlock + 1) % (N + 1)) != 0)
                    { // check if along right column
                        if ((visited.find({jcurrBlock, jcurrBlock + 1}) != visited.end()) || visited.find({jcurrBlock + 1, jcurrBlock}) != visited.end())
                        {
                        }
                        else
                        {
                            if (adjacencyMatrix[jcurrBlock][jcurrBlock + 1][currOdd] != 'X')
                            {
                                ksegs.push_back({jcurrBlock, jcurrBlock + 1});
                                visited.insert({jcurrBlock, jcurrBlock + 1});
                            }
                        }
                    }
                    if (jcurrBlock > (N + 1))
                    { // check if along bottom row
                        if ((visited.find({jcurrBlock, jcurrBlock - (N + 1)}) != visited.end()) || visited.find({jcurrBlock - (N + 1), jcurrBlock}) != visited.end())
                        {
                        }
                        else
                        {
                            if (adjacencyMatrix[jcurrBlock][jcurrBlock - (N + 1)][currOdd] != 'X')
                            {
                                ksegs.push_back({jcurrBlock, jcurrBlock - (N + 1)});
                                visited.insert({jcurrBlock, jcurrBlock - (N + 1)});
                            }
                        }
                    }
                    if (jcurrBlock < ((N + 1) * N))
                    { // check if along top row
                        if ((visited.find({jcurrBlock, jcurrBlock + (N + 1)}) != visited.end()) || visited.find({jcurrBlock + (N + 1), jcurrBlock}) != visited.end())
                        {
                        }
                        else
                        {
                            if (adjacencyMatrix[jcurrBlock][jcurrBlock + (N + 1)][currOdd] != 'X')
                            {
                                ksegs.push_back({jcurrBlock, jcurrBlock + (N + 1)});
                                visited.insert({jcurrBlock, jcurrBlock + (N + 1)});
                            }
                        }
                    }
                }
                for (auto k : ksegs)
                {
                    if (!((k[0] == segmentJ[0] && k[1] == segmentJ[1]) || (k[0] == segmentJ[1] && k[1] == segmentJ[0])))
                    {
                        parents[k[0]][k[1]] = make_pair(segmentJ[0], segmentJ[1]);
                        parents[k[1]][k[0]] = make_pair(segmentJ[0], segmentJ[1]);
                        costadjacencyMatrix[k[0]][k[1]] = min(costadjacencyMatrix[k[0]][k[1]], costadjacencyMatrix[segmentJ[0]][segmentJ[1]] + 1);
                        costadjacencyMatrix[k[1]][k[0]] = min(costadjacencyMatrix[k[1]][k[0]], costadjacencyMatrix[segmentJ[1]][segmentJ[0]] + 1);
                        if (adjacencyMatrix[k[0]][k[1]][currOdd] == 'T')
                        {
                            // if k is target, exit while loop
                            breakExpansionList = true;
                            break;
                            // if k is target, exit while loop

                            // loadpin[0],loadpin[1] = end pair
                            // Lblock1, Lblock2 = end
                        }
                        // else if (adjacencyMatrix[k[0]][k[1]][currOdd] == 'U' || isdigit(adjacencyMatrix[k[0]][k[1]][currOdd]))
                        else if (adjacencyMatrix[k[0]][k[1]][currOdd] != '$')
                        {
                            // else if k unavailable or already landed ignore
                            continue;
                        }
                        else
                        {
                            // else
                            // label k with label (j) +=1
                            // put k on expansion list
                            costadjacencyMatrix[k[0]][k[1]] = costadjacencyMatrix[segmentJ[0]][segmentJ[1]] + 1;
                            parents[k[0]][k[1]] = {segmentJ[0], segmentJ[1]};
                            expansionList.push({k[0], k[1], segmentJ[2] + 1});
                        }
                    }
                }
                expansionList.pop();
            }
            if (costadjacencyMatrix[Lblock1][Lblock2] != INT_MAX || costadjacencyMatrix[Lblock2][Lblock1] != INT_MAX)
            {
                int temp = min(costadjacencyMatrix[Lblock1][Lblock2], costadjacencyMatrix[Lblock2][Lblock1]);
                if (temp < shortestcost)
                {
                    cout << currOdd << endl;
                    shortestcost = temp;
                    tracknum = currOdd;
                    mincostadjacencyMatrix = costadjacencyMatrix;
                }
            }
        }
    }
    if (shortestcost != INT_MAX)
    {
        vector<vector<int>> shortestpath;
        int cost = shortestcost;
        vector<int> currBlock = {Lblock1, Lblock2};
        shortestpath.push_back({Lblock1, Lblock2});
        cout << N + 1 << "--N" << endl;

        // for (auto row : mincostadjacencyMatrix)
        // {
        //     for (auto val : row)
        //     {
        //         cout << val << " ";
        //     }
        //     cout << endl;
        // }
        while (cost != 1)
        // for (int j = 0; j < 3; j++)
        {
            for (int i = 0; i < 2; i++)
            {
                jcurrBlock = currBlock[i];
                // cout << mincostadjacencyMatrix[jcurrBlock][jcurrBlock + N + 1] << endl;
                if ((jcurrBlock % (N + 1)) != 0)
                { // check if along left column

                    // if (adjacencyMatrix[jcurrBlock][jcurrBlock - 1][currOdd] != 'X')
                    if (mincostadjacencyMatrix[jcurrBlock][jcurrBlock - 1] == (cost - 1) || mincostadjacencyMatrix[jcurrBlock - 1][jcurrBlock] == (cost - 1))
                    {

                        // cout << mincostadjacencyMatrix[jcurrBlock][jcurrBlock - 1] << "Left.." << jcurrBlock;
                        shortestpath.push_back({jcurrBlock, jcurrBlock - 1});
                        currBlock[i] -= 1;
                        // cout << jcurrBlock << endl;
                        cost--;
                    }
                }
                if (((jcurrBlock + 1) % (N + 1)) != 0)
                { // check if along right column
                  // if (adjacencyMatrix[jcurrBlock][jcurrBlock + 1][currOdd] != 'X')
                    if (mincostadjacencyMatrix[jcurrBlock][jcurrBlock + 1] == (cost - 1) || mincostadjacencyMatrix[jcurrBlock][jcurrBlock + 1] == (cost - 1))
                    {
                        // cout << mincostadjacencyMatrix[jcurrBlock][jcurrBlock + 1] << "Right.." << jcurrBlock;
                        shortestpath.push_back({jcurrBlock, jcurrBlock + 1});
                        currBlock[i] += 1;
                        // cout << jcurrBlock << endl;
                        cost--;
                    }
                }
                if (jcurrBlock > (N + 1))
                { // check if along bottom row
                    // if (adjacencyMatrix[jcurrBlock][jcurrBlock - (N + 1)][currOdd] != 'X')
                    if (mincostadjacencyMatrix[jcurrBlock][jcurrBlock - (N + 1)] == (cost - 1) || mincostadjacencyMatrix[jcurrBlock - (N - 1)][jcurrBlock] == (cost - 1))
                    {
                        // cout << mincostadjacencyMatrix[jcurrBlock][jcurrBlock - 1] << "down.." << jcurrBlock;
                        shortestpath.push_back({jcurrBlock, jcurrBlock - (N + 1)});
                        currBlock[i] -= (N + 1);
                        // cout << jcurrBlock << endl;
                        cost--;
                    }
                }

                if (jcurrBlock < ((N + 1) * N))
                { // check if along top row
                  // if (adjacencyMatrix[jcurrBlock][jcurrBlock + (N + 1)][currOdd] != 'X')
                    if (mincostadjacencyMatrix[jcurrBlock][jcurrBlock + (N + 1)] == (cost - 1) || mincostadjacencyMatrix[jcurrBlock + (N + 1)][jcurrBlock] == (cost - 1))
                    {

                        // cout << mincostadjacencyMatrix[jcurrBlock][jcurrBlock - 1] << "up.." << jcurrBlock;
                        shortestpath.push_back({jcurrBlock, jcurrBlock + N + 1});
                        currBlock[i] += (N + 1);
                        // cout << jcurrBlock << endl;
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

void Grid::mazeRouter()
{
    int i = 0;
    int totalsegments = 0;
    while (i < i_connections.size())
    {
        // cout << i_connections[i][0] << i_connections[i][1] << i_connections.size() << endl;
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
            completeResetMatrix();
        }
    }
    cout << to_string(totalsegments) << " total segments were made." << endl;
    resetMatrix();
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

/*

X 3$ X X $$ X X X X X X X X X X X
3$ X $$ X X 13 X X X X X X X X X X
X $$ X $$ X X $$ X X X X X X X X X
X X $$ X X X X $$ X X X X X X X X
$$ X X X X 2$ X X $$ X X X X X X X
X 13 X X 2$ X 2$ X X 13 X X X X X X
X X $$ X X 2$ X $$ X X 2$ X X X X X
X X X $$ X X $$ X X X X $$ X X X X
X X X X $$ X X X X $$ X X $$ X X X
X X X X X 13 X X $$ X 1$ X X 3$ X X
X X X X X X 2$ X X 1$ X 1$ X X $$ X
X X X X X X X $$ X X 1$ X X X X 1$
X X X X X X X X $$ X X X X $$ X X
X X X X X X X X X 3$ X X $$ X $$ X
X X X X X X X X X X $$ X X $$ X $$
X X X X X X X X X X X 1$ X X $$ X

*/