#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>
#include <tuple>
#include <limits>
#include <chrono>
#include <iomanip>
#include <queue>

using namespace std;

const int INF = numeric_limits<int>::max();
int visit = 0, explored = 0, leaf = 0, unfeasible = 0, not_promising = 0;

void usage() {
    cerr << "Usage:" << endl;
    cerr << "mcp [--p2D] [-p] -f file" << endl;
}

vector<vector<int>> matrix(const string &file_name, int &r, int &c) {
    ifstream is(file_name);
    if (!is) {
        cerr << "ERROR: can't open file: " << file_name << "." << endl;
        exit(EXIT_FAILURE);
    }

    is >> r >> c;
    vector<vector<int>> mat(r, vector<int>(c));

    for (int i = 0; i < r; ++i) {
        for (int j = 0; j < c; ++j) {
            is >> mat[i][j];
        }
    }

    return mat;
}

bool inside_matrix(int i, int j, int n, int m) {
    return i >= 0 && i < n && j >= 0 && j < m;
}

// Cota pesimista
int mcp_it_matrix(const vector<vector<int>> &map, vector<vector<int>> &iterative) {
    int n = map.size();
    int m = map[0].size();

    iterative[0][0] = map[0][0];

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            if (i > 0) {
                iterative[i][j] = min(iterative[i][j], iterative[i-1][j] + map[i][j]);
            }
            if (j > 0) {
                iterative[i][j] = min(iterative[i][j], iterative[i][j-1] + map[i][j]);
            }
            if (i > 0 && j > 0) {
                iterative[i][j] = min(iterative[i][j], iterative[i-1][j-1] + map[i][j]);
            }
        }
    }

    return iterative[n-1][m-1];
}

// Cota optimista
vector<vector<int>> optimistic_bound(const vector<vector<int>> &maze) {
    vector<vector<int>> optimistic = maze;
    int n = maze.size();
    int m = maze[0].size();
    vector<int> min_rows(n, INF);
    vector<int> min_cols(m, INF);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            min_rows[i] = min(min_rows[i], maze[i][j]);
            min_cols[j] = min(min_cols[j], maze[i][j]);
        }
    }

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            if (i < n - 1) optimistic[i][j] = min(optimistic[i][j], min_rows[i + 1]);
            if (j < m - 1) optimistic[i][j] = min(optimistic[i][j], min_cols[j + 1]);
        }
    }

    return optimistic;
}

void print_path(const vector<vector<bool>> path){
    for (int i = 0; i < path.size(); ++i) {
        for (int j = 0; j < path[0].size(); ++j) {   
            cout<<path[i][j];
        }
        cout << endl;
    }
}

void mcp_bb(const vector<vector<int>> &maze, vector<vector<int>> &iterative, vector<vector<int>> &optimistic, vector<vector<bool>> &visited, vector<vector<bool>> &best_path, vector<vector<bool>> &current_path, vector<string> &best_directions, vector<string> &current_directions, int x, int y, int &best_cost, int &pesimistic) {
    int n = maze.size();
    int m = maze[0].size();

    enum Step {N, NE, E, SE, S, SW, W, NW};

    using Node = tuple<int, int, int, int>;
    priority_queue<Node> pq;
    int current_cost = maze[0][0];
    pq.emplace(-optimistic[0][0], x, y, current_cost);

    while (!pq.empty()) {
        auto [ignore, x, y, current_cost] = pq.top();
        pq.pop();

        if (x == n - 1 && y == m - 1) {
            leaf++;
            current_path[x][y] = true;
            if (current_cost <= best_cost) {
                best_cost = current_cost;
                best_path = current_path;
                best_directions = current_directions;
                break;
            }else{
                current_path[x][y] = false;
            }
        }

        current_path[x][y] = true;

        int pesimistic_bound = current_cost + pesimistic;
        if (pesimistic_bound < best_cost) {
            best_cost = pesimistic_bound;
        }

        if(current_cost < iterative[x][y]){
            iterative[x][y] = current_cost;
        }
        else if (current_cost > iterative[x][y]) {
            not_promising++;
            current_path[x][y] = false;
            continue;
        }

        if (current_cost + optimistic[x][y] > best_cost) {
            not_promising++;
            current_path[x][y] = false;
            continue;
        }

        cout<<"ANTES DEL FOR"<<endl;
        print_path(current_path);

        for (int i = N; i <= NW; ++i) {
            visit++;
            int incx, incy;
            string direction;

            switch (i) {
                case N:  incx = -1; incy = 0;  direction = "N"; break;
                case NE: incx = -1; incy = 1;  direction = "NE"; break;
                case E:  incx = 0;  incy = 1;  direction = "E"; break;
                case SE: incx = 1;  incy = 1;  direction = "SE"; break;
                case S:  incx = 1;  incy = 0;  direction = "S"; break;
                case SW: incx = 1;  incy = -1; direction = "SW"; break;
                case W:  incx = 0;  incy = -1; direction = "W"; break;
                case NW: incx = -1; incy = -1; direction = "NW"; break;
            }

            int newx = x + incx;
            int newy = y + incy;

            if (inside_matrix(newx, newy, n, m) && !visited[newx][newy]) {
                visited[newx][newy] = true;
                current_path[newx][newy] = true;
                explored++;
                current_directions.push_back(direction);
                cout<<direction<<endl;
                print_path(current_path);
                pq.emplace(-optimistic[newx][newy], newx, newy, current_cost + maze[newx][newy]);
                visited[newx][newy] = false;
            } else {
                unfeasible++;
            }
        }
    }
}

void mcp_bb_parser(const vector<vector<int>> &maze, const vector<vector<bool>> best_path) {
    int cost = 0;
    for (int i = 0; i < maze.size(); ++i) {
        for (int j = 0; j < maze[0].size(); ++j) {
            
            if (best_path[i][j] == true) {
                cout << "x";
                cost += maze[i][j];
            } else {
                cout << ".";
            }
        }
        cout << endl;
    }

    cout << cost << endl;
}

void print_directions(const vector<string> &directions) {
    cout << "<";
    for (const auto &dir : directions) {
        if (dir == "N") cout << 1;
        else if (dir == "NE") cout << 2;
        else if (dir == "E") cout << 3;
        else if (dir == "SE") cout << 4;
        else if (dir == "S") cout << 5;
        else if (dir == "SW") cout << 6;
        else if (dir == "W") cout << 7;
        else if (dir == "NW") cout << 8;
    }
    cout << ">" << endl;
}

void output(bool p, bool p2D, const vector<vector<int>> &map, int r, int c) {
    vector<vector<bool>> visited(r, vector<bool>(c, false));
    vector<vector<int>> iterative(r, vector<int>(c, INF));
    int shortest_path_iterative = mcp_it_matrix(map, iterative);
    int pesimistic = shortest_path_iterative;

    vector<vector<bool>> best_path(r, vector<bool>(c, false));
    vector<vector<bool>> current_path(r, vector<bool>(c, false));
    vector<string> best_directions;
    vector<string> current_directions;

    auto start = chrono::steady_clock::now();
    vector<vector<int>> optimistic = optimistic_bound(map);
    int best_cost = iterative[r-1][c-1];
    mcp_bb(map, iterative, optimistic, visited, best_path, current_path, best_directions, current_directions, 0, 0, best_cost, pesimistic);
    auto end = chrono::steady_clock::now();

    chrono::duration<double> duration = end - start;
    double seconds = duration.count();

    cout << best_cost << endl;
    cout << visit << " " << explored << " " << leaf << " " << unfeasible << " " << not_promising << endl;
    cout << fixed << setprecision(3) << seconds << endl;

    if (p2D) {
        mcp_bb_parser(map, best_path);
    }

    if (p) {
        print_directions(best_directions);
    }
}

int main(int argc, char *argv[]) {
    string filename;
    bool p2D = false;
    bool p = false;

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "-p") {
            p = true;
        } else if (arg == "--p2D") {
            p2D = true;
        } else if (arg == "-f") {
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                filename = argv[++i];
            } else {
                cerr << "ERROR: missing filename." << endl;
                usage();
                return 1;
            }
        } else {
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
    output(p, p2D, map, rows, cols);

    return 0;
}
