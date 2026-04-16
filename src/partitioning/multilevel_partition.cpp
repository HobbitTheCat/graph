#include <vector>
#include <cstdio>
#include <ctime>

#include "Graph.h"
#include "partitioning/coarsening.h"
#include "partitioning/multilevel_partition.h"

#include "visual/visualisation.h"

std::vector<int> project_partition(int num_vertices, const std::vector<int>& coarse_partition, const std::vector<int>& cmap) {
    std::vector<int> fine_partition(num_vertices);
    for (int v = 0; v < num_vertices; v++) {
        int super_v = cmap[v];
        fine_partition[v] = coarse_partition[super_v];
    }
    return fine_partition;
}

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

void multilevel_partition(Graph& G, std::vector<int>& partition) {
    std::vector<CoarseLevel> hierarchy;
    Graph* current_G = &G;
    int prev_vertices = -1;

    while (!stop_condition(*current_G, prev_vertices, 2)) {
        prev_vertices = current_G->num_vertices;
        CoarseLevel level;

        coarse_step(*current_G, level);
        hierarchy.push_back(std::move(level));
        current_G = &hierarchy.back().graph;

        std::cout << "Level " << hierarchy.size()
          << ": Vertices = " << current_G->num_vertices
          << ", Edges = " << current_G->edges.size() << std::endl;
    }

    int num_vertices = current_G->num_vertices;
    partition.resize(num_vertices, 0);
    two_way_partition(G, partition);
    two_way_refinement(G, partition);

    for (int i = (int)hierarchy.size() - 1; i >= 0; i--) {
        const Graph& fine_G = (i == 0) ? G : hierarchy[i - 1].graph;
        const std::vector<int>& cmap = hierarchy[i].cmap;

        std::vector<int> next_partition = project_partition(fine_G.num_vertices, partition, cmap);
        two_way_refinement(fine_G, next_partition);
        partition = std::move(next_partition);
    }

    bridge_islands(G, partition, 2);
}
