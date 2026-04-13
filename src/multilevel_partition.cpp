#include <vector>

#include "Graph.h"

struct CoarseLevel {
    Graph G;
    std::vector<int> cmap;
};

bool stop_condition(Graph& G) {
    return true;
}

// void multilevel_partition(Graph& G) {
//     std::vector<CoarseLevel> levels;
//     Graph current_G = G;
//
//     while (!stop_condition(current_G)) {
//         CoarseLevel level;
//         level.cmap = generate_matching(current_G);
//         level.G = compress_graph(current_G, level.cmap);
//         hierarchy.push_back(level);
//         current_G = level.G;
//     }
//
//     std::vector<int> partition = initial_partition(current_G);
//
//     for (int i = hierarchy.size() - 1; i >= 0; --i) {
//         std::vector<int> next_partition(hierarchy[i].G);
//     }
// }
