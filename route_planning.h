#pragma once
#include <iostream>          // ����������⣬���ڿ���̨������������Ϣ�����չʾ��
#include <vector>            // ��̬�������������ڴ洢DP��ǰ���ڵ���·������
#include <map>               // �������������ڽ������굽��Դ��ֵ��ӳ���ϵ
#include <stack>             // ջ����������·������ʱ��ʱ�洢����
#include <algorithm>         // �㷨�⣬�ṩmax�ȳ����㷨��������������
#include <stdexcept>         // ��׼�쳣�⣬�����׳�·�����ɴ���쳣
#include "maze.h"            // �Զ����Թ�����ͷ�ļ����ṩMaze�ṹ��������㷨
using namespace std;
// ��Դ��ṹ�壬��������ͼ�ֵ
struct ResourcePoint {
    int x, y;     // ����
    int value;    // ��Դ��ֵ������Ϊ��ֵ��
    ResourcePoint(int _x, int _y, int _value) : x(_x), y(_y), value(_value) {}
};

// ·���滮����
class ResourcePathPlanner {
private:
    Maze maze;                            // �Թ���������maze.h�ж���Ľṹ
    vector<vector<int>> dp;               // ��̬�滮��ʹ��vectorʵ�ֶ�ά����
    vector<vector<pair<int, int>>> prev;  // ǰ���ڵ���洢·��������Ϣ
    map<pair<int, int>, int> resourceMap;  // ��Դӳ�����mapʵ�����굽��ֵ��ӳ��

    // �������ת��ΪΨһ��
    pair<int, int> makePair(int x, int y) const {
        return make_pair(x, y);
    }

public:

    // ��������Ƿ���Ч����ǽ�������塢���Թ���Χ�ڣ�
    bool isValid(int x, int y) const {
        return x >= 0 && x < maze.size && y >= 0 && y < maze.size &&
            maze.grid[x][y] != '#' && maze.grid[x][y] != 'T';
    }

    // ���캯���������Թ����󲢳�ʼ��DP��
    ResourcePathPlanner(const Maze& m) : maze(m) {
        dp.resize(maze.size, vector<int>(maze.size, -1e9));     // vector��̬������С
        prev.resize(maze.size, vector<pair<int, int>>(maze.size, { -1, -1 }));
        initializeResourceMap();
    }

    // ��ʼ����Դӳ������Թ�������ȡ��Դ��ֵ��
    void initializeResourceMap() {
        resourceMap.clear();//ȷ��ÿ�γ�ʼ�����Ǵ�ͷ��ʼ���������������
        for (int i = 0; i < maze.size; i++) {
            for (int j = 0; j < maze.size; j++) {
                char cell = maze.grid[i][j];
                // ʹ��map�����ֵ�ԣ�����-��Դ��ֵ��
                if (cell == 'G') resourceMap[makePair(i, j)] = 5;
                else if (cell == 'L') resourceMap[makePair(i, j)] = 10;
                else if (cell == 'B') resourceMap[makePair(i, j)] = 20;
                else if (cell == 'T') resourceMap[makePair(i, j)] = -3;
            }
        }
    }

    // �����Զ�����Դ�ֲ�������Ĭ����Դ��
    void setResourceDistribution(const vector<ResourcePoint>& resources) {
        resourceMap.clear();
        for (const auto& point : resources) {
            resourceMap[makePair(point.x, point.y)] = point.value;
        }
    }

    // ִ�ж�̬�滮�������·��
    bool solve() {
        int startX = maze.startX, startY = maze.startY;
        int exitX = maze.exitX, exitY = maze.exitY;

        dp[startX][startY] = getResourceValue(startX, startY);

        // �������ȱ����Թ���vector�Ķ�ά�������ʣ�
        for (int i = 0; i < maze.size; i++) {
            for (int j = 0; j < maze.size; j++) {
                if (!isValid(i, j) || dp[i][j] == -1e9) continue;

                // �����ĸ������ƶ���ʹ��algorithm�е�max�Ƚ�ֵ��
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

        // ����յ��Ƿ�ɴʹ���߼��ǲ�����
        return dp[exitX][exitY] != -1e9;
    }

    // ��ȡ�������Դ��ֵ��ͨ��map���ң�
    int getResourceValue(int x, int y) const {
        auto it = resourceMap.find(makePair(x, y));
        return it != resourceMap.end() ? it->second : 0;
    }

    // ��ȡ�����Դֵ�������ɴ����׳��쳣��
    int getMaxResourceValue() const {
        int exitX = maze.exitX, exitY = maze.exitY;
        if (dp[exitX][exitY] == -1e9) {
            throw runtime_error("û���ҵ��ɴ��·��"); // �׳���׼�쳣
        }
        return dp[exitX][exitY];
    }

    // ��ȡ����·�����У�ͨ��ջ����·����
    vector<pair<int, int>> getOptimalPath() const {
        int exitX = maze.exitX, exitY = maze.exitY;
        if (dp[exitX][exitY] == -1e9) {
            throw runtime_error("û���ҵ��ɴ��·��");
        }

        vector<pair<int, int>> path;
        stack<pair<int, int>> pathStack; // ʹ��ջ����·��

        int x = exitX, y = exitY;
        while (x != -1 && y != -1) {
            pathStack.push({ x, y });
            auto p = prev[x][y];
            x = p.first;
            y = p.second;
        }

        // ��תջ��Ԫ��˳��vector��push_back������
        while (!pathStack.empty()) {
            path.push_back(pathStack.top());
            pathStack.pop();
        }

        return path;
    }
};