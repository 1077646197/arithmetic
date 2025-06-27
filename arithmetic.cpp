#include <iostream>
#include <cstring>
#include"route_planning.h"
#include"maze.h"
#include"resource_collecting.h"
#include "puzzle_solving.h"
#include"boss_bettle.h"

using namespace std;

Maze generateFixedMaze() {
    Maze maze;
    maze.size = 10;
    maze.startX = 6;
    maze.startY = 3;
    maze.exitX = 8;
    maze.exitY = 8;

    // 定义10×10固定迷宫布局（四周为墙，中间设计复杂通路）
    const char* layout[20] = {
        "##########",
        "# G     ##",
        "### ### ##",
        "# G  G#G##",
        "# # #T#T##",
        "# # ######",
        "# #S  T G#",
        "# ########",
        "#G  T   E#",
        "##########"
    };

    // 安全复制迷宫布局到maze.grid
    for (int i = 0; i < maze.size; ++i) {
        snprintf(maze.grid[i], sizeof(maze.grid[i]), "%s", layout[i]);
    }

    // 输出迷宫布局用于测试
    std::cout << "n×n固定测试迷宫布局：" << std::endl;
    for (int i = 0; i < maze.size; ++i) {
        std::cout << maze.grid[i] << std::endl;
    }

    return maze;
}

int main()
{

    Maze maze= generateFixedMaze();
    //maze.init(7);  // 初始化7x7的迷宫
    //divideAndConquerUniquePath(maze, 0, 0, maze.size - 1, maze.size - 1, VERTICAL);
    //setStartExitUnique(maze);
    //placeResourcesUnique(maze);

    ResourcePathPlanner planner(maze);
    if (planner.solveWithPruning()) {
        //int maxResource = planner.getMaxResourceValue();
    }
    else {
        std::cout << "无法找到从起点到终点的有效路径" << std::endl;
    }

    //// 示例：设置目标哈希值（实际使用时应从安全存储中获取）
    //std::string targetHash = "003a44b04e2e9eac5eb7597955068e745d78bb18b17a60d26645beebe111de40";

    //// 示例线索：每位都是不重复的素数
    //std::vector<std::vector<int>> clues = { {1,1} ,{ 2,0 },{,1} };

    //// 初始资源数量(替换为玩家资源)
    //int resources = 100;

    //// 开始回溯猜测密码
    //guessPassword(clues, resources, targetHash);


    //// 示例：BOSS血量100，玩家有3个技能
    //int bossHP = 100;
    //std::vector<std::vector<int>> skills = {
    //    {5, 0},  // 普通攻击：伤害5，无冷却
    //    {10, 2}, // 大招：伤害10，冷却2回合
    //    {15, 3}  // 超级技能：伤害15，冷却3回合
    //};

    //std::vector<int> optimalSequence = findOptimalSkillSequence(bossHP, skills);
    //printSkillSequence(optimalSequence, skills);

    return 0;
}
// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
