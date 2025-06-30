#include <algorithm>    // 提供算法函数（如std::reverse、std::max等）
#include <climits>      // 提供整数极限值宏（如INT_MAX）
#include <queue>        // 提供队列容器（用于BFS）
#include <iomanip>      // 提供输入输出格式控制（如setw）
#include <thread>       // 提供线程相关功能（用于延迟显示）
#include <chrono>       // 提供时间相关功能（用于控制延迟）
#include <iostream>     // 提供输入输出流
#include <vector>       // 提供动态数组容器
#include <string>       // 提供字符串类
#include <map>          // 提供映射容器（键值对存储）
#include <limits>       // 提供数值极限（如std::numeric_limits）
#include <fstream>      // 提供文件输入输出流
#include <sstream>      // 提供字符串流（用于解析CSV）
#include <utility>      // 提供pair模板（存储坐标等成对数据）
#include <tuple>        // 提供tuple模板（存储多元组数据）

// 定义ANSI颜色宏（终端支持时生效，用于美化输出）
#define COLOR_GREEN "\033[32m"   // 绿色（当前位置标记）
#define COLOR_BLUE "\033[34m"    // 蓝色（墙壁）
#define COLOR_RED "\033[31m"     // 红色（陷阱）
#define COLOR_YELLOW "\033[33m"  // 黄色（资源点如金币）
#define COLOR_RESET "\033[0m"    // 重置颜色

// 定义分数常量
const int COIN_SCORE = 5;       // 收集金币获得的分数
const int TRAP_DEDUCTION = -3;  // 触发陷阱扣除的分数

class MazePathFinder {
private:
    // 迷宫基础信息
    std::vector<std::vector<char>> grid;  // 迷宫网格：存储每个单元格的字符（'S'起点、'E'终点等）
    int gridSize;                         // 网格尺寸：假设为正方形迷宫，记录边长

    // 关键位置信息
    std::pair<int, int> startPoint;       // 起点坐标：(x, y)
    std::pair<int, int> endPoint;         // 终点坐标：(x, y)

    // 特殊元素信息
    std::vector<std::pair<int, int>> coinPositions;  // 所有金币位置列表
    std::map<std::pair<int, int>, int> coinIndexMap; // 金币位置到索引的映射（用于状态掩码）
    std::vector<std::pair<int, int>> trapPositions;  // 所有陷阱位置列表
    std::map<std::pair<int, int>, int> trapIndexMap; // 陷阱位置到索引的映射（用于状态掩码）

    // 动态规划相关
    // DP表：[y][x][mask] = {分数, 距离}，记录在坐标(x,y)、状态mask下的最大分数和对应距离
    std::vector<std::vector<std::vector<std::pair<int, int>>>> dpTable;
    // 父节点位置：记录每个状态的前一步坐标，用于回溯路径
    std::vector<std::vector<std::vector<std::pair<int, int>>>> prevPositions;
    // 父节点状态：记录每个状态的前一步掩码，用于回溯路径
    std::vector<std::vector<std::vector<int>>> prevMasks;

    // 结果数据
    std::vector<std::pair<int, int>> bestPath;  // 最优路径：按顺序存储路径坐标
    int highestScore;                           // 最高得分：最优路径的总分数
    int shortestDistance;                       // 最短距离：最高得分对应的路径长度

    // 私有方法
    void identifyKeyPositions();                // 识别关键位置（起点、终点、金币、陷阱）
    bool loadGridFromCSV(const std::string& filename); // 从CSV文件加载迷宫网格数据

public:
    // 构造函数
    MazePathFinder();
    explicit MazePathFinder(const std::string& filename);  // 从CSV文件构造
    explicit MazePathFinder(const std::vector<std::vector<char>>& gridData);  // 从网格数据构造

    // 公共方法
    bool reloadGridFromCSV(const std::string& filename); // 重新加载CSV网格数据
    void calculateOptimalPath();                        // 计算最优路径（核心方法）

    // 结果获取
    int getHighestScore() const;                        // 获取最高得分
    int getShortestDistance() const;                    // 获取最高得分对应的最短距离
    const std::vector<std::pair<int, int>>& getBestPath() const; // 获取最优路径

    // 路径可视化
    void displayPath() const;                           // 显示路径详情
};

// 识别起点、终点、金币和陷阱位置，并记录其坐标和索引映射
void MazePathFinder::identifyKeyPositions() {
    int coinIdx = 0;  // 金币索引计数器
    int trapIdx = 0;  // 陷阱索引计数器
    // 遍历网格的每个单元格
    for (int i = 0; i < gridSize; ++i) {
        for (int j = 0; j < gridSize; ++j) {
            if (grid[i][j] == 'S') {  // 识别起点
                startPoint = { j, i };  // 存储起点坐标(x=j, y=i)
            }
            else if (grid[i][j] == 'E') {  // 识别终点
                endPoint = { j, i };  // 存储终点坐标(x=j, y=i)
            }
            else if (grid[i][j] == 'G') {  // 识别金币
                coinPositions.push_back({ j, i });  // 添加金币坐标到列表
                coinIndexMap[{j, i}] = coinIdx++;  // 建立坐标到索引的映射
            }
            else if (grid[i][j] == 'T') {  // 识别陷阱
                trapPositions.push_back({ j, i });  // 添加陷阱坐标到列表
                trapIndexMap[{j, i}] = trapIdx++;  // 建立坐标到索引的映射
            }
        }
    }
}

// 从CSV文件读取迷宫数据（单元格以逗号分隔）
bool MazePathFinder::loadGridFromCSV(const std::string& filename) {
    std::ifstream file(filename);  // 打开CSV文件
    if (!file.is_open()) {  // 检查文件是否成功打开
        std::cerr << "无法打开文件: " << filename << std::endl;
        return false;
    }

    std::string line;  // 存储每行数据
    grid.clear();  // 清空现有网格数据

    // 逐行读取文件
    while (std::getline(file, line)) {
        std::vector<char> row;  // 存储一行的单元格数据
        std::stringstream lineStream(line);  // 将行字符串转换为流
        std::string cell;  // 存储每个单元格数据

        // 按逗号分割单元格
        while (std::getline(lineStream, cell, ',')) {
            if (!cell.empty()) {  // 忽略空单元格
                row.push_back(cell[0]);  // 取单元格第一个字符作为网格值
            }
        }

        if (!row.empty()) {  // 忽略空行
            grid.push_back(row);
        }
    }

    file.close();  // 关闭文件

    // 检查网格数据有效性
    if (grid.empty() || grid[0].empty()) {
        std::cerr << "迷宫数据不能为空。" << std::endl;
        return false;
    }

    gridSize = grid.size();  // 设置网格尺寸（假设为正方形）
    return true;
}

// 默认构造函数：初始化成员变量为默认值
MazePathFinder::MazePathFinder() : gridSize(0), highestScore(std::numeric_limits<int>::min()),
shortestDistance(std::numeric_limits<int>::max()) {}

// 从CSV文件构造：加载文件并识别关键位置，失败则抛出异常
MazePathFinder::MazePathFinder(const std::string& filename) : highestScore(std::numeric_limits<int>::min()),
shortestDistance(std::numeric_limits<int>::max()) {
    if (!loadGridFromCSV(filename)) {
        throw std::invalid_argument("无法从CSV文件加载迷宫数据。");
    }
    identifyKeyPositions();
}

// 从网格数据构造：使用提供的网格数据，识别关键位置，失败则抛出异常
MazePathFinder::MazePathFinder(const std::vector<std::vector<char>>& gridData) : grid(gridData),
highestScore(std::numeric_limits<int>::min()),
shortestDistance(std::numeric_limits<int>::max()) {
    if (gridData.empty() || gridData[0].empty()) {
        throw std::invalid_argument("迷宫数据不能为空。");
    }
    gridSize = gridData.size();  // 设置网格尺寸
    identifyKeyPositions();  // 识别关键位置
}

// 重新加载CSV迷宫数据：先加载文件，再识别关键位置
bool MazePathFinder::reloadGridFromCSV(const std::string& filename) {
    if (loadGridFromCSV(filename)) {  // 加载成功后更新关键位置
        identifyKeyPositions();
        return true;
    }
    return false;
}

// 计算最优路径（动态规划+广度优先搜索实现）
void MazePathFinder::calculateOptimalPath() {
    int coinCount = coinPositions.size();  // 金币数量
    int trapCount = trapPositions.size();  // 陷阱数量
    int totalElements = coinCount + trapCount;  // 特殊元素总数（用于状态掩码）
    int maxState = 1 << totalElements;  // 最大状态数（2^总元素数，每个元素代表是否被访问）

    // 定义状态结构体：存储分数和距离
    struct StateInfo {
        int score;    // 当前状态的分数
        int distance; // 当前状态的距离（步数）
        StateInfo() : score(std::numeric_limits<int>::min()), distance(std::numeric_limits<int>::max()) {}  // 默认初始化（无效状态）
        StateInfo(int s, int d) : score(s), distance(d) {}  // 带参数的构造函数
    };

    // 初始化DP表：[y][x][mask] -> 状态信息
    std::vector<std::vector<std::vector<StateInfo>>> dp(
        gridSize, std::vector<std::vector<StateInfo>>(
            gridSize, std::vector<StateInfo>(maxState)
        )
    );

    // BFS队列：存储待处理的状态（y坐标, x坐标, 状态掩码）
    std::queue<std::tuple<int, int, int>> processingQueue;

    // 初始化父节点记录：用于回溯路径
    prevPositions.assign(gridSize, std::vector<std::vector<std::pair<int, int>>>(
        gridSize, std::vector<std::pair<int, int>>(maxState, { -1, -1 })  // 初始化为无效坐标
    ));
    prevMasks.assign(gridSize, std::vector<std::vector<int>>(
        gridSize, std::vector<int>(maxState, -1)  // 初始化为无效状态
    ));

    // 起点初始化：起点位置，初始状态（掩码0，未收集任何元素）
    dp[startPoint.second][startPoint.first][0] = StateInfo(0, 0);  // 初始分数0，距离0
    processingQueue.push({ startPoint.second, startPoint.first, 0 });  // 将起点加入队列

    // 移动方向：上、下、右、左（y和x的变化量）
    const int dirX[] = { 0, 0, 1, -1 };
    const int dirY[] = { 1, -1, 0, 0 };

    // BFS处理：遍历所有可达状态
    while (!processingQueue.empty()) {
        auto current = processingQueue.front();  // 取出队首状态
        processingQueue.pop();  // 移除队首状态
        int y = std::get<0>(current);  // 当前y坐标
        int x = std::get<1>(current);  // 当前x坐标
        int state = std::get<2>(current);  // 当前状态掩码
        auto& currentState = dp[y][x][state];  // 当前状态信息

        // 剪枝：跳过无效状态（未被访问过）
        if (currentState.score == std::numeric_limits<int>::min()) continue;

        // 到达终点不再扩展（避免重复处理）
        if (y == endPoint.second && x == endPoint.first) continue;

        // 探索四个方向的移动
        for (int dir = 0; dir < 4; ++dir) {
            int newX = x + dirX[dir];  // 新x坐标
            int newY = y + dirY[dir];  // 新y坐标

            // 边界与障碍物检查：确保新坐标在网格内且不是墙壁（'#'）
            if (newX < 0 || newX >= gridSize || newY < 0 || newY >= gridSize || grid[newY][newX] == '#') {
                continue;
            }

            int newState = state;  // 新状态掩码（初始与当前状态相同）
            int scoreChange = 0;   // 分数变化（收集金币加分，触发陷阱扣分）
            char cellType = grid[newY][newX];  // 新单元格类型

            // 处理陷阱：如果是陷阱且未被触发过，更新分数和状态
             if (cellType == 'T') {
                auto it = trapIndexMap.find({ newX, newY });  // 查找陷阱索引
                if (it != trapIndexMap.end()) {  // 找到陷阱
                    int trapIdx = it->second;  // 获取陷阱索引
                    int trapBit = coinCount + trapIdx;  // 陷阱在掩码中的位置（金币之后）
                    if (!(state & (1 << trapBit))) {  // 检查是否已触发（对应位是否为0）
                        scoreChange += TRAP_DEDUCTION;  // 扣分
                        newState |= (1 << trapBit);  // 更新状态掩码（对应位置1）
                    }
                }
            }
            // 处理金币：如果是金币且未被收集过，更新分数和状态
             else if (cellType == 'G') {
                auto it = coinIndexMap.find({ newX, newY });  // 查找金币索引
                if (it != coinIndexMap.end()) {  // 找到金币
                    int coinIdx = it->second;  // 获取金币索引
                    if (!(state & (1 << coinIdx))) {  // 检查是否已收集（对应位是否为0）
                        scoreChange += COIN_SCORE;  // 加分
                        newState |= (1 << coinIdx);  // 更新状态掩码（对应位置1）
                    }
                }
            }
            

            // 计算新状态的分数和距离
            int updatedScore = currentState.score + scoreChange;  // 新分数 = 当前分数 + 变化量
            int updatedDist = currentState.distance + 1;  // 新距离 = 当前距离 + 1（移动一步）

            // 更新DP状态：检查是否需要更新新状态的信息
            auto& nextState = dp[newY][newX][newState];
            bool needUpdate = false;  // 是否需要更新标志

            // 规则：优先选择分数高的状态；分数相同则选择距离短的状态
            if (updatedScore > nextState.score) {
                needUpdate = true;
            }
            else if (updatedScore == nextState.score && updatedDist < nextState.distance) {
                needUpdate = true;
            }

            // 如果需要更新，则更新状态信息并记录路径，将新状态加入队列
            if (needUpdate) {
                nextState = StateInfo(updatedScore, updatedDist);  // 更新状态
                prevPositions[newY][newX][newState] = { x, y };  // 记录前一步坐标
                prevMasks[newY][newX][newState] = state;  // 记录前一步状态
                processingQueue.push({ newY, newX, newState });  // 加入队列待处理
            }
        }
    }

    // 查找最优终点状态：遍历终点所有可能的状态，选择分数最高且距离最短的
    highestScore = std::numeric_limits<int>::min();  // 初始化最高分数为最小值
    shortestDistance = std::numeric_limits<int>::max();  // 初始化最短距离为最大值
    int bestState = -1;  // 最优状态掩码

    for (int state = 0; state < maxState; ++state) {
        const auto& endState = dp[endPoint.second][endPoint.first][state];  // 终点在该状态下的信息
        // 比较并更新最优状态
        if (endState.score > highestScore ||
            (endState.score == highestScore && endState.distance < shortestDistance)) {
            highestScore = endState.score;
            shortestDistance = endState.distance;
            bestState = state;
        }
    }

    // 重建路径并可视化：如果找到有效路径
    if (bestState != -1) {
        bestPath.clear();  // 清空现有路径
        std::pair<int, int> currentPos = endPoint;  // 从终点开始回溯
        int currentState = bestState;  // 最优状态

        // 回溯构建路径：从终点一直追溯到起点（父坐标为(-1,-1)时
                // 回溯构建路径：从终点一直追溯到起点（父坐标为(-1,-1)时结束）
        while (currentPos.first != -1) {
            bestPath.push_back(currentPos);  // 将当前位置加入路径
            // 获取前一步的坐标和状态
            auto prevPos = prevPositions[currentPos.second][currentPos.first][currentState];
            int prevState = prevMasks[currentPos.second][currentPos.first][currentState];
            currentPos = prevPos;  // 更新当前位置为前一步位置
            currentState = prevState;  // 更新当前状态为前一步状态
        }
        std::reverse(bestPath.begin(), bestPath.end());  // 反转路径，使其从起点到终点

        // 路径可视化输出：逐帧显示路径
        std::vector<std::pair<int, int>> pathToShow = bestPath;  // 复制最优路径用于显示
        for (size_t i = 0; i < pathToShow.size(); ++i) {
            system("cls");  // 清屏（Windows系统，Linux/macOS需改为"clear"）
            std::cout << "步骤 " << std::setw(4) << std::left << i << ": ("
                << pathToShow[i].first << ", " << pathToShow[i].second << ")  " << std::endl;

            // 绘制迷宫：根据当前步骤渲染迷宫状态
            for (int j = 0; j < gridSize; ++j) {
                for (int k = 0; k < gridSize; ++k) {
                    // 当前位置标记：用绿色&表示当前位置
                    if (pathToShow[i].first == k && pathToShow[i].second == j) {
                        std::cout << COLOR_GREEN << "&" << COLOR_RESET;
                        grid[j][k] = ' ';  // 移动后将原位置标记为空格（避免重复显示）
                    }
                    // 墙壁：用蓝色#表示
                    else if (grid[j][k] == '#') {
                        std::cout << COLOR_BLUE << grid[j][k] << COLOR_RESET;
                    }
                    // 陷阱：用红色T表示
                    else if (grid[j][k] == 'T') {
                        std::cout << COLOR_RED << grid[j][k] << COLOR_RESET;
                    }
                    // 资源点：用黄色表示（金币G、可能的其他资源L/B）
                    else if (grid[j][k] == 'G' || grid[j][k] == 'L' || grid[j][k] == 'B') {
                        std::cout << COLOR_YELLOW << grid[j][k] << COLOR_RESET;
                    }
                    else {
                        std::cout << grid[j][k];  // 普通空地直接显示
                    }
                }
                std::cout << std::endl;  // 换行
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(400)); // 延迟300毫秒，控制显示速度
        }
    }
}

// 获取最高得分
int MazePathFinder::getHighestScore() const {
    return highestScore;
}

// 获取最短距离（加1是因为路径长度=步数+1，即格子数）
int MazePathFinder::getShortestDistance() const {
    return shortestDistance + 1;
}

// 获取最优路径（返回路径坐标列表）
const std::vector<std::pair<int, int>>& MazePathFinder::getBestPath() const {
    return bestPath;
}

// 显示路径详情：输出路径长度和最高分数
void MazePathFinder::displayPath() const {
    if (bestPath.empty()) {  // 检查是否存在有效路径
        std::cout << "未找到有效路径或无法显示路径。" << std::endl;
        return;
    }
    // 输出路径长度和最高分数
    std::cout << "\n路径总长度: " << bestPath.size() << " 格 | 最高分数: " << getHighestScore() << std::endl;
}