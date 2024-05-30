// Iván Álvarez García DNI: 49623492A
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
#include <string>

using namespace std;

auto start = chrono::steady_clock::now();
const int INF = numeric_limits<int>::max();
int visit = 0, explored = 0, leaf = 0, unfeasible = 0, not_promising = 0;
int promising_but_discarded = 0, best_solution_updated_from_leafs = 0, best_solution_updated_from_pessimistic_bound = 0;

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

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
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

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            min_rows[i] = min(min_rows[i], maze[i][j]);
            min_cols[j] = min(min_cols[j], maze[i][j]);
        }
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (i < n - 1) optimistic[i][j] = min(optimistic[i][j], min_rows[i + 1]);
            if (j < m - 1) optimistic[i][j] = min(optimistic[i][j], min_cols[j + 1]);
        }
    }

    return optimistic;
}

struct Node {
    int x, y, current_cost;
    vector<string> directions;
    vector<vector<bool>> path;

    Node(int x, int y, int cost, int n, int m)
        : x(x), y(y), current_cost(cost), path(n, vector<bool>(m, false)) {}
};

void mcp_bb(const vector<vector<int>> &maze, vector<vector<int>> &iterative, vector<vector<int>> &optimistic, vector<vector<bool>> &best_path, vector<string> &best_directions, int &best_cost, int &pesimistic) {
    int n = maze.size();
    int m = maze[0].size();

    enum Step {N, NE, E, SE, S, SW, W, NW};

    auto comp = [](const Node &a, const Node &b) {
        return a.current_cost + a.directions.size() > b.current_cost + b.directions.size();
    };

    priority_queue<Node, vector<Node>, decltype(comp)> pq(comp);
    Node start(0, 0, maze[0][0], n, m);
    start.path[0][0] = true;
    pq.push(start);

    while (!pq.empty()) {
        visit++;
        Node node = pq.top();
        pq.pop();

        if (node.x == n - 1 && node.y == m - 1) {
            leaf++;
            if (node.current_cost <= best_cost) {
                best_cost = node.current_cost;
                best_path = node.path;
                best_directions = node.directions;
                best_solution_updated_from_leafs++;
                explored++;
            }
            continue;
        }

        if (node.current_cost < iterative[node.x][node.y]) {
            iterative[node.x][node.y] = node.current_cost;
            best_solution_updated_from_pessimistic_bound++;
        } else if (node.current_cost > iterative[node.x][node.y]) {
            not_promising++;
            continue;
        }

        if (node.current_cost + optimistic[node.x][node.y] > best_cost) {
            promising_but_discarded++;
            continue;
        }

        for (int i = N; i <= NW; i++) {
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

            int newx = node.x + incx;
            int newy = node.y + incy;

            if (inside_matrix(newx, newy, n, m) && !node.path[newx][newy]) {
                Node next(newx, newy, node.current_cost + maze[newx][newy], n, m);
                next.directions = node.directions;
                next.directions.push_back(direction);
                next.path = node.path;
                next.path[newx][newy] = true;
                pq.push(next);
                explored++;
            } else {
                unfeasible++;
            }
        }
    }
}

void mcp_bb_parser(const vector<vector<int>> &maze, const vector<vector<bool>> best_path) {
    int cost = 0;
    for (int i = 0; i < maze.size(); i++) {
        for (int j = 0; j < maze[0].size(); j++) {
            if (best_path[i][j]) {
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
    vector<vector<int>> iterative(r, vector<int>(c, INF));
    int shortest_path_iterative = mcp_it_matrix(map, iterative);
    int pesimistic = shortest_path_iterative;

    vector<vector<bool>> best_path(r, vector<bool>(c, false));
    vector<string> best_directions;

    vector<vector<int>> optimistic = optimistic_bound(map);
    int best_cost = iterative[r-1][c-1];
    mcp_bb(map, iterative, optimistic, best_path, best_directions, best_cost, pesimistic);

    cout << best_cost << endl;
    cout << visit << " " << explored << " " << leaf << " " << unfeasible << " " << not_promising << " " << promising_but_discarded << " " << best_solution_updated_from_leafs << " " << best_solution_updated_from_pessimistic_bound << endl;
    
    auto end = chrono::steady_clock::now();
    chrono::duration<double, milli> duration = end - start; 
    double milliseconds = duration.count();
    cout << fixed << setprecision(3) << milliseconds << endl;

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

    for (int i = 1; i < argc; i++) {
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
