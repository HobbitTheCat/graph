#include "Graph.h"

void split_into_two_graphs(const Graph& initial, const std::vector<int>& parts, Graph& g0, Graph& g1) {
    int n = initial.num_vertices;

    // 1. Создаем карты отображения: global_id -> local_id
    std::vector<int> global_to_local(n, -1);
    int n0 = 0, n1 = 0;

    for (int i = 0; i < n; i++) {
        if (parts[i] == 0) global_to_local[i] = n0++;
        else global_to_local[i] = n1++;
    }

    // Инициализируем графы
    g0.num_vertices = n0;
    g0.offsets.assign(n0 + 1, 0);
    g0.vertex_weights.reserve(n0);

    g1.num_vertices = n1;
    g1.offsets.assign(n1 + 1, 0);
    g1.vertex_weights.reserve(n1);

    auto fill_graph = [&](Graph& target, int target_part) {
        std::vector<int> new_edges;
        std::vector<int> new_edge_weights;
        int current_local_v = 0;

        for (int v = 0; v < n; v++) {
            if (parts[v] != target_part) continue;

            target.offsets[current_local_v] = new_edges.size();
            target.vertex_weights.push_back(initial.vertex_weights.empty() ? 1 : initial.vertex_weights[v]);

            for (int i = initial.offsets[v]; i < initial.offsets[v + 1]; i++) {
                int neighbor = initial.edges[i];
                int weight = initial.edge_weights.empty() ? 1 : initial.edge_weights[i];

                if (parts[neighbor] == target_part) {
                    // Внутреннее ребро
                    new_edges.push_back(global_to_local[neighbor]);
                    new_edge_weights.push_back(weight);
                } else {
                    // Граничное ребро (ведёт в другую часть)
                    // Здесь ты можешь решить: либо добавлять виртуальную вершину,
                    // либо просто игнорировать ребро для полной независимости.
                    // Если используем is_virtual_vertex, логика усложняется (нужно добавлять саму вершину).
                }
            }
            current_local_v++;
        }
        target.offsets[target.num_vertices] = new_edges.size();
        target.edges = std::move(new_edges);
        target.edge_weights = std::move(new_edge_weights);
        target.num_edges = target.edges.size();
    };

    fill_graph(g0, 0);
    fill_graph(g1, 1);
}