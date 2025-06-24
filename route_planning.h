#pragma once
#include <iostream>          // 输入输出流库，用于控制台输出（如错误信息、结果展示）
#include <vector>            // 动态数组容器，用于存储DP表、前驱节点表和路径序列
#include <map>               // 关联容器，用于建立坐标到资源价值的映射关系
#include <stack>             // 栈容器，用于路径回溯时临时存储坐标
#include <algorithm>         // 算法库，提供max等常用算法和容器操作函数
#include <stdexcept>         // 标准异常库，用于抛出路径不可达等异常
#include "maze.h"            // 自定义迷宫生成头文件，提供Maze结构体和生成算法
#include <queue>
#include <iomanip>           //规律打印
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
    Maze maze;                            // 迷宫对象，依赖maze.h中定义的结构
    vector<vector<int>> dp;               // 动态规划表，使用vector实现二维数组
    vector<vector<pair<int, int>>> prev;  // 前驱节点表，存储路径回溯信息
    map<pair<int, int>, int> resourceMap;  // 资源映射表，用map实现坐标到价值的映射

    // 将坐标对转换为唯一键
    pair<int, int> makePair(int x, int y) const {
        return make_pair(x, y);
    }

public:

    // 检查坐标是否有效（非墙、非陷阱、在迷宫范围内）
    bool isValid(int x, int y) const {
        return x >= 0 && x < maze.size && y >= 0 && y < maze.size &&
            maze.grid[x][y] != '#' && maze.grid[x][y] != 'T';
    }

    // 构造函数，接收迷宫对象并初始化DP表
    ResourcePathPlanner(const Maze& m) : maze(m) {
        dp.resize(maze.size, vector<int>(maze.size, -1e9));     // vector动态调整大小
        prev.resize(maze.size, vector<pair<int, int>>(maze.size, { -1, -1 }));
        initializeResourceMap();
    }

    // 初始化资源映射表（从迷宫矩阵提取资源价值）
    void initializeResourceMap() {
        resourceMap.clear();//确保每次初始化都是从头开始，避免残留旧数据
        for (int i = 0; i < maze.size; i++) {
            for (int j = 0; j < maze.size; j++) {
                char cell = maze.grid[i][j];
                // 使用map插入键值对（坐标-资源价值）
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

    // 执行动态规划求解最优路径
    bool solve() {
        int startX = maze.startX, startY = maze.startY;
        int exitX = maze.exitX, exitY = maze.exitY;

        // 检查起点有效性
        if (!isValid(startX, startY)) {
            throw runtime_error("起点不可通行");
        }

        // 初始化DP表、前驱表和访问标记
        dp.assign(maze.size, vector<int>(maze.size, -1e9));
        prev.assign(maze.size, vector<pair<int, int>>(maze.size, { -1, -1 }));
        vector<vector<bool>> visited(maze.size, vector<bool>(maze.size, false));

        // 起点初始化
        dp[startX][startY] =0;
        visited[startX][startY] = true;

        // 使用队列按层次处理节点
        queue<pair<int, int>> processQueue;
        processQueue.push({ startX, startY });

        // 四个移动方向
        const int directions[4][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };

        while (!processQueue.empty()) {
            pair<int, int> current = processQueue.front();
            int x = current.first;
            int y = current.second;
            processQueue.pop();

            // 探索四个方向
            for (const auto& dir : directions) {
                int nx = x + dir[0];
                int ny = y + dir[1];
                if (isValid(nx, ny)) {
                    // 计算新路径的资源值
                    int newVal = dp[x][y] + getResourceValue(nx, ny);
                    // 发现更优路径且新节点未被最优处理时更新
                    if (newVal > dp[nx][ny] && !visited[nx][ny]) {
                        dp[nx][ny] = newVal;
                        prev[nx][ny] = { x, y };

                        // 仅当节点未被访问过时加入队列
                        if (!visited[nx][ny]) {
                            visited[nx][ny] = true;
                            processQueue.push({ nx, ny });
                        }
                    }
                }
            }
        }

        // 检查终点是否可达
        return dp[exitX][exitY] != -1e9;
    }

    // 获取坐标的资源价值（获取后清空资源）
    int getResourceValue(int x, int y) {
        auto it = resourceMap.find({ x, y });
        if (it != resourceMap.end()) {
            int value = it->second;  // 获取资源价值
            resourceMap.erase(it);   // 从资源表中移除
            return value;
        }
        return 0;  // 无资源或已被采集
    }

    // 获取最大资源值（若不可达则抛出异常）
    int getMaxResourceValue() const {
        int exitX = maze.exitX, exitY = maze.exitY;
        if (dp[exitX][exitY] == -1e9) {
            throw runtime_error("没有找到可达的路径"); // 抛出标准异常
        }
        return dp[exitX][exitY];
    }

    // 获取最优路径序列（通过栈回溯路径）
    vector<pair<int, int>> getOptimalPath() const {
        int exitX = maze.exitX, exitY = maze.exitY;
        if (dp[exitX][exitY] == -1e9) {
            throw runtime_error("没有找到可达的路径");
        }

        vector<pair<int, int>> path;
        stack<pair<int, int>> pathStack; // 使用栈回溯路径

        int x = exitX, y = exitY;
        while (x != -1 && y != -1) {
            pathStack.push({ x, y });
            auto p = prev[x][y];
            x = p.first;
            y = p.second;
        }

        // 反转栈中元素顺序（vector的push_back操作）
        while (!pathStack.empty()) {
            path.push_back(pathStack.top());
            pathStack.pop();
        }

        return path;
    }

    // 简化版动态规划表打印函数（固定宽度4字符）
    void printDPTable() const {
        cout << "动态规划表（最大资源值）：" << endl;
        for (int i = 0; i < maze.size; i++) {
            for (int j = 0; j < maze.size; j++) {
                if (dp[i][j] == -1e9) {
                    cout << "*    ";  // 4字符宽度，空格填充
                }
                else {
                    cout << setw(4) << left << dp[i][j] << " ";  // 左对齐，4字符宽度
                }
            }
            cout << endl;
        }
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
    printMaze(tempMaze);
}