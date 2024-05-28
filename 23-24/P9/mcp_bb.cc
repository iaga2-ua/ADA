//Iván Álvarez García DNI: 49623492A
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
vector<vector<bool>> parser;
int visit = 0, explored = 0, leaf = 0, unfeasible = 0, not_promising = 0;

void usage() {
    cerr << "Usage:" << endl;
    cerr << "mcp [--p2D] [-t] [--ignore-naive] -f file" << endl;
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

//cota pesimista
int mcp_it_matrix(const vector<vector<int>> &map, vector<vector<int>> &iterative) {
    int n = map.size();
    int m = map[0].size();

    iterative[0][0] = map[0][0];

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            if (i > 0) {
                iterative[i][j] = min(iterative[i][j], iterative[i-1][j] + map[i][j]); // movimiento arriba
            }
            if (j > 0) {
                iterative[i][j] = min(iterative[i][j], iterative[i][j-1] + map[i][j]); // movimiento izquierda
            }
            if (i > 0 && j > 0) {
                iterative[i][j] = min(iterative[i][j], iterative[i-1][j-1] + map[i][j]); // movimiento diagonal
            }
        }
    }

    return iterative[n-1][m-1];
}

//cota optimista
vector<vector<int>> optimistic_bound(vector<vector<int>> maze){
    vector<vector<int>> optimistic = maze;
    vector<int> min_rows(maze.size());
    vector<int> min_cols(maze[0].size(), INF);
    vector<int> sum_rows(maze.size());
    vector<int> sum_cols(maze[0].size());

    int acumulativo = 0;
    // Encontramos el mínimo de cada fila
    for (const auto& fila : maze) {
        min_rows.push_back(*min_element(fila.begin(), fila.end()));
        acumulativo += *min_element(fila.begin(), fila.end());
        sum_rows.push_back(acumulativo);
    }

    acumulativo = 0;
    // Encontramos el mínimo de cada columna
    for (const auto& fila : maze) {
        for (int j = 0; j < fila.size(); ++j) {
            min_cols[j] = min(min_cols[j], fila[j]);
            acumulativo += min_cols[j];
            sum_cols.push_back(acumulativo);
        }
    }

    for(int i = 0; i < maze.size(); i++){
        for(int j = 0; j < maze[0].size(); j++){
            if(i == maze.size() - 1 && j == maze[0].size() - 1){
                continue;
            }
            else if(i == maze.size() - 1){
                 optimistic[i][j] = sum_cols[j+1];
            }
            else if(j == maze[0].size() - 1){
                optimistic[i][j] = sum_rows[i+1];
            } 
            else {
                optimistic[i][j] = max(sum_rows[i+1], sum_cols[j+1]);
            }
        }
    }

    return optimistic;
}

void mcp_bb(const vector<vector<int>> &maze, vector<vector<int>> &iterative, vector<vector<int>> &optimistic, vector<vector<bool>> &visited, vector<pair<int, int>> &best_path, vector<pair<int, int>> &current_path, vector<string> &best_directions, vector<string> &current_directions, int x, int y, int current_cost, int &best_cost ,int &pesimistic) {
    

    int n = maze.size();
    int m = maze[0].size();

    enum Step {SE, E, S, N, NE, SW, W, NW};
    /*map<Step, tuple<int, int, string>> steps_inc = {
        {N, make_tuple(-1, 0, "N")},
        {NE, make_tuple(-1, 1, "NE")},
        {E, make_tuple(0, 1, "E")},
        {SE, make_tuple(1, 1, "SE")},
        {S, make_tuple(1, 0, "S")},
        {SW, make_tuple(1, -1, "SW")},
        {W, make_tuple(0, -1, "W")},
        {NW, make_tuple(-1, -1, "NW")}
    };*/

    using Node = tuple<int, int, int, int>;
    priority_queue<Node> pq;
    pq.emplace(-optimistic[0][0],0,0,maze[0][0]);

    int incx, incy;
    string direction;

    while (!pq.empty()){
        if (x == maze.size() - 1 && y == maze[0].size() - 1) {
        leaf++;
        current_path.push_back({x, y});
        if (current_cost + maze[x][y] < best_cost) {
            best_cost = current_cost + maze[x][y];
            best_path = current_path; // Almacena una copia del mejor camino actual
            best_directions = current_directions;
            parser = visited;
        }
        current_path.pop_back();
        return;
        }

        current_cost += maze[x][y];
        current_path.push_back({x, y});

        int pesimistic_bound = current_cost + pesimistic;

        if(pesimistic_bound < best_cost){
            best_cost = pesimistic_bound;
        }

        if (current_cost > iterative[x][y]) {
            not_promising++;
            current_path.pop_back();
            return;
        }

        if(current_cost + optimistic[x][y] > best_cost){
            not_promising++;
            return;
        }
        auto [ignore, x, y, current_cost] = pq.top();
        pq.pop();

        for(int i = N; i <= NW; i++){
            visit++;
            switch (i){
                case N:
                    incx = -1;
                    incy = 0;
                    direction = "N";
                    break;

                case NE:
                    incx = -1;
                    incy = 1;
                    direction = "NE";
                    break;

                case E:
                    incx = 0;
                    incy = 1;
                    direction = "E";
                    break;

                case SE:
                    incx = 1;
                    incy = 1;
                    direction = "SE";
                    break;

                case S:
                    incx = 1;
                    incy = 0;
                    direction = "S";
                    break;
                
                case SW:
                    incx = 1;
                    incy = -1;
                    direction = "SW";
                    break;

                case W:
                    incx = 0;
                    incy = -1;
                    direction = "W";
                    break;

                case NW:
                    incx = -1;
                    incy = -1;
                    direction = "NW";
                    break;
            
            }

            int newx = x+incx;
            int newy = y+incy;

            //check feasible
            if(inside_matrix(newx, newy, n, m) && !visited[newx][newy]){
                visited[newx][newy] = true;
                explored++;
                current_directions.push_back(direction);
                pq.emplace(-optimistic[newx][newy], newx, newy, current_cost+maze[newx][newy]);
                visited[newx][newy] = false;
                current_directions.pop_back();
            }
            else{
                unfeasible++;
            }
        }
    }

    current_path.pop_back();
}

void mcp_bb_parser(const vector<vector<int>> &maze) {
    int cost = 0;
    for(int i = 0; i < parser.size(); i++){
        for(int j = 0; j < parser[0].size(); j++){

            if(parser[i][j] == true || i == 0 && j == 0 
                || i == parser.size() - 1 && j == parser[0].size() - 1){
                cout << "x";
                cost += maze[i][j];
            }
            else{
                cout << ".";
            }
        }
        cout << endl;
    }

    cout << cost << endl;
}

void print_directions(const vector<string> &directions) {
    cout<<"<";
    for (const auto &dir : directions) {
        
        if(dir == "N"){
            cout<<1;
        }
        else if(dir == "NE"){
            cout<<2;
        }
        else if(dir == "E"){
            cout<<3;
        }
        else if(dir == "SE"){
            cout<<4;
        }
        else if(dir == "S"){
            cout<<5;
        }
        else if(dir == "SW"){
            cout<<6;
        }
        else if(dir == "W"){
            cout<<7;
        }
        else if(dir == "NW"){
            cout<<8;
        }
    }
    cout <<">"<< endl;
}

void output(bool p, bool p2D, const vector<vector<int>> &map, int r, int c) {
    vector<vector<bool>> visited(r, vector<bool>(c, false));
    vector<vector<int>> iterative(r, vector<int>(c, INF));
    int shortest_path_iterative = mcp_it_matrix(map, iterative);

    int pesimistic = shortest_path_iterative;

    vector<pair<int, int>> best_path;
    vector<pair<int, int>> current_path;
    vector<string> best_directions;
    vector<string> current_directions;
    parser = visited;
    auto start = std::chrono::steady_clock::now();
    vector<vector<int>> optimistic = optimistic_bound(map);
    int best_cost = iterative[r-1][c-1];
    mcp_bb(map, iterative, optimistic, visited, best_path, current_path, best_directions, current_directions, 0, 0, 0, best_cost, pesimistic);
    auto end = std::chrono::steady_clock::now();

    std::chrono::duration<double> duration = end - start;
    double seconds = duration.count();

    cout << best_cost << endl;
    cout << visit << " " << explored << " " << leaf << " " << unfeasible << " " << not_promising << endl;
    cout << fixed << setprecision(3) << seconds << endl;

    if(p2D){
        mcp_bb_parser(map);
    }

    if(p){
        print_directions(best_directions);
    }
}

int main(int argc, char* argv[]) {
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
