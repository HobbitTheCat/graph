#include "Graph.h"
#include "partitioning/multilevel_partition.h"

#include <vector>
#include <queue>
#include <climits>
#include <algorithm>
#include <random>

void two_way_partition(const Graph& G, std::vector<int>& part) {
    int n = G.num_vertices;
    part.assign(n, -1);

    if (n == 0) return; // TODO проверить что это будет работать всегда

    // выбираем случайную вершину TODO тут явно нужно выбирать не случайную вершину
    // std::mt19937 rng(std::random_device{}());
    // std::uniform_int_distribution<int> dist(0, n-1);
    // int start_node = dist(rng);
    int start_node = 0;
    int min_deg = INT_MAX;
    for (int i = 0; i < n; i++) {
        int deg = G.offsets[i + 1] - G.offsets[i];
        if (deg < min_deg) {min_deg = deg; start_node = i;}
        if (min_deg == 1) break;
    }

    std::priority_queue<std::pair<int, int>> frontier;  // создаем массив граничащих эллементов pair<приоритет, индекс>
    std::vector<int> gains(n, 0);

    part[start_node] = 0;                               // добавляем начальную вершину в первую часть
    int current_weight_E = G.vertex_weights[start_node];// инициируем вес первой части равным весу единственной вершины в нем
    int total_weight_V = 0;                             // инициируем вес графа
    for (double w : G.vertex_weights) total_weight_V += w;  // получаем полный вес графф

    for (int i = G.offsets[start_node]; i < G.offsets[start_node+1]; i++) {     // для всех соседей start_node
        int neighbor_id = G.edges[i];                   // берем соседа
        if (neighbor_id != start_node) {                // проверяем петлю
            gains[neighbor_id] += G.edge_weights[i];    // выигрыш составляет вес связи до этого соседа
            frontier.push({gains[neighbor_id], neighbor_id});
        }
    }

    while (!frontier.empty() && current_weight_E < total_weight_V / 2.0) {
        int v = frontier.top().second;           // берем первую в очереди вершину
        frontier.pop();                          // убираем ее из очереди

        if (part[v] == 0) continue;              // проверили что вершина еще не находится в этой части

        part[v] = 0;                             // перемещаем вершину в нулевую часть
        current_weight_E += G.vertex_weights[v]; // добавляем ее вес

        for (int i = G.offsets[v]; i < G.offsets[v+1]; i++) {
            int neighbor_id = G.edges[i];        // берем соседа добавленной вершины
            if (part[neighbor_id] == -1) {       // если она еще не распределена
                gains[neighbor_id] += G.edge_weights[i];    // добавляем выигрыш
                frontier.push({gains[neighbor_id], neighbor_id});   // добавляем его в границу
            }
        }
    }

    for (int i = 0; i < n; i++) {               // все что не в 0 то в 1
        if (part[i] == -1) part[i] = 1;
    }
}