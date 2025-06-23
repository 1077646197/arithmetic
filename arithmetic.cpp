// arithmetic.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "maze.h"
int main() {
    // 初始化随机数种子
    srand(time(nullptr));

    // 设置迷宫尺寸
    int size = 15;

    // 创建并初始化迷宫
    Maze maze;
    maze.init(size);

    // 生成唯一通路迷宫
    divideAndConquerUniquePath(maze, 1, 1, size - 2, size - 2, VERTICAL);

    // 设置起点和终点
    setStartExitUnique(maze);

    // 放置资源
    placeResourcesUnique(maze);

    // 打印迷宫
    std::cout << "唯一通路迷宫（尺寸 " << size << "x" << size << "）：" << std::endl;
    printMaze(maze);

    // 保存到文件（可选）
    // saveToJSON(maze, "unique_maze.json");
    // saveToCSV(maze, "unique_maze.csv");

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
