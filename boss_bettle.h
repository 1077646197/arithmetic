#include <iostream>
#include <vector>
#include <queue>
#include <climits>
#include <algorithm>
#include <unordered_map>
#include <string>

/**
 * ʹ�÷�֧�޽編������BOSS����С�غ����������У����������棩
 * @param bossHP BOSS��Ѫ��
 * @param skills ��ҿ��ü��ܣ�ÿ�����ܰ����˺�����ȴʱ�� [[�˺�, ��ȴʱ��], ...]
 * @return ��С�غ����ļ������У���ʽΪ������������
 */
std::vector<int> findOptimalSkillSequence(int bossHP, const std::vector<std::vector<int>>& skills) {
    // �������Ϸ���
    if (bossHP <= 0 || skills.empty()) {
        return {};
    }

    // �������ƽ���˺�������Ԥ��ʣ��غ���
    int totalDamage = 0;
    for (const auto& skill : skills) {
        totalDamage += skill[0];
    }
    int avgDamage = totalDamage / std::max(1, static_cast<int>(skills.size())); // �������

    // �ڵ�״̬��(��ǰBOSSѪ��, ���ûغ���, ������ȴ״̬, ��������)
    struct Node {
        int hp;
        int rounds;
        std::vector<int> cooldowns;
        std::vector<int> sequence;

        // ���캯��
        Node() : hp(0), rounds(0) {}
        Node(int _hp, int _rounds, const std::vector<int>& _cooldowns, const std::vector<int>& _sequence)
            : hp(_hp), rounds(_rounds), cooldowns(_cooldowns), sequence(_sequence) {}
    };

    // ���ȶ��бȽ���
    struct CompareNode {
        int avgDamage;
        CompareNode(int _avgDamage) : avgDamage(_avgDamage) {}

        bool operator()(const Node& a, const Node& b) const {
            int costA = a.rounds + (a.hp + avgDamage - 1) / avgDamage;
            int costB = b.rounds + (b.hp + avgDamage - 1) / avgDamage;
            return costA > costB; // С����
        }
    };

    // ��ȷ�������ȶ��У�ע�⣺����������ʱ���ݱȽ�����������������ָ����
    using PQType = std::priority_queue<Node, std::vector<Node>, CompareNode>;
    PQType pq{ CompareNode(avgDamage) };

    // ��ʼ�����ȶ��У�ʹ��push��
    std::vector<int> initialCooldowns(skills.size(), 0);
    pq.push(Node(bossHP, 0, initialCooldowns, std::vector<int>()));

    // ��¼���Ž�
    int minRounds = INT_MAX;
    std::vector<int> bestSequence;

    // �ѷ���״̬��¼�������ظ�����
    std::unordered_map<std::string, bool> visited;

    while (!pq.empty()) {
        // ȡ�����ж����ڵ�
        Node node = pq.top();
        pq.pop();
        int currentHP = node.hp;
        int rounds = node.rounds;
        std::vector<int> cooldowns = node.cooldowns;
        std::vector<int> sequence = node.sequence;

        // ���㵱ǰ�ڵ����
        int currentCost = rounds + (currentHP + avgDamage - 1) / avgDamage;
        // �����ǰ�����Ѵ��ڵ�����֪���Ž⣬��֦
        if (currentCost >= minRounds) {
            continue;
        }

        // ����Ƿ����BOSS
        if (currentHP <= 0) {
            if (rounds < minRounds) {
                minRounds = rounds;
                bestSequence = sequence;
            }
            continue;
        }

        // ������ȴ�ݼ�
        for (size_t i = 0; i < cooldowns.size(); i++) {
            if (cooldowns[i] > 0) {
                cooldowns[i]--;
            }
        }

        // ����ʹ��ÿ������
        for (size_t i = 0; i < skills.size(); i++) {
            if (cooldowns[i] > 0) {
                continue; // ��������ȴ�У�����ʹ��
            }

            int damage = skills[i][0];
            int cooldown = skills[i][1];

            // ����ʹ�ü��ܺ��״̬
            int newHP = currentHP - damage;
            int newRounds = rounds + 1;
            std::vector<int> newCooldowns = cooldowns;
            newCooldowns[i] = cooldown;
            std::vector<int> newSequence = sequence;
            newSequence.push_back(static_cast<int>(i)); // ��ʽת��size_t��int

            // ����״̬��������ȥ��
            std::string stateKey = std::to_string(newHP) + ",";
            for (int cd : newCooldowns) {
                stateKey += std::to_string(cd) + ",";
            }

            // ���״̬�Ƿ��ѷ���
            if (visited.find(stateKey) != visited.end()) {
                continue;
            }
            visited[stateKey] = true;

            // �������ȶ��У�ʹ��push��
            pq.push(Node(newHP, newRounds, newCooldowns, newSequence));
        }
    }

    return bestSequence;
}

/**
 * ��ӡ�������кͻغ���
 */
void printSkillSequence(const std::vector<int>& sequence, const std::vector<std::vector<int>>& skills) {
    if (sequence.empty()) {
        std::cout << "�޷�����BOSS" << std::endl;
        return;
    }

    std::cout << "���ż������У��غ�����" << sequence.size() << "����" << std::endl;
    for (size_t i = 0; i < sequence.size(); i++) {
        int skillIndex = sequence[i];
        std::cout << "�غ� " << (i + 1) << ": ʹ�ü��� " << skillIndex
            << " (�˺�: " << skills[skillIndex][0]
            << ", ��ȴ: " << skills[skillIndex][1] << ")" << std::endl;
    }
}