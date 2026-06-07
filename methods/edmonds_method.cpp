#include <string>
#include <utility>
#include <vector>
#include <nlohmann/json.hpp>
#include <graph.hpp>
#include <oriented_graph.hpp>
#include <weighted_graph.hpp>
#include <weighted_oriented_graph.hpp>
#include <edmonds.hpp>

using graph::Graph;
using graph::OrientedGraph;
using graph::WeightedGraph;
using graph::WeightedOrientedGraph;

namespace graph {

template<typename GraphType>
int EdmondsMethodHelper(const nlohmann::json& input,
    nlohmann::json* output);

/**
 * @brief Серверный метод построения наибольшего паросочетания.
 *
 * @param input Входные данные в формате JSON (тип графа, вершины, рёбра).
 * @param output Результат работы алгоритма в формате JSON.
 * @return 0 в случае успеха и отрицательное число, если входные данные
 *     некорректны или тип графа не поддерживается.
 *
 * Разбирает поле "graph_type" и вызывает шаблонный помощник для нужного
 * типа графа.
 */
int EdmondsMethod(const nlohmann::json& input,
    nlohmann::json* output) {
  std::string graphType = input.at("graph_type");

  if (graphType == "Graph") {
    return EdmondsMethodHelper<Graph>(input, output);
  } else if (graphType == "WeightedGraph") {
    std::string weightType = input.at("weight_type");
    if (weightType == "int") {
      return EdmondsMethodHelper<WeightedGraph<int>>(input, output);
    } else {
      return -1;
    }
  }

  // Ориентированные графы (OrientedGraph, WeightedOrientedGraph) не
  // поддерживаем: наибольшее паросочетание определено для
  // неориентированных графов.
  return -1;
}

/**
 * @brief Запустить алгоритм Эдмондса для конкретного типа графа.
 *
 * @param input Входные данные в формате JSON.
 * @param output Результат в формате JSON: массив "result" из пар
 *     {"from", "to"} — рёбер найденного паросочетания.
 * @return 0 (корректность входных данных уже проверена диспетчером).
 *
 * Собирает граф из JSON, запускает graph::Edmonds и записывает найденное
 * паросочетание в выходной JSON.
 */
template<typename GraphType>
int EdmondsMethodHelper(const nlohmann::json& input,
    nlohmann::json* output) {
  GraphType graph;

  for (auto& vertex : input.at("vertices")) {
    graph.AddVertex(vertex);
  }

  for (auto& edge : input.at("edges")) {
    graph.AddEdge(edge.at("from"), edge.at("to"));
  }

  std::vector<std::pair<size_t, size_t>> matching = Edmonds(graph);

  (*output)["result"] = nlohmann::json::array();

  for (size_t i = 0; i < matching.size(); i++) {
    (*output)["result"][i]["from"] = matching[i].first;
    (*output)["result"][i]["to"] = matching[i].second;
  }

  return 0;
}

}  // namespace graph
