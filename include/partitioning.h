#ifndef PROJECT_GRAPH_PARTITIONING_H
#define PROJECT_GRAPH_PARTITIONING_H

#include "Graph.h"
#include <vector>

using VertexSet = std::vector<int>;
using Partition = std::vector<VertexSet>;

void bisection_rec(const Graph& G, int k, Partition& Pk);

#endif //PROJECT_GRAPH_PARTITIONING_H
