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
// ANSI ��ɫ������궨��
#define ANSI_COLOR_GREEN "\033[32m"   // ��ɫ�ı�
#define ANSI_COLOR_BLUE "\033[34m"   // ��ɫ�ı�
#define ANSI_COLOR_RED "\033[31m"   // ��ɫ�ı�
#define ANSI_COLOR_YELLOW "\033[33m"   // ��ɫ�ı�
#define ANSI_COLOR_RESET "\033[0m"    // ����ΪĬ����ɫ
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
    Maze maze;                            // �Թ�����
    vector<vector<int>> dp;               // ��̬�滮��
    map<pair<int, int>, int> resourceMap;  // ��Դӳ���
    vector<vector<pair<int, int>>> predecessor;  // ���ڴ洢·��ǰ���ڵ�
    // ��������֦��·������Դֵ�洢
    int prunedPathResource;                // ��֦��·��������Դֵ
    bool vvisited[20][20] = { 0 };
    // ����ת��ֵ
    pair<int, int> makePair(int x, int y) const {
        return make_pair(x, y);
    }

    // ��������ȡ��ǰ��Դֵ���������Դ�����ڼ�֦��
    int getCurrentResourceValue(int x, int y) const {
        auto it = resourceMap.find(makePair(x, y));
        return (it != resourceMap.end()) ? it->second : 0;
    }


public:
    // ���캯��
    ResourcePathPlanner(const Maze& m) : maze(m), prunedPathResource(0) {
        dp.resize(maze.size, vector<int>(maze.size, -1e9));
        initializeResourceMap();
    }

    // ��������Ƿ���Ч����ǽ�������塢���Թ���Χ�ڣ�
    bool isValid(int x, int y) const {
        return x >= 0 && x < maze.size && y >= 0 && y < maze.size &&
            maze.grid[x][y] != '#';
    }
    void initialdp()
    {
        for (int i = 0; i < maze.size; ++i) {//��ʼ��dp��
            for (int j = 0; j < maze.size; ++j) {
                if (isValid(i, j))
                {
                    dp[i][j] = getCurrentResourceValue(i, j);
                }
                else if (maze.grid[i][j] == 'T')
                {
                    dp[i][j] = -3;
                }
                else
                {
                    dp[i][j] = -1;
                }
            }
        }
    }
    // ��ʼ����Դӳ������Թ�������ȡ��Դ��ֵ��
    void initializeResourceMap() {
        resourceMap.clear();
        for (int i = 0; i < maze.size; i++) {
            for (int j = 0; j < maze.size; j++) {
                char cell = maze.grid[i][j];
                if (cell == 'G') resourceMap[makePair(i, j)] = 5;
                else if (cell == 'L') resourceMap[makePair(i, j)] = 10;
                else if (cell == 'B') resourceMap[makePair(i, j)] = 20;
                else if (cell == 'T') resourceMap[makePair(i, j)] = -3;
            }
        }
    }
    // ������ĺ��ĺ�������������������ͬDPֵ�ڵ㵽�յ㣨����C++11��
    bool traverseAllSameDp() {
        int startX = maze.startX;
        int startY = maze.startY;
        int exitX = maze.exitX;
        int exitY = maze.exitY;
        int targetVal = dp[startX][startY];

        // �������յ���Ч��
        if (!isValid(startX, startY) || !isValid(exitX, exitY)) {
            throw runtime_error("�����յ㲻��ͨ��");
        }
        if (dp[exitX][exitY] != targetVal) {
            cerr << "�յ�DPֵ����㲻����" << dp[exitX][exitY] << " vs " << targetVal << "��" << endl;
            return false;
        }

        // �ռ�����Ŀ��ڵ㣨DPֵ�������ͬ��
        vector<pair<int, int>> targetNodes;
        for (int i = 0; i < maze.size; ++i) {
            for (int j = 0; j < maze.size; ++j) {
                if (isValid(i, j) && dp[i][j] == targetVal) {
                    targetNodes.push_back(make_pair(i, j));
                }
            }
        }
        int totalTargets = targetNodes.size();
        if (totalTargets == 0) {
            cerr << "�޷���������Ŀ��ڵ�" << endl;
            return false;
        }

        // ��¼ÿ��Ŀ��ڵ�����������ڱ�Ƿ���״̬��
        map<pair<int, int>, int> nodeIndex;
        for (int i = 0; i < totalTargets; ++i) {
            nodeIndex[targetNodes[i]] = i;
        }

        // BFS���У��洢��x, y, �ѷ�������, ·����
        queue<tuple<int, int, unsigned long long, vector<pair<int, int>>>> q;
        // ����ʼ��
        unsigned long long startMask = 1ULL << nodeIndex[make_pair(startX, startY)];
        vector<pair<int, int>> startPath;
        startPath.push_back(make_pair(startX, startY));
        q.push(make_tuple(startX, startY, startMask, startPath));

        // �ѷ���״̬��¼�������ظ�������
        map<pair<pair<int, int>, unsigned long long>, bool> visited;
        visited[make_pair(make_pair(startX, startY), startMask)] = true;

        // ��������
        const int dirs[4][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };
        vector<pair<int, int>> finalPath;
        bool found = false;

        while (!q.empty()) {
            // ��C++11���ݷ�ʽ��ȡ����Ԫ�أ�����ṹ���󶨣�
            auto front = q.front();
            q.pop();
            int x = get<0>(front);
            int y = get<1>(front);
            unsigned long long mask = get<2>(front);
            vector<pair<int, int>> path = get<3>(front);

            // ����Ƿ񵽴��յ��ҷ������нڵ�
            if (x == exitX && y == exitY && mask == (1ULL << totalTargets) - 1) {
                finalPath = path;
                found = true;
                break;
            }

            // ̽���ĸ�����
            for (const auto& dir : dirs) {
                int nx = x + dir[0];
                int ny = y + dir[1];
                // �����������ͬDPֵ�Ľڵ�
                if (!isValid(nx, ny) || dp[nx][ny] != targetVal) {
                    continue;
                }
                // ����Ƿ�ΪĿ��ڵ�
                auto nodeKey = make_pair(nx, ny);
                if (nodeIndex.find(nodeKey) == nodeIndex.end()) {
                    continue; // ����Ŀ��ڵ��б��У������ϲ��ᷢ����
                }
                int idx = nodeIndex[nodeKey];
                unsigned long long newMask = mask | (1ULL << idx);

                // ����Ƿ��ѷ��ʸ�״̬
                auto stateKey = make_pair(make_pair(nx, ny), newMask);
                if (visited.find(stateKey) != visited.end()) {
                    continue;
                }

                // ������·��
                vector<pair<int, int>> newPath = path;
                newPath.push_back(nodeKey);
                // ���
                q.push(make_tuple(nx, ny, newMask, newPath));
                visited[stateKey] = true;
            }
        }

        if (found) {
            // ���·��
            for (size_t i = 0; i < finalPath.size(); i++) {
                system("cls");
                cout << "���� " << setw(4) << left << i << ": ("
                    << finalPath[i].first << ", " << finalPath[i].second << ")  " << endl;
                // �����Թ�
                for (int j = 0; j < maze.size; ++j) {
                    for (int k = 0; k < maze.size; ++k) {
                        // ��ǰ·��λ��
                        if (finalPath[i].first == j && finalPath[i].second == k) {
                            cout << ANSI_COLOR_GREEN << "&" << ANSI_COLOR_RESET;
                            maze.grid[j][k] = ' ';
                        }
                        // ǽ��
                        else if (maze.grid[j][k] == '#') {
                            cout << ANSI_COLOR_BLUE << maze.grid[j][k] << ANSI_COLOR_RESET;
                        }
                        // ����
                        else if (maze.grid[j][k] == 'T') {
                            cout << ANSI_COLOR_RED << maze.grid[j][k] << ANSI_COLOR_RESET;
                        }
                        // ��Դ��
                        else if (maze.grid[j][k] == 'G' || maze.grid[j][k] == 'L' || maze.grid[j][k] == 'B') {
                            // �ѷ��ʵ���Դ����ʾΪСд
                            cout << ANSI_COLOR_YELLOW << maze.grid[j][k] << ANSI_COLOR_RESET;
                        }
                        else {
                            cout << maze.grid[j][k];
                        }
                    }
                    cout << endl;
                }
                this_thread::sleep_for(chrono::milliseconds(300)); // �ʵ������ٶ�
            }
            return true;
        }
        else {
            cerr << "\nδ�ҵ���Ч·�����޷��������нڵ���յ㲻�ɴ" << endl;
            return false;
        }
    }
    // ��̬�滮·���Ż����������������
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
        initialdp();//��ʼ��dp��
        // ��һ�׶Σ���֦���Ƴ����ھӽڵ㣩
        bool updated = true;
        const int directions[4][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };
        while (updated) {
            updated = false;
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < cols; ++j) {
                    if (!isValid(i, j) || maze.grid[i][j] == 'S' || maze.grid[i][j] == 'E' || vvisited[i][j] == 1) {
                        continue;
                    }

                    // ͳ�ƿ����ھ�
                    int neighborCount = 0;
                    int nX = -1, nY = -1;
                    for (const auto& dir : directions) {
                        int ni = i + dir[0];
                        int nj = j + dir[1];
                        if (isValid(ni, nj) && vvisited[ni][nj] == 0) {
                            neighborCount++;
                            nX = ni;
                            nY = nj;
                        }
                    }
                    // ���ھӽڵ��֦
                    if (neighborCount == 1 && nX != -1 && nY != -1) {
                        vvisited[i][j] = 1;
                        updated = true;
                        if (dp[i][j] > 0) {
                            dp[nX][nY] += dp[i][j];
                        }
                    }
                }
            }
        }
        //===============================================================
        int t_num = 0;
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                if (dp[i][j] == -3 && !vvisited[i][j])//��·���������
                {
                    t_num++;
                    dp[i][j] = 0;
                }
            }
        }
        dp[startX][startY] += 100;
        dp[startX][startY] -= 3 * t_num;//������
        updated = 1;
        while (updated) {
            updated = false;
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < cols; ++j) {
                    if (!isValid(i, j) || maze.grid[i][j] == 'E' || vvisited[i][j] == 1) {
                        continue;
                    }

                    // ͳ�ƿ����ھ�
                    int neighborCount = 0;
                    int nX = -1, nY = -1;
                    for (const auto& dir : directions) {
                        int ni = i + dir[0];
                        int nj = j + dir[1];
                        if (isValid(ni, nj) && vvisited[ni][nj] != 1) {
                            neighborCount++;
                            nX = ni;
                            nY = nj;
                        }
                    }
                    // ���ھӽڵ��֦
                    if (neighborCount == 1 && nX != -1 && nY != -1) {
                        vvisited[i][j] = 1;
                        updated = true;
                        if (dp[i][j] > 0) {
                            dp[nX][nY] += dp[i][j];
                        }
                    }
                }
            }
        }
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                if (isValid(i, j)&&dp[i][j]>0) {
                    dp[i][j] = dp[exitX][exitY];
                }
            }
        }
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                cout << setw(4) << dp[i][j];
            }
            cout << endl;
        }
        traverseAllSameDp();
        cout << endl;
        cout << "��̬�滮�������Դֵ����" << dp[maze.exitX][maze.exitY] - 100 << endl;
        //printT();//��ӡ��Դ��
        return true;
    }
    // ��ȡ�������Դ��ֵ����ȡ�������Դ��
    int getResourceValue(int x, int y) {
        auto it = resourceMap.find({ x, y });
        if (it != resourceMap.end()) {
            int value = it->second;
            resourceMap.erase(it);
            return value;
        }
        return 0;
    }
    void printT()
    {
        cout << endl;
        cout << "��̬�滮�������Դֵ����" << dp[maze.exitX][maze.exitY] - 100 << endl;
        for (int i = 0; i < maze.size; i++) {
            for (int j = 0; j < maze.size; j++)
            {
                if ((dp[i][j] == -1) && maze.grid[i][j] == '#')
                {
                    cout << setw(4) << '#' << " ";  // 4�ַ���ȣ��ո����
                }
                else if ((dp[i][j] == -1) && maze.grid[i][j] == 'T')
                {
                    cout << setw(4) << "-3" << " ";  // 4�ַ���ȣ��ո����
                }
                else if ((vvisited[i][j] == 1) && dp[i][j] > 0 || maze.grid[i][j] == 'S' || maze.grid[i][j] == 'E')
                {
                    cout << setw(4) << left << dp[maze.exitX][maze.exitY] - 100 << " ";  // ����룬4�ַ����
                }
                else
                {
                    cout << setw(4) << left << dp[i][j] << " ";  // ����룬4�ַ����
                }
            }
            cout << endl;
        }
    }
};
