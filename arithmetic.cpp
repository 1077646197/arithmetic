#include <iostream>
#include"route_planning.h"
#include"maze.h"
#include"resource_collecting.h"
#include "puzzle_solving.h"
#include"boss_bettle.h"
using namespace std;
int main()
{
    std::cout << "Hello World!\n";
    Maze maze;
    maze.init(11);  // 初始化11x11的迷宫
    divideAndConquerUniquePath(maze, 0, 0, maze.size - 1, maze.size - 1, VERTICAL);
    setStartExitUnique(maze);
    placeResourcesUnique(maze);

    ResourcePathPlanner planner(maze);
    if (planner.solve()) {
        int maxResource = planner.getMaxResourceValue();
        std::cout << "最大资源收集值: " << maxResource << std::endl;

        std::vector<std::pair<int, int>> path = planner.getOptimalPath();
        std::cout << "最优路径长度: " << path.size() << " 步" << std::endl;
    }
    else {
        std::cout << "无法找到从起点到终点的有效路径" << std::endl;
    }
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
