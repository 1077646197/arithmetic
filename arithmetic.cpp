#include <iostream>
#include"maze.h"
#include"greed.h"

int main() {
    int n;
    cout << "请输入迷宫的尺寸（建议为奇数，最小为7）: ";

    // 检查输入是否为有效整数
    while (!(cin >> n)) {
        cin.clear(); // 清除错误标志
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // 忽略缓冲区
        cout << "输入无效，请输入一个整数: ";
    }

    // 确保n是奇数且至少为7
    if (n < 7) {
        cout << "尺寸过小，已自动设置为7" << endl;
        n = 7;
    }
    if (n % 2 == 0) {
        cout << "尺寸应为奇数，已自动调整为" << n + 1 << endl;
        n++;
    }

    MazeGenerator generator(n);
    generator.generate();
    generator.print();
    // 获取起点位置
    auto start = generator.getStartPosition();
    cout << "起点位置: (" << start.first << ", " << start.second << ")" << endl;

    // 询问是否保存文件
    char saveChoice;
    std::cout << "是否保存迷宫到文件？(y/n): ";
    std::cin >> saveChoice;

    if (saveChoice == 'y' || saveChoice == 'Y') {
        generator.saveToJSON("maze.json");
        generator.saveToCSV("maze.csv");
        std::cout << "迷宫已保存为maze.json和maze.csv" << std::endl;
    }
    
    // 创建玩家
    Player player(start.first, start.second);

    // 创建资源拾取策略
    ResourcePickingStrategy strategy(generator.getMaze());

    // 执行贪心策略获取路径
    cout << "\n=== 开始路径探索 ===" << endl;
    auto pickupPath = strategy.executeGreedyStrategy(player);
    cout << "=== 路径探索完成 ===" << endl;

    
    cout << "\n=== 探索结果 ===" << endl;
    cout << "玩家最终分数：" << player.getScore() << endl;
    cout << "路径长度: " << pickupPath.size() << " 步" << endl;
    // 输出完整路径
    cout << "\n=== 完整路径 (序偶形式) ===" << endl;
    cout << "[";
    for (size_t i = 0; i < pickupPath.size(); i++) {
        cout << "(" << pickupPath[i].first << ", " << pickupPath[i].second << ")";
        if (i < pickupPath.size() - 1) {
            cout << "-> ";
        }
    }
    cout << "]" << endl;
    return 0;
}