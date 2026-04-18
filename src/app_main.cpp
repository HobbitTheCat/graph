#include <iostream>
#include <cstdio>
#include <numeric>
#include <metis.h>

#include <vector>
#include "Graph.h"
#include "../include/partitioning/coarsening.h"
#include "../include/partitioning/multilevel_partition.h"
#include "visual/visualisation.h"
#include "import/load_dimacs.h"

#include "partitioning/coarsening.h"
#include "partitioning/multilevel_partition.h"

void metis_partition(Graph& g, int num_parts, std::vector<int>& part) {
    part.assign(g.num_vertices, 0);

    idx_t nvtxs = static_cast<idx_t>(g.num_vertices);
    idx_t ncon = 1;
    idx_t nparts = static_cast<idx_t>(num_parts);
    idx_t objval;

    idx_t options[METIS_NOPTIONS];
    METIS_SetDefaultOptions(options);

    // options[METIS_OPTION_CONTIG] = 1;
    options[METIS_OPTION_UFACTOR] = 100;
    options[METIS_OPTION_NITER] = 10;

    int status = METIS_PartGraphKway(
        &nvtxs,
        &ncon,
        reinterpret_cast<idx_t*>(g.offsets.data()),
        reinterpret_cast<idx_t*>(g.edges.data()),
        g.vertex_weights.empty() ? NULL : reinterpret_cast<idx_t*>(g.vertex_weights.data()),
        NULL,
        g.edge_weights.empty() ? NULL : reinterpret_cast<idx_t*>(g.edge_weights.data()),
        &nparts,
        NULL,
        NULL,
        options,
        &objval,
        reinterpret_cast<idx_t*>(part.data())
    );

    if (status != METIS_OK) {
        throw std::runtime_error("METIS partition failed");
    }
}

long long calculateCutWeight(const Graph& graph, const std::vector<int>& partition) {
    long long total_cut_weight = 0;
    int part1_weight = 0, part2_weight = 0;

    for (int u = 0; u < graph.num_vertices; ++u) {
        int part_u = partition[u];
        if (part_u == 0) part1_weight+= graph.vertex_weights[u];
        else part2_weight+= graph.vertex_weights[u];

        for (int i = graph.offsets[u]; i < graph.offsets[u + 1]; ++i) {
            int v = graph.edges[i];
            int part_v = partition[v];

            if (part_u != part_v) {
                if (!graph.edge_weights.empty()) {
                    total_cut_weight += graph.edge_weights[i];
                } else {
                    total_cut_weight += 1;
                }
            }
        }
    }

    std::cout << "Cut weight: " << total_cut_weight / 2 << ", Part 1 weight: " << part1_weight << ", Part 2 weight: " << part2_weight << std::endl;
    return total_cut_weight / 2;
}

int main() {
    setenv("__NV_PRIME_RENDER_OFFLOAD", "1", 1);
    setenv("__GLX_VENDOR_LIBRARY_NAME", "nvidia", 1);

    Graph hGraph;
    try {
        // hGraph = load_txt("graph/p2p-Gnutella30.txt");
        hGraph = load_dimacs("jean.col");
        std::cout << "Vertices: " << hGraph.num_vertices << ", Edges: " << hGraph.edges.size() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    Graph graph;
    clean_disconnected_parts(hGraph, graph);

    // int V = 8;
    // Graph hGraph(V, 24);
    // hGraph.offsets = {0, 3, 6, 9, 12, 15, 18, 21, 24};
    // hGraph.edges = {1,3,5, 0,2,6, 1,3,7, 0,2,4, 3,5,7, 0,4,6, 1,5,7, 2,4,6};
    // hGraph.edge_weights.assign(hGraph.edges.size(), 1);
    // hGraph.vertex_weights.assign(V, 3);


    // std::cout << "Vertices: " << level_2.graph.num_vertices << ", Edges: " << level_2.graph.edges.size() << std::endl;

    // std::cout << "Offsets: ";
    // for (int v : cmap) std::cout << v << " ";
    // std::cout << std::endl;

    std::vector<int> parts;
    std::vector<int> parts_2;

    multilevel_partition(graph, parts);
    metis_partition(graph, 2, parts_2);

    std::cout << "Partition 1:" << std::endl;
    long long cut_1 = calculateCutWeight(graph, parts);
    std::cout << "Partition 2:" << std::endl;
    long long cut_2 = calculateCutWeight(graph, parts_2);

    // runVisualisation(graph, parts_2);

    return 0;
}

void k_way_test() {
    Graph hGraph;
    try {
        hGraph = load_txt("graph/CA-GrQc.txt");
        // hGraph = load_dimacs("jean.col");
        std::cout << "Vertices: " << hGraph.num_vertices << ", Edges: " << hGraph.edges.size() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    Graph graph;
    clean_disconnected_parts(hGraph, graph);

    std::vector<int> parts(graph.num_vertices, -1);
    std::vector<int> parts_2;
    std::vector<int> current_nodes(graph.num_vertices);
    for (int i = 0; i < graph.num_vertices; ++i) current_nodes[i] = i;
    kway_partition(graph, current_nodes, 4, 0, parts);

    metis_partition(graph, 4, parts_2);

    runVisualisation(graph, parts);
}