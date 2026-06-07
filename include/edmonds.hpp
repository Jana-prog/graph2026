/**
 * @file include/edmonds.hpp
 * @author Jana K
 *
 * Алгоритм Эдмондса нахождения наибольшего паросочетания в графе.
 */

#ifndef INCLUDE_EDMONDS_HPP_
#define INCLUDE_EDMONDS_HPP_

#include <unordered_map>
#include <utility>
#include <vector>
#include <queue>

namespace graph {

class EdmondsHelper {
 public:
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

  const std::vector<int>& FindEdmonds() {
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

  void MarkBlossomPath(int vertex, int blossomBase, int child) {
    while (base[vertex] != blossomBase) {
      inBlossom[base[vertex]] = true;
      inBlossom[base[match[vertex]]] = true;

      parent[vertex] = child;
      child = match[vertex];
      vertex = parent[match[vertex]];
    }
  }

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

  const std::vector<std::vector<int>>& adjacency;
  int numVertices;
  std::vector<int> match;
  std::vector<int> parent;
  std::vector<int> base;
  std::vector<bool> used;
  std::vector<bool> inBlossom;
};

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
  const std::vector<int>& match = helper.FindEdmonds();

  std::vector<std::pair<size_t, size_t>> result;

  for (size_t i = 0; i < match.size(); i++) {
    int j = match[i];

    if (j != -1 && static_cast<size_t>(j) > i)
      result.push_back(std::make_pair(indexToId[i], indexToId[j]));
  }

  return result;
}

}  // namespace graph

#endif  // INCLUDE_EDMONDS_HPP_
