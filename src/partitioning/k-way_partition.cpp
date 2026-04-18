#include <queue>
#include <set>
#include <algorithm>
#include <unordered_map>
#include <vector>

#include "Graph.h"
#include "partitioning/multilevel_partition.h"

struct PartitionResult {
    std::vector<int> partA;
    std::vector<int> partB;
};

PartitionResult gggp_bisect_weighted(const Graph& graph,
                                     const std::vector<int>& current_indices,
                                     double target_weight) {
    if (current_indices.empty()) return {};

    std::unordered_map<int, bool> isInSubset;
    for (int v_idx : current_indices) isInSubset[v_idx] = true;

    // Finding a seed vertex — we choose the vertex with the minimum degree
    int seed = current_indices[0];
    int min_deg = 1e9;
    for (int v_idx : current_indices) {
        int deg = graph.offsets[v_idx + 1] - graph.offsets[v_idx];
        if (deg < min_deg) { min_deg = deg; seed = v_idx; }
    }

    std::vector<int> partA;
    std::vector<bool> visited(graph.num_vertices, false);
    double current_weight = 0;

    std::priority_queue<std::pair<int, int>> pq;
    pq.push({0, seed});
    visited[seed] = true;

    while (!pq.empty() && current_weight < target_weight) {
        int u = pq.top().second;
        pq.pop();

        partA.push_back(u);
        current_weight += graph.vertex_weights.empty() ? 1 : graph.vertex_weights[u];

        for (int i = graph.offsets[u]; i < graph.offsets[u + 1]; ++i) {
            int v = graph.edges[i];
            if (isInSubset.count(v) && !visited[v]) {
                visited[v] = true;
                // Считаем Gain (притяжение к partA)
                int gain = 0;
                for (int j = graph.offsets[v]; j < graph.offsets[v + 1]; ++j) {
                    if (visited[graph.edges[j]]) gain++;
                }
                pq.push({gain, v});
            }
        }

        // Processing of disconnected components
        if (pq.empty() && current_weight < target_weight) {
            for (int v_idx : current_indices) {
                if (!visited[v_idx]) {
                    pq.push({0, v_idx});
                    visited[v_idx] = true;
                    break;
                }
            }
        }
    }

    // Creating partB from the remaining
    std::vector<int> partB;
    std::vector<bool> inA(graph.num_vertices, false);
    for (int v : partA) inA[v] = true;
    for (int v : current_indices) {
        if (!inA[v]) partB.push_back(v);
    }

    return {partA, partB};
}

void kway_partition(const Graph& g,
                         std::vector<int> current_nodes,
                         int num_parts,
                         int part_offset,
                         std::vector<int>& final_parts)
{
    if (num_parts <= 1 || current_nodes.empty()) {
        for (int v_idx : current_nodes) {
            final_parts[v_idx] = part_offset;
        }
        return;
    }

    int parts_left = num_parts / 2;
    int parts_right = num_parts - parts_left;

    double total_weight = 0;
    for (int v : current_nodes) {
        total_weight += g.vertex_weights.empty() ? 1.0 : g.vertex_weights[v];
    }

    double target_weight = total_weight * ((double)parts_left / num_parts);

    PartitionResult res = gggp_bisect_weighted(g, current_nodes, target_weight);

    kway_partition(g, res.partA, parts_left, part_offset, final_parts);
    kway_partition(g, res.partB, parts_right, part_offset + parts_left, final_parts);
}