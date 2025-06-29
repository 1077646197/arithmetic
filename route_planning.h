#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <stack>
#include <algorithm>
#include <stdexcept>
#include "maze.h"
#include <queue>
#include <iomanip>
#include <functional>
#include <thread>
#include <chrono> 
// ANSI 颜色控制码宏定义
#define ANSI_COLOR_GREEN "\033[32m"   // 绿色文本
#define ANSI_COLOR_BLUE "\033[34m"   // 蓝色文本
#define ANSI_COLOR_RED "\033[31m"   // 红色文本
#define ANSI_COLOR_YELLOW "\033[33m"   // 黄色文本
#define ANSI_COLOR_RESET "\033[0m"    // 重置为默认颜色
using namespace std;

// 资源点结构体，包含坐标和价值
struct ResourcePoint {
    int x, y;     // 坐标
    int value;    // 资源价值（陷阱为负值）
    ResourcePoint(int _x, int _y, int _value) : x(_x), y(_y), value(_value) {}
};


// 路径规划器类
class ResourcePathPlanner {
private:
    Maze maze;                            // 迷宫对象
    vector<vector<int>> dp;               // 动态规划表
    map<pair<int, int>, int> resourceMap;  // 资源映射表
    vector<vector<pair<int, int>>> predecessor;  // 用于存储路径前驱节点
    // 新增：剪枝法路径和资源值存储
    int prunedPathResource;                // 剪枝法路径的总资源值
    bool vvisited[20][20] = { 0 };
    // 坐标转键值
    pair<int, int> makePair(int x, int y) const {
        return make_pair(x, y);
    }

    // 新增：获取当前资源值（不清除资源，用于剪枝）
    int getCurrentResourceValue(int x, int y) const {
        auto it = resourceMap.find(makePair(x, y));
        return (it != resourceMap.end()) ? it->second : 0;
    }


public:
    // 构造函数
    ResourcePathPlanner(const Maze& m) : maze(m), prunedPathResource(0) {
        dp.resize(maze.size, vector<int>(maze.size, -1e9));
        initializeResourceMap();
    }

    // 检查坐标是否有效（非墙、非陷阱、在迷宫范围内）
    bool isValid(int x, int y) const {
        return x >= 0 && x < maze.size && y >= 0 && y < maze.size &&
            maze.grid[x][y] != '#';
    }
    void initialdp()
    {
        for (int i = 0; i < maze.size; ++i) {//初始化dp表
            for (int j = 0; j < maze.size; ++j) {
                if (isValid(i, j))
                {
                    dp[i][j] = getCurrentResourceValue(i, j);
                }
                else if (maze.grid[i][j] == 'T')
                {
                    dp[i][j] = -3;
                }
                else
                {
                    dp[i][j] = -1;
                }
            }
        }
    }
    // 初始化资源映射表（从迷宫矩阵提取资源价值）
    void initializeResourceMap() {
        resourceMap.clear();
        for (int i = 0; i < maze.size; i++) {
            for (int j = 0; j < maze.size; j++) {
                char cell = maze.grid[i][j];
                if (cell == 'G') resourceMap[makePair(i, j)] = 5;
                else if (cell == 'L') resourceMap[makePair(i, j)] = 10;
                else if (cell == 'B') resourceMap[makePair(i, j)] = 20;
                else if (cell == 'T') resourceMap[makePair(i, j)] = -3;
            }
        }
    }
    // 辅助函数：使用BFS从起点出发，标记所有可达节点
    vector<vector<bool>> findReachableFromStart() {
        int startX = maze.startX;
        int startY = maze.startY;
        // 初始化可达性标记数组
        vector<vector<bool>> reachable(maze.size, vector<bool>(maze.size, false));

        // 起点本身不可达则直接返回空标记
        if (!isValid(startX, startY)) {
            return reachable;
        }

        // BFS队列初始化
        queue<pair<int, int>> q;
        q.push({ startX, startY });
        reachable[startX][startY] = true;  // 标记起点可达

        // 方向数组（上下左右）
        const int dirs[4][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };

        // BFS遍历所有可达节点
        while (!q.empty()) {
            auto current = q.front();
            q.pop();
            int x = current.first;
            int y = current.second;

            // 探索四个方向
            for (const auto& dir : dirs) {
                int nx = x + dir[0];
                int ny = y + dir[1];

                // 检查：节点有效 + 未被标记 + 原始DP值>0（可通行）
                if (isValid(nx, ny) && !reachable[nx][ny] && dp[nx][ny] > 0) {
                    reachable[nx][ny] = true;  // 标记为可达
                    q.push({ nx, ny });          // 加入队列继续探索
                }
            }
        }

        return reachable;
    }
    // 修正后的核心函数：从起点遍历所有相同DP值节点到终点（兼容C++11）
    bool traverseAllSameDp() {
        int startX = maze.startX;
        int startY = maze.startY;
        int exitX = maze.exitX;
        int exitY = maze.exitY;
        int targetVal = dp[startX][startY];

        // 检查起点终点有效性
        if (!isValid(startX, startY) || !isValid(exitX, exitY)) {
            throw runtime_error("起点或终点不可通行");
        }
        if (dp[exitX][exitY] != targetVal) {
            cerr << "终点DP值与起点不符（" << dp[exitX][exitY] << " vs " << targetVal << "）" << endl;
            return false;
        }

        // 收集所有目标节点（DP值与起点相同）
        vector<pair<int, int>> targetNodes;
        for (int i = 0; i < maze.size; ++i) {
            for (int j = 0; j < maze.size; ++j) {
                if (isValid(i, j) && dp[i][j] == targetVal) {
                    targetNodes.push_back(make_pair(i, j));
                }
            }
        }
        int totalTargets = targetNodes.size();
        if (totalTargets == 0) {
            cerr << "无符合条件的目标节点" << endl;
            return false;
        }

        // 记录每个目标节点的索引（用于标记访问状态）
        map<pair<int, int>, int> nodeIndex;
        for (int i = 0; i < totalTargets; ++i) {
            nodeIndex[targetNodes[i]] = i;
        }

        // BFS队列：存储（x, y, 已访问掩码, 路径）
        queue<tuple<int, int, unsigned long long, vector<pair<int, int>>>> q;
        // 起点初始化
        unsigned long long startMask = 1ULL << nodeIndex[make_pair(startX, startY)];
        vector<pair<int, int>> startPath;
        startPath.push_back(make_pair(startX, startY));
        q.push(make_tuple(startX, startY, startMask, startPath));

        // 已访问状态记录（避免重复搜索）
        map<pair<pair<int, int>, unsigned long long>, bool> visited;
        visited[make_pair(make_pair(startX, startY), startMask)] = true;

        // 方向数组
        const int dirs[4][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };
        vector<pair<int, int>> finalPath;
        bool found = false;

        while (!q.empty()) {
            // 用C++11兼容方式获取队列元素（替代结构化绑定）
            auto front = q.front();
            q.pop();
            int x = get<0>(front);
            int y = get<1>(front);
            unsigned long long mask = get<2>(front);
            vector<pair<int, int>> path = get<3>(front);

            // 检查是否到达终点且访问所有节点
            if (x == exitX && y == exitY && mask == (1ULL << totalTargets) - 1) {
                finalPath = path;
                found = true;
                break;
            }

            // 探索四个方向
            for (const auto& dir : dirs) {
                int nx = x + dir[0];
                int ny = y + dir[1];
                // 仅允许访问相同DP值的节点
                if (!isValid(nx, ny) || dp[nx][ny] != targetVal) {
                    continue;
                }
                // 检查是否为目标节点
                auto nodeKey = make_pair(nx, ny);
                if (nodeIndex.find(nodeKey) == nodeIndex.end()) {
                    continue; // 不在目标节点列表中（理论上不会发生）
                }
                int idx = nodeIndex[nodeKey];
                unsigned long long newMask = mask | (1ULL << idx);

                // 检查是否已访问该状态
                auto stateKey = make_pair(make_pair(nx, ny), newMask);
                if (visited.find(stateKey) != visited.end()) {
                    continue;
                }

                // 生成新路径
                vector<pair<int, int>> newPath = path;
                newPath.push_back(nodeKey);
                // 入队
                q.push(make_tuple(nx, ny, newMask, newPath));
                visited[stateKey] = true;
            }
        }

        if (found) {
            // 输出路径
            for (size_t i = 0; i < finalPath.size(); i++) {
                system("cls");
                cout << "步骤 " << setw(4) << left << i << ": ("
                    << finalPath[i].first << ", " << finalPath[i].second << ")  " << endl;
                // 绘制迷宫
                for (int j = 0; j < maze.size; ++j) {
                    for (int k = 0; k < maze.size; ++k) {
                        // 当前路径位置
                        if (finalPath[i].first == j && finalPath[i].second == k) {
                            cout << ANSI_COLOR_GREEN << "&" << ANSI_COLOR_RESET;
                            maze.grid[j][k] = ' ';
                        }
                        // 墙壁
                        else if (maze.grid[j][k] == '#') {
                            cout << ANSI_COLOR_BLUE << maze.grid[j][k] << ANSI_COLOR_RESET;
                        }
                        // 陷阱
                        else if (maze.grid[j][k] == 'T') {
                            cout << ANSI_COLOR_RED << maze.grid[j][k] << ANSI_COLOR_RESET;
                        }
                        // 资源点
                        else if (maze.grid[j][k] == 'G' || maze.grid[j][k] == 'L' || maze.grid[j][k] == 'B') {
                            // 已访问的资源点显示为小写
                            cout << ANSI_COLOR_YELLOW << maze.grid[j][k] << ANSI_COLOR_RESET;
                        }
                        else {
                            cout << maze.grid[j][k];
                        }
                    }
                    cout << endl;
                }
                this_thread::sleep_for(chrono::milliseconds(300)); // 适当减慢速度
            }
            return true;
        }
        else {
            cerr << "\n未找到有效路径（无法访问所有节点或终点不可达）" << endl;
            return false;
        }
    }
    // 动态规划路径优化函数（无需参数）
    bool solveWithPruning() {
        int startX = maze.startX;
        int startY = maze.startY;
        int exitX = maze.exitX;
        int exitY = maze.exitY;
        int rows = maze.size;
        int cols = maze.size;
        dp[startX][startY] += 100;
        // 检查起点终点有效性
        if (!isValid(startX, startY)) {
            throw runtime_error("起点不可通行");
        }
        if (!isValid(exitX, exitY)) {
            throw runtime_error("终点不可通行");
        }
        initialdp();//初始化dp表
        // 第一阶段：剪枝（移除单邻居节点）
        bool updated = true;
        const int directions[4][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };
        while (updated) {
            updated = false;
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < cols; ++j) {
                    if (!isValid(i, j) || maze.grid[i][j] == 'E' || vvisited[i][j] == 1) {
                        continue;
                    }

                    // 统计可用邻居
                    int neighborCount = 0;
                    int nX = -1, nY = -1;
                    for (const auto& dir : directions) {
                        int ni = i + dir[0];
                        int nj = j + dir[1];
                        if (isValid(ni, nj) && vvisited[ni][nj] == 0) {
                            neighborCount++;
                            nX = ni;
                            nY = nj;
                        }
                    }
                    // 单邻居节点剪枝
                    if (neighborCount == 1 && nX != -1 && nY != -1) {
                        vvisited[i][j] = 1;
                        updated = true;
                        if (dp[i][j] > 0) {
                            dp[nX][nY] += dp[i][j];
                        }
                    }
                }
            }
        }
        //===============================================================
        int t_num = 0;
        updated = 1;
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                cout << setw(4) << dp[i][j];
            }
            cout << endl;
        }
        vector<vector<bool>> startReachable = findReachableFromStart();
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                if (startReachable[i][j] == 1) {
                    dp[i][j] = dp[exitX][exitY];
                }
            }
        }
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                cout << setw(4) << dp[i][j];
            }
            cout << endl;
        }
        traverseAllSameDp();
        cout << endl;
        cout << "动态规划表（最大资源值）：" << dp[maze.exitX][maze.exitY] - 100 << endl;
        //printT();//打印资源表
        return true;
    }
    // 获取坐标的资源价值（获取后清空资源）
    int getResourceValue(int x, int y) {
        auto it = resourceMap.find({ x, y });
        if (it != resourceMap.end()) {
            int value = it->second;
            resourceMap.erase(it);
            return value;
        }
        return 0;
    }
    void printT()
    {
        cout << endl;
        cout << "动态规划表（最大资源值）：" << dp[maze.exitX][maze.exitY] << endl;
        for (int i = 0; i < maze.size; i++) {
            for (int j = 0; j < maze.size; j++)
            {
                if ((dp[i][j] == -1) && maze.grid[i][j] == '#')
                {
                    cout << setw(4) << '#' << " ";  // 4字符宽度，空格填充
                }
                else if ((dp[i][j] == -1) && maze.grid[i][j] == 'T')
                {
                    cout << setw(4) << "-3" << " ";  // 4字符宽度，空格填充
                }
                else if ((vvisited[i][j] == 1) && dp[i][j] > 0 || maze.grid[i][j] == 'S' || maze.grid[i][j] == 'E')
                {
                    cout << setw(4) << left << dp[maze.exitX][maze.exitY] << " ";  // 左对齐，4字符宽度
                }
                else
                {
                    cout << setw(4) << left << dp[i][j] << " ";  // 左对齐，4字符宽度
                }
            }
            cout << endl;
        }
    }
};
