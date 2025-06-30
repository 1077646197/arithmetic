#include <iostream>
#include <vector>
#include <queue>
#include <climits>
#include <algorithm>
#include <unordered_map>
#include <numeric>
#include <functional>

using namespace std;

struct Node {
    int currentBoss;       // 当前攻击的BOSS索引
    int currentBossHP;     // 当前BOSS剩余血量
    int totalRemainingHP;  // 所有剩余BOSS的总血量（当前+后续）
    int rounds;            // 已用回合数
    vector<int> cooldowns; // 各技能冷却状态（0表示可用）
    vector<int> sequence;  // 技能序列

    Node(int cb, int cbhp, int trhp, int r, const vector<int>& cd, const vector<int>& seq)
        : currentBoss(cb), currentBossHP(cbhp), totalRemainingHP(trhp),
        rounds(r), cooldowns(cd), sequence(seq) {}

    // 代价函数：已用回合 + 预估剩余回合（基于总剩余血量和平均伤害）
    double calculateCost(double avgDamage) const {
        return rounds + static_cast<double>(totalRemainingHP) / avgDamage;
    }

    // 用于哈希表的相等判断
    bool operator==(const Node& other) const {
        if (currentBoss != other.currentBoss || currentBossHP != other.currentBossHP ||
            totalRemainingHP != other.totalRemainingHP || rounds != other.rounds ||
            cooldowns.size() != other.cooldowns.size()) {
            return false;
        }
        for (size_t i = 0; i < cooldowns.size(); ++i) {
            if (cooldowns[i] != other.cooldowns[i]) return false;
        }
        return true;
    }
};

// 哈希函数用于Node的unordered_map存储
struct NodeHash {
    size_t operator()(const Node& node) const {
        size_t hash = node.currentBoss;
        hash = hash * 31 + node.currentBossHP;
        hash = hash * 31 + node.totalRemainingHP;
        hash = hash * 31 + node.rounds;
        for (int cd : node.cooldowns) {
            hash = hash * 31 + cd;
        }
        return hash;
    }
};

vector<int> findOptimalSkillSequence(
    const vector<int>& bossHP,
    const vector<vector<int>>& skills,
    int maxRounds) {

    if (bossHP.empty() || skills.empty() || maxRounds <= 0) {
        return {};
    }

    // 总血量（所有BOSS初始血量之和）
    int totalHP = accumulate(bossHP.begin(), bossHP.end(), 0);
    // 计算平均伤害（用于代价预估）
    int totalDamage = 0;
    for (const auto& skill : skills) {
        totalDamage += skill[0];
    }
    double avgDamage = totalDamage > 0 ? static_cast<double>(totalDamage) / skills.size() : 1.0;

    // 优先队列排序规则：代价小的节点优先
    auto compare = [avgDamage](const Node& a, const Node& b) {
        return a.calculateCost(avgDamage) > b.calculateCost(avgDamage);
        };
    priority_queue<Node, vector<Node>, decltype(compare)> pq(compare);

    // 初始化队列：从第一个BOSS开始，初始冷却全为0
    vector<int> initialCooldowns(skills.size(), 0);
    pq.push(Node(0, bossHP[0], totalHP, 0, initialCooldowns, {}));

    int minRounds = INT_MAX;
    vector<int> bestSequence;
    // 记录每个状态的最小回合数（用于剪枝）
    unordered_map<Node, int, NodeHash> visited;

    while (!pq.empty()) {
        Node node = pq.top();
        pq.pop();

        // 剪枝：超过最大回合数，直接跳过
        if (node.rounds >= maxRounds) {
            continue;
        }

        // 核心修正：仅当最后一个BOSS被杀死时，判定为完成
        if (node.currentBoss == bossHP.size() - 1 && node.currentBossHP <= 0) {
            if (node.rounds < minRounds) {
                minRounds = node.rounds;
                bestSequence = node.sequence;
            }
            continue;
        }

        // 尝试使用每个非冷却中的技能
        for (size_t i = 0; i < skills.size(); ++i) {
            if (node.cooldowns[i] > 0) { // 技能冷却中，不能使用
                continue;
            }

            int damage = skills[i][0];   // 技能伤害
            int cooldown = skills[i][1]; // 技能冷却时间（使用后多少回合不能用）

            // 计算对当前BOSS的有效伤害（不超过其剩余血量，超额伤害无效）
            int effectiveDamage = min(damage, node.currentBossHP);
            // 当前BOSS剩余血量（可能为负，但有效伤害已限定）
            int newCurrentBossHP = node.currentBossHP - damage;
            // 总剩余血量 = 原总剩余 - 当前BOSS实际损失的血量（有效伤害）
            int newTotalRemainingHP = node.totalRemainingHP - effectiveDamage;

            // 回合数+1
            int newRounds = node.rounds + 1;
            // 下一个BOSS索引（默认不变）
            int newBossIndex = node.currentBoss;

            // 如果当前BOSS被杀死且不是最后一个，切换到下一个BOSS
            if (newCurrentBossHP <= 0 && newBossIndex < bossHP.size() - 1) {
                newBossIndex++;
                newCurrentBossHP = bossHP[newBossIndex]; // 下一个BOSS的初始血量
            }

            // 更新技能冷却（使用后设置为冷却时间+1，确保之后cooldown回合不能用）
            vector<int> nextCooldowns = node.cooldowns;
            nextCooldowns[i] = cooldown + 1;  // 关键修正：冷却时间+1

            // 所有技能冷却时间递减（当前回合结束，下一回合开始前的冷却变化）
            for (int& cd : nextCooldowns) {
                if (cd > 0) cd--;
            }

            // 更新技能序列
            vector<int> newSequence = node.sequence;
            newSequence.push_back(i);

            // 创建新节点
            Node nextNode(newBossIndex, newCurrentBossHP, newTotalRemainingHP,
                newRounds, nextCooldowns, newSequence);

            // 剪枝：如果该状态已访问过，且当前回合数不更少，则跳过
            auto it = visited.find(nextNode);
            if (it != visited.end() && newRounds >= it->second) {
                continue;
            }
            // 记录或更新该状态的最小回合数
            visited[nextNode] = newRounds;
            pq.push(nextNode);
        }
    }

    return bestSequence;
}

void printSkillSequence(const vector<int>& sequence,
    const vector<vector<int>>& skills,
    const vector<int>& bossHP) {
    if (sequence.empty()) {
        cout << "无法在限定回合内击败所有BOSS" << endl;
        return;
    }

    cout << "最优技能序列（回合数：" << sequence.size() << "）：" << endl;

    int currentBoss = 0;
    int currentBossHP = bossHP[0]; // 当前BOSS剩余血量
    vector<int> cooldowns(skills.size(), 0); // 技能冷却状态

    for (size_t i = 0; i < sequence.size(); ++i) {
        int skillIndex = sequence[i];
        int damage = skills[skillIndex][0];
        int cooldown = skills[skillIndex][1];

        // 输出当前回合信息
        cout << "回合 " << (i + 1) << ": 使用技能 " << skillIndex
            << " (伤害: " << damage
            << ", 冷却: " << cooldown << ")";

        // 应用伤害
        currentBossHP -= damage;
        cout << " -> BOSS" << currentBoss << " 剩余血量: " << max(0, currentBossHP) << endl;

        // 设置技能冷却（冷却时间+1）
        cooldowns[skillIndex] = cooldown + 1;  // 关键修正：冷却时间+1

        // 所有技能冷却时间递减（下一回合开始前的冷却变化）
        for (int& cd : cooldowns) {
            if (cd > 0) cd--;
        }

        // 检查当前BOSS是否被击败
        if (currentBossHP <= 0) {
            // 如果是最后一个BOSS，提示所有BOSS被击败
            if (currentBoss == bossHP.size() - 1) {
                cout << "    -> BOSS" << currentBoss << " 被击败！所有BOSS已全部击败！" << endl;
            }
            else {
                // 切换到下一个BOSS
                currentBoss++;
                currentBossHP = bossHP[currentBoss];
                cout << "    -> BOSS" << currentBoss - 1 << " 被击败！开始攻击 BOSS" << currentBoss
                    << " (初始血量: " << currentBossHP << ")" << endl;
            }
        }
    }
}
