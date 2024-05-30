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

int mcp_greedy(const vector<vector<int>>& matrix, int rows, int cols, vector<vector<string>>& parser, bool fromOrigin) {
    int totalCost;
    int r, c;
    
    if (fromOrigin) {
        r = 0;
        c = 0;
        totalCost = matrix[0][0];
        parser[0][0] = "x";

        while (r < rows || c < cols) {
            if (r == rows) {
                totalCost += matrix[r][c + 1];
                parser[r][c + 1] = "x";
                c++;
            } else if (c == cols) {
                totalCost += matrix[r + 1][c];
                parser[r + 1][c] = "x";
                r++;
            } else {
                int nextCost = min(matrix[r + 1][c], min(matrix[r][c + 1], matrix[r + 1][c + 1]));
                totalCost += nextCost;
                if (nextCost == matrix[r + 1][c]) {
                    parser[r + 1][c] = "x";
                    r++;
                } else if (nextCost == matrix[r][c + 1]) {
                    parser[r][c + 1] = "x";
                    c++;
                } else {
                    parser[r + 1][c + 1] = "x";
                    r++;
                    c++;
                }
            }
        }
    } else {
        r = rows;
        c = cols;
        totalCost = matrix[r][c];
        parser[r][c] = "x";

        while (r > 0 || c > 0) {
            if (r == 0) {
                totalCost += matrix[r][c - 1];
                parser[r][c - 1] = "x";
                c--;
            } else if (c == 0) {
                totalCost += matrix[r - 1][c];
                parser[r - 1][c] = "x";
                r--;
            } else {
                int nextCost = min(matrix[r - 1][c], min(matrix[r][c - 1], matrix[r - 1][c - 1]));
                totalCost += nextCost;
                if (nextCost == matrix[r - 1][c]) {
                    parser[r - 1][c] = "x";
                    r--;
                } else if (nextCost == matrix[r][c - 1]) {
                    parser[r][c - 1] = "x";
                    c--;
                } else {
                    parser[r - 1][c - 1] = "x";
                    r--;
                    c--;
                }
            }
        }
    }

    return totalCost;
}

void output(bool p, const vector<vector<int>> &map, int r, int c){

    vector<vector<string>> parser(map.size(), vector<string>(map[0].size(), "."));
    vector<vector<string>> x(r, vector<string>(c, "."));
    parser = x;


    int orig = mcp_greedy(map, r-1, c-1, parser, true);
    parser = x;
    int dst = mcp_greedy(map, r-1, c-1, parser ,false);
    int min = dst;

    if(orig<=dst){
        parser = x;
        mcp_greedy(map, r-1, c-1,parser ,true);  
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
