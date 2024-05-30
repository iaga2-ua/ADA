// Iván Álvarez García DNI: 49623492A
#include <iostream>
#include <fstream>
#include <vector>
#include <climits>

using namespace std;

void usage() {
    cerr << "Usage:" << endl;
    cerr << "mcp [--p2D] [-t] [--ignore-naive] -f file" << endl;
}

vector<vector<int>> matrix(string file_name, int &r, int &c){
    ifstream is (file_name);
    if (!is) {
        cerr << "ERROR: Can't open file: " << file_name << "." << endl;
        exit(EXIT_FAILURE);
    }

    is >> r >> c;
    vector<vector<int>> mat(r, vector<int>(c));

    for(int i = 0; i < r; i++) {
        for(int j = 0; j < c; j++) {
            is >> mat[i][j];
        }
    }
    
    return mat;
}

void mcp_greedy_parser(const vector<vector<string>>& parser, int minValue) {
    for (const auto& row : parser) {
        for (const auto& cell : row) {
            cout << cell;
        }
        cout << endl;
    }
    cout << minValue << endl;
}

int mcp_greedy(const vector<vector<int>>& matrix, vector<vector<string>>& path, bool fromOrigin) {
    int rows = matrix.size();
    int cols = matrix[0].size();
    int totalCost;
    int r, c;

    if (fromOrigin) {
        r = 0;
        c = 0;
        totalCost = matrix[0][0];
        path[r][c] = "x";

        while (r < rows - 1 || c < cols - 1) {
            if (r == rows - 1) {
                totalCost += matrix[r][c + 1];
                path[r][c + 1] = "x";
                c++;
            } else if (c == cols - 1) {
                totalCost += matrix[r + 1][c];
                path[r + 1][c] = "x";
                r++;
            } else {
                int right = matrix[r][c + 1];
                int down = matrix[r + 1][c];
                int diag = matrix[r + 1][c + 1];
                int nextCost = min(right, min(down, diag));

                totalCost += nextCost;
                if (nextCost == down) {
                    r++;
                } else if (nextCost == right) {
                    c++;
                } else {
                    r++;
                    c++;
                }
                path[r][c] = "x";
            }
        }
    } else {
        r = rows - 1;
        c = cols - 1;
        totalCost = matrix[r][c];
        path[r][c] = "x";

        while (r > 0 || c > 0) {
            if (r == 0) {
                totalCost += matrix[r][c - 1];
                path[r][c - 1] = "x";
                c--;
            } else if (c == 0) {
                totalCost += matrix[r - 1][c];
                path[r - 1][c] = "x";
                r--;
            } else {
                int left = matrix[r][c - 1];
                int up = matrix[r - 1][c];
                int diag = matrix[r - 1][c - 1];
                int nextCost = min(left, min(up, diag));

                totalCost += nextCost;
                if (nextCost == up) {
                    r--;
                } else if (nextCost == left) {
                    c--;
                } else {
                    r--;
                    c--;
                }
                path[r][c] = "x";
            }
        }
    }

    return totalCost;
}
void output(bool p, const vector<vector<int>> &map, int r, int c){

    vector<vector<string>> parser(map.size(), vector<string>(map[0].size(), "."));
    vector<vector<string>> x(r, vector<string>(c, "."));
    parser = x;


    int orig = mcp_greedy(map, parser, true);
    parser = x;
    int dst = mcp_greedy(map, parser ,false);
    int min = dst;

    if(orig<=dst){
        parser = x;
        mcp_greedy(map, parser ,true);  
        min = orig;
    }

    cout<<orig<<" "<<dst<<endl;


    if(p){
        mcp_greedy_parser(parser, min);
    } 
}


int main(int argc, char* argv[]) {
    string filename;
    bool p = false;
    
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];

        if (arg == "--p2D") {
            p = true;
        } 
        else if (arg == "-f") {
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                filename = argv[i + 1];
                i++;
            } else {
                cerr << "ERROR: missing filename." << endl;
                usage();
                return 1;
            }
        } 
        else {
            cerr << "ERROR: unknown option " << arg << "." << endl;
            usage();
            return 1;
        }
    }

    if (filename.empty()) {
        cerr << "ERROR: missing filename." << endl;
        usage();
        return 1;
    }

    int rows, cols;
    vector<vector<int>> map = matrix(filename, rows, cols);
    output(p, map, rows, cols);

    return 0;
}
