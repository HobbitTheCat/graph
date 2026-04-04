#include "Graph.h"
#include <algorithm>

Graph::Graph(CSR &&source_csr)
    :csr(std::move(source_csr)),
     colors(csr.size(), -1),
     sat_deg(csr.size(), 0),
     neighbor_colors(csr.size()),
     buckets(csr.size()),
     last_seen_at(csr.size(), -1) {}

// int Graph::find_min_available_color(int v) {
//     std::set<int> used;
//     for (int neighbor: csr.get_neighbors(v)) {
//         if (colors[neighbor] != -1) {
//             used.insert(colors[neighbor]);
//         }
//     }
//     int color = 0;
//     while (used.contains(color)) color++;
//     return color;
// }

int Graph::find_min_available_color(int v) {
    for (int neighbor : csr.get_neighbors(v)) {
        int c = colors[neighbor];
        if (c != -1) last_seen_at[c] = v;
    }
    int color = 0;
    while (color < last_seen_at.size() && last_seen_at[color] == v) color++;
    return color;
}

void Graph::run_dsatur() {
    int n = csr.size();
    int colored_count = 0;

    for (int i = 0; i < n; i++) buckets[0].push_back(i);

    max_sat = 0;

    while (colored_count < n) {
        while (max_sat >= 0 && buckets[max_sat].empty()) max_sat--; // ищем ведро в котором на данный момент вершины с макс сатурацией
        if (max_sat < 0) break;

        int v = -1;
        while (!buckets[max_sat].empty()) {         // для каждой вершины среди вершин с макс сатурацией
            int curr = buckets[max_sat].back();     // берем последнюю вершину
            buckets[max_sat].pop_back();            // и удаляем ее из ведра
            if (colors[curr] == -1) {               // если она еще не покрашена
                v = curr;                           // то мы нашли вершину для покраски и прекращаем поиск
                break;
            }
        }

        if (v == -1) continue;                      // если вершины пока не нашли, то получается, что текщее ведро пустое

        int color = find_min_available_color(v);    // ищем подходящий цвет
        colors[v] = color;                          // красим вершину
        colored_count++;                            // обновляем количество покрашенных вершин

        for (int neighbor: csr.get_neighbors(v)) {  // для каждого соседа
            if (colors[neighbor] == -1) {           // если он еще не покрашен
                if (neighbor_colors[neighbor].insert(color).second) { // если этого цвета еще не было
                    // int old_sat = sat_deg[neighbor];
                    sat_deg[neighbor]++;            // поднимаем сатурацию
                    int new_sat = sat_deg[neighbor];// получаем новую сатурцию

                    buckets[new_sat].push_back(neighbor);       // поднимаем вершину в ведрах
                    if (new_sat > max_sat) max_sat = new_sat;   // поднимаем максимальную насыщенность
                }
            }
        }

    }
}
