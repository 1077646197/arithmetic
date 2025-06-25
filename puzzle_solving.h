#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <set>
#include <sstream>

bool checkClues(const std::string& password, const std::vector<std::vector<int>>& clues);

// �򵥹�ϣ������ - ��������λ��������
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

// ��������Ƿ�Ϊ����
bool isPrime(int num) {
    if (num <= 1) return false;
    if (num == 2) return true;
    if (num % 2 == 0) return false;
    for (int i = 3; i * i <= num; i += 2) {
        if (num % i == 0) return false;
    }
    return true;
}

// ��鵱ǰ����Ƿ��������
bool isValid(const std::vector<int>& combination, const std::vector<std::vector<int>>& clues) {
    if (combination.size() < 3) return true; // ���δ���ʱ�����

    std::string password = std::to_string(combination[0]) +
        std::to_string(combination[1]) +
        std::to_string(combination[2]);

    return checkClues(password, clues);
}

// ��������Ƿ������������
bool checkClues(const std::string& password, const std::vector<std::vector<int>>& clues) {
    for (const auto& clue : clues) {
        if (clue.size() == 2) {
            int a = clue[0];
            int b = clue[1];
            if (a == -1 && b == -1) {
                // ÿλ���ǲ��ظ�������
                std::set<char> digits;
                for (char c : password) {
                    int digit = c - '0';
                    if (!isPrime(digit) || digits.count(c) > 0) return false;
                    digits.insert(c);
                }
            }
            else if (a >= 1 && a <= 3) {
                // ��aλ����ż��
                int digit = password[a - 1] - '0';
                if ((b == 0 && digit % 2 != 0) || (b == 1 && digit % 2 != 1)) return false;
            }
        }
        else if (clue.size() == 3) {
            // ĳһλ�ǹ̶�����
            if (clue[0] != -1 && password[0] - '0' != clue[0]) return false;
            if (clue[1] != -1 && password[1] - '0' != clue[1]) return false;
            if (clue[2] != -1 && password[2] - '0' != clue[2]) return false;
        }
    }
    return true;
}

// ���ݷ��������п��ܵ��������
bool backtrack(std::vector<int>& combination, std::vector<bool>& used,
    const std::vector<std::vector<int>>& clues,
    int& resources, const std::string& targetHash, bool& found) {

    // ����ϳ���Ϊ3ʱ������Ƿ�Ϊ��ȷ����
    if (combination.size() == 3) {
        std::string password = std::to_string(combination[0]) +
            std::to_string(combination[1]) +
            std::to_string(combination[2]);

        std::string currentHash = SimpleHash::hash(password);

        if (currentHash == targetHash) {
            std::cout << "��ϲ���¶������룺" << password << "��ʣ����Դ��" << resources << std::endl;
            found = true;
            return true;
        }
        else {
            resources--;
            std::cout << "�´��ˣ����������룺" << password << "��ʣ����Դ��" << resources << std::endl;
            return false;
        }
    }

    // ����ÿһ�����ܵ�����
    for (int digit = 0; digit <= 9; digit++) {
        if (!used[digit]) {
            // ѡ��ǰ����
            combination.push_back(digit);
            used[digit] = true;

            // ��鵱ǰѡ���Ƿ���Ч
            if (isValid(combination, clues)) {
                // ����������һλ
                if (resources > 0 && !found) {
                    if (backtrack(combination, used, clues, resources, targetHash, found)) {
                        return true;
                    }
                }
            }

            // ���ݣ�����ѡ��
            combination.pop_back();
            used[digit] = false;
        }
    }

    return false;
}

// ��������ʹ�û��ݷ��²�����
void guessPassword(const std::vector<std::vector<int>>& clues, int& resources, const std::string& targetHash) {
    std::vector<int> combination;
    std::vector<bool> used(10, false);
    bool found = false;

    std::cout << "��ʼʹ�û��ݷ������룡" << std::endl;
    backtrack(combination, used, clues, resources, targetHash, found);

    if (!found) {
        std::cout << "û�и�����ܵ���������ˣ�" << std::endl;
    }
}



//���ݷ���ʵ��˵��
//���ݷ��ĺ������ڵݹ�س������п��ܵ���ϣ����ڲ���������ʱ "����" ����һ�������������²������У����ݷ���ʵ����Ҫ���������¼������棺
//
//�ݹ�������backtrack����ͨ���ݹ�ķ�ʽ�������п��ܵ���λ�������
//״̬����ʹ��combination������¼��ǰ��ѡ������֣�used��������Щ�����ѱ�ʹ��
//��֦�Ż�����ÿһ��ѡ���ͨ��isValid������鵱ǰ����Ƿ���������������ϵ���ϻᱻ��ǰ��֦
//���ݲ������ڵݹ鷵�غ󣬳�����ǰѡ�񣬼�����������������
//�㷨����
//�ӵ�һλ���ֿ�ʼ������ 0 - 9 �е�ÿ������
//ÿѡ��һ�����֣���鵱ǰ����Ƿ��������
//������ϣ������ݹ�������һλ����
//�����ϳ��ȴﵽ 3�������ϣֵ����Ŀ��Ƚ�
//���ƥ��ɹ������ؽ����������ݲ������������
//�����п��ܵ���϶�������ϻ���Դ�ľ�ʱֹͣ

//��ϣ�㷨˵��
//����򵥹�ϣ�㷨�ĺ����߼��ǳ�ֱ�ۣ�
//
//��Ȩ��ͣ�����λ���ֱַ���Բ�ͬ������Ȩ�أ�101, 107, 109����Ȼ�����
//λ�ƻ�����ʹ�����ƺ����Ʋ����ı��ϣֵ��λ�ֲ�
//��������ͨ������һ��������ϣֵ
//���ת������ 32 λ����ת��Ϊ 8 λʮ�������ַ���
//�㷨�ص�
//���׶��������㷨ֻ�м��д��룬û�и��ӵ���ѧ����
//������������ȫ��ʹ���κ��ⲿ����Ӻ���
//���ټ��㣺������λ�������룬���㼸����˲�����
//�㹻���֣�������λ���ֵ���ϣ�1000 �ֿ��ܣ��������ϣ�㷨�ĳ�ͻ���ʼ���
//ʹ��ʾ��
//������ "235" Ϊ������ϣ����������£�
//
//�����Ȩ�ͣ�2��101 + 3��107 + 5��109 = 202 + 321 + 545 = 1068
//λ�Ʋ�����(1068 << 5) | (1068 >> 27) = 34176 | 0 = 34176
//��������34176 ^ (34176 >> 16) ^ (34176 >> 8) = 34176 ^ 53 ^ 134 = 34089
//ת��Ϊʮ�����ƣ�0x8589
