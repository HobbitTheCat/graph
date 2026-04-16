#include "Graph.h"
#include "partitioning/multilevel_partition.h"

#include <vector>
#include <queue>
#include <algorithm>
#include <unordered_map>

std::vector<int> find_shortest_bridge(const Graph& G, const std::vector<int>& partition, const std::vector<int>& island, int p) {
    int n = G.num_vertices;
    std::queue<int> q;
    std::vector<int> parent(n, -1);
    std::vector<int> dist(n, -1);

    std::vector<bool> in_island(n, false);
    for (int v : island) {
        q.push(v);
        dist[v] = 0;
        in_island[v] = true;
    }

    int target_v = -1;
    while (!q.empty()) {
        int u = q.front(); q.pop();

        if (partition[u] == p && !in_island[u]) {
            target_v = u;
            break;
        }

        for (int i = G.offsets[u]; i < G.offsets[u + 1]; i++) {
            int v = G.edges[i];
            if (dist[v] == -1) {
                dist[v] = dist[u] + 1;
                parent[v] = u;
                q.push(v);
            }
        }
    }

    std::vector<int> path;
    if (target_v != -1) {
        int curr = parent[target_v];
        while (curr != -1 && partition[curr] != p) {
            path.push_back(curr);
            curr = parent[curr];
        }
    }
    return path;
}

std::vector<int> find_component(const Graph& G, const std::vector<int>& partition, int start, int p, std::vector<bool>& visited) {
    std::vector<int> component;
    std::queue<int> q;
    q.push(start);
    visited[start] = true;

    while (!q.empty()) {
        int u = q.front(); q.pop();
        component.push_back(u);
        for (int i = G.offsets[u]; i < G.offsets[u + 1]; i++) {
            int v = G.edges[i];
            if (partition[v] == p && !visited[v]) {
                visited[v] = true;
                q.push(v);
            }
        }
    }
    return component;
}

void bridge_islands(const Graph& G, std::vector<int>& partition, int num_parts) {
    int n = G.num_vertices;

    for (int p = 0; p < num_parts; p++) {
        std::vector<bool> visited(n, false);
        std::vector<std::vector<int>> components;

        for (int i = 0; i < n; i++) {
            if (partition[i] == p && !visited[i]) {
                components.push_back(find_component(G, partition, i, p, visited));
            }
        }

        if (components.size() <= 1) continue;

        std::sort(components.begin(), components.end(), [&](const std::vector<int>& a, const std::vector<int>& b) {
            long long wa = 0, wb = 0;
            for (int v : a) wa += G.vertex_weights.empty() ? 1 : G.vertex_weights[v];
            for (int v : b) wb += G.vertex_weights.empty() ? 1 : G.vertex_weights[v];
            return wa > wb;
        });

        for (size_t i = 1; i < components.size(); ++i) {
            std::vector<int> path = find_shortest_bridge(G, partition, components[i], p);

            for (int v_on_path : path) {
                partition[v_on_path] = p;
            }
        }
    }
}