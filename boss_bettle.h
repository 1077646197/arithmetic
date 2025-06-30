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
    int currentBoss;       // ��ǰ������BOSS����
    int currentBossHP;     // ��ǰBOSSʣ��Ѫ��
    int totalRemainingHP;  // ����ʣ��BOSS����Ѫ������ǰ+������
    int rounds;            // ���ûغ���
    vector<int> cooldowns; // ��������ȴ״̬��0��ʾ���ã�
    vector<int> sequence;  // ��������

    Node(int cb, int cbhp, int trhp, int r, const vector<int>& cd, const vector<int>& seq)
        : currentBoss(cb), currentBossHP(cbhp), totalRemainingHP(trhp),
        rounds(r), cooldowns(cd), sequence(seq) {}

    // ���ۺ��������ûغ� + Ԥ��ʣ��غϣ�������ʣ��Ѫ����ƽ���˺���
    double calculateCost(double avgDamage) const {
        return rounds + static_cast<double>(totalRemainingHP) / avgDamage;
    }

    // ���ڹ�ϣ�������ж�
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

// ��ϣ��������Node��unordered_map�洢
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

    // ��Ѫ��������BOSS��ʼѪ��֮�ͣ�
    int totalHP = accumulate(bossHP.begin(), bossHP.end(), 0);
    // ����ƽ���˺������ڴ���Ԥ����
    int totalDamage = 0;
    for (const auto& skill : skills) {
        totalDamage += skill[0];
    }
    double avgDamage = totalDamage > 0 ? static_cast<double>(totalDamage) / skills.size() : 1.0;

    // ���ȶ���������򣺴���С�Ľڵ�����
    auto compare = [avgDamage](const Node& a, const Node& b) {
        return a.calculateCost(avgDamage) > b.calculateCost(avgDamage);
        };
    priority_queue<Node, vector<Node>, decltype(compare)> pq(compare);

    // ��ʼ�����У��ӵ�һ��BOSS��ʼ����ʼ��ȴȫΪ0
    vector<int> initialCooldowns(skills.size(), 0);
    pq.push(Node(0, bossHP[0], totalHP, 0, initialCooldowns, {}));

    int minRounds = INT_MAX;
    vector<int> bestSequence;
    // ��¼ÿ��״̬����С�غ��������ڼ�֦��
    unordered_map<Node, int, NodeHash> visited;

    while (!pq.empty()) {
        Node node = pq.top();
        pq.pop();

        // ��֦���������غ�����ֱ������
        if (node.rounds >= maxRounds) {
            continue;
        }

        // �����������������һ��BOSS��ɱ��ʱ���ж�Ϊ���
        if (node.currentBoss == bossHP.size() - 1 && node.currentBossHP <= 0) {
            if (node.rounds < minRounds) {
                minRounds = node.rounds;
                bestSequence = node.sequence;
            }
            continue;
        }

        // ����ʹ��ÿ������ȴ�еļ���
        for (size_t i = 0; i < skills.size(); ++i) {
            if (node.cooldowns[i] > 0) { // ������ȴ�У�����ʹ��
                continue;
            }

            int damage = skills[i][0];   // �����˺�
            int cooldown = skills[i][1]; // ������ȴʱ�䣨ʹ�ú���ٻغϲ����ã�

            // ����Ե�ǰBOSS����Ч�˺�����������ʣ��Ѫ���������˺���Ч��
            int effectiveDamage = min(damage, node.currentBossHP);
            // ��ǰBOSSʣ��Ѫ��������Ϊ��������Ч�˺����޶���
            int newCurrentBossHP = node.currentBossHP - damage;
            // ��ʣ��Ѫ�� = ԭ��ʣ�� - ��ǰBOSSʵ����ʧ��Ѫ������Ч�˺���
            int newTotalRemainingHP = node.totalRemainingHP - effectiveDamage;

            // �غ���+1
            int newRounds = node.rounds + 1;
            // ��һ��BOSS������Ĭ�ϲ��䣩
            int newBossIndex = node.currentBoss;

            // �����ǰBOSS��ɱ���Ҳ������һ�����л�����һ��BOSS
            if (newCurrentBossHP <= 0 && newBossIndex < bossHP.size() - 1) {
                newBossIndex++;
                newCurrentBossHP = bossHP[newBossIndex]; // ��һ��BOSS�ĳ�ʼѪ��
            }

            // ���¼�����ȴ��ʹ�ú�����Ϊ��ȴʱ��+1��ȷ��֮��cooldown�غϲ����ã�
            vector<int> nextCooldowns = node.cooldowns;
            nextCooldowns[i] = cooldown + 1;  // �ؼ���������ȴʱ��+1

            // ���м�����ȴʱ��ݼ�����ǰ�غϽ�������һ�غϿ�ʼǰ����ȴ�仯��
            for (int& cd : nextCooldowns) {
                if (cd > 0) cd--;
            }

            // ���¼�������
            vector<int> newSequence = node.sequence;
            newSequence.push_back(i);

            // �����½ڵ�
            Node nextNode(newBossIndex, newCurrentBossHP, newTotalRemainingHP,
                newRounds, nextCooldowns, newSequence);

            // ��֦�������״̬�ѷ��ʹ����ҵ�ǰ�غ��������٣�������
            auto it = visited.find(nextNode);
            if (it != visited.end() && newRounds >= it->second) {
                continue;
            }
            // ��¼����¸�״̬����С�غ���
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
        cout << "�޷����޶��غ��ڻ�������BOSS" << endl;
        return;
    }

    cout << "���ż������У��غ�����" << sequence.size() << "����" << endl;

    int currentBoss = 0;
    int currentBossHP = bossHP[0]; // ��ǰBOSSʣ��Ѫ��
    vector<int> cooldowns(skills.size(), 0); // ������ȴ״̬

    for (size_t i = 0; i < sequence.size(); ++i) {
        int skillIndex = sequence[i];
        int damage = skills[skillIndex][0];
        int cooldown = skills[skillIndex][1];

        // �����ǰ�غ���Ϣ
        cout << "�غ� " << (i + 1) << ": ʹ�ü��� " << skillIndex
            << " (�˺�: " << damage
            << ", ��ȴ: " << cooldown << ")";

        // Ӧ���˺�
        currentBossHP -= damage;
        cout << " -> BOSS" << currentBoss << " ʣ��Ѫ��: " << max(0, currentBossHP) << endl;

        // ���ü�����ȴ����ȴʱ��+1��
        cooldowns[skillIndex] = cooldown + 1;  // �ؼ���������ȴʱ��+1

        // ���м�����ȴʱ��ݼ�����һ�غϿ�ʼǰ����ȴ�仯��
        for (int& cd : cooldowns) {
            if (cd > 0) cd--;
        }

        // ��鵱ǰBOSS�Ƿ񱻻���
        if (currentBossHP <= 0) {
            // ��������һ��BOSS����ʾ����BOSS������
            if (currentBoss == bossHP.size() - 1) {
                cout << "    -> BOSS" << currentBoss << " �����ܣ�����BOSS��ȫ�����ܣ�" << endl;
            }
            else {
                // �л�����һ��BOSS
                currentBoss++;
                currentBossHP = bossHP[currentBoss];
                cout << "    -> BOSS" << currentBoss - 1 << " �����ܣ���ʼ���� BOSS" << currentBoss
                    << " (��ʼѪ��: " << currentBossHP << ")" << endl;
            }
        }
    }
}
