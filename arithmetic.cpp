#include <iostream>
#include <cstring>
#include"route_planning.h"
#include"maze.h"
#include"resource_collecting.h"
#include "puzzle_solving.h"
#include"boss_bettle.h"


using namespace std;

// 生成固定的7×7迷宫用于测试
Maze generateFixedMaze() {
    Maze maze;
    maze.size = 7;
    maze.startX = 1;
    maze.startY = 1;
    maze.exitX = 5;
    maze.exitY = 5;

    // 定义固定的迷宫布局，每行明确以'\0'结尾
    const char* layout[7] = {
        "#######",
        "#SG#G##",
        "## # ##",
        "#   T##",
        "# #####",
        "#G   E#",
        "#######"
    };

    // 使用std::string安全复制字符串
    for (int i = 0; i < 7; ++i) {
        // 确保每行有足够空间并正确终止
        snprintf(maze.grid[i], sizeof(maze.grid[i]), "%s", layout[i]);
    }

    // 输出迷宫进行测试
    for (int i = 0; i < 7; ++i) {
        cout << maze.grid[i] << endl;
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

    /*ResourcePathPlanner planner(maze);
    if (planner.solve()) {
        int maxResource = planner.getMaxResourceValue();
        std::cout << "最大资源收集值: " << maxResource << std::endl;
        planner.printDPTable();
        vector<pair<int, int>> path = planner.getOptimalPath();
        std::cout << "最优路径长度: " << path.size() << " 步" << std::endl;
        visualizePath(maze, planner.getOptimalPath());
    }
    else {
        std::cout << "无法找到从起点到终点的有效路径" << std::endl;
    }*/
    // 示例用法
    std::vector<std::vector<int>> clues = {
        {1, 0},    // 第1位是偶数
        {-1, -1}   // 每位都是不重复的素数
    };
    int resources = 20;
    std::string targetPassword = "237";

    // 计算目标密码的哈希值
    std::string targetHash = SimpleHash::hash(targetPassword);

    std::cout << "目标哈希值: " << targetHash << std::endl;
    guessPassword(clues, resources, targetHash);

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
