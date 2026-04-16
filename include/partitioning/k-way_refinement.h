//
// Created by basilisk on 16/04/2026.
//

#ifndef PROJECT_GRAPH_REFINEMENT_H
#define PROJECT_GRAPH_REFINEMENT_H
#include <list>
#include <unordered_map>

struct Move {
    int vertex_id;
    int to_part;
};

class KWayFM {
    const Graph& G;
    int k;
    std::vector<int>& partition;
    std::vector<long long> part_weights;

    using Bucket = std::list<int>;
    std::vector<std::vector<std::vector<Bucket>>> buckets;

    std::vector<std::unordered_map<int, Bucket::iterator>> node_iters;

    int max_degree_weight;
    const int gain_offset;

public:
    KWayFM(const Graph& G, int k, std::vector<int>& parts);

    int calculate_gain(int v, int target_part);
    void refine(int max_iterations = 10);

private:
    void remove_node_from_all_buckets(int v);

    void update_node_in_buckets(int v, const std::vector<uint8_t>& locked);
};

#endif //PROJECT_GRAPH_REFINEMENT_H
