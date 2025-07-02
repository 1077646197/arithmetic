#pragma once
#include <iostream>
#include <vector>
#include <stack>
#include <utility>
#include <algorithm>
#include <cmath>
#include <map>
#include <iomanip>         // ���������ʽ����
#include <thread>          // �߳�֧��
#include <chrono>          // ʱ�����

// ANSI��ɫ�궨�壬������֧�ֵ��ն�����ʾ��ɫ���
#define COLOR_RESET "\033[0m"      // ������ɫ
#define COLOR_GREEN "\033[32m"    // ·�������ɫ
#define COLOR_BLUE "\033[34m"     // ǽ����ɫ
#define COLOR_RED "\033[31m"      // ������ɫ
#define COLOR_YELLOW "\033[33m"   // ��ҵ���Դ��ɫ
#define COLOR_CYAN "\033[36m"     // �����ɫ
#define COLOR_MAGENTA "\033[35m"  // �յ���ɫ

using namespace std;

// ������
const int dx[4] = { -1, 1, 0, 0 };
const int dy[4] = { 0, 0, -1, 1 };
const string dirStr[4] = { "UP", "DOWN", "LEFT", "RIGHT" };

// ��Դ��ֵ����
const map<char, int> RESOURCE_VALUES = {
    {'G', 5},   // ���
    {'T', -3},  // ����
    {'B', 80},   // BOSS
    {'L', 0}    // ����
};

class MAZE {
public:
    int rows, cols;
    vector<vector<char>> grid;
    pair<int, int> start, end;

    MAZE(int r, int c) : rows(r), cols(c), grid(r, vector<char>(c)) {}

    bool isValid(int x, int y) {
        return x >= 0 && x < rows && y >= 0 && y < cols;
    }

    bool isWall(int x, int y) {
        return isValid(x, y) && grid[x][y] == '#';
    }

    bool isResource(char c) {
        return RESOURCE_VALUES.find(c) != RESOURCE_VALUES.end();
    }

    int getResourceValue(char c) {
        auto it = RESOURCE_VALUES.find(c);
        return it != RESOURCE_VALUES.end() ? it->second : 0;
    }

    bool isEndpoint(int x, int y) {
        return isValid(x, y) && grid[x][y] == 'E';
    }

    bool isPath(int x, int y) {
        return isValid(x, y) && (grid[x][y] == '.' || grid[x][y] == 'R' ||
            grid[x][y] == 'S' || RESOURCE_VALUES.find(grid[x][y]) != RESOURCE_VALUES.end());
    }
};

class Player {
public:
    MAZE* maze;
    MAZE* mazecopy2;
    int x, y;  // ��ǰλ��
    int resources;
    vector<vector<bool>> visited;
    vector<vector<bool>> resourceTaken;
    stack<pair<int, int>> path;  // ·������ջ
    vector<pair<int, int>> moves; // ��¼�ƶ�·��

    Player(MAZE* m) : maze(m), mazecopy2(m), resources(1000) {
        // ��ʼ����������
        visited.resize(maze->rows, vector<bool>(maze->cols, false));
        resourceTaken.resize(maze->rows, vector<bool>(maze->cols, false));

        // �������
        x = maze->start.first;
        y = maze->start.second;
        visited[x][y] = true;
        path.push({ x, y });
        moves.push_back({ x, y });

        // ����������Դ��ʰȡ
        takeResource();
    }

    // ��ȡ�����پ���
    int manhattanDistance(int x1, int y1, int x2, int y2) {
        return abs(x1 - x2) + abs(y1 - y2);
    }

    // ��ȡ3x3��Ұ�ڵ���Դ��Ϣ
    vector<tuple<int, int, char, double>> getResourcesInSight() {
        vector<tuple<int, int, char, double>> resources;
        for (int i = max(0, x - 1); i <= min(maze->rows - 1, x + 1); i++) {
            for (int j = max(0, y - 1); j <= min(maze->cols - 1, y + 1); j++) {
                char cell = maze->grid[i][j];
                if (maze->isResource(cell) && !resourceTaken[i][j]) {
                    int distance = manhattanDistance(x, y, i, j);
                    double valuePerDistance = (distance > 0) ?
                        static_cast<double>(maze->getResourceValue(cell)) / distance :
                        maze->getResourceValue(cell);
                    resources.emplace_back(i, j, cell, valuePerDistance);
                }
            }
        }
        return resources;
    }

    // ʰȡ��ǰ��Դ
    void takeResource() {
        char cell = maze->grid[x][y];
        if (maze->isResource(cell) && !resourceTaken[x][y]) {
            int value = maze->getResourceValue(cell);
            resources += value;
            resourceTaken[x][y] = true;

            string resourceName;
            if (cell == 'G') resourceName = "���";
            else if (cell == 'T') resourceName = "����";
            else if (cell == 'B') resourceName = "BOSS";
            else if (cell == 'L') resourceName = "����";

            cout << "ʰȡ" << resourceName << "��(" << x << ", " << y << ")��";
            cout << "��ֵ: " << value << "����ǰ��Դ: " << resources << endl;
        }
    }

    // ��ȡ���п�ͨ���ھ�
    vector<pair<int, int>> getAllNeighbors() {
        vector<pair<int, int>> neighbors;
        for (int i = 0; i < 4; i++) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            if (maze->isValid(nx, ny) && !maze->isWall(nx, ny)) {
                neighbors.push_back({ nx, ny });
            }
        }
        return neighbors;
    }

    // ��ȡδ�����ھ�
    vector<pair<int, int>> getUnvisitedNeighbors() {
        vector<pair<int, int>> neighbors = getAllNeighbors();
        vector<pair<int, int>> unvisited;
        for (auto& p : neighbors) {
            if (!visited[p.first][p.second]) {
                unvisited.push_back(p);
            }
        }
        return unvisited;
    }

    // �ƶ�����λ��
    void moveTo(int nx, int ny) {
        // ȷ���ƶ�����
        string direction;
        for (int i = 0; i < 4; i++) {
            if (x + dx[i] == nx && y + dy[i] == ny) {
                direction = dirStr[i];
                break;
            }
        }

        cout << "��(" << x << ", " << y << ")�ƶ���(" << nx << ", " << ny << ") [" << direction << "]" << endl;

        x = nx;
        y = ny;
        visited[x][y] = true;
        path.push({ x, y });
        moves.push_back({ x, y });

        // ʰȡ��λ����Դ
        takeResource();
    }

    // ���ݵ���һ��λ��
    void backtrack() {
        if (path.empty()) return;

        // ������ǰλ��
        path.pop();
        if (path.empty()) return;

        // ��ȡ��һ��λ��
        pair<int, int> prev = path.top();
        cout << "���ݴ�(" << x << ", " << y << ")��(" << prev.first << ", " << prev.second << ")" << endl;

        x = prev.first;
        y = prev.second;
        moves.push_back({ x, y });
    }

    // ̰�Ĳ���ѡ����һ���ƶ�
    bool selectAndMove() {
        // ��ȡ3x3��Ұ�ڵ���Դ��Ϣ
        auto resourcesInSight = getResourcesInSight();

        // ���û��δ�����ھӣ�����Ƿ����յ�
        vector<pair<int, int>> unvisited = getUnvisitedNeighbors();
        if (unvisited.empty()) {
            if (maze->isEndpoint(x, y)) {
                cout << "�����յ�(" << x << ", " << y << ")" << endl;
                return false;
            }
            backtrack();
            return true;
        }

        // ���ȴ�����Ұ�ڵĸ��Լ۱���Դ
        if (!resourcesInSight.empty()) {
            // ���Լ۱����� (����)
            sort(resourcesInSight.begin(), resourcesInSight.end(),
                [](const tuple<int, int, char, double>& a, const tuple<int, int, char, double>& b) {
                    return get<3>(a) > get<3>(b);
                });

            // ѡ���Լ۱���ߵ���Դ
            auto bestResource = resourcesInSight[0];
            int bestX = get<0>(bestResource);
            int bestY = get<1>(bestResource);
            char resType = get<2>(bestResource);
            double valuePerDist = get<3>(bestResource);

            // ����Լ۱�Ϊ�����Ǳ�����Դ�������ƶ�
            if (valuePerDist > 0 || resType == 'G') {
                // ����Ƿ���δ�����ھ���
                for (auto& p : unvisited) {
                    if (p.first == bestX && p.second == bestY) {
                        string resName = (resType == 'G') ? "���" :
                            (resType == 'T') ? "����" :
                            (resType == 'B') ? "BOSS" : "����";
                        cout << "����" << resName << "��(" << bestX << ", " << bestY
                            << ")���Լ۱�: " << valuePerDist << "�������ƶ�" << endl;
                        moveTo(bestX, bestY);
                        return true;
                    }
                }
            }
        }

        // �����յ�ͷ��յ��ھ�
        vector<pair<int, int>> nonEndNeighbors;
        pair<int, int> endNeighbor = { -1, -1 };

        for (auto& p : unvisited) {
            if (maze->isEndpoint(p.first, p.second)) {
                endNeighbor = p;
            }
            else {
                nonEndNeighbors.push_back(p);
            }
        }

        // ����ѡ����յ��ھ�
        if (!nonEndNeighbors.empty()) {
            // ����ѡ������Դ���ھ�
            for (auto& p : nonEndNeighbors) {
                if (maze->isResource(maze->grid[p.first][p.second])) {
                    moveTo(p.first, p.second);
                    return true;
                }
            }
            // û����Դ�ھӣ�ѡ���һ�����յ��ھ�
            moveTo(nonEndNeighbors[0].first, nonEndNeighbors[0].second);
            return true;
        }
        // ֻ���յ��ھ�
        else if (endNeighbor.first != -1) {
            moveTo(endNeighbor.first, endNeighbor.second);
            cout << "�����յ�(" << x << ", " << y << ")" << endl;
            return false;
        }

        // ��Ҫ����
        backtrack();
        return true;
    }

    // ����ֱ�������յ�
    void runUntilEnd() {
        while (selectAndMove()) {
            // ����ִ��
        }
    }

    // ��ӡ���ս��
    void printResults() {


        // ��̬��ʾ·��̽������
        cout << "\n=== ��ʼ·�����ӻ� ===" << endl;
        for (size_t step = 0; step < moves.size(); step++) {
            // ���ݲ���ϵͳ������Windowsʹ��cls������ϵͳʹ��clear��
#ifdef _WIN32
            system("cls");
#else
            system("clear");
#endif

            // ��ʾ��ǰ������Ϣ
            cout << "���� " << setw(3) << step + 1 << "/" << moves.size()
                << " | λ��: (" << moves[step].first << ", " << moves[step].second << ")"
                << " | ����: " << resources << endl;
            cout << "------------------------" << endl;

            // ���Ƶ�ǰ������Թ�״̬
            for (int i = 0; i < mazecopy2->rows; i++) {
                for (int j = 0; j < mazecopy2->cols; j++) {
                    char cell = mazecopy2->grid[i][j];
                    pair<int, int> pos = { i, j };

                    // ��ǰ���λ��
                    if (pos == moves[step]) {
                        cout << COLOR_GREEN << "&" << COLOR_RESET;
                        mazecopy2->grid[i][j] = ' ';
                    }
                    // ���λ��
                    else if (pos == moves[0]) {
                        cout << COLOR_CYAN << "S" << COLOR_RESET;
                    }
                    // �յ�λ��
                    else if (pos == maze->end) {
                        cout << COLOR_MAGENTA << "E" << COLOR_RESET;
                    }
                    // ǽ��
                    else if (cell == '#') {
                        cout << COLOR_BLUE << "#" << COLOR_RESET;
                    }
                    // ����
                    else if (cell == 'T') {
                        cout << COLOR_RED << "T" << COLOR_RESET;
                    }
                    // ���
                    else if (cell == 'G') {
                        cout << COLOR_YELLOW << "G" << COLOR_RESET;
                    }
                    // BOSS
                    else if (cell == 'B') {
                        cout << COLOR_YELLOW << "B" << COLOR_RESET;
                    }
                    // ����
                    else if (cell == 'L') {
                        cout << COLOR_YELLOW << "L" << COLOR_RESET;
                    }
                    // ��ͨͨ·
                    else if (cell == '.' || cell == ' ') {
                        // ����Ƿ���ʹ�
                        if (visited[i][j]) {
                            cout << COLOR_GREEN << "." << COLOR_RESET;
                        }
                        else {
                            cout << " ";
                        }
                    }
                    // �������
                    else {
                        cout << cell;
                    }
                }
                cout << endl;
            }

            // ��ʾ·��ͳ����Ϣ
            cout << "------------------------" << endl;
            cout << "��̽������: " << step + 1 << "/" << moves.size() << endl;

            // ���ƶ����ٶȣ��ӳ�300����
            this_thread::sleep_for(chrono::milliseconds(300));
        }

        // ��ʾ����·���ܽ�
        cout << "\n=== ·�����ӻ���� ===" << endl;
        cout << "��·������: " << moves.size() << " ��" << endl;
        cout << "���շ���: " << resources << endl;
        cout << "=====================" << endl;


    }
};