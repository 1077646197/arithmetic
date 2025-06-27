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
            maze.grid[x][y] != '#' && maze.grid[x][y] != 'T';
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

    // ִ�ж�̬�滮�����Դ��󻯵�����·��
    bool solve() {
        int startX = maze.startX, startY = maze.startY;
        int exitX = maze.exitX, exitY = maze.exitY;

        // ��������Ч��
        if (!isValid(startX, startY)) {
            throw runtime_error("��㲻��ͨ��");
        }

        // ��ʼ����̬�滮��ǰ�����·����¼
        vector<vector<int>> resourceDp(maze.size, vector<int>(maze.size, -1));
        vector<vector<vector<pair<int, int>>>> path(maze.size, vector<vector<pair<int, int>>>(maze.size));
        // ����ʼ��
        resourceDp[startX][startY] = getResourceValue(startX, startY)+100;
        path[startX][startY].push_back({ startX, startY });

        // ʹ�����ȶ��а���Դ��ֵ������ڵ㣨����̽����Դ�����·����
        priority_queue<pair<int, pair<int, int>>> pq;
        pq.push({ resourceDp[startX][startY], {startX, startY} });

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
            if (currentVal < resourceDp[x][y]) {
                continue;
            }

            // ̽���ĸ�����
            for (const auto& dir : directions) {
                int nx = x + dir[0];
                int ny = y + dir[1];
                if (dp[nx][ny] != -1) {
                    // ������·������Դֵ��ʹ��originalDp��ȡ��Դֵ
                    int newVal = currentVal + getResourceValue(nx, ny);
                    // ���ָ���·��ʱ����
                    if (newVal > resourceDp[nx][ny]) {
                        resourceDp[nx][ny] = newVal;
                        path[nx][ny] = path[x][y];
                        path[nx][ny].push_back({ nx, ny });
                        pq.push({ newVal, {nx, ny} });

                    }
                }
            }
        }

        // ����յ��Ƿ�ɴ�
        if (resourceDp[exitX][exitY] == -1e9) {
            return false;
        }

        // ����finalPath��������ֵ
        vector<pair<int, int>> finalPath = path[exitX][exitY];

        // �����Դ��󻯵�·��
        cout << "��Դ���·��: " << endl;
        for (size_t i = 0; i < finalPath.size(); i++) {
            system("cls");
            cout << "���� " << setw(4) << left << i << ": (" << finalPath[i].first << ", " << finalPath[i].second << ")  " << endl;
            for (int j = 0; j < maze.size; ++j) {
                for (int k = 0; k < maze.size; ++k) {
                    if (finalPath[i].first == j && finalPath[i].second == k)
                    {
                        maze.grid[j][k] = ' ';
                        std::cout << ANSI_COLOR_GREEN << "&" << ANSI_COLOR_RESET;
                    }
                    else if (maze.grid[j][k] == '#')
                    {
                        std::cout << ANSI_COLOR_BLUE << maze.grid[j][k] << ANSI_COLOR_RESET;
                    }
                    else if (maze.grid[j][k] == 'T')
                    {
                        std::cout << ANSI_COLOR_RED << maze.grid[j][k] << ANSI_COLOR_RESET;
                    }
                    else if (maze.grid[j][k] == 'G')
                    {
                        std::cout << ANSI_COLOR_YELLOW << maze.grid[j][k] << ANSI_COLOR_RESET;
                    }
                    else cout << maze.grid[j][k];
                }
                cout << endl;
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        for (size_t i = 0; i < finalPath.size(); i++) {
            cout << "���� " << setw(4) << left << i << ": (" << finalPath[i].first << ", " << finalPath[i].second << ")  ";
            if ((i + 1) % 6 == 0) cout << endl;
        }
        return true;
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
                    if (dp[i][j] == -1 || maze.grid[i][j] == 'S' || maze.grid[i][j] == 'E' || vvisited[i][j] == 1) {
                        continue;
                    }

                    // ͳ�ƿ����ھ�
                    int neighborCount = 0;
                    int nX = -1, nY = -1;
                    for (const auto& dir : directions) {
                        int ni = i + dir[0];
                        int nj = j + dir[1];
                        if (dp[ni][nj] != -1 && vvisited[ni][nj] == 0) {
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
                if (vvisited[i][j] == 0 && maze.grid[i][j] == 'T')//��·���������
                {
                    t_num++;
                    maze.grid[i][j] == ' ';
                    dp[i][j] = 0;
                }
                else if ((vvisited[i][j] == 1) && dp[i][j] > 0 || maze.grid[i][j] == 'S')//����ֵǿ��
                {
                    dp[i][j] += 100;
                }
            }
        }
        dp[startX][startY] -= 3 * t_num;//������
        updated = 1;

        
        while (updated) {
            updated = false;
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < cols; ++j) {
                    if (dp[i][j] == -1 || maze.grid[i][j] == 'E' || vvisited[i][j] == 1) {
                        continue;
                    }

                    // ͳ�ƿ����ھ�
                    int neighborCount = 0;
                    int nX = -1, nY = -1;
                    for (const auto& dir : directions) {
                        int ni = i + dir[0];
                        int nj = j + dir[1];
                        if ((dp[ni][nj] != -1) && vvisited[ni][nj] != 1) {
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
        solve();////�滮·��
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
