#pragma once
#include <iostream>
#include <vector>
#include <stack>
#include <utility>
#include <algorithm>
#include <cmath>
#include <map>
#include <iomanip>         // 输入输出格式控制
#include <thread>          // 线程支持
#include <chrono>          // 时间测量

// ANSI颜色宏定义，用于在支持的终端中显示彩色输出
#define COLOR_RESET "\033[0m"      // 重置颜色
#define COLOR_GREEN "\033[32m"    // 路径标记颜色
#define COLOR_BLUE "\033[34m"     // 墙壁颜色
#define COLOR_RED "\033[31m"      // 陷阱颜色
#define COLOR_YELLOW "\033[33m"   // 金币等资源颜色
#define COLOR_CYAN "\033[36m"     // 起点颜色
#define COLOR_MAGENTA "\033[35m"  // 终点颜色

using namespace std;

// 方向常量
const int dx[4] = { -1, 1, 0, 0 };
const int dy[4] = { 0, 0, -1, 1 };
const string dirStr[4] = { "UP", "DOWN", "LEFT", "RIGHT" };

// 资源价值定义
const map<char, int> RESOURCE_VALUES = {
    {'G', 5},   // 金币
    {'T', -3},  // 陷阱
    {'B', 80},   // BOSS
    {'L', 0}    // 机关
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
    int x, y;  // 当前位置
    int resources;
    vector<vector<bool>> visited;
    vector<vector<bool>> resourceTaken;
    stack<pair<int, int>> path;  // 路径回溯栈
    vector<pair<int, int>> moves; // 记录移动路径

    Player(MAZE* m) : maze(m), mazecopy2(m), resources(1000) {
        // 初始化访问数组
        visited.resize(maze->rows, vector<bool>(maze->cols, false));
        resourceTaken.resize(maze->rows, vector<bool>(maze->cols, false));

        // 设置起点
        x = maze->start.first;
        y = maze->start.second;
        visited[x][y] = true;
        path.push({ x, y });
        moves.push_back({ x, y });

        // 如果起点有资源则拾取
        takeResource();
    }

    // 获取曼哈顿距离
    int manhattanDistance(int x1, int y1, int x2, int y2) {
        return abs(x1 - x2) + abs(y1 - y2);
    }

    // 获取3x3视野内的资源信息
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

    // 拾取当前资源
    void takeResource() {
        char cell = maze->grid[x][y];
        if (maze->isResource(cell) && !resourceTaken[x][y]) {
            int value = maze->getResourceValue(cell);
            resources += value;
            resourceTaken[x][y] = true;

            string resourceName;
            if (cell == 'G') resourceName = "金币";
            else if (cell == 'T') resourceName = "陷阱";
            else if (cell == 'B') resourceName = "BOSS";
            else if (cell == 'L') resourceName = "机关";

            cout << "拾取" << resourceName << "在(" << x << ", " << y << ")。";
            cout << "价值: " << value << "，当前资源: " << resources << endl;
        }
    }

    // 获取所有可通行邻居
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

    // 获取未访问邻居
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

    // 移动到新位置
    void moveTo(int nx, int ny) {
        // 确定移动方向
        string direction;
        for (int i = 0; i < 4; i++) {
            if (x + dx[i] == nx && y + dy[i] == ny) {
                direction = dirStr[i];
                break;
            }
        }

        cout << "从(" << x << ", " << y << ")移动到(" << nx << ", " << ny << ") [" << direction << "]" << endl;

        x = nx;
        y = ny;
        visited[x][y] = true;
        path.push({ x, y });
        moves.push_back({ x, y });

        // 拾取新位置资源
        takeResource();
    }

    // 回溯到上一个位置
    void backtrack() {
        if (path.empty()) return;

        // 弹出当前位置
        path.pop();
        if (path.empty()) return;

        // 获取上一个位置
        pair<int, int> prev = path.top();
        cout << "回溯从(" << x << ", " << y << ")到(" << prev.first << ", " << prev.second << ")" << endl;

        x = prev.first;
        y = prev.second;
        moves.push_back({ x, y });
    }

    // 贪心策略选择下一个移动
    bool selectAndMove() {
        // 获取3x3视野内的资源信息
        auto resourcesInSight = getResourcesInSight();

        // 如果没有未访问邻居，检查是否在终点
        vector<pair<int, int>> unvisited = getUnvisitedNeighbors();
        if (unvisited.empty()) {
            if (maze->isEndpoint(x, y)) {
                cout << "到达终点(" << x << ", " << y << ")" << endl;
                return false;
            }
            backtrack();
            return true;
        }

        // 优先处理视野内的高性价比资源
        if (!resourcesInSight.empty()) {
            // 按性价比排序 (降序)
            sort(resourcesInSight.begin(), resourcesInSight.end(),
                [](const tuple<int, int, char, double>& a, const tuple<int, int, char, double>& b) {
                    return get<3>(a) > get<3>(b);
                });

            // 选择性价比最高的资源
            auto bestResource = resourcesInSight[0];
            int bestX = get<0>(bestResource);
            int bestY = get<1>(bestResource);
            char resType = get<2>(bestResource);
            double valuePerDist = get<3>(bestResource);

            // 如果性价比为正或是必须资源，优先移动
            if (valuePerDist > 0 || resType == 'G') {
                // 检查是否在未访问邻居中
                for (auto& p : unvisited) {
                    if (p.first == bestX && p.second == bestY) {
                        string resName = (resType == 'G') ? "金币" :
                            (resType == 'T') ? "陷阱" :
                            (resType == 'B') ? "BOSS" : "机关";
                        cout << "发现" << resName << "在(" << bestX << ", " << bestY
                            << ")，性价比: " << valuePerDist << "，优先移动" << endl;
                        moveTo(bestX, bestY);
                        return true;
                    }
                }
            }
        }

        // 分离终点和非终点邻居
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

        // 优先选择非终点邻居
        if (!nonEndNeighbors.empty()) {
            // 尝试选择有资源的邻居
            for (auto& p : nonEndNeighbors) {
                if (maze->isResource(maze->grid[p.first][p.second])) {
                    moveTo(p.first, p.second);
                    return true;
                }
            }
            // 没有资源邻居，选择第一个非终点邻居
            moveTo(nonEndNeighbors[0].first, nonEndNeighbors[0].second);
            return true;
        }
        // 只有终点邻居
        else if (endNeighbor.first != -1) {
            moveTo(endNeighbor.first, endNeighbor.second);
            cout << "到达终点(" << x << ", " << y << ")" << endl;
            return false;
        }

        // 需要回溯
        backtrack();
        return true;
    }

    // 运行直到到达终点
    void runUntilEnd() {
        while (selectAndMove()) {
            // 继续执行
        }
    }

    // 打印最终结果
    void printResults() {


        // 动态显示路径探索过程
        cout << "\n=== 开始路径可视化 ===" << endl;
        for (size_t step = 0; step < moves.size(); step++) {
            // 根据操作系统清屏（Windows使用cls，其他系统使用clear）
#ifdef _WIN32
            system("cls");
#else
            system("clear");
#endif

            // 显示当前步骤信息
            cout << "步骤 " << setw(3) << step + 1 << "/" << moves.size()
                << " | 位置: (" << moves[step].first << ", " << moves[step].second << ")"
                << " | 分数: " << resources << endl;
            cout << "------------------------" << endl;

            // 绘制当前步骤的迷宫状态
            for (int i = 0; i < mazecopy2->rows; i++) {
                for (int j = 0; j < mazecopy2->cols; j++) {
                    char cell = mazecopy2->grid[i][j];
                    pair<int, int> pos = { i, j };

                    // 当前玩家位置
                    if (pos == moves[step]) {
                        cout << COLOR_GREEN << "&" << COLOR_RESET;
                        mazecopy2->grid[i][j] = ' ';
                    }
                    // 起点位置
                    else if (pos == moves[0]) {
                        cout << COLOR_CYAN << "S" << COLOR_RESET;
                    }
                    // 终点位置
                    else if (pos == maze->end) {
                        cout << COLOR_MAGENTA << "E" << COLOR_RESET;
                    }
                    // 墙壁
                    else if (cell == '#') {
                        cout << COLOR_BLUE << "#" << COLOR_RESET;
                    }
                    // 陷阱
                    else if (cell == 'T') {
                        cout << COLOR_RED << "T" << COLOR_RESET;
                    }
                    // 金币
                    else if (cell == 'G') {
                        cout << COLOR_YELLOW << "G" << COLOR_RESET;
                    }
                    // BOSS
                    else if (cell == 'B') {
                        cout << COLOR_YELLOW << "B" << COLOR_RESET;
                    }
                    // 机关
                    else if (cell == 'L') {
                        cout << COLOR_YELLOW << "L" << COLOR_RESET;
                    }
                    // 普通通路
                    else if (cell == '.' || cell == ' ') {
                        // 检查是否访问过
                        if (visited[i][j]) {
                            cout << COLOR_GREEN << "." << COLOR_RESET;
                        }
                        else {
                            cout << " ";
                        }
                    }
                    // 其他情况
                    else {
                        cout << cell;
                    }
                }
                cout << endl;
            }

            // 显示路径统计信息
            cout << "------------------------" << endl;
            cout << "已探索步数: " << step + 1 << "/" << moves.size() << endl;

            // 控制动画速度，延迟300毫秒
            this_thread::sleep_for(chrono::milliseconds(300));
        }

        // 显示最终路径总结
        cout << "\n=== 路径可视化完成 ===" << endl;
        cout << "总路径长度: " << moves.size() << " 步" << endl;
        cout << "最终分数: " << resources << endl;
        cout << "=====================" << endl;


    }
};