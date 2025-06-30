#pragma once
#ifndef PATH_PLANNING_H
#define PATH_PLANNING_H

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <limits>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <utility>  // for std::pair

using namespace std;

// �����������
const int GOLD_VALUE = 5;     // ��Ҽ�5��
const int TRAP_PENALTY = -3;  // �����3��

class path_planning {
private:
    vector<vector<char>> maze;  // �Թ���ͼ
    int size;                  // �Թ��ߴ�

    // ����λ��
    pair<int, int> startPos;   // �������
    pair<int, int> exitPos;    // �յ�����

    // ��Һ�������Ϣ
    vector<pair<int, int>> gold_locations;  // ���λ���б�
    map<pair<int, int>, int> gold_map;      // ���λ�õ�������ӳ��
    vector<pair<int, int>> trap_locations;  // ����λ���б�
    map<pair<int, int>, int> trap_map;      // ����λ�õ�������ӳ��

    // ��̬�滮���·���ؽ���Ϣ
    vector<vector<vector<pair<int, int>>>> dp_table;  // DP��: dp[y][x][mask] = {max_score, min_distance}
    vector<vector<vector<pair<int, int>>>> parent_pos; // ���ڵ�λ����Ϣ
    vector<vector<vector<int>>> parent_mask;          // ���ڵ�״̬��Ϣ

    // �����Ϣ
    vector<pair<int, int>> optimalPath;  // ����·��
    int maxScore;                       // ������
    int minDistanceForMaxScore;         // ���������ʱ����С����

    // ˽�з���
    void findStartExitAndFeatures();    // ������㡢�յ����������
    bool readMazeFromCSV(const string& filename);  // ��CSV��ȡ�Թ�

public:
    // ���캯��
    path_planning();
    explicit path_planning(const string& filename);
    explicit path_planning(const vector<vector<char>>& m);

    // �Թ����ط���
    bool loadMazeFromCSV(const string& filename);

    // ��ⷽ��
    void solve();  // ʹ�ö�̬�滮�������·��

    // �����ȡ����
    int getMaxScore() const;  // ��ȡ������
    int getMinDistance() const;  // ��ȡ���·������
    const vector<pair<int, int>>& getOptimalPath() const;  // ��ȡ����·��

    // ���ӻ�����
    void visualizePath() const;  // ���ӻ�·���ͷ����仯
};

#endif // PATH_PLANNING_H