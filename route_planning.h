#include <algorithm>    // �ṩ�㷨��������std::reverse��std::max�ȣ�
#include <climits>      // �ṩ��������ֵ�꣨��INT_MAX��
#include <queue>        // �ṩ��������������BFS��
#include <iomanip>      // �ṩ���������ʽ���ƣ���setw��
#include <thread>       // �ṩ�߳���ع��ܣ������ӳ���ʾ��
#include <chrono>       // �ṩʱ����ع��ܣ����ڿ����ӳ٣�
#include <iostream>     // �ṩ���������
#include <vector>       // �ṩ��̬��������
#include <string>       // �ṩ�ַ�����
#include <map>          // �ṩӳ����������ֵ�Դ洢��
#include <limits>       // �ṩ��ֵ���ޣ���std::numeric_limits��
#include <fstream>      // �ṩ�ļ����������
#include <sstream>      // �ṩ�ַ����������ڽ���CSV��
#include <utility>      // �ṩpairģ�壨�洢����ȳɶ����ݣ�
#include <tuple>        // �ṩtupleģ�壨�洢��Ԫ�����ݣ�

// ����ANSI��ɫ�꣨�ն�֧��ʱ��Ч���������������
#define COLOR_GREEN "\033[32m"   // ��ɫ����ǰλ�ñ�ǣ�
#define COLOR_BLUE "\033[34m"    // ��ɫ��ǽ�ڣ�
#define COLOR_RED "\033[31m"     // ��ɫ�����壩
#define COLOR_YELLOW "\033[33m"  // ��ɫ����Դ�����ң�
#define COLOR_RESET "\033[0m"    // ������ɫ

// �����������
const int COIN_SCORE = 5;       // �ռ���һ�õķ���
const int TRAP_DEDUCTION = -3;  // ��������۳��ķ���

class MazePathFinder {
private:
    // �Թ�������Ϣ
    std::vector<std::vector<char>> grid;  // �Թ����񣺴洢ÿ����Ԫ����ַ���'S'��㡢'E'�յ�ȣ�
    int gridSize;                         // ����ߴ磺����Ϊ�������Թ�����¼�߳�

    // �ؼ�λ����Ϣ
    std::pair<int, int> startPoint;       // ������꣺(x, y)
    std::pair<int, int> endPoint;         // �յ����꣺(x, y)

    // ����Ԫ����Ϣ
    std::vector<std::pair<int, int>> coinPositions;  // ���н��λ���б�
    std::map<std::pair<int, int>, int> coinIndexMap; // ���λ�õ�������ӳ�䣨����״̬���룩
    std::vector<std::pair<int, int>> trapPositions;  // ��������λ���б�
    std::map<std::pair<int, int>, int> trapIndexMap; // ����λ�õ�������ӳ�䣨����״̬���룩

    // ��̬�滮���
    // DP��[y][x][mask] = {����, ����}����¼������(x,y)��״̬mask�µ��������Ͷ�Ӧ����
    std::vector<std::vector<std::vector<std::pair<int, int>>>> dpTable;
    // ���ڵ�λ�ã���¼ÿ��״̬��ǰһ�����꣬���ڻ���·��
    std::vector<std::vector<std::vector<std::pair<int, int>>>> prevPositions;
    // ���ڵ�״̬����¼ÿ��״̬��ǰһ�����룬���ڻ���·��
    std::vector<std::vector<std::vector<int>>> prevMasks;

    // �������
    std::vector<std::pair<int, int>> bestPath;  // ����·������˳��洢·������
    int highestScore;                           // ��ߵ÷֣�����·�����ܷ���
    int shortestDistance;                       // ��̾��룺��ߵ÷ֶ�Ӧ��·������

    // ˽�з���
    void identifyKeyPositions();                // ʶ��ؼ�λ�ã���㡢�յ㡢��ҡ����壩
    bool loadGridFromCSV(const std::string& filename); // ��CSV�ļ������Թ���������

public:
    // ���캯��
    MazePathFinder();
    explicit MazePathFinder(const std::string& filename);  // ��CSV�ļ�����
    explicit MazePathFinder(const std::vector<std::vector<char>>& gridData);  // ���������ݹ���

    // ��������
    bool reloadGridFromCSV(const std::string& filename); // ���¼���CSV��������
    void calculateOptimalPath();                        // ��������·�������ķ�����

    // �����ȡ
    int getHighestScore() const;                        // ��ȡ��ߵ÷�
    int getShortestDistance() const;                    // ��ȡ��ߵ÷ֶ�Ӧ����̾���
    const std::vector<std::pair<int, int>>& getBestPath() const; // ��ȡ����·��

    // ·�����ӻ�
    void displayPath() const;                           // ��ʾ·������
};

// ʶ����㡢�յ㡢��Һ�����λ�ã�����¼�����������ӳ��
void MazePathFinder::identifyKeyPositions() {
    int coinIdx = 0;  // �������������
    int trapIdx = 0;  // ��������������
    // ���������ÿ����Ԫ��
    for (int i = 0; i < gridSize; ++i) {
        for (int j = 0; j < gridSize; ++j) {
            if (grid[i][j] == 'S') {  // ʶ�����
                startPoint = { j, i };  // �洢�������(x=j, y=i)
            }
            else if (grid[i][j] == 'E') {  // ʶ���յ�
                endPoint = { j, i };  // �洢�յ�����(x=j, y=i)
            }
            else if (grid[i][j] == 'G') {  // ʶ����
                coinPositions.push_back({ j, i });  // ��ӽ�����굽�б�
                coinIndexMap[{j, i}] = coinIdx++;  // �������굽������ӳ��
            }
            else if (grid[i][j] == 'T') {  // ʶ������
                trapPositions.push_back({ j, i });  // ����������굽�б�
                trapIndexMap[{j, i}] = trapIdx++;  // �������굽������ӳ��
            }
        }
    }
}

// ��CSV�ļ���ȡ�Թ����ݣ���Ԫ���Զ��ŷָ���
bool MazePathFinder::loadGridFromCSV(const std::string& filename) {
    std::ifstream file(filename);  // ��CSV�ļ�
    if (!file.is_open()) {  // ����ļ��Ƿ�ɹ���
        std::cerr << "�޷����ļ�: " << filename << std::endl;
        return false;
    }

    std::string line;  // �洢ÿ������
    grid.clear();  // ���������������

    // ���ж�ȡ�ļ�
    while (std::getline(file, line)) {
        std::vector<char> row;  // �洢һ�еĵ�Ԫ������
        std::stringstream lineStream(line);  // �����ַ���ת��Ϊ��
        std::string cell;  // �洢ÿ����Ԫ������

        // �����ŷָԪ��
        while (std::getline(lineStream, cell, ',')) {
            if (!cell.empty()) {  // ���Կյ�Ԫ��
                row.push_back(cell[0]);  // ȡ��Ԫ���һ���ַ���Ϊ����ֵ
            }
        }

        if (!row.empty()) {  // ���Կ���
            grid.push_back(row);
        }
    }

    file.close();  // �ر��ļ�

    // �������������Ч��
    if (grid.empty() || grid[0].empty()) {
        std::cerr << "�Թ����ݲ���Ϊ�ա�" << std::endl;
        return false;
    }

    gridSize = grid.size();  // ��������ߴ磨����Ϊ�����Σ�
    return true;
}

// Ĭ�Ϲ��캯������ʼ����Ա����ΪĬ��ֵ
MazePathFinder::MazePathFinder() : gridSize(0), highestScore(std::numeric_limits<int>::min()),
shortestDistance(std::numeric_limits<int>::max()) {}

// ��CSV�ļ����죺�����ļ���ʶ��ؼ�λ�ã�ʧ�����׳��쳣
MazePathFinder::MazePathFinder(const std::string& filename) : highestScore(std::numeric_limits<int>::min()),
shortestDistance(std::numeric_limits<int>::max()) {
    if (!loadGridFromCSV(filename)) {
        throw std::invalid_argument("�޷���CSV�ļ������Թ����ݡ�");
    }
    identifyKeyPositions();
}

// ���������ݹ��죺ʹ���ṩ���������ݣ�ʶ��ؼ�λ�ã�ʧ�����׳��쳣
MazePathFinder::MazePathFinder(const std::vector<std::vector<char>>& gridData) : grid(gridData),
highestScore(std::numeric_limits<int>::min()),
shortestDistance(std::numeric_limits<int>::max()) {
    if (gridData.empty() || gridData[0].empty()) {
        throw std::invalid_argument("�Թ����ݲ���Ϊ�ա�");
    }
    gridSize = gridData.size();  // ��������ߴ�
    identifyKeyPositions();  // ʶ��ؼ�λ��
}

// ���¼���CSV�Թ����ݣ��ȼ����ļ�����ʶ��ؼ�λ��
bool MazePathFinder::reloadGridFromCSV(const std::string& filename) {
    if (loadGridFromCSV(filename)) {  // ���سɹ�����¹ؼ�λ��
        identifyKeyPositions();
        return true;
    }
    return false;
}

// ��������·������̬�滮+�����������ʵ�֣�
void MazePathFinder::calculateOptimalPath() {
    int coinCount = coinPositions.size();  // �������
    int trapCount = trapPositions.size();  // ��������
    int totalElements = coinCount + trapCount;  // ����Ԫ������������״̬���룩
    int maxState = 1 << totalElements;  // ���״̬����2^��Ԫ������ÿ��Ԫ�ش����Ƿ񱻷��ʣ�

    // ����״̬�ṹ�壺�洢�����;���
    struct StateInfo {
        int score;    // ��ǰ״̬�ķ���
        int distance; // ��ǰ״̬�ľ��루������
        StateInfo() : score(std::numeric_limits<int>::min()), distance(std::numeric_limits<int>::max()) {}  // Ĭ�ϳ�ʼ������Ч״̬��
        StateInfo(int s, int d) : score(s), distance(d) {}  // �������Ĺ��캯��
    };

    // ��ʼ��DP��[y][x][mask] -> ״̬��Ϣ
    std::vector<std::vector<std::vector<StateInfo>>> dp(
        gridSize, std::vector<std::vector<StateInfo>>(
            gridSize, std::vector<StateInfo>(maxState)
        )
    );

    // BFS���У��洢�������״̬��y����, x����, ״̬���룩
    std::queue<std::tuple<int, int, int>> processingQueue;

    // ��ʼ�����ڵ��¼�����ڻ���·��
    prevPositions.assign(gridSize, std::vector<std::vector<std::pair<int, int>>>(
        gridSize, std::vector<std::pair<int, int>>(maxState, { -1, -1 })  // ��ʼ��Ϊ��Ч����
    ));
    prevMasks.assign(gridSize, std::vector<std::vector<int>>(
        gridSize, std::vector<int>(maxState, -1)  // ��ʼ��Ϊ��Ч״̬
    ));

    // ����ʼ�������λ�ã���ʼ״̬������0��δ�ռ��κ�Ԫ�أ�
    dp[startPoint.second][startPoint.first][0] = StateInfo(0, 0);  // ��ʼ����0������0
    processingQueue.push({ startPoint.second, startPoint.first, 0 });  // �����������

    // �ƶ������ϡ��¡��ҡ���y��x�ı仯����
    const int dirX[] = { 0, 0, 1, -1 };
    const int dirY[] = { 1, -1, 0, 0 };

    // BFS�����������пɴ�״̬
    while (!processingQueue.empty()) {
        auto current = processingQueue.front();  // ȡ������״̬
        processingQueue.pop();  // �Ƴ�����״̬
        int y = std::get<0>(current);  // ��ǰy����
        int x = std::get<1>(current);  // ��ǰx����
        int state = std::get<2>(current);  // ��ǰ״̬����
        auto& currentState = dp[y][x][state];  // ��ǰ״̬��Ϣ

        // ��֦��������Ч״̬��δ�����ʹ���
        if (currentState.score == std::numeric_limits<int>::min()) continue;

        // �����յ㲻����չ�������ظ�����
        if (y == endPoint.second && x == endPoint.first) continue;

        // ̽���ĸ�������ƶ�
        for (int dir = 0; dir < 4; ++dir) {
            int newX = x + dirX[dir];  // ��x����
            int newY = y + dirY[dir];  // ��y����

            // �߽����ϰ����飺ȷ�����������������Ҳ���ǽ�ڣ�'#'��
            if (newX < 0 || newX >= gridSize || newY < 0 || newY >= gridSize || grid[newY][newX] == '#') {
                continue;
            }

            int newState = state;  // ��״̬���루��ʼ�뵱ǰ״̬��ͬ��
            int scoreChange = 0;   // �����仯���ռ���Ҽӷ֣���������۷֣�
            char cellType = grid[newY][newX];  // �µ�Ԫ������

            // �������壺�����������δ�������������·�����״̬
             if (cellType == 'T') {
                auto it = trapIndexMap.find({ newX, newY });  // ������������
                if (it != trapIndexMap.end()) {  // �ҵ�����
                    int trapIdx = it->second;  // ��ȡ��������
                    int trapBit = coinCount + trapIdx;  // �����������е�λ�ã����֮��
                    if (!(state & (1 << trapBit))) {  // ����Ƿ��Ѵ�������Ӧλ�Ƿ�Ϊ0��
                        scoreChange += TRAP_DEDUCTION;  // �۷�
                        newState |= (1 << trapBit);  // ����״̬���루��Ӧλ��1��
                    }
                }
            }
            // �����ң�����ǽ����δ���ռ��������·�����״̬
             else if (cellType == 'G') {
                auto it = coinIndexMap.find({ newX, newY });  // ���ҽ������
                if (it != coinIndexMap.end()) {  // �ҵ����
                    int coinIdx = it->second;  // ��ȡ�������
                    if (!(state & (1 << coinIdx))) {  // ����Ƿ����ռ�����Ӧλ�Ƿ�Ϊ0��
                        scoreChange += COIN_SCORE;  // �ӷ�
                        newState |= (1 << coinIdx);  // ����״̬���루��Ӧλ��1��
                    }
                }
            }
            

            // ������״̬�ķ����;���
            int updatedScore = currentState.score + scoreChange;  // �·��� = ��ǰ���� + �仯��
            int updatedDist = currentState.distance + 1;  // �¾��� = ��ǰ���� + 1���ƶ�һ����

            // ����DP״̬������Ƿ���Ҫ������״̬����Ϣ
            auto& nextState = dp[newY][newX][newState];
            bool needUpdate = false;  // �Ƿ���Ҫ���±�־

            // ��������ѡ������ߵ�״̬��������ͬ��ѡ�����̵�״̬
            if (updatedScore > nextState.score) {
                needUpdate = true;
            }
            else if (updatedScore == nextState.score && updatedDist < nextState.distance) {
                needUpdate = true;
            }

            // �����Ҫ���£������״̬��Ϣ����¼·��������״̬�������
            if (needUpdate) {
                nextState = StateInfo(updatedScore, updatedDist);  // ����״̬
                prevPositions[newY][newX][newState] = { x, y };  // ��¼ǰһ������
                prevMasks[newY][newX][newState] = state;  // ��¼ǰһ��״̬
                processingQueue.push({ newY, newX, newState });  // ������д�����
            }
        }
    }

    // ���������յ�״̬�������յ����п��ܵ�״̬��ѡ���������Ҿ�����̵�
    highestScore = std::numeric_limits<int>::min();  // ��ʼ����߷���Ϊ��Сֵ
    shortestDistance = std::numeric_limits<int>::max();  // ��ʼ����̾���Ϊ���ֵ
    int bestState = -1;  // ����״̬����

    for (int state = 0; state < maxState; ++state) {
        const auto& endState = dp[endPoint.second][endPoint.first][state];  // �յ��ڸ�״̬�µ���Ϣ
        // �Ƚϲ���������״̬
        if (endState.score > highestScore ||
            (endState.score == highestScore && endState.distance < shortestDistance)) {
            highestScore = endState.score;
            shortestDistance = endState.distance;
            bestState = state;
        }
    }

    // �ؽ�·�������ӻ�������ҵ���Ч·��
    if (bestState != -1) {
        bestPath.clear();  // �������·��
        std::pair<int, int> currentPos = endPoint;  // ���յ㿪ʼ����
        int currentState = bestState;  // ����״̬

        // ���ݹ���·�������յ�һֱ׷�ݵ���㣨������Ϊ(-1,-1)ʱ
                // ���ݹ���·�������յ�һֱ׷�ݵ���㣨������Ϊ(-1,-1)ʱ������
        while (currentPos.first != -1) {
            bestPath.push_back(currentPos);  // ����ǰλ�ü���·��
            // ��ȡǰһ���������״̬
            auto prevPos = prevPositions[currentPos.second][currentPos.first][currentState];
            int prevState = prevMasks[currentPos.second][currentPos.first][currentState];
            currentPos = prevPos;  // ���µ�ǰλ��Ϊǰһ��λ��
            currentState = prevState;  // ���µ�ǰ״̬Ϊǰһ��״̬
        }
        std::reverse(bestPath.begin(), bestPath.end());  // ��ת·����ʹ�����㵽�յ�

        // ·�����ӻ��������֡��ʾ·��
        std::vector<std::pair<int, int>> pathToShow = bestPath;  // ��������·��������ʾ
        for (size_t i = 0; i < pathToShow.size(); ++i) {
            system("cls");  // ������Windowsϵͳ��Linux/macOS���Ϊ"clear"��
            std::cout << "���� " << std::setw(4) << std::left << i << ": ("
                << pathToShow[i].first << ", " << pathToShow[i].second << ")  " << std::endl;

            // �����Թ������ݵ�ǰ������Ⱦ�Թ�״̬
            for (int j = 0; j < gridSize; ++j) {
                for (int k = 0; k < gridSize; ++k) {
                    // ��ǰλ�ñ�ǣ�����ɫ&��ʾ��ǰλ��
                    if (pathToShow[i].first == k && pathToShow[i].second == j) {
                        std::cout << COLOR_GREEN << "&" << COLOR_RESET;
                        grid[j][k] = ' ';  // �ƶ���ԭλ�ñ��Ϊ�ո񣨱����ظ���ʾ��
                    }
                    // ǽ�ڣ�����ɫ#��ʾ
                    else if (grid[j][k] == '#') {
                        std::cout << COLOR_BLUE << grid[j][k] << COLOR_RESET;
                    }
                    // ���壺�ú�ɫT��ʾ
                    else if (grid[j][k] == 'T') {
                        std::cout << COLOR_RED << grid[j][k] << COLOR_RESET;
                    }
                    // ��Դ�㣺�û�ɫ��ʾ�����G�����ܵ�������ԴL/B��
                    else if (grid[j][k] == 'G' || grid[j][k] == 'L' || grid[j][k] == 'B') {
                        std::cout << COLOR_YELLOW << grid[j][k] << COLOR_RESET;
                    }
                    else {
                        std::cout << grid[j][k];  // ��ͨ�յ�ֱ����ʾ
                    }
                }
                std::cout << std::endl;  // ����
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(400)); // �ӳ�300���룬������ʾ�ٶ�
        }
    }
}

// ��ȡ��ߵ÷�
int MazePathFinder::getHighestScore() const {
    return highestScore;
}

// ��ȡ��̾��루��1����Ϊ·������=����+1������������
int MazePathFinder::getShortestDistance() const {
    return shortestDistance + 1;
}

// ��ȡ����·��������·�������б�
const std::vector<std::pair<int, int>>& MazePathFinder::getBestPath() const {
    return bestPath;
}

// ��ʾ·�����飺���·�����Ⱥ���߷���
void MazePathFinder::displayPath() const {
    if (bestPath.empty()) {  // ����Ƿ������Ч·��
        std::cout << "δ�ҵ���Ч·�����޷���ʾ·����" << std::endl;
        return;
    }
    // ���·�����Ⱥ���߷���
    std::cout << "\n·���ܳ���: " << bestPath.size() << " �� | ��߷���: " << getHighestScore() << std::endl;
}