#pragma once
#include <iostream>
#include <vector>
#include <random>
#include <fstream>
#include <string>
#include <algorithm>

using namespace std;

// 迷宫生成器类，使用分治法生成迷宫
class MazeGenerator {
private:
    vector<vector<char>> maze;  // 迷宫二维字符数组
    int n;                      // 迷宫大小（行数和列数）  
    random_device rd;           // 用于生成随机数的种子设备
    mt19937 gen;                // Mersenne Twister随机数生成器

public:
    // 构造函数，初始化迷宫大小并创建外围墙壁
    MazeGenerator(int size) : n(size), gen(rd()) {
        // 确保n是奇数且至少为7（满足迷宫生成的最小条件）
        if (n < 7) n = 7;
        if (n % 2 == 0) n++;

        // 初始化迷宫，所有单元格默认为通路（空格字符）
        maze.resize(n, vector<char>(n, ' '));

        // 添加外围墙壁（边界全部设为#）
        for (int i = 0; i < n; i++) {
            maze[i][0] = '#';
            maze[i][n - 1] = '#';
        }
        for (int j = 0; j < n; j++) {
            maze[0][j] = '#';
            maze[n - 1][j] = '#';
        }
    }

    // 生成迷宫的主函数，调用分治法递归分割迷宫并添加特殊元素
    void generate() {
        divide(1, n - 2, 1, n - 2);  // 从内部区域开始分割（不包括外围墙壁）
        addSpecialElements();         // 添加特殊元素（起点、终点、金币等）
    }

    // 打印迷宫到控制台
    void print() const {
        cout << "原始迷宫：" << endl;
        for (const auto& row : maze) {
            for (char c : row) {
                cout << c;
            }
            cout << endl;
        }
    }

    // 将迷宫保存为JSON格式文件，符合Python代码的读取要求
    void saveToJSON(const string& filename) const {
        // 打开文件以写入JSON数据
        ofstream file(filename,ios::binary);
        // 检查文件是否成功打开
        if (!file.is_open()) {
            // 若打开失败，输出错误信息到标准错误流
            cerr << "无法打开文件: " << filename << endl;
            return;
        }

        // 开始写入JSON对象的起始符号
        file << "{\n";
        // 写入"maze"键和对应的数组起始符号，缩进2个空格
        file << "  \"maze\": [\n";

        // 遍历迷宫的每一行
        for (size_t i = 0; i < maze.size(); i++) {
            // 写入当前行的起始符号，缩进4个空格
            file << "    [";
            // 遍历当前行的每个元素
            for (size_t j = 0; j < maze[i].size(); j++) {
                // 将迷宫元素用双引号包裹写入文件（符合JSON字符串格式）
                file << "\"" << maze[i][j] << "\"";
                // 除最后一个元素外，每个元素后添加逗号分隔
                if (j < maze[i].size() - 1) file << ", ";
            }
            // 写入当前行的结束符号
            file << "]";
            // 除最后一行外，每行后添加逗号分隔
            if (i < maze.size() - 1) file << ",";
            // 换行，保持JSON格式的可读性
            file << "\n";
        }

        // 写入JSON数组和对象的结束符号并换行
        file << "  ]\n";
        file << "}" << endl;
    }

    // 将迷宫保存为CSV格式文件
    void saveToCSV(const string& filename) const {
        ofstream file(filename);
        if (!file.is_open()) {
            cerr << "无法打开文件: " << filename << endl;
            return;
        }

        // 按行输出，单元格用逗号分隔
        for (const auto& row : maze) {
            for (size_t j = 0; j < row.size(); j++) {
                file << row[j];
                if (j < row.size() - 1) file << ",";
            }
            file << endl;
        }
    }

    // 获取起点位置
    pair<int, int> getStartPosition() const {
        int startRow = -1, startCol = -1;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (maze[i][j] == 'S') {
                    startRow = i;
                    startCol = j;
                    break;
                }
            }
            if (startRow != -1) break;
        }
        return { startRow, startCol };
    }

    // 添加获取迷宫副本的公共方法
    vector<vector<char>> getMaze() const {
        return maze;
    }



private:
    // 分治法核心函数：递归分割迷宫区域
    void divide(int rowStart, int rowEnd, int colStart, int colEnd) {
        // 计算当前区域的高度和宽度
        int height = rowEnd - rowStart + 1;
        int width = colEnd - colStart + 1;

        // 如果区域太小（不足以分割），停止递归
        if (height < 3 || width < 3) return;

        // 收集所有可能的分割线位置（必须是偶数行/列，确保墙壁与通路交替）
        vector<int> possibleRows, possibleCols;
        for (int i = rowStart; i <= rowEnd; i++) {
            if (i % 2 == 0) possibleRows.push_back(i);
        }
        for (int j = colStart; j <= colEnd; j++) {
            if (j % 2 == 0) possibleCols.push_back(j);
        }

        // 如果没有合适的分割线，停止递归
        if (possibleRows.empty() || possibleCols.empty()) return;

        // 随机选择分割行和列
        uniform_int_distribution<> rowDist(0, possibleRows.size() - 1);
        uniform_int_distribution<> colDist(0, possibleCols.size() - 1);

        int splitRow = possibleRows[rowDist(gen)];
        int splitCol = possibleCols[colDist(gen)];

        // 在分割线上放置墙壁（将选定的行和列全部设为#）
        for (int j = colStart; j <= colEnd; j++) {
            maze[splitRow][j] = '#';
        }
        for (int i = rowStart; i <= rowEnd; i++) {
            maze[i][splitCol] = '#';
        }

        // 在四个新形成的墙壁上随机选择三个位置开洞，保持迷宫连通性
        vector<pair<int, int>> walls = {
            {splitRow, uniform_int_distribution<>(colStart, splitCol - 1)(gen) | 1},  // 上边（奇数位置开洞）
            {splitRow, uniform_int_distribution<>(splitCol + 1, colEnd)(gen) | 1},    // 下边（奇数位置开洞）
            {uniform_int_distribution<>(rowStart, splitRow - 1)(gen) | 1, splitCol},  // 左边（奇数位置开洞）
            {uniform_int_distribution<>(splitRow + 1, rowEnd)(gen) | 1, splitCol}     // 右边（奇数位置开洞）
        };

        // 随机选择一堵墙不打通（确保迷宫有唯一通路）
        uniform_int_distribution<> wallDist(0, 3);
        int blockedWall = wallDist(gen);

        // 在选中的三个墙壁上开洞
        for (int i = 0; i < 4; i++) {
            if (i != blockedWall) {
                maze[walls[i].first][walls[i].second] = ' ';
            }
        }

        // 递归处理四个子区域
        divide(rowStart, splitRow - 1, colStart, splitCol - 1);  // 左上区域
        divide(rowStart, splitRow - 1, splitCol + 1, colEnd);    // 右上区域
        divide(splitRow + 1, rowEnd, colStart, splitCol - 1);    // 左下区域
        divide(splitRow + 1, rowEnd, splitCol + 1, colEnd);      // 右下区域
    }


    // 在迷宫中随机添加特殊元素（起点、终点、金币、陷阱等）
    void addSpecialElements() {
        vector<pair<int, int>> availableCells;

        // 收集所有可用的通路单元格（非墙壁的位置）
        for (int i = 1; i < n - 1; i++) {
            for (int j = 1; j < n - 1; j++) {
                if (maze[i][j] == ' ') {
                    availableCells.push_back({ i, j });
                }
            }
        }

        // 确保有足够的空间放置元素
        if (availableCells.size() < 6) return;

        // 首先放置起点和终点，各一个
        shuffle(availableCells.begin(), availableCells.end(), gen);
        int index = 0;

        // 放置起点
        if (index < availableCells.size()) {
            maze[availableCells[index].first][availableCells[index].second] = 'S';
            index++;
        }

        // 放置终点
        if (index < availableCells.size()) {
            maze[availableCells[index].first][availableCells[index].second] = 'E';
            index++;
        }

        // 如果放置完起点和终点后没有剩余空间，直接返回
        if (index >= availableCells.size()) return;

        // 定义必须存在的其他元素及其最低数量
        vector<pair<char, int>> requiredElements = {
            {'G', 1},  // 金币至少1个
            {'T', 1},  // 陷阱至少1个
            {'L', 1},  // 机关至少1个
            {'B', 1}   // BOSS至少1个
        };

        // 放置其他必须元素
        for (const auto& element : requiredElements) {
            for (int j = 0; j < element.second && index < availableCells.size(); j++) {
                maze[availableCells[index].first][availableCells[index].second] = element.first;
                index++;
            }
        }

        // 如果放置完必须元素后没有剩余空间，直接返回
        if (index >= availableCells.size()) return;

        // 剩余元素根据权重分配
        vector<char> elementTypes = { 'G', 'T', 'L', 'B' };
        vector<int> elementWeights = { 4, 2, 2, 1 }; // 资源和陷阱的权重更高

        // 计算剩余可用位置
        int remainingCells = availableCells.size() - index;
        if (remainingCells <= 0) return;

        // 计算每种元素的额外数量
        vector<int> elementCounts(elementTypes.size(), 0);
        int totalWeight = 0;

        for (int weight : elementWeights) {
            totalWeight += weight;
        }

        int remainingElements = remainingCells / 2; // 只使用部分剩余空间放置额外元素

        for (int i = 0; i < elementTypes.size() - 1; i++) {
            elementCounts[i] = remainingElements * elementWeights[i] / totalWeight;
            remainingElements -= elementCounts[i];
        }
        elementCounts.back() = remainingElements; // 最后一种元素获得剩余数量

        // 继续放置额外元素
        for (int i = 0; i < elementTypes.size(); i++) {
            for (int j = 0; j < elementCounts[i] && index < availableCells.size(); j++) {
                maze[availableCells[index].first][availableCells[index].second] = elementTypes[i];
                index++;
            }
        }
    }

};