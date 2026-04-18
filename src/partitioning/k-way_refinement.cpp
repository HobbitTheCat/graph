#include <vector>
#include <cstdint>
#include <limits>

#include "Graph.h"
#include "partitioning/k-way_refinement.h"

KWayFM::KWayFM(const Graph &G, int k, std::vector<int> &parts) :
    G(G), k(k), partition(parts), max_degree_weight(0), gain_offset(1000000) {
    this->part_weights.resize(k, 0);
    node_iters.resize(G.num_vertices);

    for (int i = 0; i < G.num_vertices; i++) {
        int w = G.vertex_weights.empty() ? 1 : G.vertex_weights[i];
        part_weights[partition[i]] += w;
    }

    int max_w = 0;
    for (int i = 0; i < G.num_vertices; i++) {
        int current_deg_w = 0;
        for (int j = G.offsets[i]; j < G.offsets[i + 1]; j++) {
            current_deg_w += G.edge_weights.empty() ? 1 : G.edge_weights[j];
        }
        max_w = std::max(max_w, current_deg_w);
    }

    this->buckets.resize(k, std::vector<std::vector<Bucket>>(k, std::vector<Bucket>(2 * max_w + 1)));
}

int KWayFM::calculate_gain(int v, int target_part) {
    int gain = 0;
    int current_part = partition[v];

    for (int i = this->G.offsets[v]; i < this->G.offsets[v + 1]; i++) {
        int neighbor = this->G.edges[i];
        int edge_w = this->G.edge_weights.empty() ? 1 : this->G.edge_weights[i];

        if (partition[neighbor] == target_part) gain += edge_w;
        else if (partition[neighbor] == current_part) gain -= edge_w;
    }
    return gain;
}

void KWayFM::refine(int max_iterations) {
    for (int iter = 0; iter < max_iterations; iter++) {
        std::vector<uint8_t> locked(this->G.num_vertices, 0);
        std::vector<std::pair<int, int>> move_history;

        for (int v = 0; v < this->G.num_vertices; v++) {
            update_node_in_buckets(v, locked);
        }

        uint8_t moved = 0;
        while (true) {
            int best_v = -1, best_to_part = -1, max_gain = -std::numeric_limits<int>::max();

            for (int i = 0; i < this->k; i++) {
                for (int j = 0; j < this->k; j++) {
                    if (i == j) continue;

                    auto& current_bucket_set = buckets[i][j];
                    for (int g_idx = current_bucket_set.size() - 1; g_idx >= 0; g_idx--) {
                        if (!current_bucket_set[g_idx].empty()) {
                            int current_gain = g_idx - (current_bucket_set.size() / 2);
                            if (current_gain > max_gain) {
                                int v_idx = current_bucket_set[g_idx].front();
                                int v_w = this->G.vertex_weights.empty() ? 1 : this->G.vertex_weights[v_idx];
                                max_gain = current_gain;
                                best_v = v_idx;
                                best_to_part = j;
                            }
                            break;
                        }
                    }
                }
            }

            if (best_v == -1 || max_gain < 0) break;

            int old_part = partition[best_v];
            move_history.push_back({best_v, old_part});

            int v_w = this->G.vertex_weights.empty() ? 1 : this->G.vertex_weights[best_v];
            part_weights[old_part] -= v_w;
            part_weights[best_to_part] += v_w;
            partition[best_v] = best_to_part;
            locked[best_v] = 1;

            remove_node_from_all_buckets(best_v);

            for (int i = this->G.offsets[best_v]; i < this->G.offsets[best_v + 1]; i++) {
                int neighbor = this->G.edges[i];
                if (locked[neighbor] == 0) {
                    update_node_in_buckets(neighbor, locked);
                }
            }
            moved = 1;
        }
        if (moved == 0) break;
        // There should be a rollback logic here to revert to a better state of the iteration
    }
}

void KWayFM::remove_node_from_all_buckets(int v) {
    for (auto& [to_part, it] : this->node_iters[v]) {
        int from_part = partition[v];
        // We need to know the gain to find the bucket.
        // In a real-world implementation, it's better to store the gain inside an iterator or a structure.
    }
    node_iters[v].clear();
}

void KWayFM::update_node_in_buckets(int v, const std::vector<uint8_t> &locked) {
    if (locked[v] == 1) return;

    remove_node_from_all_buckets(v);
    int from_part = partition[v];

    std::unordered_map<int, int> neighbor_parts;
    for (int i = this->G.offsets[v]; i < this->G.offsets[v + 1]; i++) {
        int p = partition[this->G.edges[i]];
        if (p != from_part) neighbor_parts[p]++;
    }

    for (auto const& [to_part, count] : neighbor_parts) {
        int gain = calculate_gain(v, to_part);
        int gain_idx = gain + (buckets[from_part][to_part].size() / 2);

        buckets[from_part][to_part][gain_idx].push_front(v);
        node_iters[v][to_part] = buckets[from_part][to_part][gain_idx].begin();
    }
}
