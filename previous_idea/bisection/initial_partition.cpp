#include <queue>
#include <algorithm>
#include <vector>

#include "Graph.h"
#include "partitioning/multilevel_partition.h"

int findFarthestVertex(const Graph& G, int start_node, const std::vector<int>& mask) {
    std::queue<int> q;
    q.push(start_node);

    std::vector<int> dist(G.num_vertices, -1);
    dist[start_node] = 0;

    int farthest_node = start_node;

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        farthest_node = u;

        for (int i = G.offsets[u]; i < G.offsets[u + 1]; i++) {
            int v = G.edges[i];
            if (mask[v] != -1 && dist[v] == -1) {
                dist[v] = dist[u] + 1;
                q.push(v);
            }
        }
    }
    return farthest_node;
}

void grow(const Graph& G, std::vector<int>& part, const std::vector<int>& active_mask) {
    int n = G.num_vertices;
    part.assign(n, -1);

    int root = -1;
    for (int i = 0; i < n; i++) if (active_mask[i] != -1) { root = i; break; }

    int seed1 = findFarthestVertex(G, root, active_mask);
    int seed2 = findFarthestVertex(G, seed1, active_mask);

    std::queue<int> q1, q2;
    q1.push(seed1);
    part[seed1] = 0;
    q2.push(seed2);
    part[seed2] = 1;

    while (!q1.empty() || !q2.empty()) {
        int size1 = q1.size();
        while (size1 --) {
            int u = q1.front(); q1.pop();
            for (int i = G.offsets[u]; i < G.offsets[u + 1]; i++) {
                int v = G.edges[i];
                if (active_mask[v] != -1 && part[v] == -1) {
                    part[v] = 0;
                    q1.push(v);
                }
            }
        }

        int size2 = q2.size();
        while (size2--) {
            int u = q2.front(); q2.pop();
            for (int i = G.offsets[u]; i < G.offsets[u + 1]; i++) {
                int v = G.edges[i];
                if (active_mask[v] != -1 && part[v] == -1) {
                    part[v] = 1;
                    q2.push(v);
                }
            }
        }
    }
}

void recursive_partition(const Graph& G, std::vector<int>& final_parts,
    std::vector<int>& current_mask, int num_parts, int offset) {
    // final_parts.assign(G.num_vertices, -1);
    if (num_parts <= 1) {
        for (int i = 0; i < G.num_vertices; i++) {
            if (current_mask[i] != -1) final_parts[i] = offset;
        }
        return;
    }

    std::vector<int> part;
    grow(G, part, current_mask);

    std::vector<int> mask_left(G.num_vertices, -1);
    std::vector<int> mask_right(G.num_vertices, -1);

    for (int i = 0; i < G.num_vertices; i++) {
        if (part[i] == 0) mask_left[i] = 1;
        else if (part[i] == 1) mask_right[i] = 1;
    }

    recursive_partition(G, final_parts, mask_left, num_parts / 2, offset);
    recursive_partition(G, final_parts, mask_right, num_parts - (num_parts / 2), offset + num_parts / 2);
}

