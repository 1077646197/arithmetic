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

// 定义ANSI颜色宏（终端支持时生效）
#define COLOR_GREEN "\033[32m"
#define COLOR_BLUE "\033[34m"
#define COLOR_RED "\033[31m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_RESET "\033[0m"

// 定义分数常量
const int COIN_SCORE = 5;      // 金币得分
const int TRAP_DEDUCTION = -3; // 陷阱扣分

class MazePathFinder {
private:
    // 迷宫基础信息
    std::vector<std::vector<char>> grid;  // 迷宫网格
    int gridSize;                         // 网格尺寸

    // 关键位置信息
    std::pair<int, int> startPoint;       // 起点坐标
    std::pair<int, int> endPoint;         // 终点坐标

    // 特殊元素信息
    std::vector<std::pair<int, int>> coinPositions;  // 金币位置列表
    std::map<std::pair<int, int>, int> coinIndexMap; // 金币索引映射
    std::vector<std::pair<int, int>> trapPositions;  // 陷阱位置列表
    std::map<std::pair<int, int>, int> trapIndexMap; // 陷阱索引映射

    // 动态规划相关
    std::vector<std::vector<std::vector<std::pair<int, int>>>> dpTable; // DP表：[y][x][mask] = {分数, 距离}
    std::vector<std::vector<std::vector<std::pair<int, int>>>> prevPositions; // 父节点位置
    std::vector<std::vector<std::vector<int>>> prevMasks; // 父节点状态

    // 结果数据
    std::vector<std::pair<int, int>> bestPath;  // 最优路径
    int highestScore;                           // 最高得分
    int shortestDistance;                       // 最高得分对应的最短距离

    // 私有方法
    void identifyKeyPositions();                // 识别关键位置
    bool loadGridFromCSV(const std::string& filename); // 从CSV加载网格

public:
    // 构造函数
    MazePathFinder();
    explicit MazePathFinder(const std::string& filename);
    explicit MazePathFinder(const std::vector<std::vector<char>>& gridData);

    // 公共方法
    bool reloadGridFromCSV(const std::string& filename); // 重新加载CSV网格
    void calculateOptimalPath();                        // 计算最优路径

    // 结果获取
    int getHighestScore() const;                        // 获取最高得分
    int getShortestDistance() const;                    // 获取最短距离
    const std::vector<std::pair<int, int>>& getBestPath() const; // 获取最优路径

    // 路径可视化
    void displayPath() const;                           // 显示路径详情
};

// 识别起点、终点、金币和陷阱位置
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

// 从CSV文件读取迷宫数据
bool MazePathFinder::loadGridFromCSV(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
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
        std::cerr << "迷宫数据不能为空。" << std::endl;
        return false;
    }

    gridSize = grid.size();
    return true;
}

// 默认构造函数
MazePathFinder::MazePathFinder() : gridSize(0), highestScore(std::numeric_limits<int>::min()),
shortestDistance(std::numeric_limits<int>::max()) {}

// 从CSV文件构造
MazePathFinder::MazePathFinder(const std::string& filename) : highestScore(std::numeric_limits<int>::min()),
shortestDistance(std::numeric_limits<int>::max()) {
    if (!loadGridFromCSV(filename)) {
        throw std::invalid_argument("无法从CSV文件加载迷宫数据。");
    }
    identifyKeyPositions();
}

// 从网格数据构造
MazePathFinder::MazePathFinder(const std::vector<std::vector<char>>& gridData) : grid(gridData),
highestScore(std::numeric_limits<int>::min()),
shortestDistance(std::numeric_limits<int>::max()) {
    if (gridData.empty() || gridData[0].empty()) {
        throw std::invalid_argument("迷宫数据不能为空。");
    }
    gridSize = gridData.size();
    identifyKeyPositions();
}

// 重新加载CSV迷宫
bool MazePathFinder::reloadGridFromCSV(const std::string& filename) {
    if (loadGridFromCSV(filename)) {
        identifyKeyPositions();
        return true;
    }
    return false;
}

// 计算最优路径（动态规划实现）
void MazePathFinder::calculateOptimalPath() {
    int coinCount = coinPositions.size();
    int trapCount = trapPositions.size();
    int totalElements = coinCount + trapCount;
    int maxState = 1 << totalElements;

    // 定义状态结构体
    struct StateInfo {
        int score;
        int distance;
        StateInfo() : score(std::numeric_limits<int>::min()), distance(std::numeric_limits<int>::max()) {}
        StateInfo(int s, int d) : score(s), distance(d) {}
    };

    // 初始化DP表
    std::vector<std::vector<std::vector<StateInfo>>> dp(
        gridSize, std::vector<std::vector<StateInfo>>(
            gridSize, std::vector<StateInfo>(maxState)
        )
    );

    // BFS队列
    std::queue<std::tuple<int, int, int>> processingQueue;

    // 初始化父节点记录
    prevPositions.assign(gridSize, std::vector<std::vector<std::pair<int, int>>>(
        gridSize, std::vector<std::pair<int, int>>(maxState, { -1, -1 })
    ));
    prevMasks.assign(gridSize, std::vector<std::vector<int>>(
        gridSize, std::vector<int>(maxState, -1)
    ));

    // 起点初始化
    dp[startPoint.second][startPoint.first][0] = StateInfo(0, 0);
    processingQueue.push({ startPoint.second, startPoint.first, 0 });

    // 移动方向
    const int dirX[] = { 0, 0, 1, -1 };
    const int dirY[] = { 1, -1, 0, 0 };

    // BFS处理
    while (!processingQueue.empty()) {
        auto current = processingQueue.front();
        processingQueue.pop();
        int y = std::get<0>(current);
        int x = std::get<1>(current);
        int state = std::get<2>(current);
        auto& currentState = dp[y][x][state];

        // 剪枝：跳过无效状态
        if (currentState.score == std::numeric_limits<int>::min()) continue;

        // 到达终点不再扩展
        if (y == endPoint.second && x == endPoint.first) continue;

        // 探索四个方向
        for (int dir = 0; dir < 4; ++dir) {
            int newX = x + dirX[dir];
            int newY = y + dirY[dir];

            // 边界与障碍物检查
            if (newX < 0 || newX >= gridSize || newY < 0 || newY >= gridSize || grid[newY][newX] == '#') {
                continue;
            }

            int newState = state;
            int scoreChange = 0;
            char cellType = grid[newY][newX];

            // 处理金币
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
            // 处理陷阱
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

            // 计算新状态值
            int updatedScore = currentState.score + scoreChange;
            int updatedDist = currentState.distance + 1;

            // 更新DP状态
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

    // 查找最优终点状态
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

    // 重建路径并可视化
    if (bestState != -1) {
        bestPath.clear();
        std::pair<int, int> currentPos = endPoint;
        int currentState = bestState;

        // 回溯构建路径
        while (currentPos.first != -1) {
            bestPath.push_back(currentPos);
            auto prevPos = prevPositions[currentPos.second][currentPos.first][currentState];
            int prevState = prevMasks[currentPos.second][currentPos.first][currentState];
            currentPos = prevPos;
            currentState = prevState;
        }
        std::reverse(bestPath.begin(), bestPath.end());

        // 路径可视化输出
        std::vector<std::pair<int, int>> pathToShow = bestPath;
        for (size_t i = 0; i < pathToShow.size(); ++i) {
            system("cls");  // 清屏（Windows系统）
            std::cout << "步骤 " << std::setw(4) << std::left << i << ": ("
                << pathToShow[i].first << ", " << pathToShow[i].second << ")  " << std::endl;

            // 绘制迷宫
            for (int j = 0; j < gridSize; ++j) {
                for (int k = 0; k < gridSize; ++k) {
                    // 当前位置标记
                    if (pathToShow[i].first == k && pathToShow[i].second == j) {
                        std::cout << COLOR_GREEN << "&" << COLOR_RESET;
                        grid[j][k] = ' ';
                    }
                    // 墙壁
                    else if (grid[j][k] == '#') {
                        std::cout << COLOR_BLUE << grid[j][k] << COLOR_RESET;
                    }
                    // 陷阱
                    else if (grid[j][k] == 'T') {
                        std::cout << COLOR_RED << grid[j][k] << COLOR_RESET;
                    }
                    // 资源点
                    else if (grid[j][k] == 'G' || grid[j][k] == 'L' || grid[j][k] == 'B') {
                        std::cout << COLOR_YELLOW << grid[j][k] << COLOR_RESET;
                    }
                    else {
                        std::cout << grid[j][k];
                    }
                }
                std::cout << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(300)); // 延迟显示
        }
    }
}

// 获取最高得分
int MazePathFinder::getHighestScore() const {
    return highestScore;
}

// 获取最短距离
int MazePathFinder::getShortestDistance() const {
    return shortestDistance + 1;
}

// 获取最优路径
const std::vector<std::pair<int, int>>& MazePathFinder::getBestPath() const {
    return bestPath;
}

// 显示路径详情
void MazePathFinder::displayPath() const {
    if (bestPath.empty()) {
        std::cout << "未找到有效路径或无法显示路径。" << std::endl;
        return;
    }
    std::cout << "\n路径总长度: " << bestPath.size() << " 格 | 最高分数: " << getHighestScore() << std::endl;
}