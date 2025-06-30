#include <iostream>
#include <cstring>
#include"route_planning.h"
#include"maze.h"
#include"resource_collecting.h"
#include "puzzle_solving.h"
#include "boss_bettle.h"

using namespace std;

int main()
{

    //string mazeFilePath = R"(C:\Users\张喆\Desktop\maze_15_15_2.csv)";

    //MazePathFinder planner(mazeFilePath);

    //// 计算最优路径（包含动态规划求解和可视化过程）
    //planner.calculateOptimalPath();

    //// 输出结果信息
    //std::cout << "最大分数: " << planner.getHighestScore() << std::endl;
    //std::cout << "最短路径长度: " << planner.getShortestDistance() << std::endl;

    // 显示路径详细信息（文本形式）
   /* planner.displayPath();*/
    //// 示例：设置目标哈希值（实际使用时应从安全存储中获取）
    //std::string targetHash = "003a44b04e2e9eac5eb7597955068e745d78bb18b17a60d26645beebe111de40";

    //// 示例线索：每位都是不重复的素数
    //std::vector<std::vector<int>> clues = { {1,1} ,{ 2,0 },{,1} };

    //// 初始资源数量(替换为玩家资源)
    //int resources = 100;

    //// 开始回溯猜测密码
    //guessPassword(clues, resources, targetHash);


 

        vector<int> bossHP = { 13, 13, 20 };
        vector<vector<int>> skills = {
              // 普通攻击：伤害6，冷却2
            {9, 5},   // 小技能：伤害2，冷却0
                       // 中技能：伤害4，冷却1
            {3,0}
        };
        int maxRounds = 100;

        vector<int> sequence = findOptimalSkillSequence(bossHP, skills, maxRounds);
        printSkillSequence(sequence, skills, bossHP);

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
