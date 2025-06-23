#pragma once
#include <iostream>          // 输入输出流库，用于控制台输出（如错误信息、结果展示）
#include <vector>            // 动态数组容器，用于存储DP表、前驱节点表和路径序列
#include <map>               // 关联容器，用于建立坐标到资源价值的映射关系
#include <stack>             // 栈容器，用于路径回溯时临时存储坐标
#include <algorithm>         // 算法库，提供max等常用算法和容器操作函数
#include <stdexcept>         // 标准异常库，用于抛出路径不可达等异常
#include "maze.h"            // 自定义迷宫生成头文件，提供Maze结构体和生成算法
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

        dp[startX][startY] = getResourceValue(startX, startY);

        // 按行优先遍历迷宫（vector的二维索引访问）
        for (int i = 0; i < maze.size; i++) {
            for (int j = 0; j < maze.size; j++) {
                if (!isValid(i, j) || dp[i][j] == -1e9) continue;

                // 尝试四个方向移动（使用algorithm中的max比较值）
                int directions[4][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };
                for (const auto& dir : directions) {
                    int nx = i + dir[0], ny = j + dir[1];
                    if (isValid(nx, ny)) {
                        int newVal = dp[i][j] + getResourceValue(nx, ny);
                        if (newVal > dp[nx][ny]) {
                            dp[nx][ny] = newVal;
                            prev[nx][ny] = { i, j };
                        }
                    }
                }
            }
        }

        // 检查终点是否可达（使用逻辑非操作）
        return dp[exitX][exitY] != -1e9;
    }

    // 获取坐标的资源价值（通过map查找）
    int getResourceValue(int x, int y) const {
        auto it = resourceMap.find(makePair(x, y));
        return it != resourceMap.end() ? it->second : 0;
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
};