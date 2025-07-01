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
#include <set> 
#include <unordered_set>

// ANSI 颜色控制码宏定义
#define ANSI_COLOR_GREEN "\033[32m"   // 绿色文本
#define ANSI_COLOR_BLUE "\033[34m"    // 蓝色文本
#define ANSI_COLOR_RED "\033[31m"     // 红色文本
#define ANSI_COLOR_YELLOW "\033[33m"  // 黄色文本
#define ANSI_COLOR_RESET "\033[0m"    // 重置为默认颜色

using namespace std;
#define MAX_SIZE 101  // 最大迷宫尺寸
// 迷宫结构体
struct Maze {
    char grid[MAX_SIZE][MAX_SIZE];
    int size;
    int startX, startY;
    int exitX, exitY;

    // 初始化迷宫
    void init(int s) {
        if (s < 7 || s > MAX_SIZE || s % 2 == 0) {
            std::cerr << "错误：迷宫尺寸必须是大于等于7且小于等于" << MAX_SIZE << "的奇数" << std::endl;
            exit(1);
        }
        size = s;
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                grid[i][j] = '#';
            }
        }
    }
};

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
    vector<vector<int>> dp2;              // dp2数组
    vector<pair<int, int>> fullPath;      // 完整路径存储
    map<pair<int, int>, int> resourceMap;  // 资源映射表
    vector<vector<pair<int, int>>> predecessor;  // 前驱节点存储
    int prunedPathResource;               // 剪枝法路径的总资源值
    bool vvisited[20][20];
    bool visited[20][20];                 // 访问标记数组
    vector<pair<int, int>> validPoints;   // 收集的有效点集合(dp>0且与起点连通)
    int requiredPointsCount;              // 需要访问的点的数量
    vector<pair<int, int>> path;          // 当前回溯路径
    vector<vector<pair<int, int>>> allPaths; // 所有可能路径
    int startX, startY, exitX, exitY;     // 起点和终点坐标
    const int dirs[4][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} }; // 方向数组

    // 坐标转键值（辅助函数）
    pair<int, int> makePair(int x, int y) const {
        return make_pair(x, y);
    }

    // 获取当前资源值（不清除资源，用于剪枝）
    int getCurrentResourceValue(int x, int y) const {
        auto it = resourceMap.find(makePair(x, y));
        return (it != resourceMap.end()) ? it->second : 0;
    }

    // 检查两点是否连通
    bool isConnected(int startX, int startY, int targetX, int targetY) {
        vector<vector<bool>> visited(maze.size, vector<bool>(maze.size, false));
        queue<pair<int, int>> q;
        q.push(make_pair(startX, startY));
        visited[startX][startY] = true;

        while (!q.empty()) {
            pair<int, int> curr = q.front();
            q.pop();
            int x = curr.first;
            int y = curr.second;

            if (x == targetX && y == targetY) {
                return true;
            }

            for (int d = 0; d < 4; ++d) {
                int nx = x + dirs[d][0];
                int ny = y + dirs[d][1];

                if (nx >= 0 && nx < maze.size && ny >= 0 && ny < maze.size &&
                    isValid(nx, ny) && !visited[nx][ny]) {
                    q.push(make_pair(nx, ny));
                    visited[nx][ny] = true;
                }
            }
        }
        return false;
    }
    // 判断是否为分支点（多于1个未访问的有效邻居）
    bool isBranchPoint(int x, int y, const vector<vector<bool>>& pathVisited) {
        int neighborCount = 0;
        for (int d = 0; d < 4; ++d) {
            int nx = x + dirs[d][0];
            int ny = y + dirs[d][1];
            if (isValid(nx, ny) && dp[nx][ny] > 0 && !pathVisited[nx][ny]) {
                neighborCount++;
            }
        }
        return neighborCount > 1;
    }

    // 回溯函数
    bool backtrack(int x, int y, vector<vector<bool>>& pathVisited,
        vector<pair<int, int>>& currentPath, int visitedCount) {
        // 将当前点加入路径
        currentPath.push_back(make_pair(x, y));
        pathVisited[x][y] = true;

        // 检查是否访问了所有有效点并到达终点
        if (x == exitX && y == exitY && visitedCount == requiredPointsCount) {
            fullPath = currentPath;
            return true;
        }

        // 探索四个方向
        for (int d = 0; d < 4; ++d) {
            int nx = x + dirs[d][0];
            int ny = y + dirs[d][1];

            if (isValid(nx, ny) && dp[nx][ny] > 0 && !pathVisited[nx][ny]) {
                // 计算访问的有效点数
                int newVisitedCount = visitedCount;
                if (find(validPoints.begin(), validPoints.end(), make_pair(nx, ny)) != validPoints.end()) {
                    newVisitedCount++;
                }

                if (backtrack(nx, ny, pathVisited, currentPath, newVisitedCount)) {
                    return true;
                }
            }
        }

        // 回溯：从路径中移除当前点，标记为未访问
        currentPath.pop_back();
        pathVisited[x][y] = false;

        // 如果是分支点，尝试其他路径
        if (isBranchPoint(x, y, pathVisited)) {
            return false; // 继续尝试其他分支
        }

        return false;
    }

public:
    // 构造函数
    ResourcePathPlanner(const Maze& m) : maze(m), prunedPathResource(0) {
        startX = maze.startX;
        startY = maze.startY;
        exitX = maze.exitX;
        exitY = maze.exitY;

        // 初始化访问标记数组
        for (int i = 0; i < 20; ++i) {
            for (int j = 0; j < 20; ++j) {
                visited[i][j] = false;
            }
        }

        // 初始化动态规划表
        dp.resize(maze.size, vector<int>(maze.size, -1e9));
        dp2.resize(maze.size, vector<int>(maze.size, -1));
        predecessor.resize(maze.size, vector<pair<int, int>>(maze.size, make_pair(-1, -1)));
        initializeResourceMap();
    }

    // 检查坐标是否有效（非墙、在迷宫范围内）
    bool isValid(int x, int y) const {
        return x >= 0 && x < maze.size && y >= 0 && y < maze.size &&
            maze.grid[x][y] != '#';
    }

    // 初始化dp表
    void initialdp() {
        for (int i = 0; i < maze.size; ++i) {
            for (int j = 0; j < maze.size; ++j) {
                if (isValid(i, j)) {
                    dp[i][j] = getCurrentResourceValue(i, j);
                }
                else if (maze.grid[i][j] == 'T') {
                    dp[i][j] = -3;
                }
                else {
                    dp[i][j] = -1;
                }
            }
        }
    }

    // 初始化资源映射表（从迷宫矩阵提取资源价值）
    void initializeResourceMap() {
        resourceMap.clear();
        for (int i = 0; i < maze.size; ++i) {
            for (int j = 0; j < maze.size; ++j) {
                char cell = maze.grid[i][j];
                if (cell == 'G') {
                    resourceMap[makePair(i, j)] = 5;
                }
                else if (cell == 'L') {
                    resourceMap[makePair(i, j)] = 10;
                }
                else if (cell == 'B') {
                    resourceMap[makePair(i, j)] = 20;
                }
                else if (cell == 'T') {
                    resourceMap[makePair(i, j)] = -3;
                }
            }
        }
    }
    // 判断是否为分支点（多于2个有效邻居）
    bool isfp(int x, int y) {
        int neighborCount = 0;
        for (int d = 0; d < 4; ++d) {
            int ni = x + dirs[d][0];
            int nj = y + dirs[d][1];
            if (isValid(ni, nj) && dp[ni][nj] > 0) {
                neighborCount++;
            }
        }
        return neighborCount > 2;
    }

    // 判断是否为末端点（3个无效邻居）
    bool isfq(int x, int y) {
        int neighborCount = 0;
        for (int d = 0; d < 4; ++d) {
            int ni = x + dirs[d][0];
            int nj = y + dirs[d][1];
            if (isValid(ni, nj) && dp[ni][nj] > 0) {
                neighborCount++;
            }
        }
        return neighborCount == 1;
    }
    // 获取坐标的资源价值（获取后清空资源）
    int getResourceValue(int x, int y) {
        auto it = resourceMap.find(make_pair(x, y));
        if (it != resourceMap.end()) {
            int value = it->second;
            resourceMap.erase(it);
            return value;
        }
        return 0;
    }

    // 打印动态规划表
    void printT() {
        cout << endl << "动态规划表（最大资源值）:" << dp[maze.exitX][maze.exitY] << endl;
        for (int i = 0; i < maze.size; ++i) {
            for (int j = 0; j < maze.size; ++j) {
                if (maze.grid[i][j] == '#') {
                    cout << setw(4) << '#' << " ";
                }
                else if (maze.grid[i][j] == 'T') {
                    cout << setw(4) << "-3" << " ";
                }
                else if ((visited[i][j] && dp[i][j] > 0) || maze.grid[i][j] == 'S' || maze.grid[i][j] == 'E') {
                    cout << setw(4) << left << dp[maze.exitX][maze.exitY] << " ";
                }
                else {
                    cout << setw(4) << left << dp[i][j] << " ";
                }
            }
            cout << endl;
        }
        // 输出dp2数组（包含0值节点）
        cout << "dp2数组（包含>=0的节点）：" << endl;
        for (int i = 0; i < maze.size; ++i) {
            for (int j = 0; j < maze.size; ++j) {
                cout << setw(4) << dp2[i][j];
            }
            cout << endl;
        }
    }
    // 动态规划路径优化函数（剪枝法）
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

        initialdp(); // 初始化dp表
        dp[startX][startY] += 100; // 起点初始值加成

        // 第一阶段：剪枝（移除单邻居节点）
        bool updated = true;
        while (updated) {
            updated = false;
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < cols; ++j) {
                    if (!isValid(i, j) || maze.grid[i][j] == 'E' || vvisited[i][j]) {
                        continue;
                    }

                    // 统计可用邻居
                    int neighborCount = 0;
                    int nX = -1, nY = -1;
                    for (int d = 0; d < 4; ++d) {
                        int ni = i + dirs[d][0];
                        int nj = j + dirs[d][1];
                        if (isValid(ni, nj) && !vvisited[ni][nj]) {
                            neighborCount++;
                            nX = ni;
                            nY = nj;
                        }
                    }

                    // 处理末端点（确保dp2=0被包含）
                    if (neighborCount == 1 && isfq(i, j)) {
                        dp2[i][j] = 0; // 明确设置为0，确保被纳入遍历
                    }

                    // 单邻居节点剪枝
                    if (neighborCount == 1 && nX != -1 && nY != -1) {
                        vvisited[i][j] = true;
                        updated = true;
                        // 资源值传递
                        if (dp[i][j] > 0) {
                            dp[nX][nY] += dp[i][j];
                        }
                    }
                }
            }
        }

        // 输出dp2数组（包含0值节点）
        cout << "dp2数组（包含>=0的节点）：" << endl;
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                dp2[i][j] = 0;
            }
        }

        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                if (dp[i][j] > 0) {
                    for (int d = 0; d < 4; ++d) {
                        int ni = i + dirs[d][0];
                        int nj = j + dirs[d][1];
                        if (!isValid(ni, nj))continue;
                        if (dp[ni][nj] > 0) {
                            dp2[i][j]++;
                        }
                    }
                    if ((dp[i][j] > 100 && maze.grid[i][j] != 'E' && maze.grid[i][j] != 'S')) {
                        dp2[i][j]--;
                    }
                    if (isfq(i, j) && maze.grid[i][j] != 'E' && maze.grid[i][j] != 'S') {
                        dp2[i][j]++;
                    }
                }
            }
        }
        solve();
        // 执行按dp2值遍历（包含0值节点）

        return true;
    }
    bool solve() {
        int startX = maze.startX, startY = maze.startY;
        int exitX = maze.exitX, exitY = maze.exitY;

        // 检查起点有效性
        if (!isValid(startX, startY)) {
            throw runtime_error("起点不可通行");
        }

        // 初始化路径记录
        vector<vector<vector<pair<int, int>>>> path(maze.size, vector<vector<pair<int, int>>>(maze.size));
        vector<vector<pair<int, int>>> parent(maze.size, vector<pair<int, int>>(maze.size, { -1, -1 }));
        // 备份原始dp2数组，用于显示
        int finalpath[400][2]; int steps = 0;
        vector<vector<int>> originalDp2 = dp2;

        // 起点初始化
        path[startX][startY].push_back({ startX, startY });
        dp2[startX][startY]--; // 减少起点的可用次数

        // 使用优先队列按dp2值降序处理节点
        priority_queue<pair<int, pair<int, int>>> pq;
        pq.push({ dp2[startX][startY], {startX, startY} });

        // 四个移动方向
        const int directions[4][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };

        while (!pq.empty()) {
            int currentDp2 = pq.top().first;
            int x = pq.top().second.first;
            int y = pq.top().second.second;

            pq.pop();

            // 如果找到终点，直接返回
            if (x == exitX && y == exitY) {
                finalpath[steps][0] = x;
                finalpath[steps][1] = y;
                steps++;
                break;
            }
            int t = 0;
            int ndp2[4] = { 0 };
            for (const auto& dir : directions) {

                int nx = x + dir[0];
                int ny = y + dir[1];

                if (!isValid(nx, ny))continue;

                ndp2[t++] = dp2[nx][ny];
            }
            std::sort(ndp2, ndp2 + 4);
            // 探索四个方向
            for (const auto& dir : directions) {
                int nx = x + dir[0];
                int ny = y + dir[1];
                if (!isValid(nx, ny))continue;
                if (dp2[nx][ny] <= 0) continue;
                if (parent[x][y].first == nx && parent[x][y].second == ny && !isfq(x, y)) continue;
                if (dp2[nx][ny] == ndp2[3])
                {
                    if (1) {
                        finalpath[steps][0] = x;
                        finalpath[steps][1] = y;
                        steps++;
                        path[nx][ny] = path[x][y];
                        path[nx][ny].push_back({ nx, ny });
                        parent[nx][ny] = { x, y };
                        // 减少目标点的可用次数
                        dp2[nx][ny]--;
                        pq.push({ dp2[nx][ny], {nx, ny} });
                    }
                    break;
                }
            }
        }

        // 检查终点是否可达
        if (path[exitX][exitY].empty()) {
            cout << "无法找到到达终点的路径" << endl;

            // 恢复原始dp2数组
            dp2 = originalDp2;
            return false;
        }

        // 恢复原始dp2数组
        dp2 = originalDp2;

        // 输出基于dp2值最大的路径
        cout << "基于dp2值最大优先的路径:" << endl;
        for (size_t i = 0; i < steps; i++) {
            system("cls");
            cout << "步骤 " << setw(4) << left << i << ": (" << finalpath[i][0] << ", " << finalpath[i][1] << ")  " << endl;

            // 显示迷宫状态
            for (int j = 0; j < maze.size; ++j) {
                for (int k = 0; k < maze.size; ++k) {
                    if (finalpath[i][0] == j && finalpath[i][1] == k) {
                        cout << ANSI_COLOR_GREEN << "&" << ANSI_COLOR_RESET;
                        maze.grid[j][k] = ' ';
                    }
                    else if (maze.grid[j][k] == '#') {
                        cout << ANSI_COLOR_BLUE << maze.grid[j][k] << ANSI_COLOR_RESET;
                    }
                    else if (maze.grid[j][k] == 'T') {
                        cout << ANSI_COLOR_RED << maze.grid[j][k] << ANSI_COLOR_RESET;
                    }
                    else if (maze.grid[j][k] == 'G' || maze.grid[j][k] == 'L' || maze.grid[j][k] == 'B') {
                        cout << ANSI_COLOR_YELLOW << maze.grid[j][k] << ANSI_COLOR_RESET;
                    }
                    else {
                        cout << maze.grid[j][k];
                    }
                }
                cout << endl;
            }
            this_thread::sleep_for(chrono::milliseconds(300));
        }

        // 文本形式输出完整路径
        cout << "\n完整路径:" << endl;
        for (size_t i = 0; i < steps; i++) {
            cout << "步骤 " << setw(4) << left << i << ": (" << finalpath[i][0] << ", " << finalpath[i][1] << ")  ";
            if ((i + 1) % 6 == 0) cout << endl;
        }
        return true;
    }
};