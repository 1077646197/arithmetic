#include <iostream>
#include <vector>
#include <queue>
#include <climits>
#include <algorithm>
#include <unordered_map>
#include <string>

/**
 * 使用分支限界法求解击败BOSS的最小回合数技能序列（最终修正版）
 * @param bossHP BOSS的血量
 * @param skills 玩家可用技能，每个技能包含伤害和冷却时间 [[伤害, 冷却时间], ...]
 * @return 最小回合数的技能序列，格式为技能索引序列
 */
std::vector<int> findOptimalSkillSequence(int bossHP, const std::vector<std::vector<int>>& skills) {
    // 检查输入合法性
    if (bossHP <= 0 || skills.empty()) {
        return {};
    }

    // 计算玩家平均伤害，用于预估剩余回合数
    int totalDamage = 0;
    for (const auto& skill : skills) {
        totalDamage += skill[0];
    }
    int avgDamage = totalDamage / std::max(1, static_cast<int>(skills.size())); // 避免除零

    // 节点状态：(当前BOSS血量, 已用回合数, 技能冷却状态, 技能序列)
    struct Node {
        int hp;
        int rounds;
        std::vector<int> cooldowns;
        std::vector<int> sequence;

        // 构造函数
        Node() : hp(0), rounds(0) {}
        Node(int _hp, int _rounds, const std::vector<int>& _cooldowns, const std::vector<int>& _sequence)
            : hp(_hp), rounds(_rounds), cooldowns(_cooldowns), sequence(_sequence) {}
    };

    // 优先队列比较类
    struct CompareNode {
        int avgDamage;
        CompareNode(int _avgDamage) : avgDamage(_avgDamage) {}

        bool operator()(const Node& a, const Node& b) const {
            int costA = a.rounds + (a.hp + avgDamage - 1) / avgDamage;
            int costB = b.rounds + (b.hp + avgDamage - 1) / avgDamage;
            return costA > costB; // 小顶堆
        }
    };

    // 正确声明优先队列（注意：不再在声明时传递比较器，而是在类型中指定）
    using PQType = std::priority_queue<Node, std::vector<Node>, CompareNode>;
    PQType pq{ CompareNode(avgDamage) };

    // 初始化优先队列（使用push）
    std::vector<int> initialCooldowns(skills.size(), 0);
    pq.push(Node(bossHP, 0, initialCooldowns, std::vector<int>()));

    // 记录最优解
    int minRounds = INT_MAX;
    std::vector<int> bestSequence;

    // 已访问状态记录，避免重复搜索
    std::unordered_map<std::string, bool> visited;

    while (!pq.empty()) {
        // 取出队列顶部节点
        Node node = pq.top();
        pq.pop();
        int currentHP = node.hp;
        int rounds = node.rounds;
        std::vector<int> cooldowns = node.cooldowns;
        std::vector<int> sequence = node.sequence;

        // 计算当前节点代价
        int currentCost = rounds + (currentHP + avgDamage - 1) / avgDamage;
        // 如果当前代价已大于等于已知最优解，剪枝
        if (currentCost >= minRounds) {
            continue;
        }

        // 检查是否击败BOSS
        if (currentHP <= 0) {
            if (rounds < minRounds) {
                minRounds = rounds;
                bestSequence = sequence;
            }
            continue;
        }

        // 技能冷却递减
        for (size_t i = 0; i < cooldowns.size(); i++) {
            if (cooldowns[i] > 0) {
                cooldowns[i]--;
            }
        }

        // 尝试使用每个技能
        for (size_t i = 0; i < skills.size(); i++) {
            if (cooldowns[i] > 0) {
                continue; // 技能在冷却中，不能使用
            }

            int damage = skills[i][0];
            int cooldown = skills[i][1];

            // 计算使用技能后的状态
            int newHP = currentHP - damage;
            int newRounds = rounds + 1;
            std::vector<int> newCooldowns = cooldowns;
            newCooldowns[i] = cooldown;
            std::vector<int> newSequence = sequence;
            newSequence.push_back(static_cast<int>(i)); // 显式转换size_t到int

            // 生成状态键，用于去重
            std::string stateKey = std::to_string(newHP) + ",";
            for (int cd : newCooldowns) {
                stateKey += std::to_string(cd) + ",";
            }

            // 检查状态是否已访问
            if (visited.find(stateKey) != visited.end()) {
                continue;
            }
            visited[stateKey] = true;

            // 加入优先队列（使用push）
            pq.push(Node(newHP, newRounds, newCooldowns, newSequence));
        }
    }

    return bestSequence;
}

/**
 * 打印技能序列和回合数
 */
void printSkillSequence(const std::vector<int>& sequence, const std::vector<std::vector<int>>& skills) {
    if (sequence.empty()) {
        std::cout << "无法击败BOSS" << std::endl;
        return;
    }

    std::cout << "最优技能序列（回合数：" << sequence.size() << "）：" << std::endl;
    for (size_t i = 0; i < sequence.size(); i++) {
        int skillIndex = sequence[i];
        std::cout << "回合 " << (i + 1) << ": 使用技能 " << skillIndex
            << " (伤害: " << skills[skillIndex][0]
            << ", 冷却: " << skills[skillIndex][1] << ")" << std::endl;
    }
}