#include "path_planning.h"
#include <algorithm>
#include <climits>
#include <queue>

// ���Թ����ҳ���㡢�յ㡢��Һ������λ�ã����ֲ��䣩
void path_planning::findStartExitAndFeatures() {
    int gold_idx = 0;
    int trap_idx = 0;
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            if (maze[i][j] == 'S') startPos = { j, i };
            else if (maze[i][j] == 'E') exitPos = { j, i };
            else if (maze[i][j] == 'G') {
                gold_locations.push_back({ j, i });
                gold_map[{j, i}] = gold_idx++;
            }
            else if (maze[i][j] == 'T') {
                trap_locations.push_back({ j, i });
                trap_map[{j, i}] = trap_idx++;
            }
        }
    }
}

// ��CSV�ļ���ȡ�Թ���ͼ�����ֲ��䣩
bool path_planning::readMazeFromCSV(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "�޷����ļ�: " << filename << endl;
        return false;
    }

    string line;
    maze.clear();

    while (getline(file, line)) {
        vector<char> row;
        stringstream ss(line);
        string cell;

        while (getline(ss, cell, ',')) {
            if (!cell.empty()) {
                row.push_back(cell[0]);
            }
        }

        if (!row.empty()) {
            maze.push_back(row);
        }
    }

    file.close();

    if (maze.empty() || maze[0].empty()) {
        cerr << "�Թ�����Ϊ�ա�" << endl;
        return false;
    }

    size = maze.size();
    return true;
}

// Ĭ�Ϲ��캯�������ֲ��䣩
path_planning::path_planning() : size(0), maxScore(numeric_limits<int>::min()),
minDistanceForMaxScore(numeric_limits<int>::max()) {}

// ��CSV�ļ������Թ��Ĺ��캯�������ֲ��䣩
path_planning::path_planning(const string& filename) : maxScore(numeric_limits<int>::min()),
minDistanceForMaxScore(numeric_limits<int>::max()) {
    if (!readMazeFromCSV(filename)) {
        throw invalid_argument("�޷���CSV�ļ������Թ���");
    }
    findStartExitAndFeatures();
}

// �Ӷ�ά�ַ���������Թ��Ĺ��캯�������ֲ��䣩
path_planning::path_planning(const vector<vector<char>>& m) : maze(m), maxScore(numeric_limits<int>::min()),
minDistanceForMaxScore(numeric_limits<int>::max()) {
    if (m.empty() || m[0].empty()) {
        throw invalid_argument("�Թ�����Ϊ�ա�");
    }
    size = m.size();
    findStartExitAndFeatures();
}

// ��CSV�ļ������Թ������ֲ��䣩
bool path_planning::loadMazeFromCSV(const string& filename) {
    if (readMazeFromCSV(filename)) {
        findStartExitAndFeatures();
        return true;
    }
    return false;
}

// ��̬�滮����Թ����Ż��棩
void path_planning::solve() {
    int num_gold = gold_locations.size();
    int num_traps = trap_locations.size();
    int total_features = num_gold + num_traps;
    int max_mask = 1 << total_features;

    // ʹ�ø����յ����ݽṹ��ʾDP״̬
    struct DPState {
        int score;
        int distance;
        DPState() : score(numeric_limits<int>::min()), distance(numeric_limits<int>::max()) {}
        DPState(int s, int d) : score(s), distance(d) {}
    };

    vector<vector<vector<DPState>>> dp(
        size, vector<vector<DPState>>(
            size, vector<DPState>(max_mask)
        )
    );

    // ʹ�ö��н���BFS�������봦��״̬
    queue<tuple<int, int, int>> q;

    // ��ʼ�����ڵ���Ϣ
    parent_pos.assign(size, vector<vector<pair<int, int>>>(size, vector<pair<int, int>>(max_mask, { -1, -1 })));
    parent_mask.assign(size, vector<vector<int>>(size, vector<int>(max_mask, -1)));

    // ��ʼ״̬����㣬maskΪ0������0������0
    dp[startPos.second][startPos.first][0] = DPState(0, 0);
    q.push({ startPos.second, startPos.first, 0 });

    // �ĸ��ƶ�����
    const int dx[] = { 0, 0, 1, -1 };
    const int dy[] = { 1, -1, 0, 0 };

    // ���������BFS
    while (!q.empty()) {
        auto current_tuple = q.front();
        q.pop();
        int y = get<0>(current_tuple);
        int x = get<1>(current_tuple);
        int mask = get<2>(current_tuple);
        auto& current = dp[y][x][mask];

        // �����ǰ״̬�������ŵģ���������֦��
        if (current.score == numeric_limits<int>::min()) continue;

        // ����Ѿ������յ㣬����Ҫ������չ����֦��
        if (y == exitPos.second && x == exitPos.first) continue;

        // ������п��ܵ��ƶ�
        for (int dir = 0; dir < 4; ++dir) {
            int nx = x + dx[dir];
            int ny = y + dy[dir];

            // ���߽��ǽ��
            if (nx < 0 || nx >= size || ny < 0 || ny >= size || maze[ny][nx] == '#') {
                continue;
            }

            int new_mask = mask;
            int score_change = 0;
            char cell = maze[ny][nx];

            // �����Һ�����
            if (cell == 'G') {
                auto it = gold_map.find({ nx, ny });
                if (it != gold_map.end()) {
                    int gold_idx = it->second;
                    if (!(mask & (1 << gold_idx))) {
                        score_change += GOLD_VALUE;
                        new_mask |= (1 << gold_idx);
                    }
                }
            }
            else if (cell == 'T') {
                auto it = trap_map.find({ nx, ny });
                if (it != trap_map.end()) {
                    int trap_idx = it->second;
                    int trap_bit_pos = num_gold + trap_idx;
                    if (!(mask & (1 << trap_bit_pos))) {
                        score_change += TRAP_PENALTY;
                        new_mask |= (1 << trap_bit_pos);
                    }
                }
            }

            int new_score = current.score + score_change;
            int new_dist = current.distance + 1;

            // ����DP��
            auto& next_state = dp[ny][nx][new_mask];
            bool need_update = false;

            if (new_score > next_state.score) {
                need_update = true;
            }
            else if (new_score == next_state.score && new_dist < next_state.distance) {
                need_update = true;
            }

            if (need_update) {
                next_state = DPState(new_score, new_dist);
                parent_pos[ny][nx][new_mask] = { x, y };
                parent_mask[ny][nx][new_mask] = mask;
                q.push({ ny, nx, new_mask });
            }
        }
    }

    // �ڳ��ڴ�Ѱ�����״̬
    maxScore = numeric_limits<int>::min();
    minDistanceForMaxScore = numeric_limits<int>::max();
    int best_mask = -1;

    for (int mask = 0; mask < max_mask; ++mask) {
        const auto& state = dp[exitPos.second][exitPos.first][mask];
        if (state.score > maxScore ||
            (state.score == maxScore && state.distance < minDistanceForMaxScore)) {
            maxScore = state.score;
            minDistanceForMaxScore = state.distance;
            best_mask = mask;
        }
    }

    // �ؽ�·��
    if (best_mask != -1) {
        optimalPath.clear();
        pair<int, int> curr_pos = exitPos;
        int curr_mask = best_mask;

        while (curr_pos.first != -1) {
            optimalPath.push_back(curr_pos);
            auto prev_pos = parent_pos[curr_pos.second][curr_pos.first][curr_mask];
            int prev_mask = parent_mask[curr_pos.second][curr_pos.first][curr_mask];
            curr_pos = prev_pos;
            curr_mask = prev_mask;
        }
        reverse(optimalPath.begin(), optimalPath.end());
    }
}

// ��ȡ�����������ֲ��䣩
int path_planning::getMaxScore() const {
    return maxScore;
}

// ��ȡ���·�����ȣ����ֲ��䣩
int path_planning::getMinDistance() const {
    return minDistanceForMaxScore + 1;
}

// ��ȡ����·�������ֲ��䣩
const vector<pair<int, int>>& path_planning::getOptimalPath() const {
    return optimalPath;
}

// ���ӻ�·�������ֲ��䣩
void path_planning::visualizePath() const {
    if (optimalPath.empty()) {
        cout << "δ�ҵ�·����·�������ӻ���" << endl;
        return;
    }

    cout << "\n����·������������仯 (����㵽�յ�):" << endl;

    const int GOLD_VALUE = 5;
    const int TRAP_PENALTY = -3;

    int currentScore = 0;
    cout << "λ�� 1: (" << startPos.first << ", " << startPos.second
        << ") [��� S] [��ǰ����: " << currentScore << "]" << endl;

    for (size_t i = 1; i < optimalPath.size() - 1; ++i) {
        int x = optimalPath[i].first;
        int y = optimalPath[i].second;
        char cell = maze[y][x];
        int scoreChange = 0;

        if (cell == 'G') {
            scoreChange = GOLD_VALUE;
            currentScore += scoreChange;
        }
        else if (cell == 'T') {
            scoreChange = TRAP_PENALTY;
            currentScore += scoreChange;
        }

        cout << "λ�� " << i + 1 << ": (" << x << ", " << y << ") ";

        if (scoreChange != 0) {
            cout << "[�����仯: " << (scoreChange > 0 ? "+" : "") << scoreChange
                << " | ��ǰ����: " << currentScore << "]";
        }
        else {
            cout << "[��ǰ����: " << currentScore << "]";
        }
        cout << endl;
    }

    currentScore = getMaxScore();
    cout << "λ�� " << optimalPath.size() << ": (" << exitPos.first << ", " << exitPos.second
        << ") [�յ� E] [��ǰ����: " << currentScore << "]" << endl;

    cout << "\n·���ܳ���: " << optimalPath.size() << " ����Ԫ��"
        << " | ���շ���: " << getMaxScore() << endl;
}