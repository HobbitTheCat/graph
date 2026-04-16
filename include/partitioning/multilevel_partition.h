#ifndef PROJECT_GRAPH_MULTILEVEL_PARTITION_H
#define PROJECT_GRAPH_MULTILEVEL_PARTITION_H

#include "Graph.h"

void multilevel_partition(Graph& G, std::vector<int>& partition);

void kway_partition(const Graph& g, std::vector<int> current_nodes, int num_parts, int part_offset,
                         std::vector<int>& final_parts);

void two_way_partition(const Graph& G, std::vector<int>& part);
void two_way_refinement(const Graph& G, std::vector<int>& part, int max_passes = 10);

void bridge_islands(const Graph& G, std::vector<int>& partition, int num_parts);

#endif //PROJECT_GRAPH_MULTILEVEL_PARTITION_H
