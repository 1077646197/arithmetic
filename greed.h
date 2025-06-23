#pragma once
#include <iostream>
#include <vector>
#include <random>
#include <fstream>
#include <string>
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <cmath>
#include "maze.h"

class Player {
private:
    int row, col;         // 玩家当前位置
    int score;            // 玩家分数

public:
    // 初始化玩家
    Player(int startRow, int startCol) : row(startRow), col(startCol), score(0) {}

    // 获取玩家位置
    pair<int, int> getPosition() const {
        return { row, col };
    }

    // 更新玩家位置
    void setPosition(int newRow, int newCol) {
        row = newRow;
        col = newCol;
    }

    // 获取玩家分数
    int getScore() const {
        return score;
    }

    // 更新玩家分数
    void updateScore(int delta) {
        score += delta;
    }
};

// 资源信息结构体
struct Resource {
    int row, col;         // 资源位置
    char type;            // 资源类型 'G'(金币), 'T'(陷阱)
    int value;            // 资源价值
    int distance;         // 到玩家的距离
    double costPerformance; // 性价比(单位距离收益)

    Resource(int r, int c, char t, int val) : row(r), col(c), type(t), value(val),
        distance(0), costPerformance(0.0) {}
};

// 路径节点结构体，用于BFS寻路
struct PathNode {
    int row, col;         // 节点位置
    int distance;         // 从起点到当前节点的距离
    PathNode* parent;     // 父节点，用于重建路径

    PathNode(int r, int c, int d, PathNode* p) : row(r), col(c), distance(d), parent(p) {}
};

// 资源拾取策略类，实现贪心算法
class ResourcePickingStrategy {
private:
    vector<vector<char>> mazeCopy; // 存储迷宫副本,记录资源被拾取后的状态变化
    int mazeSize;                    // 迷宫大小，行数/列数
    vector<pair<int, int>> visitedPositions; // 记录已访问的位置，用于检测循环。
    int loopThreshold;               // 循环检测阈值，当最近访问的位置重复次数超过阈值的一半时认为陷入循环。

    // 四个方向的移动向量
    const int dirs[4][2] = { {-1, 0}, {0, 1}, {1, 0}, {0, -1} };

public:
    ResourcePickingStrategy(vector<vector<char>> m) : mazeCopy(m), mazeSize(m.size()) {
        loopThreshold = mazeSize * 2; // 设置循环检测阈值
    }

    // 在迷宫上绘制路径并输出
    void displayMazeWithPath(const vector<pair<int, int>>& path) {
        // 创建迷宫副本用于绘制路径
        vector<vector<char>> mazeWithPath = mazeCopy;


        // 在路径上绘制标记（使用*表示路径）
        for (size_t i = 0; i < path.size(); i++) {
            int row = path[i].first;
            int col = path[i].second;

            // 确保位置有效
            if (row >= 0 && row < mazeSize && col >= 0 && col < mazeSize) {
                // 起点用S标记，终点用E标记，中间路径用*标记
                if (i == 0) {
                    mazeWithPath[row][col] = 'S'; // 起点
                }
                else if (i == path.size() - 1) {
                    mazeWithPath[row][col] = 'N'; // 终点
                }
                else {
                    mazeWithPath[row][col] = '*'; // 路径
                }
            }
        }

        // 输出带有路径的迷宫
        cout << "\n=== 路径可视化 (" << path.size() << "步) ===" << endl;
        for (int i = 0; i < mazeSize; i++) {
            for (int j = 0; j < mazeSize; j++) {
                cout << mazeWithPath[i][j];
            }
            cout << endl;
        }
        cout << "=====================" << endl;
    }

    // 执行贪心策略
    vector<pair<int, int>> executeGreedyStrategy(Player& player) {
        vector<pair<int, int>> path; // 存储拾取路径
        path.push_back(player.getPosition()); // 添加起点
        int stepCount = 0;
        const int MAX_STEPS = mazeSize * mazeSize * 2; // 添加最大步数限制
        visitedPositions.clear(); // 清空访问记录

        while (stepCount++ < MAX_STEPS) {
            // 检查是否到达终点'E'
            auto [playerRow, playerCol] = player.getPosition();
            if (mazeCopy[playerRow][playerCol] == 'E') {
                cout << "到达终点'E'，终止探索进程！" << endl;
                break;
            }

            // 记录当前位置
            visitedPositions.push_back(player.getPosition());

            // 检测循环
            if (detectLoop()) {
                cout << "检测到循环! 尝试脱困..." << endl;
                pair<int, int> newPos = escapeLoop(player);
                if (newPos.first == -1) {
                    cout << "无法脱困，终止策略！" << endl;
                    break;
                }

                path.push_back(newPos);
                player.setPosition(newPos.first, newPos.second);
                continue;
            }

            // 获取视野内的资源
            vector<Resource> visibleResources = getVisibleResources(player);
            // 修复：检查资源列表是否为空
            if (visibleResources.empty()) {
                pair<int, int> newPos = movePlayerDefaultDirection(player);
                if (newPos.first == -1) {
                    cout << "所有方向都被阻挡，无法移动！" << endl;
                    break;
                }

                path.push_back(newPos);
                player.setPosition(newPos.first, newPos.second);
                cout << "视野内无资源，移动至(" << newPos.first << "," << newPos.second << ")" << endl;
                continue;
            }

            // 计算性价比并选择最优资源
            Resource bestResource = selectBestResource(visibleResources);

            // 规划到最优资源的路径
            vector<pair<int, int>> pathToResource = findPath(player.getPosition(), { bestResource.row, bestResource.col });
            if (pathToResource.empty()) {
                cout << "无法到达资源位置，尝试移动..." << endl;
                pair<int, int> newPos = movePlayerDefaultDirection(player);
                if (newPos.first == -1) break;

                path.push_back(newPos);
                player.setPosition(newPos.first, newPos.second);
                continue;
            }

            // 更新路径和玩家位置
            for (size_t i = 1; i < pathToResource.size(); i++) {
                path.push_back(pathToResource[i]); // 只添加新路径点
                visitedPositions.push_back(pathToResource[i]); // 记录访问位置
            }
            player.setPosition(pathToResource.back().first, pathToResource.back().second);

            // 处理资源拾取前检查是否到达终点
            if (mazeCopy[player.getPosition().first][player.getPosition().second] == 'E') {
                cout << "到达终点'E'，终止探索进程！" << endl;
                break;
            }

            // 处理资源拾取
            processResourcePickup(player, bestResource);
        }

        // 显示完整路径（可选：策略执行完毕后显示）
        if (!path.empty()) {
            displayMazeWithPath(path);
            cout << "最终玩家位置: (" << player.getPosition().first << ", "
                << player.getPosition().second << ")" << endl;
            cout << "总步数: " << stepCount << endl;
            cout << "最终分数: " << player.getScore() << endl;
        }

        return path;
    }

private:

    // 检测是否陷入循环
    bool detectLoop() {
        //总路径太短，不视作循环
        if (visitedPositions.size() < loopThreshold) return false;

        // 检查最近的loopThreshold个位置中是否有重复
        int recentCount = min(loopThreshold, (int)visitedPositions.size());
        unordered_map<string, int> posCount;//创建一个哈希表，统计位置出现的次数

        for (int i = visitedPositions.size() - recentCount; i < visitedPositions.size(); i++) {
            string posKey = to_string(visitedPositions[i].first) + "," + to_string(visitedPositions[i].second);
            posCount[posKey]++;

            // 如果某个位置出现次数超过阈值的一半，认为陷入循环
            if (posCount[posKey] > recentCount / 2) {
                return true;
            }
        }

        return false;
    }

    // 尝试脱离循环
    pair<int, int> escapeLoop(const Player& player) {
        auto [playerRow, playerCol] = player.getPosition();

        // 尝试寻找未访问过的方向
        vector<pair<int, int>> possibleMoves;

        // 四个方向的移动向量
        const int escapeDirs[8][2] = { {-1, 0},  {0, 1}, {1, 0},  {0, -1} };

        for (const auto& dir : escapeDirs) {
            int newRow = playerRow + dir[0];
            int newCol = playerCol + dir[1];

            // 检查新位置是否有效且未访问过
            if (newRow >= 1 && newRow < mazeSize - 1 &&
                newCol >= 1 && newCol < mazeSize - 1 &&
                mazeCopy[newRow][newCol] != '#' &&
                !isPositionVisited({ newRow, newCol })) {
                possibleMoves.push_back({ newRow, newCol });
            }
        }

        // 如果有未访问过的方向，随机选择一个
        if (!possibleMoves.empty()) {
            random_device rd;
            mt19937 gen(rd());
            uniform_int_distribution<> dis(0, possibleMoves.size() - 1);
            return possibleMoves[dis(gen)];
        }

        // 如果没有未访问过的方向，尝试沿墙走
        return followWall(player);
    }

    // 检查位置是否已访问
    bool isPositionVisited(pair<int, int> pos) {
        for (const auto& visitedPos : visitedPositions) {
            if (visitedPos == pos) return true;
        }
        return false;
    }

    // 沿墙走算法
    pair<int, int> followWall(const Player& player) {
        auto [playerRow, playerCol] = player.getPosition();

        // 尝试找到最近的墙
        for (int distance = 1; distance <= 3; distance++) {
            for (int dr = -distance; dr <= distance; dr++) {
                for (int dc = -distance; dc <= distance; dc++) {
                    if (abs(dr) + abs(dc) > distance) continue;

                    int row = playerRow + dr;
                    int col = playerCol + dc;

                    if (row < 0 || row >= mazeSize || col < 0 || col >= mazeSize) continue;

                    if (mazeCopy[row][col] == '#') {
                        // 找到墙，尝试沿墙走
                        // 定义顺时针方向的移动顺序
                        const vector<pair<int, int>> directions = {
                            {0, 1},   // 右
                            {1, 0},   // 下   
                            {0, -1},  // 左
                            {-1, 0},  // 上
                        };

                        // 尝试每个方向
                        for (const auto& dir : directions) {
                            int newRow = playerRow + dir.first;
                            int newCol = playerCol + dir.second;

                            if (newRow >= 1 && newRow < mazeSize - 1 &&
                                newCol >= 1 && newCol < mazeSize - 1 &&
                                mazeCopy[newRow][newCol] != '#') {
                                return { newRow, newCol };
                            }
                        }
                    }
                }
            }
        }

        // 如果找不到墙，尝试默认移动
        return movePlayerDefaultDirection(player);
    }

    // 新增：默认方向移动方法
    pair<int, int> movePlayerDefaultDirection(const Player& player) {
        auto [playerRow, playerCol] = player.getPosition();
        vector<pair<int, int>> directions = {
           {0, 1},   // 右
           {-1, 0},  // 上
           {0, -1},  // 左
           {1, 0}    // 下
        };
        random_device rd;
        mt19937 gen(rd());
        shuffle(directions.begin(), directions.end(), gen);

        // 按照随机顺序尝试每个方向
        for (const auto& dir : directions) {
            int newRow = playerRow + dir.first;
            int newCol = playerCol + dir.second;

            // 检查新位置是否有效（边界内且不是墙壁）
            if (newRow >= 1 && newRow < mazeSize - 1 &&
                newCol >= 1 && newCol < mazeSize - 1 &&
                mazeCopy[newRow][newCol] != '#') {
                return { newRow, newCol };
            }
        }

        // 所有方向都不可行，返回无效位置
        return { -1, -1 };
    }

    // 获取玩家视野内的资源 (3x3区域)
    vector<Resource> getVisibleResources(const Player& player) {
        vector<Resource> resources;
        auto [playerRow, playerCol] = player.getPosition();

        // 定义3x3视野范围
        int startRow = max(1, playerRow - 1);
        int endRow = min(mazeSize - 2, playerRow + 1);
        int startCol = max(1, playerCol - 1);
        int endCol = min(mazeSize - 2, playerCol + 1);

        // 遍历视野内的所有单元格
        for (int i = startRow; i <= endRow; i++) {
            for (int j = startCol; j <= endCol; j++) {
                if (i == playerRow && j == playerCol) continue; // 跳过玩家位置

                char cell = mazeCopy[i][j];
                if (cell == ' ' || cell == '#' || cell == 'S' || cell == 'E') continue; // 跳过通路、墙壁、起点和终点

                // 根据资源类型设置价值
                int value = 0;
                switch (cell) {
                case 'G': value = 5; break; // 金币
                case 'T': value = -3; break; // 陷阱
                default: value = 0;
                }

                // 计算到玩家的距离 (曼哈顿距离)
                int distance = abs(i - playerRow) + abs(j - playerCol);

                // 创建资源对象并添加到列表
                resources.emplace_back(i, j, cell, value);
                resources.back().distance = distance;
                resources.back().costPerformance = static_cast<double>(value) / distance;
            }
        }

        return resources;
    }

    // 选择性价比最高的资源 (贪心策略核心)
    Resource selectBestResource(const vector<Resource>& resources) {
        if (resources.empty()) {
            throw runtime_error("没有可选择的资源");
        }

        Resource best = resources[0];

        // 遍历资源，找到性价比最高的
        for (const auto& res : resources) {
            // 优先选择性价比高的资源
            if (res.costPerformance > best.costPerformance) {
                best = res;
            }
            // 性价比相同时，选择价值高的
            else if (res.costPerformance == best.costPerformance && res.value > best.value) {
                best = res;
            }
        }

        return best;
    }

    // 使用BFS寻找从起点到终点的最短路径
    vector<pair<int, int>> findPath(pair<int, int> start, pair<int, int> end) {
        vector<vector<bool>> visited(mazeSize, vector<bool>(mazeSize, false));
        queue<PathNode*> q;

        // 检查起点和终点是否有效
        if (mazeCopy[start.first][start.second] == '#' || mazeCopy[end.first][end.second] == '#') {
            return {};
        }

        // 创建起点节点并加入队列
        PathNode* startNode = new PathNode(start.first, start.second, 0, nullptr);
        q.push(startNode);
        visited[start.first][start.second] = true;

        // BFS寻路
        while (!q.empty()) {
            PathNode* current = q.front();
            q.pop();

            // 到达终点，重建路径
            if (current->row == end.first && current->col == end.second) {
                vector<pair<int, int>> path;
                while (current) {
                    path.emplace_back(current->row, current->col);
                    current = current->parent;
                }
                reverse(path.begin(), path.end()); // 反转路径，从起点到终点
                // 释放内存
                while (!q.empty()) {
                    delete q.front();
                    q.pop();
                }

                return path;
            }

            // 探索四个方向
            for (const auto& dir : dirs) {
                int newRow = current->row + dir[0];
                int newCol = current->col + dir[1];

                // 检查新位置是否有效
                if (newRow >= 1 && newRow < mazeSize - 1 && newCol >= 1 && newCol < mazeSize - 1 &&
                    !visited[newRow][newCol] && mazeCopy[newRow][newCol] != '#') {
                    visited[newRow][newCol] = true;
                    q.push(new PathNode(newRow, newCol, current->distance + 1, current));
                }
            }
        }

        // 无法找到路径，释放内存
        while (!q.empty()) {
            delete q.front();
            q.pop();
        }

        return {};
    }

    // 添加资源后处理逻辑
    void processResourcePickup(Player& player, const Resource& resource) {
        player.updateScore(resource.value);

        // 更新迷宫状态（拾取后变为通路）
        mazeCopy[resource.row][resource.col] = ' ';

        // 添加资源拾取反馈
        string resourceName;
        switch (resource.type) {
        case 'G': resourceName = "金币"; break;
        case 'T': resourceName = "陷阱"; break;
        case 'L': resourceName = "机关"; break;
        case 'B': resourceName = "BOSS"; break;
        default: resourceName = "未知资源";
        }

        cout << "拾取[" << resourceName << "] "
            << (resource.value >= 0 ? "+" : "") << resource.value << "分! "
            << "当前位置: (" << resource.row << "," << resource.col << ")"
            << endl;
    }
};