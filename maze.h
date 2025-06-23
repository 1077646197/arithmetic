#pragma once
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include <utility>

#define MAX_SIZE 101  // 最大迷宫尺寸

// 迷宫结构体
struct Maze {
    char grid[MAX_SIZE][MAX_SIZE];
    int size;
    int startX, startY;
    int exitX, exitY;

    // 初始化迷宫
    void init(int s) {
        if (s < 7 || s > MAX_SIZE || s % 2 == 0) {
            std::cerr << "错误：迷宫尺寸必须是大于等于7且小于等于" << MAX_SIZE << "的奇数" << std::endl;
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

// 分割方向枚举
enum DivideDirection {
    VERTICAL,   // 垂直分割
    HORIZONTAL  // 水平分割
};

// 改进的分治法生成唯一通路迷宫（增加门位置随机性）
void divideAndConquerUniquePath(Maze& maze, int x1, int y1, int x2, int y2, DivideDirection direction) {
    // 确保区域至少有3x3大小才能继续分割
    if (x2 - x1 < 2 || y2 - y1 < 2) {
        // 区域太小，直接设为通路
        for (int x = x1; x <= x2; x++) {
            for (int y = y1; y <= y2; y++) {
                maze.grid[x][y] = ' ';
            }
        }
        return;
    }

    int split, door;

    if (direction == VERTICAL) {
        // 垂直分割 - 选择中间奇数列
        split = x1 + 1 + ((x2 - x1 - 1) / 2);
        if (split % 2 == 0) split--;  // 确保奇数

        // 唯一通道门 - 选择中间偶数行
        door = y1 + ((y2 - y1) / 2);
        if (door % 2 == 1) door--;  // 确保偶数

        // 添加随机偏移，但确保门不会超出区域边界
        int maxOffset = std::min(door - y1, y2 - door);
        maxOffset = std::min(maxOffset, 2);  // 限制最大偏移量
        if (maxOffset > 0) {
            int randomOffset = (rand() % (maxOffset + 1)) * 2;  // 偶数偏移
            if (rand() % 2 == 0) randomOffset = -randomOffset;  // 随机正负
            door += randomOffset;
        }

        // 确保门在有效范围内
        door = std::max(y1, std::min(y2, door));
        if (door % 2 == 1) door--;  // 确保偶数

        // 绘制分割墙，仅保留门
        for (int y = y1; y <= y2; y++) {
            maze.grid[split][y] = '#';
        }
        maze.grid[split][door] = ' ';  // 唯一通道

        // 递归处理子区域，切换为水平分割
        divideAndConquerUniquePath(maze, x1, y1, split - 1, y2, HORIZONTAL);
        divideAndConquerUniquePath(maze, split + 1, y1, x2, y2, HORIZONTAL);
    }
    else {
        // 水平分割 - 选择中间奇数行
        split = y1 + 1 + ((y2 - y1 - 1) / 2);
        if (split % 2 == 0) split--;  // 确保奇数

        // 唯一通道门 - 选择中间偶数列
        door = x1 + ((x2 - x1) / 2);
        if (door % 2 == 1) door--;  // 确保偶数

        // 添加随机偏移，但确保门不会超出区域边界
        int maxOffset = std::min(door - x1, x2 - door);
        maxOffset = std::min(maxOffset, 2);  // 限制最大偏移量
        if (maxOffset > 0) {
            int randomOffset = (rand() % (maxOffset + 1)) * 2;  // 偶数偏移
            if (rand() % 2 == 0) randomOffset = -randomOffset;  // 随机正负
            door += randomOffset;
        }

        // 确保门在有效范围内
        door = std::max(x1, std::min(x2, door));
        if (door % 2 == 1) door--;  // 确保偶数

        // 绘制分割墙，仅保留门
        for (int x = x1; x <= x2; x++) {
            maze.grid[x][split] = '#';
        }
        maze.grid[door][split] = ' ';  // 唯一通道

        // 递归处理子区域，切换为垂直分割
        divideAndConquerUniquePath(maze, x1, y1, x2, split - 1, VERTICAL);
        divideAndConquerUniquePath(maze, x1, split + 1, x2, y2, VERTICAL);
    }
}

// 设置固定起点和终点（确保唯一路径）
void setStartExitUnique(Maze& maze) {
    // 起点设为左上角
    maze.startX = 1;
    maze.startY = 1;
    maze.grid[maze.startX][maze.startY] = 'S';

    // 终点设为右下角
    maze.exitX = maze.size - 2;
    maze.exitY = maze.size - 2;
    maze.grid[maze.exitX][maze.exitY] = 'E';
}

// 放置资源（避免阻塞唯一路径）
void placeResourcesUnique(Maze& maze) {
    std::vector<std::pair<int, int>> availablePositions;

    // 收集所有可用位置（排除起点和终点）
    for (int i = 1; i < maze.size - 1; i++) {
        for (int j = 1; j < maze.size - 1; j++) {
            if (maze.grid[i][j] == ' ' &&
                !(i == maze.startX && j == maze.startY) &&
                !(i == maze.exitX && j == maze.exitY)) {
                availablePositions.push_back({ i, j });
            }
        }
    }

    if (availablePositions.empty()) return;

    // 计算放置数量（少量放置）
    int numItems = std::max(1, (int)availablePositions.size() / 10);

    // 随机打乱位置顺序
    for (int i = availablePositions.size() - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        std::swap(availablePositions[i], availablePositions[j]);
    }

    // 放置物品（第一个位置放BOSS）
    for (int i = 0; i < numItems && i < availablePositions.size(); i++) {
        int x = availablePositions[i].first;
        int y = availablePositions[i].second;

        if (i == 0) {
            maze.grid[x][y] = 'B';  // BOSS
        }
        else {
            char items[] = { 'G', 'T', 'L' };  // 金币、陷阱、机关
            maze.grid[x][y] = items[rand() % 3];
        }
    }
}

// 打印迷宫
void printMaze(const Maze& maze) {
    for (int i = 0; i < maze.size; i++) {
        for (int j = 0; j < maze.size; j++) {
            std::cout << maze.grid[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

// 保存为JSON文件
void saveToJSON(const Maze& maze, const std::string& filename) {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "无法打开文件 " << filename << std::endl;
        return;
    }

    file << "{\n";
    file << "  \"size\": " << maze.size << ",\n";
    file << "  \"start\": [" << maze.startX << ", " << maze.startY << "],\n";
    file << "  \"exit\": [" << maze.exitX << ", " << maze.exitY << "],\n";
    file << "  \"grid\": [\n";

    for (int i = 0; i < maze.size; i++) {
        file << "    [";
        for (int j = 0; j < maze.size; j++) {
            file << "\"" << maze.grid[i][j] << "\"";
            if (j < maze.size - 1) file << ", ";
        }
        if (i < maze.size - 1) file << "],\n";
        else file << "]\n";
    }

    file << "  ]\n";
    file << "}\n";
    file.close();
}

// 保存为CSV文件
void saveToCSV(const Maze& maze, const std::string& filename) {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "无法打开文件 " << filename << std::endl;
        return;
    }

    for (int i = 0; i < maze.size; i++) {
        for (int j = 0; j < maze.size; j++) {
            file << maze.grid[i][j];
            if (j < maze.size - 1) file << ",";
        }
        file << std::endl;
    }
    file.close();
}