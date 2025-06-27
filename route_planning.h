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
            maze.grid[x][y] != '#' && maze.grid[x][y] != 'T';
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

    // 执行动态规划求解资源最大化的最优路径
    bool solve() {
        int startX = maze.startX, startY = maze.startY;
        int exitX = maze.exitX, exitY = maze.exitY;

        // 检查起点有效性
        if (!isValid(startX, startY)) {
            throw runtime_error("起点不可通行");
        }

        // 初始化动态规划表、前驱表和路径记录
        vector<vector<int>> resourceDp(maze.size, vector<int>(maze.size, -1));
        vector<vector<vector<pair<int, int>>>> path(maze.size, vector<vector<pair<int, int>>>(maze.size));
        // 起点初始化
        resourceDp[startX][startY] = getResourceValue(startX, startY)+100;
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
                if (dp[nx][ny] != -1) {
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

        // 输出资源最大化的路径
        cout << "资源最大化路径: " << endl;
        for (size_t i = 0; i < finalPath.size(); i++) {
            system("cls");
            cout << "步骤 " << setw(4) << left << i << ": (" << finalPath[i].first << ", " << finalPath[i].second << ")  " << endl;
            for (int j = 0; j < maze.size; ++j) {
                for (int k = 0; k < maze.size; ++k) {
                    if (finalPath[i].first == j && finalPath[i].second == k)
                    {
                        maze.grid[j][k] = ' ';
                        std::cout << ANSI_COLOR_GREEN << "&" << ANSI_COLOR_RESET;
                    }
                    else if (maze.grid[j][k] == '#')
                    {
                        std::cout << ANSI_COLOR_BLUE << maze.grid[j][k] << ANSI_COLOR_RESET;
                    }
                    else if (maze.grid[j][k] == 'T')
                    {
                        std::cout << ANSI_COLOR_RED << maze.grid[j][k] << ANSI_COLOR_RESET;
                    }
                    else if (maze.grid[j][k] == 'G')
                    {
                        std::cout << ANSI_COLOR_YELLOW << maze.grid[j][k] << ANSI_COLOR_RESET;
                    }
                    else cout << maze.grid[j][k];
                }
                cout << endl;
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        for (size_t i = 0; i < finalPath.size(); i++) {
            cout << "步骤 " << setw(4) << left << i << ": (" << finalPath[i].first << ", " << finalPath[i].second << ")  ";
            if ((i + 1) % 6 == 0) cout << endl;
        }
        return true;
    }

    // 动态规划路径优化函数（无需参数）
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
        initialdp();//初始化dp表

        // 第一阶段：剪枝（移除单邻居节点）
        bool updated = true;
        const int directions[4][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };
        while (updated) {
            updated = false;
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < cols; ++j) {
                    if (dp[i][j] == -1 || maze.grid[i][j] == 'S' || maze.grid[i][j] == 'E' || vvisited[i][j] == 1) {
                        continue;
                    }

                    // 统计可用邻居
                    int neighborCount = 0;
                    int nX = -1, nY = -1;
                    for (const auto& dir : directions) {
                        int ni = i + dir[0];
                        int nj = j + dir[1];
                        if (dp[ni][nj] != -1 && vvisited[ni][nj] == 0) {
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
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                if (vvisited[i][j] == 0 && maze.grid[i][j] == 'T')//主路经陷阱清除
                {
                    t_num++;
                    maze.grid[i][j] == ' ';
                    dp[i][j] = 0;
                }
                else if ((vvisited[i][j] == 1) && dp[i][j] > 0 || maze.grid[i][j] == 'S')//可用值强化
                {
                    dp[i][j] += 100;
                }
            }
        }
        dp[startX][startY] -= 3 * t_num;//起点加载
        updated = 1;

        
        while (updated) {
            updated = false;
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < cols; ++j) {
                    if (dp[i][j] == -1 || maze.grid[i][j] == 'E' || vvisited[i][j] == 1) {
                        continue;
                    }

                    // 统计可用邻居
                    int neighborCount = 0;
                    int nX = -1, nY = -1;
                    for (const auto& dir : directions) {
                        int ni = i + dir[0];
                        int nj = j + dir[1];
                        if ((dp[ni][nj] != -1) && vvisited[ni][nj] != 1) {
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
        solve();////规划路径
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
        cout << "动态规划表（最大资源值）：" << dp[maze.exitX][maze.exitY] - 100 << endl;
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
                    cout << setw(4) << left << dp[maze.exitX][maze.exitY] - 100 << " ";  // 左对齐，4字符宽度
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
