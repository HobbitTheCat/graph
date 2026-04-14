#include <vector>

#include "Graph.h"
#include "partitioning/coarsening.h"
#include "partitioning/multilevel_partition.h"

#include "visual/visualisation.h"

bool stop_condition(const Graph& current_G, int prev_vertices, int k) {
    const int MIN_VERTICES = 100;
    if (current_G.num_vertices <= MIN_VERTICES) return true;
    if (current_G.num_vertices <= 20 * k) return true;
    if (prev_vertices > 0) {
        double reduction_rate = static_cast<double>(current_G.num_vertices) / prev_vertices;
        if (reduction_rate > 0.95) return true;
    }
    return false;
}

void multilevel_partition(Graph& G, int k) {
    std::vector<CoarseLevel> hierarchy;
    Graph* current_G = &G;
    int prev_vertices = -1;

    while (!stop_condition(*current_G, prev_vertices, k)) {
        prev_vertices = current_G->num_vertices;
        CoarseLevel level;

        coarse_step(*current_G, level);
        hierarchy.push_back(std::move(level));
        current_G = &hierarchy.back().graph;
    }

    std::vector<int> part;
    initial_partition(hierarchy[hierarchy.size() - 1].graph, part);

    runVisualisation(hierarchy[hierarchy.size() - 1].graph, part);
}
