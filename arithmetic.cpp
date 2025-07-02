#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <string>
#include "route_planning.h"
#include "maze.h"
#include "resource_collecting.h"
#include "puzzle_solving.h"
#include "boss_bettle.h"
#include <nlohmann/json.hpp>
#include "greed.h"
using namespace std;
using json = nlohmann::json;

Maze generateMazeFromJson(const std::string& filePath) {
    std::ifstream file(filePath);
    json data;
    file >> data;

    Maze maze;
    maze.size = 15;
    maze.startX = -1;
    maze.startY = -1;
    maze.exitX = -1;
    maze.exitY = -1;

    // 解析迷宫：每行是一个数组
    for (int y = 0; y < maze.size; ++y) {
        for (int x = 0; x < maze.size; ++x) {
            if (x >= data["maze"][y].size()) {
                std::cerr << "Error: 迷宫行 " << y << " 长度不足！" << std::endl;
                exit(1);
            }
            std::string cell = data["maze"][y][x].get<std::string>();
            if (cell.size() != 1) {
                std::cerr << "Error: 迷宫单元格 (" << x << "," << y << ") 包含多个字符！" << std::endl;
                exit(1);
            }
            maze.grid[y][x] = cell[0];

            // 记录起点和终点
            if (cell[0] == 'S') {
                maze.startX = y;
                maze.startY = x;
                cout << maze.startX <<" " << maze.startY;
                cout << endl;
            }
            else if (cell[0] == 'E') {
                maze.exitX = y;
                maze.exitY = x;
                cout << maze.exitX << " " << maze.exitY;
                cout << endl;
            }
        }
        maze.grid[y][maze.size] = '\0'; // 确保 C 字符串结尾
    }

    // 验证起点和终点是否存在
    if (maze.startX == -1 || maze.startY == -1 || maze.exitX == -1 || maze.exitY == -1) {
        std::cerr << "Error: 迷宫中未找到起点 (S) 或终点 (E)！" << std::endl;
        exit(1);
    }

    // 输出迷宫（测试用）
    std::cout << "15×15 fixed test maze layout:" << std::endl;
    for (int i = 0; i < maze.size; ++i) {
        std::cout << maze.grid[i] << std::endl;
    }

    return maze;
}


int main() {
    std::string mazeFilePath = R"(C:\Users\张喆\Desktop\arithmetic\maze_15_15.json)";
    Maze maze = generateMazeFromJson(mazeFilePath);
    MAZE maze01(15,15);
    maze01.rows = 15;
    maze01.cols = 15;
    for (int i = 0; i < maze.size; i++)
    {
        for (int j = 0; j < maze.size; j++)
        {
            maze01.grid[i][j] = maze.grid[i][j];
        }
    }
    maze01.start = make_pair(maze.startX, maze.startY); 
    maze01.end = make_pair(maze.exitX, maze.exitY);


    ResourcePathPlanner planner(maze);
    if (planner.solveWithPruning()) {
        // You can add code here to handle the successful path finding
        this_thread::sleep_for(chrono::milliseconds(2000));
    }
    else {
        std::cout << "无法找到从起点到终点的有效路径" << std::endl;
    }
    

    Player player(&maze01);
    player.runUntilEnd();
    player.printResults();


    // 从JSON文件中读取密码破解数据
    std::ifstream mazeFile(mazeFilePath);
    json mazeData;
    mazeFile >> mazeData;

    // 处理密码破解
    std::string directory = "C:\\Users\\张喆\\Desktop\\password测试集\\password_test\\";
    int allguessCount = 0;

    // 使用JSON中的C和L数据，而不是从文件读取
    std::string targetHash = mazeData["L"];
    std::vector<std::vector<int>> clues = mazeData["C"];

    int resources = 1000;
    PasswordLock lock;
    int guessCount = guessPassword(clues, resources, targetHash);
    allguessCount += guessCount;
    std::cout << "密码破解猜测次数为: " << guessCount << std::endl;

    // 处理Boss战
    std::vector<int> bossHP = mazeData["B"]; // 从JSON中读取Boss血量
    std::vector<std::vector<int>> skills = mazeData["PlayerSkills"];
    int maxRounds = 100;

    std::vector<int> sequence = findOptimalSkillSequence(bossHP, skills, maxRounds);
    printSkillSequence(sequence, skills, bossHP);

    return 0;
}