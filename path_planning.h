#pragma once
#ifndef PATH_PLANNING_H
#define PATH_PLANNING_H

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <limits>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <utility>  // for std::pair

using namespace std;

// 定义分数常量
const int GOLD_VALUE = 5;     // 金币加5分
const int TRAP_PENALTY = -3;  // 陷阱扣3分

class path_planning {
private:
    vector<vector<char>> maze;  // 迷宫地图
    int size;                  // 迷宫尺寸

    // 特殊位置
    pair<int, int> startPos;   // 起点坐标
    pair<int, int> exitPos;    // 终点坐标

    // 金币和陷阱信息
    vector<pair<int, int>> gold_locations;  // 金币位置列表
    map<pair<int, int>, int> gold_map;      // 金币位置到索引的映射
    vector<pair<int, int>> trap_locations;  // 陷阱位置列表
    map<pair<int, int>, int> trap_map;      // 陷阱位置到索引的映射

    // 动态规划表和路径重建信息
    vector<vector<vector<pair<int, int>>>> dp_table;  // DP表: dp[y][x][mask] = {max_score, min_distance}
    vector<vector<vector<pair<int, int>>>> parent_pos; // 父节点位置信息
    vector<vector<vector<int>>> parent_mask;          // 父节点状态信息

    // 结果信息
    vector<pair<int, int>> optimalPath;  // 最优路径
    int maxScore;                       // 最大分数
    int minDistanceForMaxScore;         // 获得最大分数时的最小距离

    // 私有方法
    void findStartExitAndFeatures();    // 查找起点、终点和特殊特征
    bool readMazeFromCSV(const string& filename);  // 从CSV读取迷宫

public:
    // 构造函数
    path_planning();
    explicit path_planning(const string& filename);
    explicit path_planning(const vector<vector<char>>& m);

    // 迷宫加载方法
    bool loadMazeFromCSV(const string& filename);

    // 求解方法
    void solve();  // 使用动态规划求解最优路径

    // 结果获取方法
    int getMaxScore() const;  // 获取最大分数
    int getMinDistance() const;  // 获取最短路径长度
    const vector<pair<int, int>>& getOptimalPath() const;  // 获取最优路径

    // 可视化方法
    void visualizePath() const;  // 可视化路径和分数变化
};

#endif // PATH_PLANNING_H