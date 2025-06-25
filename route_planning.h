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

    // ִ�ж�̬�滮�����Դ��󻯵�����·��
    bool solve() {
        int startX = maze.startX, startY = maze.startY;
        int exitX = maze.exitX, exitY = maze.exitY;

        // ��������Ч��
        if (!isValid(startX, startY)) {
            throw runtime_error("��㲻��ͨ��");
        }

        // ��ʼ��DP��ǰ�����·����¼
        dp.assign(maze.size, vector<int>(maze.size, -1e9));
        vector<vector<vector<pair<int, int>>>> path(maze.size, vector<vector<pair<int, int>>>(maze.size));

        // ����ʼ��
        dp[startX][startY] = getResourceValue(startX, startY);
        path[startX][startY].push_back({ startX, startY });

        // ʹ�����ȶ��а���Դ��ֵ������ڵ㣨����̽����Դ�����·����
        priority_queue<pair<int, pair<int, int>>> pq;
        pq.push({ dp[startX][startY], {startX, startY} });

        // �ĸ��ƶ�����
        const int directions[4][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };

        while (!pq.empty()) {
            // ��ȷ��������
            pair<int, pair<int, int>> currentPair = pq.top();
            pq.pop();
            int currentVal = currentPair.first;
            pair<int, int> current = currentPair.second;
            int x = current.first;
            int y = current.second;

            // �����ǰ�ڵ����ԴֵС��DP���еļ�¼��˵�����ҵ�����·��������
            if (currentVal < dp[x][y]) {
                continue;
            }

            // ̽���ĸ�����
            for (const auto& dir : directions) {
                int nx = x + dir[0];
                int ny = y + dir[1];
                if (isValid(nx, ny)) {
                    // ������·������Դֵ
                    int newVal = currentVal + getResourceValue(nx, ny);
                    // ���ָ���·��ʱ����
                    if (newVal > dp[nx][ny]) {
                        dp[nx][ny] = newVal;
                        path[nx][ny] = path[x][y];
                        path[nx][ny].push_back({ nx, ny });
                        pq.push({ newVal, {nx, ny} });
                    }
                }
            }
        }

        // ����յ��Ƿ�ɴ�
        if (dp[exitX][exitY] == -1e9) {
            return false;
        }

        // ����finalPath��������ֵ
        vector<pair<int, int>> finalPath = path[exitX][exitY];

        // �����Դ��󻯵�·��������Դֵ
        cout << "��Դ���·��������Դֵ: " << dp[exitX][exitY] << "����" << endl;
        for (size_t i = 0; i < finalPath.size(); i++) {
            cout << "���� " << setw(4) << left<< i << ": (" << finalPath[i].first << ", " << finalPath[i].second << ")  ";
            if ((i+1) % 6 == 0) cout << endl;
        }

        return true;
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


    // �򻯰涯̬�滮���ӡ�������̶����4�ַ���
    void printDPTable() const {
        cout << endl;
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