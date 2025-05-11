// 你需要在这里参考论文实现相应代码

int graph_v_of_v_idealID_DPBF_only_ec(
    vector<vector<pair<int, int>>>& v_instance_graph,        // 不包含分组的原始图邻接表，pair中第一项是下一节点，第二项为权重
    vector<vector<pair<int, int>>>& v_generated_group_graph, // 包含若干虚拟节点，表示一个分组，每个邻接项都表示一个在这个分组的节点
    std::unordered_set<int>& group_vertices                  // 所有组代表的虚拟节点的集合
) 
{
    // 常量 V K PMASK
    int V = v_instance_graph.size();      // V 表示节点总数
    int K = group_vertices.size();        // K 表示组数
    const long long PMASK = (1 << K) - 1; // 使用位操作，每一位表示书中是否包含某一个关键词，当掩码等于 PMASK 时即全包含

    // 准备工作 1：建立一个单节点-关键词掩码的映射
    std::unordered_map<int, long long> gmap;
    for (int g : group_vertices) {
        for (auto& virtual_edge : v_generated_group_graph[g]) {
            int v = virtual_edge.first;
            if (gmap.find(v) == gmap.end())
                gmap[v] = 0;
            gmap[v] += 1 << (g - V);                             // 把所有关键词累加即可
        }
    }

    // 准备工作 2：建立优先队列，并初始化
    // 准备工作 3：建立 T->cost 记忆
    using T = tuple<int, int, long long>;                  // 使用一个三元组来表示树的状态，三个元素分别为cost（当前树的代价），v（当前树的根节点），pmask（当前树包含哪些关键词）
    std::priority_queue<T, vector<T>, greater<T>> pq;
    std::vector<std::unordered_map<long long, int>> dp(V); // dp[v][pmask] 数据结构用来记录以 v 为根节点，包含的关键词可用 pmask 表示的所有树中最优的一棵的代价
    for (int v = 0; v < V; ++v) {
        if (gmap.find(v) != gmap.end()) {
            pq.emplace(0, v, gmap[v]);                     // 对于每一个包含关键词的节点都可以作为根节点形成一棵代价为 0 的树
            dp[v][gmap[v]] = 0;
        }
    }

    // 主循环
    while (!pq.empty()) {
        auto [cost, v, pmask] = pq.top();
        pq.pop();                          // 出队目前代价最小的树为
        if (pmask == PMASK) return cost;   // 已经不可能有其他树，能更小的代价包含所有关键词
        auto it = dp[v].find(pmask);
        if (it->second < cost)
            continue;

        // Tree Grow -- 具体步骤完全按照伪代码
        for (auto& e : v_instance_graph[v]) {
            int u = e.first, w = e.second;
            int new_cost = cost + w;                            // 由于回走头路或者循环路一定会比之前差，所以自然排除这种可能不需额外判断
            long long new_pmask = (gmap.find(u) == gmap.end()) ? pmask : pmask | gmap[u]; 
            auto it2 = dp[u].find(new_pmask);
            if (it2 == dp[u].end() || new_cost < it2->second) { // 若新的关键词组合没出现过，或者比已经出现的关键词组合的树更优，则更新
                dp[u][new_pmask] = new_cost;
                pq.emplace(new_cost, u, new_pmask);
            }           
        }

        // Tree Merge -- 具体步骤完全按照伪代码
        for (auto& T : dp[v]) {
            long long p2mask = T.first;
            int cost2 = T.second;
            if (pmask & p2mask) continue;                       // 仅选取两棵没有任何交集的树，防止循环路的出现
            long long new_pmask = pmask | p2mask;
            long long new_cost = cost + cost2;
            auto it3 = dp[v].find(new_pmask);
            if (it3 == dp[v].end() || new_cost < it3->second) { // 若新的关键词组合没出现过，或者比已经出现的关键词组合的树更优，则更新
                dp[v][new_pmask] = new_cost;
                pq.emplace(new_cost, v, new_pmask);
            }
        }
    }

    // 从所有根节点出发都没法完成覆盖所有关键词，无可行解
    return -1;
}