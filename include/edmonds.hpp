/**
 * @file include/edmonds.hpp
 * @author Jana K
 *
 * Алгоритм Эдмондса построения наибольшего паросочетания в графе.
 */

#ifndef INCLUDE_EDMONDS_HPP_
#define INCLUDE_EDMONDS_HPP_

#include <unordered_map>
#include <utility>
#include <vector>
#include <queue>

namespace graph {

/**
 * @brief Реализация алгоритма Эдмондса (алгоритма сжатия соцветий).
 *
 * Класс хранит состояние алгоритма — текущее паросочетание, лес
 * чередующихся цепей и базы сжатых нечётных циклов ("соцветий") — и ищет
 * увеличивающие цепи, сжимая встречающиеся нечётные циклы. Предполагается,
 * что вершины графа занумерованы подряд числами от 0 до n - 1.
 */
class EdmondsHelper {
 public:
  /**
   * @brief Конструктор.
   *
   * @param adjacency Список смежности графа. Вершины должны быть
   *     занумерованы подряд от 0 до adjacency.size() - 1.
   */
  explicit EdmondsHelper(
      const std::vector<std::vector<int>>& adjacency) :
    adjacency(adjacency),
    numVertices(static_cast<int>(adjacency.size())),
    match(numVertices, -1),
    parent(numVertices, -1),
    base(numVertices, 0),
    used(numVertices, false),
    inBlossom(numVertices, false) {
  }

  /**
   * @brief Построить наибольшее паросочетание.
   *
   * @return Вектор match: match[i] — вершина, образующая пару с вершиной i,
   *     или -1, если вершина i не входит в паросочетание.
   *
   * Перебираем непарные вершины и, пока для очередной находится
   * увеличивающая цепь, расширяем вдоль неё текущее паросочетание.
   */
  const std::vector<int>& FindMaximumMatching() {
    for (int vertex = 0; vertex < numVertices; vertex++) {
      if (match[vertex] == -1) {
        int last = FindAugmentingPath(vertex);

        while (last != -1) {
          int parentVertex = parent[last];
          int matchedWithParent = match[parentVertex];

          match[last] = parentVertex;
          match[parentVertex] = last;

          last = matchedWithParent;
        }
      }
    }

    return match;
  }

 private:
  /**
   * @brief Найти базу общего соцветия двух вершин.
   *
   * @param first Первая вершина.
   * @param second Вторая вершина.
   * @return База соцветия — ближайший общий предок вершин в лесу
   *     чередующихся цепей.
   *
   * Поднимаемся от обеих вершин к корню по чередующимся цепям; первая
   * вершина, встреченная с обеих сторон, и есть искомая база.
   */
  int FindBlossomBase(int first, int second) {
    std::vector<bool> visited(numVertices, false);

    int vertex = first;
    while (true) {
      vertex = base[vertex];
      visited[vertex] = true;

      if (match[vertex] == -1)
        break;

      vertex = parent[match[vertex]];
    }

    vertex = second;
    while (true) {
      vertex = base[vertex];

      if (visited[vertex])
        return vertex;

      vertex = parent[match[vertex]];
    }
  }

  /**
   * @brief Пройти по нечётному циклу и пометить его вершины.
   *
   * @param vertex Вершина, с которой начинаем обход цикла.
   * @param blossomBase База соцветия (до неё идём).
   * @param child Вершина, которая после сжатия становится потомком vertex.
   *
   * Идём от vertex до базы, помечаем все вершины цикла как принадлежащие
   * соцветию и попутно перестраиваем дерево цепей, чтобы их можно было
   * считать потомками базы.
   */
  void MarkBlossomPath(int vertex, int blossomBase, int child) {
    while (base[vertex] != blossomBase) {
      inBlossom[base[vertex]] = true;
      inBlossom[base[match[vertex]]] = true;

      parent[vertex] = child;
      child = match[vertex];
      vertex = parent[match[vertex]];
    }
  }

  /**
   * @brief Найти увеличивающую цепь, начинающуюся в вершине root.
   *
   * @param root Начальная непарная вершина.
   * @return Последняя вершина найденной увеличивающей цепи или -1, если
   *     такой цепи нет.
   *
   * Обходом в ширину строим лес чередующихся цепей. Если попадается ребро
   * между двумя "чётными" вершинами дерева, образовавшийся нечётный цикл
   * (соцветие) сжимаем в его базу и продолжаем поиск.
   */
  int FindAugmentingPath(int root) {
    used.assign(numVertices, false);
    parent.assign(numVertices, -1);

    for (int vertex = 0; vertex < numVertices; vertex++)
      base[vertex] = vertex;

    used[root] = true;

    std::queue<int> verticesQueue;
    verticesQueue.push(root);

    while (!verticesQueue.empty()) {
      int vertex = verticesQueue.front();
      verticesQueue.pop();

      for (int to : adjacency[vertex]) {
        if (base[vertex] == base[to] || match[vertex] == to)
          continue;

        // to - чётная вершина дерева => наткнулись на нечётный цикл,
        // сжимаем его в одну вершину (база - их общий предок).
        if (to == root ||
            (match[to] != -1 && parent[match[to]] != -1)) {
          int blossomBase = FindBlossomBase(vertex, to);

          inBlossom.assign(numVertices, false);
          MarkBlossomPath(vertex, blossomBase, to);
          MarkBlossomPath(to, blossomBase, vertex);

          for (int i = 0; i < numVertices; i++) {
            if (inBlossom[base[i]]) {
              base[i] = blossomBase;

              if (!used[i]) {
                used[i] = true;
                verticesQueue.push(i);
              }
            }
          }
        } else if (parent[to] == -1) {
          parent[to] = vertex;

          if (match[to] == -1)
            return to;

          used[match[to]] = true;
          verticesQueue.push(match[to]);
        }
      }
    }

    return -1;
  }

  //! Список смежности графа.
  const std::vector<std::vector<int>>& adjacency;
  //! Число вершин графа.
  int numVertices;
  //! Текущее паросочетание: match[i] - пара вершины i, либо -1.
  std::vector<int> match;
  //! Родитель вершины в лесу чередующихся цепей.
  std::vector<int> parent;
  //! База соцветия, которому принадлежит вершина.
  std::vector<int> base;
  //! Отметка, что вершина уже добавлена в очередь обхода.
  std::vector<bool> used;
  //! Отметка, что вершина попала в текущее сжимаемое соцветие.
  std::vector<bool> inBlossom;
};

/**
 * @brief Построить наибольшее паросочетание алгоритмом Эдмондса.
 *
 * @param graph Неориентированный граф, в котором ищем паросочетание.
 * @return Рёбра наибольшего паросочетания, заданные парами идентификаторов
 *     вершин графа.
 *
 * Идентификаторы вершин могут быть произвольными, поэтому сначала
 * перенумеровываем их подряд, строим список смежности и запускаем
 * EdmondsHelper, после чего переводим результат обратно в исходные
 * идентификаторы. Время работы — O(V^3), где V — число вершин.
 */
template<typename GraphType>
std::vector<std::pair<size_t, size_t>> Edmonds(
    const GraphType& graph) {
  std::vector<size_t> indexToId;
  std::unordered_map<size_t, int> idToIndex;

  for (size_t id : graph.Vertices()) {
    idToIndex[id] = static_cast<int>(indexToId.size());
    indexToId.push_back(id);
  }

  std::vector<std::vector<int>> adjacency(indexToId.size());

  for (size_t id : graph.Vertices()) {
    int from = idToIndex[id];

    for (size_t neighbour : graph.Edges(id))
      adjacency[from].push_back(idToIndex[neighbour]);
  }

  EdmondsHelper helper(adjacency);
  const std::vector<int>& match = helper.FindMaximumMatching();

  std::vector<std::pair<size_t, size_t>> result;

  for (size_t i = 0; i < match.size(); i++) {
    int j = match[i];

    // match симметричен, берём каждую пару один раз.
    if (j != -1 && static_cast<size_t>(j) > i)
      result.push_back(std::make_pair(indexToId[i], indexToId[j]));
  }

  return result;
}

}  // namespace graph

#endif  // INCLUDE_EDMONDS_HPP_
