#pragma once
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include <utility>

#define MAX_SIZE 101  // ����Թ��ߴ�

// �Թ��ṹ��
struct Maze {
    char grid[MAX_SIZE][MAX_SIZE];
    int size;
    int startX, startY;
    int exitX, exitY;

    // ��ʼ���Թ�
    void init(int s) {
        if (s < 7 || s > MAX_SIZE || s % 2 == 0) {
            std::cerr << "�����Թ��ߴ�����Ǵ��ڵ���7��С�ڵ���" << MAX_SIZE << "������" << std::endl;
            exit(1);
        }
        size = s;
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                grid[i][j] = '#';
            }
        }
    }
};

// �ָ��ö��
enum DivideDirection {
    VERTICAL,   // ��ֱ�ָ�
    HORIZONTAL  // ˮƽ�ָ�
};

// �Ľ��ķ��η�����Ψһͨ·�Թ���������λ������ԣ�
void divideAndConquerUniquePath(Maze& maze, int x1, int y1, int x2, int y2, DivideDirection direction) {
    // ȷ������������3x3��С���ܼ����ָ�
    if (x2 - x1 < 2 || y2 - y1 < 2) {
        // ����̫С��ֱ����Ϊͨ·
        for (int x = x1; x <= x2; x++) {
            for (int y = y1; y <= y2; y++) {
                maze.grid[x][y] = ' ';
            }
        }
        return;
    }

    int split, door;

    if (direction == VERTICAL) {
        // ��ֱ�ָ� - ѡ���м�������
        split = x1 + 1 + ((x2 - x1 - 1) / 2);
        if (split % 2 == 0) split--;  // ȷ������

        // Ψһͨ���� - ѡ���м�ż����
        door = y1 + ((y2 - y1) / 2);
        if (door % 2 == 1) door--;  // ȷ��ż��

        // ������ƫ�ƣ���ȷ���Ų��ᳬ������߽�
        int maxOffset = std::min(door - y1, y2 - door);
        maxOffset = std::min(maxOffset, 2);  // �������ƫ����
        if (maxOffset > 0) {
            int randomOffset = (rand() % (maxOffset + 1)) * 2;  // ż��ƫ��
            if (rand() % 2 == 0) randomOffset = -randomOffset;  // �������
            door += randomOffset;
        }

        // ȷ��������Ч��Χ��
        door = std::max(y1, std::min(y2, door));
        if (door % 2 == 1) door--;  // ȷ��ż��

        // ���Ʒָ�ǽ����������
        for (int y = y1; y <= y2; y++) {
            maze.grid[split][y] = '#';
        }
        maze.grid[split][door] = ' ';  // Ψһͨ��

        // �ݹ鴦���������л�Ϊˮƽ�ָ�
        divideAndConquerUniquePath(maze, x1, y1, split - 1, y2, HORIZONTAL);
        divideAndConquerUniquePath(maze, split + 1, y1, x2, y2, HORIZONTAL);
    }
    else {
        // ˮƽ�ָ� - ѡ���м�������
        split = y1 + 1 + ((y2 - y1 - 1) / 2);
        if (split % 2 == 0) split--;  // ȷ������

        // Ψһͨ���� - ѡ���м�ż����
        door = x1 + ((x2 - x1) / 2);
        if (door % 2 == 1) door--;  // ȷ��ż��

        // ������ƫ�ƣ���ȷ���Ų��ᳬ������߽�
        int maxOffset = std::min(door - x1, x2 - door);
        maxOffset = std::min(maxOffset, 2);  // �������ƫ����
        if (maxOffset > 0) {
            int randomOffset = (rand() % (maxOffset + 1)) * 2;  // ż��ƫ��
            if (rand() % 2 == 0) randomOffset = -randomOffset;  // �������
            door += randomOffset;
        }

        // ȷ��������Ч��Χ��
        door = std::max(x1, std::min(x2, door));
        if (door % 2 == 1) door--;  // ȷ��ż��

        // ���Ʒָ�ǽ����������
        for (int x = x1; x <= x2; x++) {
            maze.grid[x][split] = '#';
        }
        maze.grid[door][split] = ' ';  // Ψһͨ��

        // �ݹ鴦���������л�Ϊ��ֱ�ָ�
        divideAndConquerUniquePath(maze, x1, y1, x2, split - 1, VERTICAL);
        divideAndConquerUniquePath(maze, x1, split + 1, x2, y2, VERTICAL);
    }
}

// ���ù̶������յ㣨ȷ��Ψһ·����
void setStartExitUnique(Maze& maze) {
    // �����Ϊ���Ͻ�
    maze.startX = 1;
    maze.startY = 1;
    maze.grid[maze.startX][maze.startY] = 'S';

    // �յ���Ϊ���½�
    maze.exitX = maze.size - 2;
    maze.exitY = maze.size - 2;
    maze.grid[maze.exitX][maze.exitY] = 'E';
}

// ������Դ����������Ψһ·����
void placeResourcesUnique(Maze& maze) {
    std::vector<std::pair<int, int>> availablePositions;

    // �ռ����п���λ�ã��ų������յ㣩
    for (int i = 1; i < maze.size - 1; i++) {
        for (int j = 1; j < maze.size - 1; j++) {
            if (maze.grid[i][j] == ' ' &&
                !(i == maze.startX && j == maze.startY) &&
                !(i == maze.exitX && j == maze.exitY)) {
                availablePositions.push_back({ i, j });
            }
        }
    }

    if (availablePositions.empty()) return;

    // ��������������������ã�
    int numItems = std::max(1, (int)availablePositions.size() / 10);

    // �������λ��˳��
    for (int i = availablePositions.size() - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        std::swap(availablePositions[i], availablePositions[j]);
    }

    // ������Ʒ����һ��λ�÷�BOSS��
    for (int i = 0; i < numItems && i < availablePositions.size(); i++) {
        int x = availablePositions[i].first;
        int y = availablePositions[i].second;

        if (i == 0) {
            maze.grid[x][y] = 'B';  // BOSS
        }
        else {
            char items[] = { 'G', 'T', 'L' };  // ��ҡ����塢����
            maze.grid[x][y] = items[rand() % 3];
        }
    }
}

// ��ӡ�Թ�
void printMaze(const Maze& maze) {
    for (int i = 0; i < maze.size; i++) {
        for (int j = 0; j < maze.size; j++) {
            std::cout << maze.grid[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

// ����ΪJSON�ļ�
void saveToJSON(const Maze& maze, const std::string& filename) {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "�޷����ļ� " << filename << std::endl;
        return;
    }

    file << "{\n";
    file << "  \"size\": " << maze.size << ",\n";
    file << "  \"start\": [" << maze.startX << ", " << maze.startY << "],\n";
    file << "  \"exit\": [" << maze.exitX << ", " << maze.exitY << "],\n";
    file << "  \"grid\": [\n";

    for (int i = 0; i < maze.size; i++) {
        file << "    [";
        for (int j = 0; j < maze.size; j++) {
            file << "\"" << maze.grid[i][j] << "\"";
            if (j < maze.size - 1) file << ", ";
        }
        if (i < maze.size - 1) file << "],\n";
        else file << "]\n";
    }

    file << "  ]\n";
    file << "}\n";
    file.close();
}

// ����ΪCSV�ļ�
void saveToCSV(const Maze& maze, const std::string& filename) {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "�޷����ļ� " << filename << std::endl;
        return;
    }

    for (int i = 0; i < maze.size; i++) {
        for (int j = 0; j < maze.size; j++) {
            file << maze.grid[i][j];
            if (j < maze.size - 1) file << ",";
        }
        file << std::endl;
    }
    file.close();
}