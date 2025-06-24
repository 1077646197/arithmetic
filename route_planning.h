#pragma once
#include <iostream>          // ����������⣬���ڿ���̨������������Ϣ�����չʾ��
#include <vector>            // ��̬�������������ڴ洢DP��ǰ���ڵ���·������
#include <map>               // �������������ڽ������굽��Դ��ֵ��ӳ���ϵ
#include <stack>             // ջ����������·������ʱ��ʱ�洢����
#include <algorithm>         // �㷨�⣬�ṩmax�ȳ����㷨��������������
#include <stdexcept>         // ��׼�쳣�⣬�����׳�·�����ɴ���쳣
#include "maze.h"            // �Զ����Թ�����ͷ�ļ����ṩMaze�ṹ��������㷨
#include <queue>
#include <iomanip>           //���ɴ�ӡ
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

        // ��������Ч��
        if (!isValid(startX, startY)) {
            throw runtime_error("��㲻��ͨ��");
        }

        // ��ʼ��DP��ǰ����ͷ��ʱ��
        dp.assign(maze.size, vector<int>(maze.size, -1e9));
        prev.assign(maze.size, vector<pair<int, int>>(maze.size, { -1, -1 }));
        vector<vector<bool>> visited(maze.size, vector<bool>(maze.size, false));

        // ����ʼ��
        dp[startX][startY] =0;
        visited[startX][startY] = true;

        // ʹ�ö��а���δ���ڵ�
        queue<pair<int, int>> processQueue;
        processQueue.push({ startX, startY });

        // �ĸ��ƶ�����
        const int directions[4][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };

        while (!processQueue.empty()) {
            pair<int, int> current = processQueue.front();
            int x = current.first;
            int y = current.second;
            processQueue.pop();

            // ̽���ĸ�����
            for (const auto& dir : directions) {
                int nx = x + dir[0];
                int ny = y + dir[1];
                if (isValid(nx, ny)) {
                    // ������·������Դֵ
                    int newVal = dp[x][y] + getResourceValue(nx, ny);
                    // ���ָ���·�����½ڵ�δ�����Ŵ���ʱ����
                    if (newVal > dp[nx][ny] && !visited[nx][ny]) {
                        dp[nx][ny] = newVal;
                        prev[nx][ny] = { x, y };

                        // �����ڵ�δ�����ʹ�ʱ�������
                        if (!visited[nx][ny]) {
                            visited[nx][ny] = true;
                            processQueue.push({ nx, ny });
                        }
                    }
                }
            }
        }

        // ����յ��Ƿ�ɴ�
        return dp[exitX][exitY] != -1e9;
    }

    // ��ȡ�������Դ��ֵ����ȡ�������Դ��
    int getResourceValue(int x, int y) {
        auto it = resourceMap.find({ x, y });
        if (it != resourceMap.end()) {
            int value = it->second;  // ��ȡ��Դ��ֵ
            resourceMap.erase(it);   // ����Դ�����Ƴ�
            return value;
        }
        return 0;  // ����Դ���ѱ��ɼ�
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

    // �򻯰涯̬�滮���ӡ�������̶����4�ַ���
    void printDPTable() const {
        cout << "��̬�滮�������Դֵ����" << endl;
        for (int i = 0; i < maze.size; i++) {
            for (int j = 0; j < maze.size; j++) {
                if (dp[i][j] == -1e9) {
                    cout << "*    ";  // 4�ַ���ȣ��ո����
                }
                else {
                    cout << setw(4) << left << dp[i][j] << " ";  // ����룬4�ַ����
                }
            }
            cout << endl;
        }
    }
};

// ·�����ӻ�����
void visualizePath(const Maze& maze, const std::vector<std::pair<int, int>>& path) {
    Maze tempMaze = maze;
    for (size_t i = 0; i < path.size(); i++) {
        int x = path[i].first;
        int y = path[i].second;
        if (i == 0) {
            tempMaze.grid[x][y] = 'S';  // ���
        }
        else if (i == path.size() - 1) {
            tempMaze.grid[x][y] = 'E';  // �յ�
        }
        else {
            tempMaze.grid[x][y] = '$';  // ·�����
        }
    }
    cout << "������Դ�ռ�·����" << endl;
    printMaze(tempMaze);
}