#include "partitioning.h"

#include <cmath>
#include <vector>
#include <algorithm>

static int bisection(const Graph& G, std::vector<int> partition_map, int first, int last) {
    return first + (last - first) / 2;
}

static void iterate(const Graph& G, VertexSet& V, int first, int last, int k_prime, int num, Partition& Pk) {
    // передаем G: граф, V: будет использовано для разбиения,
    // first: откуда начинать, last: до куда идти, k_prime: сколько должно быть частей, num: текущий номер части, Pk: сюда будем записывать

    if (k_prime > 1) {
        int k1 = std::floor(k_prime / 2);       // Определяем где половина

        int mid = bisection(G, V, first, last); // Разбиваем текущий участок графа на два участка

        iterate(G, V, first, mid, k1, num, Pk); // Вызываем повторно для первой половины
        iterate(G, V, mid, last, k_prime - k1, num + k1, Pk);    // Вызываем повторно для второй половины
    } else {
        for (int i = first; i < last; ++i) {     // Переносим все вершины из разбиения в массив с разбиениями
            Pk[num - 1].push_back(V[i]);
        }
    }
}

void bisection_rec(const Graph& G, int k, Partition& Pk) {  // передаем G: изначальный граф, k: требуемое количество частей, Pk: массив куда складываются части
    Pk.assign(k, VertexSet());                          // создаем в каждом эллементе массива подмассив для хранения вершин текущего участка
    VertexSet all_vertices(G.num_vertices);                 // создаем массив вершин длинной с количество вершин графа

    for (int i = 0; i < G.num_vertices; ++i) all_vertices[i] = i; // в каждый эллемент массива записываем его номер

    iterate(G, all_vertices, 0, G.num_vertices, k, 1, Pk); // вызываем функцию разбиения
}

/* Наблюдения:
 *  Возможно такой подход с частичным копированием графа в памяти не подходит для больших графов
 *
 * По работе алгоритма:
 *  bisection должна перествалять индексы вершин в V, а возвращать точку разделения между первой и второй частями
 */