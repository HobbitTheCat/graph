#include <vector>
#include <numeric>
#include <algorithm>
#include <cassert>
#include <climits>


#include "Graph.h"
#include "partitioning/multilevel_partition.h"


struct DLL {
    std::vector<int> prev, next;
    int head;

    void init(int n) {
        prev.assign(n, -1);
        next.assign(n, -1);
        head = -1;
    }

    void push_front(int v) {
        prev[v] = -1;
        next[v] = head;
        if (head != -1) prev[head] = v;
        head = v;
    }

    void remove(int v) {
        if (prev[v] != -1) next[prev[v]] = next[v];
        else head = next[v];
        if (next[v] != -1) prev[next[v]] = prev[v];
        prev[v] = next[v] = -1;
    }

    bool empty() const { return head == -1; }
};

struct BucketArray {
    int max_gain;
    int best;
    std::vector<DLL> buckets;
    int n;

    void init(int n_vertices, int mg) {
        n = n_vertices;
        max_gain = mg;
        best = - mg - 1;
        int sz = 2 * mg + 1;
        buckets.resize(sz);
        for (auto& b : buckets) b.init(n_vertices);
    }

    int to_idx(int gain) const {return gain + max_gain;}

    void insert(int v, int g) {
        assert(g >= -max_gain && g <= max_gain);
        buckets[to_idx(g)].push_front(v);
        if (g > best) best = g;
    }

    void remove(int v, int g) {
        buckets[to_idx(g)].remove(v);
        if (g == best) {
            while (best >= -max_gain && buckets[to_idx(best)].empty()) --best;
        }
    }

    void update(int v, int old_g, int new_g) {
        remove(v, old_g);
        insert(v, new_g);
    }

    int pick_best() const {
        if (best < -max_gain) return -1;
        return buckets[to_idx(best)].head;
    }

    bool empty() const {return best < -max_gain;}
};

static int compute_gain(const Graph& G, int v, const std::vector<int>& part) {
    int p = part[v];
    int g = 0;
    for (int i = G.offsets[v]; i < G.offsets[v + 1]; ++i) {
        int neighbor = G.edges[i];
        int w = G.edge_weights[i];
        if (part[neighbor] == p) g -= w;
        else g+= w;
    }
    return g;
}

static int compute_cut(const Graph& G, const std::vector<int>& part) {
    int cut = 0;
    for (int v = 0; v < G.num_vertices; v++)
        for (int i = G.offsets[v]; i < G.offsets[v + 1]; i++)
            if (part[G.edges[i]] != part[v])
                cut += G.edge_weights[i];
    return cut / 2;
}

static int fm_pass (const Graph& G, std::vector<int>& part, double balance_factor) {
    int n = G.num_vertices;

    int max_gain = 0;
    int total_weight = 0;
    std::vector<int> part_weight(2, 0);

    for (int v = 0; v < n; v++) {
        int vg = 0;
        for (int i = G.offsets[v]; i < G.offsets[v + 1]; i++) vg += G.edge_weights[i];
        max_gain = std::max(max_gain, vg);

        part_weight[part[v]] += G.vertex_weights[v];
        total_weight += G.vertex_weights[v];
    }
    if (max_gain == 0) max_gain = 1;

    int max_allowed = (int)(total_weight * (0.5 + balance_factor / 2.0));
    int min_allowed = total_weight - max_allowed;

    std::vector<int> gain(n);
    for (int v = 0; v < n; v++) gain[v] = compute_gain(G, v, part);

    BucketArray buckets[2];
    buckets[0].init(n, max_gain);
    buckets[1].init(n, max_gain);

    std::vector<bool> locked(n, false);
    for (int v = 0; v < n; v++) buckets[part[v]].insert(v, gain[v]);

    struct Move { int v; int from; int gain_val; };
    std::vector<Move> history;
    history.reserve(n);

    int current_cut_delta = 0;
    int best_delta = 0;
    int best_step = -1;

    for (int step = 0; step < n; step++) {
        int chosen_part = -1;
        int best_g = INT_MIN;

        // for (int p = 0; p <= 1; p++) {
        //     if (buckets[p].empty()) continue;
        //
        //     int cand = buckets[p].pick_best();
        //     if (cand == -1) continue;
        //
        //     int new_pw0 = part_weight[0] + (p == 1 ? G.vertex_weights[cand] : -G.vertex_weights[cand]);
        //     int new_pw1 = part_weight[1] + (p == 0 ? G.vertex_weights[cand] : -G.vertex_weights[cand]);
        //
        //     if (new_pw0 < min_allowed || new_pw0 > max_allowed) continue;
        //     if (new_pw1 < min_allowed || new_pw1 > max_allowed) continue;
        //
        //     if (gain[cand] > best_g) {
        //         best_g = gain[cand];
        //         chosen_part = p;
        //     }
        // }
        //
        // if (chosen_part == -1) {
        //     for (int p = 0; p <= 1; p++)
        //         if (!buckets[p].empty()) { chosen_part = p; break; }
        //     if (chosen_part == -1) break;
        // }
        for (int p = 0; p <= 1; p++) {
            if (buckets[p].empty()) continue;
            int cand = buckets[p].pick_best();
            if (cand == -1) continue;

            int v_w = G.vertex_weights[cand];
            int new_pw_from = part_weight[p] - v_w;
            int new_pt_to = part_weight[1 - p] + v_w;

            if (new_pw_from >= min_allowed && new_pt_to <= max_allowed) {
                bool has_neighbor_in_target = false;
                for (int i = G.offsets[cand]; i < G.offsets[cand + 1]; i++)
                    if (part[G.edges[i]] == (1 - p)) {has_neighbor_in_target = true; break;}

                if (has_neighbor_in_target && gain[cand] > best_g) {
                    best_g = gain[cand];
                    chosen_part = p;
                }
            }
        }

        if (chosen_part == -1) {
            if (!buckets[0].empty() && !buckets[1].empty()) {
                chosen_part = (buckets[0].best > buckets[1].best) ? 0 : 1;
            } else {
                chosen_part = buckets[0].empty() ? 1 : 0;
            }
        }

        int v = buckets[chosen_part].pick_best();
        if (v == -1) break;

        int from = part[v];
        int to = 1 - from;

        buckets[from].remove(v, gain[v]);
        locked[v] = true;
        part[v] = to;

        part_weight[from] -= G.vertex_weights[v];
        part_weight[to] += G.vertex_weights[v];
        current_cut_delta -= gain[v];
        history.push_back({v, from, gain[v]});

        if (part_weight[0] >= min_allowed && part_weight[0] <= max_allowed)
            if (current_cut_delta < best_delta) {
                best_delta = current_cut_delta;
                best_step = (int)history.size() - 1;
            }

        for (int i = G.offsets[v]; i < G.offsets[v + 1]; i++) {
            int neighbor = G.edges[i];
            int w = G.edge_weights[i];
            if (locked[neighbor]) continue;

            int old_g = gain[neighbor];
            if (part[neighbor] == from) gain[neighbor] += 2 * w;
            else gain[neighbor] -= 2 * w;

            int new_g = std::max(-max_gain, std::min(max_gain, gain[neighbor]));
            gain[neighbor] = new_g;
            buckets[part[neighbor]].update(neighbor, old_g, new_g);
        }
    }

    if (best_step < (int)history.size() - 1) {
        for (int i = (int)history.size() - 1; i > best_step; i--)
            part[history[i].v] = history[i].from;
    }

    return -best_delta;
}

void two_way_refinement(const Graph& G, std::vector<int>& part, int max_passes) {
    const double balance_factor = 0.1;

    for (int pass = 0; pass < max_passes || max_passes < 0; pass++) {
        int improvement = fm_pass(G, part, balance_factor);
        if (improvement <= 0) break;
    }
}