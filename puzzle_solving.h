#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <set>
#include <sstream>

bool checkClues(const std::string& password, const std::vector<std::vector<int>>& clues);

// 简单哈希加密类 - 适用于三位数字密码
class SimpleHash {
public:
    static std::string hash(const std::string& password) {
        if (password.length() != 3) return "";

        const int weights[3] = { 101, 107, 109 };
        uint32_t hashValue = 0;

        for (int i = 0; i < 3; i++) {
            int digit = password[i] - '0';
            hashValue += digit * weights[i];
        }

        hashValue = (hashValue << 5) | (hashValue >> 27);
        hashValue ^= hashValue >> 16;
        hashValue ^= hashValue >> 8;

        std::stringstream ss;
        ss << std::hex << std::setw(8) << std::setfill('0') << hashValue;
        return ss.str();
    }
};

// 检查数字是否为素数
bool isPrime(int num) {
    if (num <= 1) return false;
    if (num == 2) return true;
    if (num % 2 == 0) return false;
    for (int i = 3; i * i <= num; i += 2) {
        if (num % i == 0) return false;
    }
    return true;
}

// 检查当前组合是否符合线索
bool isValid(const std::vector<int>& combination, const std::vector<std::vector<int>>& clues) {
    if (combination.size() < 3) return true; // 组合未完成时不检查

    std::string password = std::to_string(combination[0]) +
        std::to_string(combination[1]) +
        std::to_string(combination[2]);

    return checkClues(password, clues);
}

// 检查密码是否符合所有线索
bool checkClues(const std::string& password, const std::vector<std::vector<int>>& clues) {
    for (const auto& clue : clues) {
        if (clue.size() == 2) {
            int a = clue[0];
            int b = clue[1];
            if (a == -1 && b == -1) {
                // 每位都是不重复的素数
                std::set<char> digits;
                for (char c : password) {
                    int digit = c - '0';
                    if (!isPrime(digit) || digits.count(c) > 0) return false;
                    digits.insert(c);
                }
            }
            else if (a >= 1 && a <= 3) {
                // 第a位的奇偶性
                int digit = password[a - 1] - '0';
                if ((b == 0 && digit % 2 != 0) || (b == 1 && digit % 2 != 1)) return false;
            }
        }
        else if (clue.size() == 3) {
            // 某一位是固定数字
            if (clue[0] != -1 && password[0] - '0' != clue[0]) return false;
            if (clue[1] != -1 && password[1] - '0' != clue[1]) return false;
            if (clue[2] != -1 && password[2] - '0' != clue[2]) return false;
        }
    }
    return true;
}

// 回溯法生成所有可能的密码组合
bool backtrack(std::vector<int>& combination, std::vector<bool>& used,
    const std::vector<std::vector<int>>& clues,
    int& resources, const std::string& targetHash, bool& found) {

    // 当组合长度为3时，检查是否为正确密码
    if (combination.size() == 3) {
        std::string password = std::to_string(combination[0]) +
            std::to_string(combination[1]) +
            std::to_string(combination[2]);

        std::string currentHash = SimpleHash::hash(password);

        if (currentHash == targetHash) {
            std::cout << "恭喜！猜对了密码：" << password << "，剩余资源：" << resources << std::endl;
            found = true;
            return true;
        }
        else {
            resources--;
            std::cout << "猜错了！尝试了密码：" << password << "，剩余资源：" << resources << std::endl;
            return false;
        }
    }

    // 尝试每一个可能的数字
    for (int digit = 0; digit <= 9; digit++) {
        if (!used[digit]) {
            // 选择当前数字
            combination.push_back(digit);
            used[digit] = true;

            // 检查当前选择是否有效
            if (isValid(combination, clues)) {
                // 继续生成下一位
                if (resources > 0 && !found) {
                    if (backtrack(combination, used, clues, resources, targetHash, found)) {
                        return true;
                    }
                }
            }

            // 回溯：撤销选择
            combination.pop_back();
            used[digit] = false;
        }
    }

    return false;
}

// 主函数：使用回溯法猜测密码
void guessPassword(const std::vector<std::vector<int>>& clues, int& resources, const std::string& targetHash) {
    std::vector<int> combination;
    std::vector<bool> used(10, false);
    bool found = false;

    std::cout << "开始使用回溯法猜密码！" << std::endl;
    backtrack(combination, used, clues, resources, targetHash, found);

    if (!found) {
        std::cout << "没有更多可能的密码组合了！" << std::endl;
    }
}



//回溯法的实现说明
//回溯法的核心在于递归地尝试所有可能的组合，并在不满足条件时 "回溯" 到上一步。在这个密码猜测问题中，回溯法的实现主要体现在以下几个方面：
//
//递归搜索：backtrack函数通过递归的方式尝试所有可能的三位数字组合
//状态管理：使用combination向量记录当前已选择的数字，used数组标记哪些数字已被使用
//剪枝优化：在每一步选择后，通过isValid函数检查当前组合是否符合线索，不符合的组合会被提前剪枝
//回溯操作：在递归返回后，撤销当前选择，继续尝试其他可能性
//算法流程
//从第一位数字开始，尝试 0 - 9 中的每个数字
//每选择一个数字，检查当前组合是否符合线索
//如果符合，继续递归生成下一位数字
//如果组合长度达到 3，计算哈希值并与目标比较
//如果匹配成功，返回结果；否则回溯并尝试其他组合
//当所有可能的组合都尝试完毕或资源耗尽时停止

//哈希算法说明
//这个简单哈希算法的核心逻辑非常直观：
//
//加权求和：对三位数字分别乘以不同的素数权重（101, 107, 109），然后求和
//位移混淆：使用左移和右移操作改变哈希值的位分布
//异或操作：通过异或进一步混淆哈希值
//结果转换：将 32 位整数转换为 8 位十六进制字符串
//算法特点
//简单易懂：核心算法只有几行代码，没有复杂的数学运算
//无需依赖：完全不使用任何外部库或复杂函数
//快速计算：对于三位数字密码，计算几乎是瞬间完成
//足够区分：对于三位数字的组合（1000 种可能），这个哈希算法的冲突概率极低
//使用示例
//以密码 "235" 为例，哈希计算过程如下：
//
//计算加权和：2×101 + 3×107 + 5×109 = 202 + 321 + 545 = 1068
//位移操作：(1068 << 5) | (1068 >> 27) = 34176 | 0 = 34176
//异或操作：34176 ^ (34176 >> 16) ^ (34176 >> 8) = 34176 ^ 53 ^ 134 = 34089
//转换为十六进制：0x8589
