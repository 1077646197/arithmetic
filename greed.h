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
    int row, col;         // ��ҵ�ǰλ��
    int score;            // ��ҷ���

public:
    // ��ʼ�����
    Player(int startRow, int startCol) : row(startRow), col(startCol), score(0) {}

    // ��ȡ���λ��
    pair<int, int> getPosition() const {
        return { row, col };
    }

    // �������λ��
    void setPosition(int newRow, int newCol) {
        row = newRow;
        col = newCol;
    }

    // ��ȡ��ҷ���
    int getScore() const {
        return score;
    }

    // ������ҷ���
    void updateScore(int delta) {
        score += delta;
    }
};

// ��Դ��Ϣ�ṹ��
struct Resource {
    int row, col;         // ��Դλ��
    char type;            // ��Դ���� 'G'(���), 'T'(����)
    int value;            // ��Դ��ֵ
    int distance;         // ����ҵľ���
    double costPerformance; // �Լ۱�(��λ��������)

    Resource(int r, int c, char t, int val) : row(r), col(c), type(t), value(val),
        distance(0), costPerformance(0.0) {}
};

// ·���ڵ�ṹ�壬����BFSѰ·
struct PathNode {
    int row, col;         // �ڵ�λ��
    int distance;         // ����㵽��ǰ�ڵ�ľ���
    PathNode* parent;     // ���ڵ㣬�����ؽ�·��

    PathNode(int r, int c, int d, PathNode* p) : row(r), col(c), distance(d), parent(p) {}
};

// ��Դʰȡ�����࣬ʵ��̰���㷨
class ResourcePickingStrategy {
private:
    vector<vector<char>> mazeCopy; // �洢�Թ�����,��¼��Դ��ʰȡ���״̬�仯
    int mazeSize;                    // �Թ���С������/����
    vector<pair<int, int>> visitedPositions; // ��¼�ѷ��ʵ�λ�ã����ڼ��ѭ����
    int loopThreshold;               // ѭ�������ֵ����������ʵ�λ���ظ�����������ֵ��һ��ʱ��Ϊ����ѭ����

    // �ĸ�������ƶ�����
    const int dirs[4][2] = { {-1, 0}, {0, 1}, {1, 0}, {0, -1} };

public:
    ResourcePickingStrategy(vector<vector<char>> m) : mazeCopy(m), mazeSize(m.size()) {
        loopThreshold = mazeSize * 2; // ����ѭ�������ֵ
    }

    // ���Թ��ϻ���·�������
    void displayMazeWithPath(const vector<pair<int, int>>& path) {
        // �����Թ��������ڻ���·��
        vector<vector<char>> mazeWithPath = mazeCopy;


        // ��·���ϻ��Ʊ�ǣ�ʹ��*��ʾ·����
        for (size_t i = 0; i < path.size(); i++) {
            int row = path[i].first;
            int col = path[i].second;

            // ȷ��λ����Ч
            if (row >= 0 && row < mazeSize && col >= 0 && col < mazeSize) {
                // �����S��ǣ��յ���E��ǣ��м�·����*���
                if (i == 0) {
                    mazeWithPath[row][col] = 'S'; // ���
                }
                else if (i == path.size() - 1) {
                    mazeWithPath[row][col] = 'N'; // �յ�
                }
                else {
                    mazeWithPath[row][col] = '*'; // ·��
                }
            }
        }

        // �������·�����Թ�
        cout << "\n=== ·�����ӻ� (" << path.size() << "��) ===" << endl;
        for (int i = 0; i < mazeSize; i++) {
            for (int j = 0; j < mazeSize; j++) {
                cout << mazeWithPath[i][j];
            }
            cout << endl;
        }
        cout << "=====================" << endl;
    }

    // ִ��̰�Ĳ���
    vector<pair<int, int>> executeGreedyStrategy(Player& player) {
        vector<pair<int, int>> path; // �洢ʰȡ·��
        path.push_back(player.getPosition()); // ������
        int stepCount = 0;
        const int MAX_STEPS = mazeSize * mazeSize * 2; // ������������
        visitedPositions.clear(); // ��շ��ʼ�¼

        while (stepCount++ < MAX_STEPS) {
            // ����Ƿ񵽴��յ�'E'
            auto [playerRow, playerCol] = player.getPosition();
            if (mazeCopy[playerRow][playerCol] == 'E') {
                cout << "�����յ�'E'����ֹ̽�����̣�" << endl;
                break;
            }

            // ��¼��ǰλ��
            visitedPositions.push_back(player.getPosition());

            // ���ѭ��
            if (detectLoop()) {
                cout << "��⵽ѭ��! ��������..." << endl;
                pair<int, int> newPos = escapeLoop(player);
                if (newPos.first == -1) {
                    cout << "�޷���������ֹ���ԣ�" << endl;
                    break;
                }

                path.push_back(newPos);
                player.setPosition(newPos.first, newPos.second);
                continue;
            }

            // ��ȡ��Ұ�ڵ���Դ
            vector<Resource> visibleResources = getVisibleResources(player);
            // �޸��������Դ�б��Ƿ�Ϊ��
            if (visibleResources.empty()) {
                pair<int, int> newPos = movePlayerDefaultDirection(player);
                if (newPos.first == -1) {
                    cout << "���з��򶼱��赲���޷��ƶ���" << endl;
                    break;
                }

                path.push_back(newPos);
                player.setPosition(newPos.first, newPos.second);
                cout << "��Ұ������Դ���ƶ���(" << newPos.first << "," << newPos.second << ")" << endl;
                continue;
            }

            // �����Լ۱Ȳ�ѡ��������Դ
            Resource bestResource = selectBestResource(visibleResources);

            // �滮��������Դ��·��
            vector<pair<int, int>> pathToResource = findPath(player.getPosition(), { bestResource.row, bestResource.col });
            if (pathToResource.empty()) {
                cout << "�޷�������Դλ�ã������ƶ�..." << endl;
                pair<int, int> newPos = movePlayerDefaultDirection(player);
                if (newPos.first == -1) break;

                path.push_back(newPos);
                player.setPosition(newPos.first, newPos.second);
                continue;
            }

            // ����·�������λ��
            for (size_t i = 1; i < pathToResource.size(); i++) {
                path.push_back(pathToResource[i]); // ֻ�����·����
                visitedPositions.push_back(pathToResource[i]); // ��¼����λ��
            }
            player.setPosition(pathToResource.back().first, pathToResource.back().second);

            // ������Դʰȡǰ����Ƿ񵽴��յ�
            if (mazeCopy[player.getPosition().first][player.getPosition().second] == 'E') {
                cout << "�����յ�'E'����ֹ̽�����̣�" << endl;
                break;
            }

            // ������Դʰȡ
            processResourcePickup(player, bestResource);
        }

        // ��ʾ����·������ѡ������ִ����Ϻ���ʾ��
        if (!path.empty()) {
            displayMazeWithPath(path);
            cout << "�������λ��: (" << player.getPosition().first << ", "
                << player.getPosition().second << ")" << endl;
            cout << "�ܲ���: " << stepCount << endl;
            cout << "���շ���: " << player.getScore() << endl;
        }

        return path;
    }

private:

    // ����Ƿ�����ѭ��
    bool detectLoop() {
        //��·��̫�̣�������ѭ��
        if (visitedPositions.size() < loopThreshold) return false;

        // ��������loopThreshold��λ�����Ƿ����ظ�
        int recentCount = min(loopThreshold, (int)visitedPositions.size());
        unordered_map<string, int> posCount;//����һ����ϣ��ͳ��λ�ó��ֵĴ���

        for (int i = visitedPositions.size() - recentCount; i < visitedPositions.size(); i++) {
            string posKey = to_string(visitedPositions[i].first) + "," + to_string(visitedPositions[i].second);
            posCount[posKey]++;

            // ���ĳ��λ�ó��ִ���������ֵ��һ�룬��Ϊ����ѭ��
            if (posCount[posKey] > recentCount / 2) {
                return true;
            }
        }

        return false;
    }

    // ��������ѭ��
    pair<int, int> escapeLoop(const Player& player) {
        auto [playerRow, playerCol] = player.getPosition();

        // ����Ѱ��δ���ʹ��ķ���
        vector<pair<int, int>> possibleMoves;

        // �ĸ�������ƶ�����
        const int escapeDirs[8][2] = { {-1, 0},  {0, 1}, {1, 0},  {0, -1} };

        for (const auto& dir : escapeDirs) {
            int newRow = playerRow + dir[0];
            int newCol = playerCol + dir[1];

            // �����λ���Ƿ���Ч��δ���ʹ�
            if (newRow >= 1 && newRow < mazeSize - 1 &&
                newCol >= 1 && newCol < mazeSize - 1 &&
                mazeCopy[newRow][newCol] != '#' &&
                !isPositionVisited({ newRow, newCol })) {
                possibleMoves.push_back({ newRow, newCol });
            }
        }

        // �����δ���ʹ��ķ������ѡ��һ��
        if (!possibleMoves.empty()) {
            random_device rd;
            mt19937 gen(rd());
            uniform_int_distribution<> dis(0, possibleMoves.size() - 1);
            return possibleMoves[dis(gen)];
        }

        // ���û��δ���ʹ��ķ��򣬳�����ǽ��
        return followWall(player);
    }

    // ���λ���Ƿ��ѷ���
    bool isPositionVisited(pair<int, int> pos) {
        for (const auto& visitedPos : visitedPositions) {
            if (visitedPos == pos) return true;
        }
        return false;
    }

    // ��ǽ���㷨
    pair<int, int> followWall(const Player& player) {
        auto [playerRow, playerCol] = player.getPosition();

        // �����ҵ������ǽ
        for (int distance = 1; distance <= 3; distance++) {
            for (int dr = -distance; dr <= distance; dr++) {
                for (int dc = -distance; dc <= distance; dc++) {
                    if (abs(dr) + abs(dc) > distance) continue;

                    int row = playerRow + dr;
                    int col = playerCol + dc;

                    if (row < 0 || row >= mazeSize || col < 0 || col >= mazeSize) continue;

                    if (mazeCopy[row][col] == '#') {
                        // �ҵ�ǽ��������ǽ��
                        // ����˳ʱ�뷽����ƶ�˳��
                        const vector<pair<int, int>> directions = {
                            {0, 1},   // ��
                            {1, 0},   // ��   
                            {0, -1},  // ��
                            {-1, 0},  // ��
                        };

                        // ����ÿ������
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

        // ����Ҳ���ǽ������Ĭ���ƶ�
        return movePlayerDefaultDirection(player);
    }

    // ������Ĭ�Ϸ����ƶ�����
    pair<int, int> movePlayerDefaultDirection(const Player& player) {
        auto [playerRow, playerCol] = player.getPosition();
        vector<pair<int, int>> directions = {
           {0, 1},   // ��
           {-1, 0},  // ��
           {0, -1},  // ��
           {1, 0}    // ��
        };
        random_device rd;
        mt19937 gen(rd());
        shuffle(directions.begin(), directions.end(), gen);

        // �������˳����ÿ������
        for (const auto& dir : directions) {
            int newRow = playerRow + dir.first;
            int newCol = playerCol + dir.second;

            // �����λ���Ƿ���Ч���߽����Ҳ���ǽ�ڣ�
            if (newRow >= 1 && newRow < mazeSize - 1 &&
                newCol >= 1 && newCol < mazeSize - 1 &&
                mazeCopy[newRow][newCol] != '#') {
                return { newRow, newCol };
            }
        }

        // ���з��򶼲����У�������Чλ��
        return { -1, -1 };
    }

    // ��ȡ�����Ұ�ڵ���Դ (3x3����)
    vector<Resource> getVisibleResources(const Player& player) {
        vector<Resource> resources;
        auto [playerRow, playerCol] = player.getPosition();

        // ����3x3��Ұ��Χ
        int startRow = max(1, playerRow - 1);
        int endRow = min(mazeSize - 2, playerRow + 1);
        int startCol = max(1, playerCol - 1);
        int endCol = min(mazeSize - 2, playerCol + 1);

        // ������Ұ�ڵ����е�Ԫ��
        for (int i = startRow; i <= endRow; i++) {
            for (int j = startCol; j <= endCol; j++) {
                if (i == playerRow && j == playerCol) continue; // �������λ��

                char cell = mazeCopy[i][j];
                if (cell == ' ' || cell == '#' || cell == 'S' || cell == 'E') continue; // ����ͨ·��ǽ�ڡ������յ�

                // ������Դ�������ü�ֵ
                int value = 0;
                switch (cell) {
                case 'G': value = 5; break; // ���
                case 'T': value = -3; break; // ����
                default: value = 0;
                }

                // ���㵽��ҵľ��� (�����پ���)
                int distance = abs(i - playerRow) + abs(j - playerCol);

                // ������Դ������ӵ��б�
                resources.emplace_back(i, j, cell, value);
                resources.back().distance = distance;
                resources.back().costPerformance = static_cast<double>(value) / distance;
            }
        }

        return resources;
    }

    // ѡ���Լ۱���ߵ���Դ (̰�Ĳ��Ժ���)
    Resource selectBestResource(const vector<Resource>& resources) {
        if (resources.empty()) {
            throw runtime_error("û�п�ѡ�����Դ");
        }

        Resource best = resources[0];

        // ������Դ���ҵ��Լ۱���ߵ�
        for (const auto& res : resources) {
            // ����ѡ���Լ۱ȸߵ���Դ
            if (res.costPerformance > best.costPerformance) {
                best = res;
            }
            // �Լ۱���ͬʱ��ѡ���ֵ�ߵ�
            else if (res.costPerformance == best.costPerformance && res.value > best.value) {
                best = res;
            }
        }

        return best;
    }

    // ʹ��BFSѰ�Ҵ���㵽�յ�����·��
    vector<pair<int, int>> findPath(pair<int, int> start, pair<int, int> end) {
        vector<vector<bool>> visited(mazeSize, vector<bool>(mazeSize, false));
        queue<PathNode*> q;

        // ��������յ��Ƿ���Ч
        if (mazeCopy[start.first][start.second] == '#' || mazeCopy[end.first][end.second] == '#') {
            return {};
        }

        // �������ڵ㲢�������
        PathNode* startNode = new PathNode(start.first, start.second, 0, nullptr);
        q.push(startNode);
        visited[start.first][start.second] = true;

        // BFSѰ·
        while (!q.empty()) {
            PathNode* current = q.front();
            q.pop();

            // �����յ㣬�ؽ�·��
            if (current->row == end.first && current->col == end.second) {
                vector<pair<int, int>> path;
                while (current) {
                    path.emplace_back(current->row, current->col);
                    current = current->parent;
                }
                reverse(path.begin(), path.end()); // ��ת·��������㵽�յ�
                // �ͷ��ڴ�
                while (!q.empty()) {
                    delete q.front();
                    q.pop();
                }

                return path;
            }

            // ̽���ĸ�����
            for (const auto& dir : dirs) {
                int newRow = current->row + dir[0];
                int newCol = current->col + dir[1];

                // �����λ���Ƿ���Ч
                if (newRow >= 1 && newRow < mazeSize - 1 && newCol >= 1 && newCol < mazeSize - 1 &&
                    !visited[newRow][newCol] && mazeCopy[newRow][newCol] != '#') {
                    visited[newRow][newCol] = true;
                    q.push(new PathNode(newRow, newCol, current->distance + 1, current));
                }
            }
        }

        // �޷��ҵ�·�����ͷ��ڴ�
        while (!q.empty()) {
            delete q.front();
            q.pop();
        }

        return {};
    }

    // �����Դ�����߼�
    void processResourcePickup(Player& player, const Resource& resource) {
        player.updateScore(resource.value);

        // �����Թ�״̬��ʰȡ���Ϊͨ·��
        mazeCopy[resource.row][resource.col] = ' ';

        // �����Դʰȡ����
        string resourceName;
        switch (resource.type) {
        case 'G': resourceName = "���"; break;
        case 'T': resourceName = "����"; break;
        case 'L': resourceName = "����"; break;
        case 'B': resourceName = "BOSS"; break;
        default: resourceName = "δ֪��Դ";
        }

        cout << "ʰȡ[" << resourceName << "] "
            << (resource.value >= 0 ? "+" : "") << resource.value << "��! "
            << "��ǰλ��: (" << resource.row << "," << resource.col << ")"
            << endl;
    }
};