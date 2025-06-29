#include <algorithm>
#include <climits>
#include <queue>
#include <iomanip>
#include <thread>
#include <chrono>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <limits>
#include <fstream>
#include <sstream>
#include <utility>
#include <tuple>

// ����ANSI��ɫ�꣨�ն�֧��ʱ��Ч��
#define COLOR_GREEN "\033[32m"
#define COLOR_BLUE "\033[34m"
#define COLOR_RED "\033[31m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_RESET "\033[0m"

// �����������
const int COIN_SCORE = 5;      // ��ҵ÷�
const int TRAP_DEDUCTION = -3; // ����۷�

class MazePathFinder {
private:
    // �Թ�������Ϣ
    std::vector<std::vector<char>> grid;  // �Թ�����
    int gridSize;                         // ����ߴ�

    // �ؼ�λ����Ϣ
    std::pair<int, int> startPoint;       // �������
    std::pair<int, int> endPoint;         // �յ�����

    // ����Ԫ����Ϣ
    std::vector<std::pair<int, int>> coinPositions;  // ���λ���б�
    std::map<std::pair<int, int>, int> coinIndexMap; // �������ӳ��
    std::vector<std::pair<int, int>> trapPositions;  // ����λ���б�
    std::map<std::pair<int, int>, int> trapIndexMap; // ��������ӳ��

    // ��̬�滮���
    std::vector<std::vector<std::vector<std::pair<int, int>>>> dpTable; // DP��[y][x][mask] = {����, ����}
    std::vector<std::vector<std::vector<std::pair<int, int>>>> prevPositions; // ���ڵ�λ��
    std::vector<std::vector<std::vector<int>>> prevMasks; // ���ڵ�״̬

    // �������
    std::vector<std::pair<int, int>> bestPath;  // ����·��
    int highestScore;                           // ��ߵ÷�
    int shortestDistance;                       // ��ߵ÷ֶ�Ӧ����̾���

    // ˽�з���
    void identifyKeyPositions();                // ʶ��ؼ�λ��
    bool loadGridFromCSV(const std::string& filename); // ��CSV��������

public:
    // ���캯��
    MazePathFinder();
    explicit MazePathFinder(const std::string& filename);
    explicit MazePathFinder(const std::vector<std::vector<char>>& gridData);

    // ��������
    bool reloadGridFromCSV(const std::string& filename); // ���¼���CSV����
    void calculateOptimalPath();                        // ��������·��

    // �����ȡ
    int getHighestScore() const;                        // ��ȡ��ߵ÷�
    int getShortestDistance() const;                    // ��ȡ��̾���
    const std::vector<std::pair<int, int>>& getBestPath() const; // ��ȡ����·��

    // ·�����ӻ�
    void displayPath() const;                           // ��ʾ·������
};

// ʶ����㡢�յ㡢��Һ�����λ��
void MazePathFinder::identifyKeyPositions() {
    int coinIdx = 0;
    int trapIdx = 0;
    for (int i = 0; i < gridSize; ++i) {
        for (int j = 0; j < gridSize; ++j) {
            if (grid[i][j] == 'S') {
                startPoint = { j, i };
            }
            else if (grid[i][j] == 'E') {
                endPoint = { j, i };
            }
            else if (grid[i][j] == 'G') {
                coinPositions.push_back({ j, i });
                coinIndexMap[{j, i}] = coinIdx++;
            }
            else if (grid[i][j] == 'T') {
                trapPositions.push_back({ j, i });
                trapIndexMap[{j, i}] = trapIdx++;
            }
        }
    }
}

// ��CSV�ļ���ȡ�Թ�����
bool MazePathFinder::loadGridFromCSV(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "�޷����ļ�: " << filename << std::endl;
        return false;
    }

    std::string line;
    grid.clear();

    while (std::getline(file, line)) {
        std::vector<char> row;
        std::stringstream lineStream(line);
        std::string cell;

        while (std::getline(lineStream, cell, ',')) {
            if (!cell.empty()) {
                row.push_back(cell[0]);
            }
        }

        if (!row.empty()) {
            grid.push_back(row);
        }
    }

    file.close();

    if (grid.empty() || grid[0].empty()) {
        std::cerr << "�Թ����ݲ���Ϊ�ա�" << std::endl;
        return false;
    }

    gridSize = grid.size();
    return true;
}

// Ĭ�Ϲ��캯��
MazePathFinder::MazePathFinder() : gridSize(0), highestScore(std::numeric_limits<int>::min()),
shortestDistance(std::numeric_limits<int>::max()) {}

// ��CSV�ļ�����
MazePathFinder::MazePathFinder(const std::string& filename) : highestScore(std::numeric_limits<int>::min()),
shortestDistance(std::numeric_limits<int>::max()) {
    if (!loadGridFromCSV(filename)) {
        throw std::invalid_argument("�޷���CSV�ļ������Թ����ݡ�");
    }
    identifyKeyPositions();
}

// ���������ݹ���
MazePathFinder::MazePathFinder(const std::vector<std::vector<char>>& gridData) : grid(gridData),
highestScore(std::numeric_limits<int>::min()),
shortestDistance(std::numeric_limits<int>::max()) {
    if (gridData.empty() || gridData[0].empty()) {
        throw std::invalid_argument("�Թ����ݲ���Ϊ�ա�");
    }
    gridSize = gridData.size();
    identifyKeyPositions();
}

// ���¼���CSV�Թ�
bool MazePathFinder::reloadGridFromCSV(const std::string& filename) {
    if (loadGridFromCSV(filename)) {
        identifyKeyPositions();
        return true;
    }
    return false;
}

// ��������·������̬�滮ʵ�֣�
void MazePathFinder::calculateOptimalPath() {
    int coinCount = coinPositions.size();
    int trapCount = trapPositions.size();
    int totalElements = coinCount + trapCount;
    int maxState = 1 << totalElements;

    // ����״̬�ṹ��
    struct StateInfo {
        int score;
        int distance;
        StateInfo() : score(std::numeric_limits<int>::min()), distance(std::numeric_limits<int>::max()) {}
        StateInfo(int s, int d) : score(s), distance(d) {}
    };

    // ��ʼ��DP��
    std::vector<std::vector<std::vector<StateInfo>>> dp(
        gridSize, std::vector<std::vector<StateInfo>>(
            gridSize, std::vector<StateInfo>(maxState)
        )
    );

    // BFS����
    std::queue<std::tuple<int, int, int>> processingQueue;

    // ��ʼ�����ڵ��¼
    prevPositions.assign(gridSize, std::vector<std::vector<std::pair<int, int>>>(
        gridSize, std::vector<std::pair<int, int>>(maxState, { -1, -1 })
    ));
    prevMasks.assign(gridSize, std::vector<std::vector<int>>(
        gridSize, std::vector<int>(maxState, -1)
    ));

    // ����ʼ��
    dp[startPoint.second][startPoint.first][0] = StateInfo(0, 0);
    processingQueue.push({ startPoint.second, startPoint.first, 0 });

    // �ƶ�����
    const int dirX[] = { 0, 0, 1, -1 };
    const int dirY[] = { 1, -1, 0, 0 };

    // BFS����
    while (!processingQueue.empty()) {
        auto current = processingQueue.front();
        processingQueue.pop();
        int y = std::get<0>(current);
        int x = std::get<1>(current);
        int state = std::get<2>(current);
        auto& currentState = dp[y][x][state];

        // ��֦��������Ч״̬
        if (currentState.score == std::numeric_limits<int>::min()) continue;

        // �����յ㲻����չ
        if (y == endPoint.second && x == endPoint.first) continue;

        // ̽���ĸ�����
        for (int dir = 0; dir < 4; ++dir) {
            int newX = x + dirX[dir];
            int newY = y + dirY[dir];

            // �߽����ϰ�����
            if (newX < 0 || newX >= gridSize || newY < 0 || newY >= gridSize || grid[newY][newX] == '#') {
                continue;
            }

            int newState = state;
            int scoreChange = 0;
            char cellType = grid[newY][newX];

            // ������
            if (cellType == 'G') {
                auto it = coinIndexMap.find({ newX, newY });
                if (it != coinIndexMap.end()) {
                    int coinIdx = it->second;
                    if (!(state & (1 << coinIdx))) {
                        scoreChange += COIN_SCORE;
                        newState |= (1 << coinIdx);
                    }
                }
            }
            // ��������
            else if (cellType == 'T') {
                auto it = trapIndexMap.find({ newX, newY });
                if (it != trapIndexMap.end()) {
                    int trapIdx = it->second;
                    int trapBit = coinCount + trapIdx;
                    if (!(state & (1 << trapBit))) {
                        scoreChange += TRAP_DEDUCTION;
                        newState |= (1 << trapBit);
                    }
                }
            }

            // ������״ֵ̬
            int updatedScore = currentState.score + scoreChange;
            int updatedDist = currentState.distance + 1;

            // ����DP״̬
            auto& nextState = dp[newY][newX][newState];
            bool needUpdate = false;

            if (updatedScore > nextState.score) {
                needUpdate = true;
            }
            else if (updatedScore == nextState.score && updatedDist < nextState.distance) {
                needUpdate = true;
            }

            if (needUpdate) {
                nextState = StateInfo(updatedScore, updatedDist);
                prevPositions[newY][newX][newState] = { x, y };
                prevMasks[newY][newX][newState] = state;
                processingQueue.push({ newY, newX, newState });
            }
        }
    }

    // ���������յ�״̬
    highestScore = std::numeric_limits<int>::min();
    shortestDistance = std::numeric_limits<int>::max();
    int bestState = -1;

    for (int state = 0; state < maxState; ++state) {
        const auto& endState = dp[endPoint.second][endPoint.first][state];
        if (endState.score > highestScore ||
            (endState.score == highestScore && endState.distance < shortestDistance)) {
            highestScore = endState.score;
            shortestDistance = endState.distance;
            bestState = state;
        }
    }

    // �ؽ�·�������ӻ�
    if (bestState != -1) {
        bestPath.clear();
        std::pair<int, int> currentPos = endPoint;
        int currentState = bestState;

        // ���ݹ���·��
        while (currentPos.first != -1) {
            bestPath.push_back(currentPos);
            auto prevPos = prevPositions[currentPos.second][currentPos.first][currentState];
            int prevState = prevMasks[currentPos.second][currentPos.first][currentState];
            currentPos = prevPos;
            currentState = prevState;
        }
        std::reverse(bestPath.begin(), bestPath.end());

        // ·�����ӻ����
        std::vector<std::pair<int, int>> pathToShow = bestPath;
        for (size_t i = 0; i < pathToShow.size(); ++i) {
            system("cls");  // ������Windowsϵͳ��
            std::cout << "���� " << std::setw(4) << std::left << i << ": ("
                << pathToShow[i].first << ", " << pathToShow[i].second << ")  " << std::endl;

            // �����Թ�
            for (int j = 0; j < gridSize; ++j) {
                for (int k = 0; k < gridSize; ++k) {
                    // ��ǰλ�ñ��
                    if (pathToShow[i].first == k && pathToShow[i].second == j) {
                        std::cout << COLOR_GREEN << "&" << COLOR_RESET;
                        grid[j][k] = ' ';
                    }
                    // ǽ��
                    else if (grid[j][k] == '#') {
                        std::cout << COLOR_BLUE << grid[j][k] << COLOR_RESET;
                    }
                    // ����
                    else if (grid[j][k] == 'T') {
                        std::cout << COLOR_RED << grid[j][k] << COLOR_RESET;
                    }
                    // ��Դ��
                    else if (grid[j][k] == 'G' || grid[j][k] == 'L' || grid[j][k] == 'B') {
                        std::cout << COLOR_YELLOW << grid[j][k] << COLOR_RESET;
                    }
                    else {
                        std::cout << grid[j][k];
                    }
                }
                std::cout << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(300)); // �ӳ���ʾ
        }
    }
}

// ��ȡ��ߵ÷�
int MazePathFinder::getHighestScore() const {
    return highestScore;
}

// ��ȡ��̾���
int MazePathFinder::getShortestDistance() const {
    return shortestDistance + 1;
}

// ��ȡ����·��
const std::vector<std::pair<int, int>>& MazePathFinder::getBestPath() const {
    return bestPath;
}

// ��ʾ·������
void MazePathFinder::displayPath() const {
    if (bestPath.empty()) {
        std::cout << "δ�ҵ���Ч·�����޷���ʾ·����" << std::endl;
        return;
    }
    std::cout << "\n·���ܳ���: " << bestPath.size() << " �� | ��߷���: " << getHighestScore() << std::endl;
}