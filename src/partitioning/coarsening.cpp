#include "partitioning/coarsening.h"

#include <numeric>
#include <random>
#include <algorithm>

void find_heavy_edge_matching(const Graph& G, std::vector<int>& cmap, int& nextVertexIdx) {
    int n = G.num_vertices;

    std::vector<int> match(n, -1);                // информация о том какие вершины уже сформировали пары
    std::vector<int> p(n);                              // создаем массив длинной сколичество вершин (permutation)
    std::iota(p.begin(), p.end(), 0);    // заполняем его индексами вершин от 0 до n-1

    std::ranges::shuffle(p, std::mt19937(std::random_device()()));  // перемешиваем для случайного порядка обхода

    nextVertexIdx = 0;
    cmap.assign(n, -1);                  // выделяем место под мапу и заполняем -1 для начала

    for (int vertexId : p) {                 // обходим все вершины в случайном порядке
        if (match[vertexId] != -1) continue; // эта вершина уже образовала пару

        int best_neighbor_id = -1;
        int max_edge_weight = -1; // TODO в случае с отрицательными весами заменить

        for (int i = G.offsets[vertexId]; i < G.offsets[vertexId + 1]; i++) {   // обходим всех соседей вершины
            int neighbor_id = G.edges[i];                                       // получаем id вершины
            if (neighbor_id != vertexId && match[neighbor_id] == -1) {          // проверяем что это не петля и вершина не схлопнулась
                if (G.edge_weights[i] > max_edge_weight) {                      // тут мы выбираем самую тяжелую из связей
                    max_edge_weight = G.edge_weights[i];
                    best_neighbor_id = neighbor_id;
                }
            }
        }

        if (best_neighbor_id != -1) {                                           // если мы нашли подходящую вершину
            match[vertexId] = best_neighbor_id;                                 // записываем как соединенную
            match[best_neighbor_id] = vertexId;                                 // и вторую тоже
            cmap[vertexId] = cmap[best_neighbor_id] = nextVertexIdx++;    // мапим обе к новой вершине придерживаясь порядкового номера
        } else {
            cmap[vertexId] = nextVertexIdx++;                        // если не нашли подходящей вершины просто мапим к новой
        }
    }
}

void build_coarse_graph(const Graph& G, const std::vector<int>& cmap, int num_coarse_vertices, Graph& new_G) {
    new_G.num_vertices = num_coarse_vertices;                       // указываем количество вершин у новго графа
    new_G.vertex_weights.assign(num_coarse_vertices, 0);        // выделяем массив для весов вершин
    new_G.offsets.assign(num_coarse_vertices + 1, 0);        // выделяем массив для сдвигов

    std::vector<std::vector<int>> groups(num_coarse_vertices);
    for (int i = 0; i < G.num_vertices; i++) {
        groups[cmap[i]].push_back(i);
        new_G.vertex_weights[cmap[i]] += G.vertex_weights[i];       // вес вершины счиатется как сумма весов вершин в нее входящих
    }

    std::vector<int> new_edges;
    std::vector<int> new_edge_weights;
    std::vector<int> marker(num_coarse_vertices, -1);
    std::vector<int> acc_weights(num_coarse_vertices, 0);     // сюда записываем веса новых связей

    for (int super_vertex = 0; super_vertex < num_coarse_vertices; super_vertex++) {
        new_G.offsets[super_vertex] = new_edges.size(); // запоминаем с какого места начинается описание текущей супервершины
        // new_edges.size() это текущий размер массива

        for (int vertex : groups[super_vertex]) {   // обходим все вершины входящие в супер верщину
            for (int i = G.offsets[vertex]; i < G.offsets[vertex + 1]; i++) { // обходим всех соседей в начальном графе
                int super_vertex_of_neighbor = cmap[G.edges[i]];  // смотрим в какую супер вершину попал сосед

                if (super_vertex_of_neighbor != super_vertex) {   // если не в ту же самую (те это не петля)
                    if (marker[super_vertex_of_neighbor] < super_vertex) {  // проверяем встречали ли мы это ребро для текущей супер вершины
                        marker[super_vertex_of_neighbor] = super_vertex;    // записали что теперь встречали
                        new_edges.push_back(super_vertex_of_neighbor);      // добавили новую связь
                        acc_weights[super_vertex_of_neighbor] = G.edge_weights[i];
                    } else {                                      // уже вестречали эту вершину
                        acc_weights[super_vertex_of_neighbor] += G.edge_weights[i];
                    }
                }
            }
        }

        for (int i = new_G.offsets[super_vertex]; i < new_edges.size(); i++) {
            new_edge_weights.push_back(acc_weights[new_edges[i]]);
        }
    }

    new_G.offsets[num_coarse_vertices] = new_edges.size();
    new_G.edges = std::move(new_edges);
    new_G.edge_weights = std::move(new_edge_weights);
}

void coarse_step(const Graph& G, CoarseLevel& next_level) {
    int num_coarse_vertices = 0;
    find_heavy_edge_matching(G, next_level.cmap, num_coarse_vertices);
    build_coarse_graph(G, next_level.cmap, num_coarse_vertices, next_level.graph);
}