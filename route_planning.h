#include "path_planning.h"
#include <algorithm>
#include <climits>
#include <queue>

// 从迷宫中找出起点、终点、金币和陷阱的位置（保持不变）
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

// 从CSV文件读取迷宫地图（保持不变）
bool path_planning::readMazeFromCSV(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "无法打开文件: " << filename << endl;
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
        cerr << "迷宫不能为空。" << endl;
        return false;
    }

    size = maze.size();
    return true;
}

// 默认构造函数（保持不变）
path_planning::path_planning() : size(0), maxScore(numeric_limits<int>::min()),
minDistanceForMaxScore(numeric_limits<int>::max()) {}

// 从CSV文件加载迷宫的构造函数（保持不变）
path_planning::path_planning(const string& filename) : maxScore(numeric_limits<int>::min()),
minDistanceForMaxScore(numeric_limits<int>::max()) {
    if (!readMazeFromCSV(filename)) {
        throw invalid_argument("无法从CSV文件加载迷宫。");
    }
    findStartExitAndFeatures();
}

// 从二维字符数组加载迷宫的构造函数（保持不变）
path_planning::path_planning(const vector<vector<char>>& m) : maze(m), maxScore(numeric_limits<int>::min()),
minDistanceForMaxScore(numeric_limits<int>::max()) {
    if (m.empty() || m[0].empty()) {
        throw invalid_argument("迷宫不能为空。");
    }
    size = m.size();
    findStartExitAndFeatures();
}

// 从CSV文件加载迷宫（保持不变）
bool path_planning::loadMazeFromCSV(const string& filename) {
    if (readMazeFromCSV(filename)) {
        findStartExitAndFeatures();
        return true;
    }
    return false;
}

// 动态规划求解迷宫（优化版）
void path_planning::solve() {
    int num_gold = gold_locations.size();
    int num_traps = trap_locations.size();
    int total_features = num_gold + num_traps;
    int max_mask = 1 << total_features;

    // 使用更紧凑的数据结构表示DP状态
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

    // 使用队列进行BFS，按距离处理状态
    queue<tuple<int, int, int>> q;

    // 初始化父节点信息
    parent_pos.assign(size, vector<vector<pair<int, int>>>(size, vector<pair<int, int>>(max_mask, { -1, -1 })));
    parent_mask.assign(size, vector<vector<int>>(size, vector<int>(max_mask, -1)));

    // 初始状态：起点，mask为0，分数0，距离0
    dp[startPos.second][startPos.first][0] = DPState(0, 0);
    q.push({ startPos.second, startPos.first, 0 });

    // 四个移动方向
    const int dx[] = { 0, 0, 1, -1 };
    const int dy[] = { 1, -1, 0, 0 };

    // 按距离进行BFS
    while (!q.empty()) {
        auto current_tuple = q.front();
        q.pop();
        int y = get<0>(current_tuple);
        int x = get<1>(current_tuple);
        int mask = get<2>(current_tuple);
        auto& current = dp[y][x][mask];

        // 如果当前状态不是最优的，跳过（剪枝）
        if (current.score == numeric_limits<int>::min()) continue;

        // 如果已经到达终点，不需要继续扩展（剪枝）
        if (y == exitPos.second && x == exitPos.first) continue;

        // 检查所有可能的移动
        for (int dir = 0; dir < 4; ++dir) {
            int nx = x + dx[dir];
            int ny = y + dy[dir];

            // 检查边界和墙壁
            if (nx < 0 || nx >= size || ny < 0 || ny >= size || maze[ny][nx] == '#') {
                continue;
            }

            int new_mask = mask;
            int score_change = 0;
            char cell = maze[ny][nx];

            // 处理金币和陷阱
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

            // 更新DP表
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

    // 在出口处寻找最佳状态
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

    // 重建路径
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

// 获取最大分数（保持不变）
int path_planning::getMaxScore() const {
    return maxScore;
}

// 获取最短路径长度（保持不变）
int path_planning::getMinDistance() const {
    return minDistanceForMaxScore + 1;
}

// 获取最优路径（保持不变）
const vector<pair<int, int>>& path_planning::getOptimalPath() const {
    return optimalPath;
}

// 可视化路径（保持不变）
void path_planning::visualizePath() const {
    if (optimalPath.empty()) {
        cout << "未找到路径或路径不可视化。" << endl;
        return;
    }

    cout << "\n最优路径坐标与分数变化 (从起点到终点):" << endl;

    const int GOLD_VALUE = 5;
    const int TRAP_PENALTY = -3;

    int currentScore = 0;
    cout << "位置 1: (" << startPos.first << ", " << startPos.second
        << ") [起点 S] [当前分数: " << currentScore << "]" << endl;

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

        cout << "位置 " << i + 1 << ": (" << x << ", " << y << ") ";

        if (scoreChange != 0) {
            cout << "[分数变化: " << (scoreChange > 0 ? "+" : "") << scoreChange
                << " | 当前分数: " << currentScore << "]";
        }
        else {
            cout << "[当前分数: " << currentScore << "]";
        }
        cout << endl;
    }

    currentScore = getMaxScore();
    cout << "位置 " << optimalPath.size() << ": (" << exitPos.first << ", " << exitPos.second
        << ") [终点 E] [当前分数: " << currentScore << "]" << endl;

    cout << "\n路径总长度: " << optimalPath.size() << " 个单元格"
        << " | 最终分数: " << getMaxScore() << endl;
}