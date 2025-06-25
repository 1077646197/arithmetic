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

    // 执行动态规划求解资源最大化的最优路径
    bool solve() {
        int startX = maze.startX, startY = maze.startY;
        int exitX = maze.exitX, exitY = maze.exitY;

        // 检查起点有效性
        if (!isValid(startX, startY)) {
            throw runtime_error("起点不可通行");
        }

        // 初始化DP表、前驱表和路径记录
        dp.assign(maze.size, vector<int>(maze.size, -1e9));
        vector<vector<vector<pair<int, int>>>> path(maze.size, vector<vector<pair<int, int>>>(maze.size));

        // 起点初始化
        dp[startX][startY] = getResourceValue(startX, startY);
        path[startX][startY].push_back({ startX, startY });

        // 使用优先队列按资源价值降序处理节点（优先探索资源更多的路径）
        priority_queue<pair<int, pair<int, int>>> pq;
        pq.push({ dp[startX][startY], {startX, startY} });

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
            if (currentVal < dp[x][y]) {
                continue;
            }

            // 探索四个方向
            for (const auto& dir : directions) {
                int nx = x + dir[0];
                int ny = y + dir[1];
                if (isValid(nx, ny)) {
                    // 计算新路径的资源值
                    int newVal = currentVal + getResourceValue(nx, ny);
                    // 发现更优路径时更新
                    if (newVal > dp[nx][ny]) {
                        dp[nx][ny] = newVal;
                        path[nx][ny] = path[x][y];
                        path[nx][ny].push_back({ nx, ny });
                        pq.push({ newVal, {nx, ny} });
                    }
                }
            }
        }

        // 检查终点是否可达
        if (dp[exitX][exitY] == -1e9) {
            return false;
        }

        // 定义finalPath变量并赋值
        vector<pair<int, int>> finalPath = path[exitX][exitY];

        // 输出资源最大化的路径和总资源值
        cout << "资源最大化路径（总资源值: " << dp[exitX][exitY] << "）：" << endl;
        for (size_t i = 0; i < finalPath.size(); i++) {
            cout << "步骤 " << setw(4) << left<< i << ": (" << finalPath[i].first << ", " << finalPath[i].second << ")  ";
            if ((i+1) % 6 == 0) cout << endl;
        }

        return true;
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


    // 简化版动态规划表打印函数（固定宽度4字符）
    void printDPTable() const {
        cout << endl;
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