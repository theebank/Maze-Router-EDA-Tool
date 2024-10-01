#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <queue>

using namespace ::std;

class Grid
// Graph D.S. to represent tracks
{
public:
    vector<vector<string>> adjacencyMatrix; // adjacency matrix for track segments
    /*
        No Connection - 'X'
        Connection but unused (Available) - 'A'
        Connection and used (Unavailable) - 'U'
        Target of search/expansion - 'T'
        Available and adjacent to driver - 'O'
    */
    int N;                             // NxN grid (N Vertices)
    int W;                             // W Channels
    vector<vector<int>> i_connections; // input connections

    void initializeGraph();
    void takeTrackSegment(int block1x, int block1y, int block2x, int block2y, int tracknum);
    void takeTrackSegment(int block1, int block2, int tracknum);
    void processFile(const string &path);
    bool processConnection(vector<int> connection, int connectionnum);
    void mazeRouter();
    void resetMatrix();
    void printInputFile();
    void printMatrix();
};

int main()
{
    Grid g;

    g.processFile("/Users/theebankumaresan/Documents/School/ECE1387/lab1/tests/cct1.txt");

    g.printInputFile();

    g.initializeGraph();

    // g.takeTrackSegment(0,0,0,1,3);

    // g.processConnection(g.i_connections[0]);

    g.printMatrix();
    g.mazeRouter();
    cout << "----------" << endl;

    g.printMatrix();
    g.resetMatrix();
    cout << "----------" << endl;
    g.printMatrix();

    return 0;
}

void Grid::initializeGraph()
{
    // Initialize empty adjacency matrix with no connections between nodes
    int numSwitchBoxes = ((N + 1)) * ((N + 1));
    adjacencyMatrix.resize(numSwitchBoxes, vector<string>(numSwitchBoxes, "X"));
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
                adjacencyMatrix[curr][left] = string(W, 'A');
                adjacencyMatrix[left][curr] = string(W, 'A');
            }
            // right connection
            if (j < N)
            {
                int right = curr + 1;
                adjacencyMatrix[curr][right] = string(W, 'A');
                adjacencyMatrix[right][curr] = string(W, 'A');
            }
            // bottom connection
            if (i < N)
            {
                int bot = curr - N - 1;
                adjacencyMatrix[curr][bot] = string(W, 'A');
                adjacencyMatrix[bot][curr] = string(W, 'A');
            }
            // top connection
            if (i > 0)
            {
                int top = curr + N + 1;
                adjacencyMatrix[curr][top] = string(W, 'A');
                adjacencyMatrix[top][curr] = string(W, 'A');
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

void Grid::takeTrackSegment(int block1, int block2, int tracknum)
{
    string temp = adjacencyMatrix[block1][block2];
    temp[tracknum - 1] = 'U';

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

bool Grid::processConnection(vector<int> connection, int connectionnum)
{
    vector<int> driverPin = {connection[0], connection[1], connection[2]};
    vector<int> loadPin = {connection[3], connection[4], connection[5]};
    queue<vector<int>> expansionList = queue<vector<int>>(); // {Block1, Block2, cost}
    string track;
    // mark available track segments adjacent to load as 'T'
    /*
        Find connected group of track

        Switch box to the bottom left of block would have same x,y coords as the connection block

    */
    cout << loadPin[0] << ", " << loadPin[1] << ", " << (loadPin[1]) * (N + 1) + loadPin[0] << endl;

    int block1, block2;
    if (loadPin[2] == 1)
    { // Check down track
        block1 = ((loadPin[1]) * (N + 1)) + loadPin[0];
        block2 = ((loadPin[1]) * (N + 1)) + loadPin[0] + 1;
        track = adjacencyMatrix[block1][block2];
    }
    else if (loadPin[2] == 2)
    { // Check left track
        block1 = ((loadPin[1]) * (N + 1)) + loadPin[0];
        block2 = ((loadPin[1] + 1) * (N + 1)) + loadPin[0];
        track = adjacencyMatrix[block1][block2];
    }
    else if (loadPin[2] == 3)
    { // Check up track
        block1 = ((loadPin[1] + 1) * (N + 1)) + loadPin[0];
        block2 = ((loadPin[1] + 1) * (N + 1)) + loadPin[0] + 1;
        track = adjacencyMatrix[block1][block2];
    }
    if (loadPin[2] == 2)
    { // Pin can only connect to W/2 odd tracks(1,3,5,etc);
        for (int i = 1; i < track.length(); i += 2)
        {
            if (track[i] == 'A')
            {
                track[i] = 'T';
                adjacencyMatrix[block1][block2] = track;
                adjacencyMatrix[block2][block1] = track;
            }
        }
    }
    else if (loadPin[2] == 1 || loadPin[2] == 3)
    { // Pins 1 and 3 can only connect to W/2 even tracks(0,2,4,etc);
        for (int i = 0; i < track.length(); i += 2)
        {
            if (track[i] == 'A')
            {
                track[i] = 'T';
                adjacencyMatrix[block1][block2] = track;
                adjacencyMatrix[block2][block1] = track;
            }
        }
    }
    // Mark available track segments adjacent to driver as O
    block1 = ((driverPin[0]) * (N + 1)) + driverPin[1] + 1;
    block2 = ((driverPin[0] + 1) * (N + 1)) + driverPin[1] + 1;
    track = adjacencyMatrix[block1][block2];
    for (int i = 0; i < track.length(); i++)
    {
        if (track[i] == 'A')
        {
            track[i] = 'O';
            adjacencyMatrix[block1][block2] = track;
            adjacencyMatrix[block2][block1] = track;
            // then put these track segments on FIFO Q
            expansionList.push({block1, block2, 0});
        }
    }
    int jBlock1, jBlock2, jcurrBlock;
    int jLabel;
    while (!expansionList.empty())
    {
        // pop seg j from fifo
        vector<int> segmentJ = expansionList.front();
        // for each seg k that connects to j thru switch
        vector<vector<int>> ksegs;
        for (int i = 0; i < 2; i++)
        {
            jcurrBlock = segmentJ[i];
            if ((jcurrBlock % (N + 1)) != 0)
            { // check if along left column
                ksegs.push_back({jcurrBlock, jcurrBlock - 1});
            }
            if (((jcurrBlock + 1) % (N + 1)) != 0)
            { // check if along right column
                ksegs.push_back({jcurrBlock, jcurrBlock + 1});
            }
            if (jcurrBlock > (N + 1))
            { // check if along bottom row
                ksegs.push_back({jcurrBlock, jcurrBlock - (N + 1)});
            }
            if (jcurrBlock <= ((N + 1) * N))
            { // check if along top row
                ksegs.push_back({jcurrBlock, jcurrBlock + (N + 1)});
            }
        }
        for (auto k : ksegs)
        {
            for (int i = 0; i < adjacencyMatrix[k[0]][k[1]].size(); i++)
            {
                if (adjacencyMatrix[k[0]][k[1]][i] == 'T')
                {
                    cout << k[0] << "," << k[1] << endl;
                    // if k is target, exit while loop
                    return true;
                }
                else if (adjacencyMatrix[k[0]][k[1]][i] == 'U' || isdigit(adjacencyMatrix[k[0]][k[1]][i]))
                {
                    // else if k unavailable or already landed ignore
                    continue;
                }
                else
                {
                    // else
                    // label k with label (j) +=1
                    // jLabel = segmentJ[2];
                    // put k on expansion list
                    expansionList.push({k[0], k[1], segmentJ[2] + 1});
                    // adjacencyMatrix[k[0]][k[1]][i] = segmentJ[2] + 1;
                }
            }
        }
        expansionList.pop();
    }
    return false;

    /*
        If while loop ends without target hit:
            - some connection could not route -> can try again with problem connection moved to front of list
            - rip up and re-route
    */
}

void Grid::mazeRouter()
{
    int i = 0;
    while (i < i_connections.size())
    {
        cout << i_connections[i][0] << i_connections[i][1] << i_connections.size() << endl;
        initializeGraph();
        try
        {
            bool success = processConnection(i_connections[i], i);
            if (!success)
            {
                throw runtime_error("Connection could not route");
            }
            cout << "connection " << i << ", was successfully made." << endl;
            return;
            i++;
        }
        catch (runtime_error e)
        {
            vector<int> failedConn = i_connections[i];
            i_connections.erase(i_connections.begin() + i);
            i_connections.insert(i_connections.begin(), failedConn);
            i = 0;
        }
    }
}

void Grid::resetMatrix()
{
    for (int i = 0; i < adjacencyMatrix.size(); i++)
    {
        for (int j = 0; j < adjacencyMatrix[i].size(); j++)
        {
            for (int x = 0; x < adjacencyMatrix[i][j].size(); x++)
            {
                if (adjacencyMatrix[i][j][x] == 'X')
                {
                    continue;
                }
                if (adjacencyMatrix[i][j][x] == 'U')
                {
                    continue;
                }
                adjacencyMatrix[i][j][x] = 'A';
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