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
    vector<pair<int, int>> prunedPath;     // 剪枝法计算的路径
    int prunedPathResource;                // 剪枝法路径的总资源值

    // 坐标转键值
    pair<int, int> makePair(int x, int y) const {
        return make_pair(x, y);
    }

    // 新增：获取当前资源值（不清除资源，用于剪枝）
    int getCurrentResourceValue(int x, int y) const {
        auto it = resourceMap.find(makePair(x, y));
        return (it != resourceMap.end()) ? it->second : 0;
    }

    // 新增：计算路径总资源值（剪枝法用）
    int calculateTotalResource(const vector<pair<int, int>>& path, const vector<vector<int>>& tempResource) const {
        int total = 0;
        for (size_t i = 0; i < path.size(); ++i) {
            int x = path[i].first;
            int y = path[i].second;
            total += tempResource[x][y];
        }
        return total;
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
            maze.grid[x][y] != '#' && maze.grid[x][y] != 'T';
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

    // 设置自定义资源分布（覆盖默认资源）
    void setResourceDistribution(const vector<ResourcePoint>& resources) {
        resourceMap.clear();
        for (const auto& point : resources) {
            resourceMap[makePair(point.x, point.y)] = point.value;
        }
    }

    // 执行动态规划求解资源最大化的最优路径
    bool solve() {
        int startX = maze.startX, startY = maze.startY;
        int exitX = maze.exitX, exitY = maze.exitY;

        // 检查起点有效性
        if (!isValid(startX, startY)) {
            throw runtime_error("起点不可通行");
        }

        // 创建dp的副本，防止被修改
        vector<vector<int>>originalDp = dp; // 假设originalDp已经在类中声明为vector<vector<int>>

        // 初始化动态规划表、前驱表和路径记录
        vector<vector<int>> resourceDp(maze.size, vector<int>(maze.size, -1));
        vector<vector<vector<pair<int, int>>>> path(maze.size, vector<vector<pair<int, int>>>(maze.size));

        // 起点初始化
        resourceDp[startX][startY] = getResourceValue(startX, startY);
        path[startX][startY].push_back({ startX, startY });

        // 使用优先队列按资源价值降序处理节点（优先探索资源更多的路径）
        priority_queue<pair<int, pair<int, int>>> pq;
        pq.push({ resourceDp[startX][startY], {startX, startY} });

        // 四个移动方向
        const int directions[4][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };

        while (!pq.empty()) {
            // 明确变量类型
            pair<int, pair<int, int>> currentPair = pq.top();
            pq.pop();
            int currentVal = currentPair.first;
            pair<int, int> current = currentPair.second;
            int x = current.first;
            int y = current.second;

            // 如果当前节点的资源值小于DP表中的记录，说明已找到更优路径，跳过
            if (currentVal < resourceDp[x][y]) {
                continue;
            }

            // 探索四个方向
            for (const auto& dir : directions) {
                int nx = x + dir[0];
                int ny = y + dir[1];
                if (isValid(nx, ny)) {
                    // 计算新路径的资源值，使用originalDp获取资源值
                    int newVal = currentVal + getResourceValue(nx, ny);
                    // 发现更优路径时更新
                    if (newVal > resourceDp[nx][ny]) {
                        resourceDp[nx][ny] = newVal;
                        path[nx][ny] = path[x][y];
                        path[nx][ny].push_back({ nx, ny });
                        pq.push({ newVal, {nx, ny} });
                    }
                }
            }
        }

        // 检查终点是否可达
        if (resourceDp[exitX][exitY] == -1e9) {
            return false;
        }

        // 定义finalPath变量并赋值
        vector<pair<int, int>> finalPath = path[exitX][exitY];

        // 输出资源最大化的路径和总资源值
        cout << "资源最大化路径（总资源值: " << resourceDp[exitX][exitY] << "）：" << endl;
        for (size_t i = 0; i < finalPath.size(); i++) {
            cout << "步骤 " << setw(4) << left << i << ": (" << finalPath[i].first << ", " << finalPath[i].second << ")  ";
            if ((i + 1) % 6 == 0) cout << endl;
        }
        return true;
    }
    // 获取最大资源值（若不可达则抛出异常）
    int getMaxResourceValue() const {
        int exitX = maze.exitX, exitY = maze.exitY;
        if (dp[exitX][exitY] == -1e9) {
            throw runtime_error("没有找到可达的路径"); // 抛出标准异常
        }
        return dp[exitX][exitY];
    }



    // 剪枝法路径优化函数（无需参数）
    bool solveWithPruning() {
        int startX = maze.startX;
        int startY = maze.startY;
        int exitX = maze.exitX;
        int exitY = maze.exitY;
        int rows = maze.size;
        int cols = maze.size;

        // 检查起点终点有效性
        if (!isValid(startX, startY)) {
            throw runtime_error("起点不可通行");
        }
        if (!isValid(exitX, exitY)) {
            throw runtime_error("终点不可通行");
        }
        bool vvisited[20][20] = { 0 };
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                if (isValid(i, j))
                {
                    dp[i][j] = getCurrentResourceValue(i, j);
                }
                else
                {
                    dp[i][j] = -1;
                }

            }
        }
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                cout << dp[i][j] << "   ";
            }
            cout << endl;
        }

        // 第一阶段：剪枝（移除单邻居节点）
        bool updated = true;
        const int directions[4][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };
        while (updated) {
            updated = false;
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < cols; ++j) {
                    if (!isValid(i, j) || maze.grid[i][j] == 'S' || maze.grid[i][j] == 'E' || vvisited[i][j] == 1) {
                        continue;
                    }

                    // 统计可用邻居
                    int neighborCount = 0;
                    int nX = -1, nY = -1;
                    for (const auto& dir : directions) {
                        int ni = i + dir[0];
                        int nj = j + dir[1];
                        if (isValid(ni, nj) && vvisited[ni][nj] != 1) {
                            neighborCount++;
                            nX = ni;
                            nY = nj;
                        }
                    }
                    // 单邻居节点剪枝
                    if (neighborCount == 1 && nX != -1 && nY != -1) {
                        cout << i << j << endl;
                        vvisited[i][j] = 1;
                        updated = true;
                        if (dp[i][j] > 0) {
                            dp[nX][nY] += dp[i][j];
                        }
                    }
                }
            }
            cout << endl;
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < cols; ++j) {
                    cout << setw(4) << left << dp[i][j] << " ";
                }
                cout << endl;
            }
        }
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                cout << vvisited[i][j] << "   ";
            }
            cout << endl;
        }
        //===================
        int t_num = 0;
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                if (vvisited[i][j] == 1 && maze.grid[i][j] == 'T')//主路经陷阱清除
                {
                    t_num++;
                    maze.grid[i][j] == ' ';
                    dp[i][j] = 0;
                }
                else if ((vvisited[i][j] == 1)&&dp[i][j]>0 || maze.grid[i][j] == 'S')//可用值强化
                {
                    dp[i][j] += 100;
                }
            }
            cout << endl;
        }
        dp[startX][startY] -=3*t_num;
        updated = 1;
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
                        if (isValid(ni, nj) && vvisited[ni][nj] != 1) {
                            neighborCount++;
                            nX = ni;
                            nY = nj;
                        }
                    }
                    // 单邻居节点剪枝
                    if (neighborCount == 1 && nX != -1 && nY != -1) {
                        cout << i << j << endl;
                        vvisited[i][j] = 1;
                        updated = true;
                        if (dp[i][j] > 0) {
                            dp[nX][nY] += dp[i][j];
                        }
                    }
                }
            }
            cout << endl;
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < cols; ++j) {
                    cout << setw(4) << left << dp[i][j] << " ";
                }
                cout << endl;
            }
        }

        solve();
         cout << endl;
        cout << "动态规划表（最大资源值）：" << endl;
        for (int i = 0; i < maze.size; i++) {
            for (int j = 0; j < maze.size; j++)
            {
                if (dp[i][j] == -1) 
                {
                    cout << "*    ";  // 4字符宽度，空格填充
                }
                else if ((vvisited[i][j] == 1) && dp[i][j] > 0 || maze.grid[i][j] == 'S'|| maze.grid[i][j] == 'E')
                {
                    cout << setw(4) << left << dp[i][j] - 100 << " ";  // 左对齐，4字符宽度
                }
                else
                {
                    cout << setw(4) << left << dp[i][j]  << " ";  // 左对齐，4字符宽度
                }
            }
            cout << endl;
        }

        return true;
    }

    // 获取剪枝法结果的路径
    vector<pair<int, int>> getPrunedPath() const {
        return prunedPath;
    }

    // 获取剪枝法结果的资源值
    int getPrunedPathResource() const {
        return prunedPathResource;
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

    // 获取最优路径
    vector<pair<int, int>> getOptimalPath() const {
        vector<pair<int, int>> path;
        int x = maze.exitX;
        int y = maze.exitY;

        // 如果终点不可达，返回空路径
        if (dp[x][y] == -1e9) {
            return path;
        }

        // 从终点回溯到起点
        while (x != -1 && y != -1) {
            path.push_back({ x, y });
            auto prevNode = predecessor[x][y];
            x = prevNode.first;
            y = prevNode.second;
        }

        // 反转路径使其从起点到终点
        reverse(path.begin(), path.end());
        return path;
    }
    // 简化版动态规划表打印函数（固定宽度4字符）
    void printDPTable() const {
       
    }
};

// 路径可视化函数
void visualizePath(const Maze& maze, const std::vector<std::pair<int, int>>& path) {
    Maze tempMaze = maze;
    for (size_t i = 0; i < path.size(); i++) {
        int x = path[i].first;
        int y = path[i].second;
        if (i == 0) {
            tempMaze.grid[x][y] = 'S';  // 起点
        }
        else if (i == path.size() - 1) {
            tempMaze.grid[x][y] = 'E';  // 终点
        }
        else {
            tempMaze.grid[x][y] = '$';  // 路径标记
        }
    }
    cout << "最优资源收集路径：" << endl;
    printMaze(tempMaze);  // 假设maze.h中存在printMaze函数
}