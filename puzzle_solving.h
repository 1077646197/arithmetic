#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <set>
#include <sstream>
#include <iomanip>
#include <array>
#include <cstring>

bool checkClues(const std::string& password, const std::vector<std::vector<int>>& clues);

// SHA-256�㷨��C++ʵ��
class SHA256 {
private:
    // SHA-256�㷨�ĳ���
    const static uint32_t K[64];

    // ��ʼ��ϣֵ������SHA-256��׼��
    std::array<uint32_t, 8> H = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };

    // ���ߺ���������ת
    inline uint32_t rightRotate(uint32_t value, unsigned int count) {
        return (value >> count) | (value << (32 - count));
    }

    // ������512λ�����ݿ�
    void processBlock(const uint8_t* block) {
        // ������Ϣ���ȱ�
        uint32_t W[64];
        for (int t = 0; t < 16; t++) {
            W[t] = (block[t * 4] << 24) | (block[t * 4 + 1] << 16) |
                (block[t * 4 + 2] << 8) | (block[t * 4 + 3]);
        }

        // ��չ��Ϣ���ȱ�
        for (int t = 16; t < 64; t++) {
            uint32_t s0 = rightRotate(W[t - 15], 7) ^ rightRotate(W[t - 15], 18) ^ (W[t - 15] >> 3);
            uint32_t s1 = rightRotate(W[t - 2], 17) ^ rightRotate(W[t - 2], 19) ^ (W[t - 2] >> 10);
            W[t] = W[t - 16] + s0 + W[t - 7] + s1;
        }

        // ��ʼ����������
        uint32_t a = H[0];
        uint32_t b = H[1];
        uint32_t c = H[2];
        uint32_t d = H[3];
        uint32_t e = H[4];
        uint32_t f = H[5];
        uint32_t g = H[6];
        uint32_t h = H[7];

        // ��ѭ��
        for (int t = 0; t < 64; t++) {
            uint32_t S1 = rightRotate(e, 6) ^ rightRotate(e, 11) ^ rightRotate(e, 25);
            uint32_t ch = (e & f) ^ ((~e) & g);
            uint32_t temp1 = h + S1 + ch + K[t] + W[t];
            uint32_t S0 = rightRotate(a, 2) ^ rightRotate(a, 13) ^ rightRotate(a, 22);
            uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            uint32_t temp2 = S0 + maj;

            h = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        // ���¹�ϣֵ
        H[0] += a;
        H[1] += b;
        H[2] += c;
        H[3] += d;
        H[4] += e;
        H[5] += f;
        H[6] += g;
        H[7] += h;
    }

public:
    // �����������ݵ�SHA-256��ϣֵ
    std::vector<uint8_t> compute(const std::vector<uint8_t>& message) {
        // ��������ĳ��ȣ�ԭʼ���� + 1�ֽڵ�1 + ���0 + 8�ֽڵĳ��ȣ�
        uint64_t originalBitLength = message.size() * 8;
        uint64_t paddedLength = message.size() + 1 + 8; // ������Ҫ���9���ֽ�
        paddedLength = (paddedLength + 63) & ~63;       // ����Ϊ64�ֽڵı�����512λ��

        // �����������Ϣ
        std::vector<uint8_t> paddedMessage(paddedLength, 0);
        std::copy(message.begin(), message.end(), paddedMessage.begin());

        // ���һ��1λ����Ϊһ���ֽڵ�0x80��
        paddedMessage[message.size()] = 0x80;

        // �����Ϣ���ȣ���λΪ��λ�������
        for (int i = 0; i < 8; i++) {
            paddedMessage[paddedLength - 8 + i] = (originalBitLength >> ((7 - i) * 8)) & 0xFF;
        }

        // ���鴦����Ϣ
        for (size_t i = 0; i < paddedLength; i += 64) {
            processBlock(&paddedMessage[i]);
        }

        // �������չ�ϣֵ��32�ֽڣ�
        std::vector<uint8_t> hash(32);
        for (int i = 0; i < 8; i++) {
            hash[i * 4] = (H[i] >> 24) & 0xFF;
            hash[i * 4 + 1] = (H[i] >> 16) & 0xFF;
            hash[i * 4 + 2] = (H[i] >> 8) & 0xFF;
            hash[i * 4 + 3] = H[i] & 0xFF;
        }

        return hash;
    }

    // ���ù�ϣ״̬
    void reset() {
        H = {
            0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
            0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
        };
    }
};

// SHA-256�㷨�ĳ���������SHA-256��׼��
const uint32_t SHA256::K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

// �������� - ���ڹ�ϣ����
class PasswordLock {
private:
    // �̶���ֵ����ԭ�߼�����һ��
    std::vector<unsigned char> salt = {
        0xb2, 0x53, 0x22, 0x65, 0x7d, 0xdf, 0xb0, 0xfe,
        0x9c, 0xde, 0xde, 0xfe, 0xf3, 0x1d, 0xdc, 0x3e
    };

    // �������������ֽ�����ת��Ϊʮ�������ַ���
    std::string bytesToHex(const std::vector<uint8_t>& bytes) {
        std::stringstream ss;
        for (const auto& byte : bytes) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
        }
        return ss.str();
    }

public:
    // ������ε�SHA-256��ϣֵ
    std::string hashPassword(const std::string& password) {
        // ������ת��Ϊ�ֽ���
        std::vector<unsigned char> passwordBytes(password.begin(), password.end());

        // �ϲ���ֵ������
        std::vector<unsigned char> combined;
        combined.insert(combined.end(), salt.begin(), salt.end());
        combined.insert(combined.end(), passwordBytes.begin(), passwordBytes.end());

        // ����SHA-256��ϣ
        SHA256 sha256;
        std::vector<uint8_t> hashBytes = sha256.compute(combined);

        return bytesToHex(hashBytes);
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

        // ʹ��SHA-256���������ϣ
        PasswordLock lock;
        std::string currentHash = lock.hashPassword(password);

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