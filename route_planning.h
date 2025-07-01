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

// ANSI ��ɫ������궨��
#define ANSI_COLOR_GREEN "\033[32m"   // ��ɫ�ı�
#define ANSI_COLOR_BLUE "\033[34m"    // ��ɫ�ı�
#define ANSI_COLOR_RED "\033[31m"     // ��ɫ�ı�
#define ANSI_COLOR_YELLOW "\033[33m"  // ��ɫ�ı�
#define ANSI_COLOR_RESET "\033[0m"    // ����ΪĬ����ɫ

using namespace std;
#define MAX_SIZE 101  // ����Թ��ߴ�
// �Թ��ṹ��
struct Maze {
    char grid[MAX_SIZE][MAX_SIZE];
    int size;
    int startX, startY;
    int exitX, exitY;

    // ��ʼ���Թ�
    void init(int s) {
        if (s < 7 || s > MAX_SIZE || s % 2 == 0) {
            std::cerr << "�����Թ��ߴ�����Ǵ��ڵ���7��С�ڵ���" << MAX_SIZE << "������" << std::endl;
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

// ��Դ��ṹ�壬��������ͼ�ֵ
struct ResourcePoint {
    int x, y;     // ����
    int value;    // ��Դ��ֵ������Ϊ��ֵ��
    ResourcePoint(int _x, int _y, int _value) : x(_x), y(_y), value(_value) {}
};

// ·���滮����
class ResourcePathPlanner {
private:
    Maze maze;                            // �Թ�����
    vector<vector<int>> dp;               // ��̬�滮��
    vector<vector<int>> dp2;              // dp2����
    vector<pair<int, int>> fullPath;      // ����·���洢
    map<pair<int, int>, int> resourceMap;  // ��Դӳ���
    vector<vector<pair<int, int>>> predecessor;  // ǰ���ڵ�洢
    int prunedPathResource;               // ��֦��·��������Դֵ
    bool vvisited[20][20];
    bool visited[20][20];                 // ���ʱ������
    vector<pair<int, int>> validPoints;   // �ռ�����Ч�㼯��(dp>0���������ͨ)
    int requiredPointsCount;              // ��Ҫ���ʵĵ������
    vector<pair<int, int>> path;          // ��ǰ����·��
    vector<vector<pair<int, int>>> allPaths; // ���п���·��
    int startX, startY, exitX, exitY;     // �����յ�����
    const int dirs[4][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} }; // ��������

    // ����ת��ֵ������������
    pair<int, int> makePair(int x, int y) const {
        return make_pair(x, y);
    }

    // ��ȡ��ǰ��Դֵ���������Դ�����ڼ�֦��
    int getCurrentResourceValue(int x, int y) const {
        auto it = resourceMap.find(makePair(x, y));
        return (it != resourceMap.end()) ? it->second : 0;
    }

    // ��������Ƿ���ͨ
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
    // �ж��Ƿ�Ϊ��֧�㣨����1��δ���ʵ���Ч�ھӣ�
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

    // ���ݺ���
    bool backtrack(int x, int y, vector<vector<bool>>& pathVisited,
        vector<pair<int, int>>& currentPath, int visitedCount) {
        // ����ǰ�����·��
        currentPath.push_back(make_pair(x, y));
        pathVisited[x][y] = true;

        // ����Ƿ������������Ч�㲢�����յ�
        if (x == exitX && y == exitY && visitedCount == requiredPointsCount) {
            fullPath = currentPath;
            return true;
        }

        // ̽���ĸ�����
        for (int d = 0; d < 4; ++d) {
            int nx = x + dirs[d][0];
            int ny = y + dirs[d][1];

            if (isValid(nx, ny) && dp[nx][ny] > 0 && !pathVisited[nx][ny]) {
                // ������ʵ���Ч����
                int newVisitedCount = visitedCount;
                if (find(validPoints.begin(), validPoints.end(), make_pair(nx, ny)) != validPoints.end()) {
                    newVisitedCount++;
                }

                if (backtrack(nx, ny, pathVisited, currentPath, newVisitedCount)) {
                    return true;
                }
            }
        }

        // ���ݣ���·�����Ƴ���ǰ�㣬���Ϊδ����
        currentPath.pop_back();
        pathVisited[x][y] = false;

        // ����Ƿ�֧�㣬��������·��
        if (isBranchPoint(x, y, pathVisited)) {
            return false; // ��������������֧
        }

        return false;
    }

public:
    // ���캯��
    ResourcePathPlanner(const Maze& m) : maze(m), prunedPathResource(0) {
        startX = maze.startX;
        startY = maze.startY;
        exitX = maze.exitX;
        exitY = maze.exitY;

        // ��ʼ�����ʱ������
        for (int i = 0; i < 20; ++i) {
            for (int j = 0; j < 20; ++j) {
                visited[i][j] = false;
            }
        }

        // ��ʼ����̬�滮��
        dp.resize(maze.size, vector<int>(maze.size, -1e9));
        dp2.resize(maze.size, vector<int>(maze.size, -1));
        predecessor.resize(maze.size, vector<pair<int, int>>(maze.size, make_pair(-1, -1)));
        initializeResourceMap();
    }

    // ��������Ƿ���Ч����ǽ�����Թ���Χ�ڣ�
    bool isValid(int x, int y) const {
        return x >= 0 && x < maze.size && y >= 0 && y < maze.size &&
            maze.grid[x][y] != '#';
    }

    // ��ʼ��dp��
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

    // ��ʼ����Դӳ������Թ�������ȡ��Դ��ֵ��
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
    // �ж��Ƿ�Ϊ��֧�㣨����2����Ч�ھӣ�
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

    // �ж��Ƿ�Ϊĩ�˵㣨3����Ч�ھӣ�
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
    // ��ȡ�������Դ��ֵ����ȡ�������Դ��
    int getResourceValue(int x, int y) {
        auto it = resourceMap.find(make_pair(x, y));
        if (it != resourceMap.end()) {
            int value = it->second;
            resourceMap.erase(it);
            return value;
        }
        return 0;
    }

    // ��ӡ��̬�滮��
    void printT() {
        cout << endl << "��̬�滮�������Դֵ��:" << dp[maze.exitX][maze.exitY] << endl;
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
        // ���dp2���飨����0ֵ�ڵ㣩
        cout << "dp2���飨����>=0�Ľڵ㣩��" << endl;
        for (int i = 0; i < maze.size; ++i) {
            for (int j = 0; j < maze.size; ++j) {
                cout << setw(4) << dp2[i][j];
            }
            cout << endl;
        }
    }
    // ��̬�滮·���Ż���������֦����
    bool solveWithPruning() {
        int startX = maze.startX;
        int startY = maze.startY;
        int exitX = maze.exitX;
        int exitY = maze.exitY;
        int rows = maze.size;
        int cols = maze.size;

        // �������յ���Ч��
        if (!isValid(startX, startY)) {
            throw runtime_error("��㲻��ͨ��");
        }
        if (!isValid(exitX, exitY)) {
            throw runtime_error("�յ㲻��ͨ��");
        }

        initialdp(); // ��ʼ��dp��
        dp[startX][startY] += 100; // ����ʼֵ�ӳ�

        // ��һ�׶Σ���֦���Ƴ����ھӽڵ㣩
        bool updated = true;
        while (updated) {
            updated = false;
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < cols; ++j) {
                    if (!isValid(i, j) || maze.grid[i][j] == 'E' || vvisited[i][j]) {
                        continue;
                    }

                    // ͳ�ƿ����ھ�
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

                    // ����ĩ�˵㣨ȷ��dp2=0��������
                    if (neighborCount == 1 && isfq(i, j)) {
                        dp2[i][j] = 0; // ��ȷ����Ϊ0��ȷ�����������
                    }

                    // ���ھӽڵ��֦
                    if (neighborCount == 1 && nX != -1 && nY != -1) {
                        vvisited[i][j] = true;
                        updated = true;
                        // ��Դֵ����
                        if (dp[i][j] > 0) {
                            dp[nX][nY] += dp[i][j];
                        }
                    }
                }
            }
        }

        // ���dp2���飨����0ֵ�ڵ㣩
        cout << "dp2���飨����>=0�Ľڵ㣩��" << endl;
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
        // ִ�а�dp2ֵ����������0ֵ�ڵ㣩

        return true;
    }
    bool solve() {
        int startX = maze.startX, startY = maze.startY;
        int exitX = maze.exitX, exitY = maze.exitY;

        // ��������Ч��
        if (!isValid(startX, startY)) {
            throw runtime_error("��㲻��ͨ��");
        }

        // ��ʼ��·����¼
        vector<vector<vector<pair<int, int>>>> path(maze.size, vector<vector<pair<int, int>>>(maze.size));
        vector<vector<pair<int, int>>> parent(maze.size, vector<pair<int, int>>(maze.size, { -1, -1 }));
        // ����ԭʼdp2���飬������ʾ
        int finalpath[400][2]; int steps = 0;
        vector<vector<int>> originalDp2 = dp2;

        // ����ʼ��
        path[startX][startY].push_back({ startX, startY });
        dp2[startX][startY]--; // �������Ŀ��ô���

        // ʹ�����ȶ��а�dp2ֵ������ڵ�
        priority_queue<pair<int, pair<int, int>>> pq;
        pq.push({ dp2[startX][startY], {startX, startY} });

        // �ĸ��ƶ�����
        const int directions[4][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };

        while (!pq.empty()) {
            int currentDp2 = pq.top().first;
            int x = pq.top().second.first;
            int y = pq.top().second.second;

            pq.pop();

            // ����ҵ��յ㣬ֱ�ӷ���
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
            // ̽���ĸ�����
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
                        // ����Ŀ���Ŀ��ô���
                        dp2[nx][ny]--;
                        pq.push({ dp2[nx][ny], {nx, ny} });
                    }
                    break;
                }
            }
        }

        // ����յ��Ƿ�ɴ�
        if (path[exitX][exitY].empty()) {
            cout << "�޷��ҵ������յ��·��" << endl;

            // �ָ�ԭʼdp2����
            dp2 = originalDp2;
            return false;
        }

        // �ָ�ԭʼdp2����
        dp2 = originalDp2;

        // �������dp2ֵ����·��
        cout << "����dp2ֵ������ȵ�·��:" << endl;
        for (size_t i = 0; i < steps; i++) {
            system("cls");
            cout << "���� " << setw(4) << left << i << ": (" << finalpath[i][0] << ", " << finalpath[i][1] << ")  " << endl;

            // ��ʾ�Թ�״̬
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

        // �ı���ʽ�������·��
        cout << "\n����·��:" << endl;
        for (size_t i = 0; i < steps; i++) {
            cout << "���� " << setw(4) << left << i << ": (" << finalpath[i][0] << ", " << finalpath[i][1] << ")  ";
            if ((i + 1) % 6 == 0) cout << endl;
        }
        return true;
    }
};