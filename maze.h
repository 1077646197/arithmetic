#pragma once
#include <iostream>
#include <vector>
#include <random>
#include <fstream>
#include <string>
#include <algorithm>

using namespace std;

// �Թ��������࣬ʹ�÷��η������Թ�
class MazeGenerator {
private:
    vector<vector<char>> maze;  // �Թ���ά�ַ�����
    int n;                      // �Թ���С��������������  
    random_device rd;           // ��������������������豸
    mt19937 gen;                // Mersenne Twister�����������

public:
    // ���캯������ʼ���Թ���С��������Χǽ��
    MazeGenerator(int size) : n(size), gen(rd()) {
        // ȷ��n������������Ϊ7�������Թ����ɵ���С������
        if (n < 7) n = 7;
        if (n % 2 == 0) n++;

        // ��ʼ���Թ������е�Ԫ��Ĭ��Ϊͨ·���ո��ַ���
        maze.resize(n, vector<char>(n, ' '));

        // �����Χǽ�ڣ��߽�ȫ����Ϊ#��
        for (int i = 0; i < n; i++) {
            maze[i][0] = '#';
            maze[i][n - 1] = '#';
        }
        for (int j = 0; j < n; j++) {
            maze[0][j] = '#';
            maze[n - 1][j] = '#';
        }
    }

    // �����Թ��������������÷��η��ݹ�ָ��Թ����������Ԫ��
    void generate() {
        divide(1, n - 2, 1, n - 2);  // ���ڲ�����ʼ�ָ��������Χǽ�ڣ�
        addSpecialElements();         // �������Ԫ�أ���㡢�յ㡢��ҵȣ�
    }

    // ��ӡ�Թ�������̨
    void print() const {
        cout << "ԭʼ�Թ���" << endl;
        for (const auto& row : maze) {
            for (char c : row) {
                cout << c;
            }
            cout << endl;
        }
    }

    // ���Թ�����ΪJSON��ʽ�ļ�������Python����Ķ�ȡҪ��
    void saveToJSON(const string& filename) const {
        // ���ļ���д��JSON����
        ofstream file(filename,ios::binary);
        // ����ļ��Ƿ�ɹ���
        if (!file.is_open()) {
            // ����ʧ�ܣ����������Ϣ����׼������
            cerr << "�޷����ļ�: " << filename << endl;
            return;
        }

        // ��ʼд��JSON�������ʼ����
        file << "{\n";
        // д��"maze"���Ͷ�Ӧ��������ʼ���ţ�����2���ո�
        file << "  \"maze\": [\n";

        // �����Թ���ÿһ��
        for (size_t i = 0; i < maze.size(); i++) {
            // д�뵱ǰ�е���ʼ���ţ�����4���ո�
            file << "    [";
            // ������ǰ�е�ÿ��Ԫ��
            for (size_t j = 0; j < maze[i].size(); j++) {
                // ���Թ�Ԫ����˫���Ű���д���ļ�������JSON�ַ�����ʽ��
                file << "\"" << maze[i][j] << "\"";
                // �����һ��Ԫ���⣬ÿ��Ԫ�غ���Ӷ��ŷָ�
                if (j < maze[i].size() - 1) file << ", ";
            }
            // д�뵱ǰ�еĽ�������
            file << "]";
            // �����һ���⣬ÿ�к���Ӷ��ŷָ�
            if (i < maze.size() - 1) file << ",";
            // ���У�����JSON��ʽ�Ŀɶ���
            file << "\n";
        }

        // д��JSON����Ͷ���Ľ������Ų�����
        file << "  ]\n";
        file << "}" << endl;
    }

    // ���Թ�����ΪCSV��ʽ�ļ�
    void saveToCSV(const string& filename) const {
        ofstream file(filename);
        if (!file.is_open()) {
            cerr << "�޷����ļ�: " << filename << endl;
            return;
        }

        // �����������Ԫ���ö��ŷָ�
        for (const auto& row : maze) {
            for (size_t j = 0; j < row.size(); j++) {
                file << row[j];
                if (j < row.size() - 1) file << ",";
            }
            file << endl;
        }
    }

    // ��ȡ���λ��
    pair<int, int> getStartPosition() const {
        int startRow = -1, startCol = -1;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (maze[i][j] == 'S') {
                    startRow = i;
                    startCol = j;
                    break;
                }
            }
            if (startRow != -1) break;
        }
        return { startRow, startCol };
    }

    // ��ӻ�ȡ�Թ������Ĺ�������
    vector<vector<char>> getMaze() const {
        return maze;
    }



private:
    // ���η����ĺ������ݹ�ָ��Թ�����
    void divide(int rowStart, int rowEnd, int colStart, int colEnd) {
        // ���㵱ǰ����ĸ߶ȺͿ��
        int height = rowEnd - rowStart + 1;
        int width = colEnd - colStart + 1;

        // �������̫С�������Էָ��ֹͣ�ݹ�
        if (height < 3 || width < 3) return;

        // �ռ����п��ܵķָ���λ�ã�������ż����/�У�ȷ��ǽ����ͨ·���棩
        vector<int> possibleRows, possibleCols;
        for (int i = rowStart; i <= rowEnd; i++) {
            if (i % 2 == 0) possibleRows.push_back(i);
        }
        for (int j = colStart; j <= colEnd; j++) {
            if (j % 2 == 0) possibleCols.push_back(j);
        }

        // ���û�к��ʵķָ��ߣ�ֹͣ�ݹ�
        if (possibleRows.empty() || possibleCols.empty()) return;

        // ���ѡ��ָ��к���
        uniform_int_distribution<> rowDist(0, possibleRows.size() - 1);
        uniform_int_distribution<> colDist(0, possibleCols.size() - 1);

        int splitRow = possibleRows[rowDist(gen)];
        int splitCol = possibleCols[colDist(gen)];

        // �ڷָ����Ϸ���ǽ�ڣ���ѡ�����к���ȫ����Ϊ#��
        for (int j = colStart; j <= colEnd; j++) {
            maze[splitRow][j] = '#';
        }
        for (int i = rowStart; i <= rowEnd; i++) {
            maze[i][splitCol] = '#';
        }

        // ���ĸ����γɵ�ǽ�������ѡ������λ�ÿ����������Թ���ͨ��
        vector<pair<int, int>> walls = {
            {splitRow, uniform_int_distribution<>(colStart, splitCol - 1)(gen) | 1},  // �ϱߣ�����λ�ÿ�����
            {splitRow, uniform_int_distribution<>(splitCol + 1, colEnd)(gen) | 1},    // �±ߣ�����λ�ÿ�����
            {uniform_int_distribution<>(rowStart, splitRow - 1)(gen) | 1, splitCol},  // ��ߣ�����λ�ÿ�����
            {uniform_int_distribution<>(splitRow + 1, rowEnd)(gen) | 1, splitCol}     // �ұߣ�����λ�ÿ�����
        };

        // ���ѡ��һ��ǽ����ͨ��ȷ���Թ���Ψһͨ·��
        uniform_int_distribution<> wallDist(0, 3);
        int blockedWall = wallDist(gen);

        // ��ѡ�е�����ǽ���Ͽ���
        for (int i = 0; i < 4; i++) {
            if (i != blockedWall) {
                maze[walls[i].first][walls[i].second] = ' ';
            }
        }

        // �ݹ鴦���ĸ�������
        divide(rowStart, splitRow - 1, colStart, splitCol - 1);  // ��������
        divide(rowStart, splitRow - 1, splitCol + 1, colEnd);    // ��������
        divide(splitRow + 1, rowEnd, colStart, splitCol - 1);    // ��������
        divide(splitRow + 1, rowEnd, splitCol + 1, colEnd);      // ��������
    }


    // ���Թ�������������Ԫ�أ���㡢�յ㡢��ҡ�����ȣ�
    void addSpecialElements() {
        vector<pair<int, int>> availableCells;

        // �ռ����п��õ�ͨ·��Ԫ�񣨷�ǽ�ڵ�λ�ã�
        for (int i = 1; i < n - 1; i++) {
            for (int j = 1; j < n - 1; j++) {
                if (maze[i][j] == ' ') {
                    availableCells.push_back({ i, j });
                }
            }
        }

        // ȷ�����㹻�Ŀռ����Ԫ��
        if (availableCells.size() < 6) return;

        // ���ȷ��������յ㣬��һ��
        shuffle(availableCells.begin(), availableCells.end(), gen);
        int index = 0;

        // �������
        if (index < availableCells.size()) {
            maze[availableCells[index].first][availableCells[index].second] = 'S';
            index++;
        }

        // �����յ�
        if (index < availableCells.size()) {
            maze[availableCells[index].first][availableCells[index].second] = 'E';
            index++;
        }

        // ��������������յ��û��ʣ��ռ䣬ֱ�ӷ���
        if (index >= availableCells.size()) return;

        // ���������ڵ�����Ԫ�ؼ����������
        vector<pair<char, int>> requiredElements = {
            {'G', 1},  // �������1��
            {'T', 1},  // ��������1��
            {'L', 1},  // ��������1��
            {'B', 1}   // BOSS����1��
        };

        // ������������Ԫ��
        for (const auto& element : requiredElements) {
            for (int j = 0; j < element.second && index < availableCells.size(); j++) {
                maze[availableCells[index].first][availableCells[index].second] = element.first;
                index++;
            }
        }

        // ������������Ԫ�غ�û��ʣ��ռ䣬ֱ�ӷ���
        if (index >= availableCells.size()) return;

        // ʣ��Ԫ�ظ���Ȩ�ط���
        vector<char> elementTypes = { 'G', 'T', 'L', 'B' };
        vector<int> elementWeights = { 4, 2, 2, 1 }; // ��Դ�������Ȩ�ظ���

        // ����ʣ�����λ��
        int remainingCells = availableCells.size() - index;
        if (remainingCells <= 0) return;

        // ����ÿ��Ԫ�صĶ�������
        vector<int> elementCounts(elementTypes.size(), 0);
        int totalWeight = 0;

        for (int weight : elementWeights) {
            totalWeight += weight;
        }

        int remainingElements = remainingCells / 2; // ֻʹ�ò���ʣ��ռ���ö���Ԫ��

        for (int i = 0; i < elementTypes.size() - 1; i++) {
            elementCounts[i] = remainingElements * elementWeights[i] / totalWeight;
            remainingElements -= elementCounts[i];
        }
        elementCounts.back() = remainingElements; // ���һ��Ԫ�ػ��ʣ������

        // �������ö���Ԫ��
        for (int i = 0; i < elementTypes.size(); i++) {
            for (int j = 0; j < elementCounts[i] && index < availableCells.size(); j++) {
                maze[availableCells[index].first][availableCells[index].second] = elementTypes[i];
                index++;
            }
        }
    }

};