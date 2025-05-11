// 你需要在这里参考论文实现相应代码

/*
int graph_v_of_v_idealID_DPBF_only_ec(
    vector<vector<pair<int, int>>>& v_instance_graph,
    vector<vector<pair<int, int>>>& v_generated_group_graph,
    std::unordered_set<int>& group_vertices
) {
    int N = v_generated_group_graph.size();
    int K = group_vertices.size();
    if (K == 0) return 0;

    // 为每个标签组节点 g 分配一个 [0..K-1] 的索引
    unordered_map<int, int> idx;
    idx.reserve(K);
    {
        int id = 0;
        for (int g : group_vertices) {
            idx[g] = id++;
        }
    }
    const int FULL_MASK = (1 << K) - 1;

    // dist[v][mask] = 在节点 v、覆盖标签掩码 mask 下的最小代价
    // 由于 K 较小，使用 vector<unordered_map> 来稀疏记录
    vector<unordered_map<int, int>> dist(N);
    for (int i = 0; i < N; i++) {
        dist[i].reserve(1 << min(K, 10));  // 经验性预分配
    }

    // 最小堆，元素 (cost, v, mask)
    using State = tuple<int, int, int>;
    priority_queue<State, vector<State>, greater<State>> pq;

    // 初始化：每个标签组节点 g 都可“自生”一棵仅含自身的树，cost=0，mask=1<<idx[g]
    for (auto [g, gi] : idx) {
        int m = 1 << gi;
        dist[g][m] = 0;
        pq.emplace(0, g, m);
    }

    // 主循环
    while (!pq.empty()) {
        auto [cost, v, mask] = pq.top();
        pq.pop();
        // 若此条目已被更新得更优，则跳过
        auto it = dist[v].find(mask);
        if (it == dist[v].end() || it->second < cost)
            continue;

        // 如果已经覆盖了所有标签，直接返回答案
        if (mask == FULL_MASK) {
            return cost;
        }

        // —— Tree Grow：沿所有出边扩展 (v→u)
        for (auto& e : v_generated_group_graph[v]) {
            int u = e.first, w = e.second;
            int nc = cost + w;
            auto it2 = dist[u].find(mask);
            if (it2 == dist[u].end() || nc < it2->second) {
                dist[u][mask] = nc;
                pq.emplace(nc, u, mask);
            }
        }

        // —— Tree Merge：同一节点 v 上与已有其他 mask 合并
        // 注意这里遍历 dist[v] 的当前快照（不含待插入的合并结果）
        vector<pair<int, int>> snapshot;
        snapshot.reserve(dist[v].size());
        for (auto& p : dist[v]) {
            snapshot.emplace_back(p.first, p.second);
        }
        for (auto& p : snapshot) {
            int m2 = p.first, c2 = p.second;
            if ((mask | m2) == mask) continue; // 没有新增标签
            int nm = mask | m2;
            int nc = cost + c2;
            auto it3 = dist[v].find(nm);
            if (it3 == dist[v].end() || nc < it3->second) {
                dist[v][nm] = nc;
                pq.emplace(nc, v, nm);
            }
        }
    }

    // 堆空仍未覆盖所有标签，说明无可行解
    return -1;
}
*/


int graph_v_of_v_idealID_DPBF_only_ec(
    vector<vector<pair<int, int>>>& v_instance_graph,
    vector<vector<pair<int, int>>>& v_generated_group_graph,
    std::unordered_set<int>& group_vertices
) 
{
    // 常量 V K PMASK
    int V = v_instance_graph.size();
    int K = group_vertices.size();
    const long long PMASK = (1 << K) - 1;

    // 准备工作 1：建立一个单节点-关键词掩码的映射
    std::unordered_map<int, long long> gmap;
    for (int g : group_vertices) {
        for (auto& virtual_edge : v_generated_group_graph[g]) {
            int v = virtual_edge.first;
            if (gmap.find(v) == gmap.end())
                gmap[v] = 0;
            gmap[v] += 1 << (g - V);
        }
    }

    // 准备工作 2：建立优先队列，并初始化
    // 准备工作 3：建立 T->cost 记忆
    using T = tuple<int, int, int>;
    std::priority_queue<T, vector<T>, greater<T>> pq;
    std::vector<std::unordered_map<long long, int>> dp(V);
    for (int v = 0; v < V; ++v) {
        if (gmap.find(v) != gmap.end()) {
            pq.emplace(0, v, gmap[v]);
            dp[v][gmap[v]] = 0;
        }
    }

    // 主循环
    while (!pq.empty()) {
        auto [cost, v, pmask] = pq.top();
        pq.pop();
        if (pmask == PMASK) return cost;
        auto it = dp[v].find(pmask);
        if (it->second < cost)
            continue;

        // Tree Grow
        for (auto& e : v_instance_graph[v]) {
            int u = e.first, w = e.second;
            int new_cost = cost + w;
            long long new_pmask = (gmap.find(u) == gmap.end()) ? pmask : pmask | gmap[u];
            auto it2 = dp[u].find(new_pmask);
            if (it2 == dp[u].end() || new_cost < it2->second) {
                dp[u][new_pmask] = new_cost;
                pq.emplace(new_cost, u, new_pmask);
            }           
        }

        // Tree Merge
        for (auto& T : dp[v]) {
            long long p2mask = T.first;
            int cost2 = T.second;
            if (pmask & p2mask) continue; // really?
            long long new_pmask = pmask | p2mask;
            long long new_cost = cost + cost2;
            auto it3 = dp[v].find(new_pmask);
            if (it3 == dp[v].end() || new_cost < it3->second) {
                dp[v][new_pmask] = new_cost;
                pq.emplace(new_cost, v, new_pmask);
            }
        }
    }

    // 无可行解
    return -1;
}