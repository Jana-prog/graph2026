/**
 * @file edmonds_test.cpp
 * @author Jana K
 *
 * Тесты для алгоритма Эдмондса построения наибольшего паросочетания
 * graph::Edmonds.
 */

#include <httplib.h>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <utility>
#include <random>
#include <nlohmann/json.hpp>
#include "test_core.hpp"

static void EmptyGraphTest(httplib::Client* cli);
static void SimpleTest(httplib::Client* cli);
static void StarTest(httplib::Client* cli);
static void RandomTest(httplib::Client* cli);
static void RandomTestHelper(httplib::Client* cli,
    const std::string& graphType);

void TestEdmonds(httplib::Client* cli) {
  TestSuite suite("TestEdmonds");

  RUN_TEST_REMOTE(suite, cli, EmptyGraphTest);
  RUN_TEST_REMOTE(suite, cli, SimpleTest);
  RUN_TEST_REMOTE(suite, cli, StarTest);
  RUN_TEST_REMOTE(suite, cli, RandomTest);
}

static size_t CheckMatchingValidity(const nlohmann::json& input,
    const nlohmann::json& output) {
  std::unordered_map<size_t, std::unordered_set<size_t>> adjacency;

  for (auto& edge : input.at("edges")) {
    size_t from = edge.at("from");
    size_t to = edge.at("to");

    adjacency[from].insert(to);
    adjacency[to].insert(from);
  }

  std::unordered_set<size_t> usedVertices;
  size_t size = 0;

  for (auto& edge : output.at("result")) {
    size_t from = edge.at("from");
    size_t to = edge.at("to");

    REQUIRE(adjacency[from].find(to) != adjacency[from].end());
    REQUIRE(usedVertices.find(from) == usedVertices.end());
    REQUIRE(usedVertices.find(to) == usedVertices.end());

    usedVertices.insert(from);
    usedVertices.insert(to);
    size++;
  }

  return size;
}

static void EmptyGraphTest(httplib::Client* cli) {
  nlohmann::json input = R"(
{
  "graph_type": "Graph",
  "vertices": [ ],
  "edges": [ ]
}
)"_json;

  auto res = cli->Post("/Edmonds", input.dump(),
      "application/json");

  if (!res) {
    REQUIRE(false);
  }

  nlohmann::json output = nlohmann::json::parse(res->body);

  size_t size = CheckMatchingValidity(input, output);

  REQUIRE_EQUAL(size, static_cast<size_t>(0));
}

// Нечётный цикл (треугольник 1-2-3) + хвост 3-4-5-6
static void SimpleTest(httplib::Client* cli) {
  nlohmann::json input;

  input["graph_type"] = "Graph";
  input["vertices"] = std::vector<int>{1, 2, 3, 4, 5, 6};

  input["edges"][0]["from"] = 1;
  input["edges"][0]["to"] = 2;

  input["edges"][1]["from"] = 2;
  input["edges"][1]["to"] = 3;

  input["edges"][2]["from"] = 3;
  input["edges"][2]["to"] = 1;

  input["edges"][3]["from"] = 3;
  input["edges"][3]["to"] = 4;

  input["edges"][4]["from"] = 4;
  input["edges"][4]["to"] = 5;

  input["edges"][5]["from"] = 5;
  input["edges"][5]["to"] = 6;

  auto res = cli->Post("/Edmonds", input.dump(),
      "application/json");

  if (!res) {
    REQUIRE(false);
  }

  nlohmann::json output = nlohmann::json::parse(res->body);

  size_t size = CheckMatchingValidity(input, output);

  REQUIRE_EQUAL(size, static_cast<size_t>(3));
}

static void StarTest(httplib::Client* cli) {
  nlohmann::json input;

  input["graph_type"] = "Graph";
  input["vertices"] = std::vector<int>{1, 2, 3, 4, 5};

  input["edges"][0]["from"] = 1;
  input["edges"][0]["to"] = 2;

  input["edges"][1]["from"] = 1;
  input["edges"][1]["to"] = 3;

  input["edges"][2]["from"] = 1;
  input["edges"][2]["to"] = 4;

  input["edges"][3]["from"] = 1;
  input["edges"][3]["to"] = 5;

  auto res = cli->Post("/Edmonds", input.dump(),
      "application/json");

  if (!res) {
    REQUIRE(false);
  }

  nlohmann::json output = nlohmann::json::parse(res->body);

  size_t size = CheckMatchingValidity(input, output);

  REQUIRE_EQUAL(size, static_cast<size_t>(1));
}

// dp[mask] - размер наиб. паросочетания на вершинах из mask: младшую вершину
// либо оставляем без пары, либо паруем с соседом, тоже входящим в mask.
static int BruteForceEdmondsSize(int numVertices,
    const std::vector<std::vector<int>>& adjacency) {
  int numMasks = 1 << numVertices;
  std::vector<int> dp(numMasks, 0);

  for (int mask = 1; mask < numMasks; mask++) {
    int first = 0;

    while (!(mask & (1 << first)))
      first++;

    dp[mask] = dp[mask & ~(1 << first)];

    for (int to : adjacency[first]) {
      if (to != first && (mask & (1 << to))) {
        int rest = mask & ~(1 << first) & ~(1 << to);

        if (dp[rest] + 1 > dp[mask])
          dp[mask] = dp[rest] + 1;
      }
    }
  }

  return dp[numMasks - 1];
}

static void RandomTest(httplib::Client* cli) {
  RandomTestHelper(cli, "Graph");
  RandomTestHelper(cli, "WeightedGraph");
}

static void RandomTestHelper(httplib::Client* cli,
    const std::string& graphType) {
  const int numTries = 100;
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> verticesCount(2, 14);
  std::uniform_int_distribution<int> edgeFlag(0, 2);

  for (int it = 0; it < numTries; it++) {
    int numVertices = verticesCount(gen);
    std::vector<std::vector<int>> adjacency(numVertices);

    nlohmann::json input;

    input["graph_type"] = graphType;

    if (graphType == "WeightedGraph")
      input["weight_type"] = "int";

    std::vector<size_t> vertices;

    for (int i = 0; i < numVertices; i++)
      vertices.push_back(100 + i);

    input["vertices"] = vertices;
    input["edges"] = nlohmann::json::array();

    size_t numEdges = 0;
    for (int i = 0; i < numVertices; i++) {
      for (int j = i + 1; j < numVertices; j++) {
        if (edgeFlag(gen) != 0)
          continue;

        adjacency[i].push_back(j);
        adjacency[j].push_back(i);

        input["edges"][numEdges]["from"] = 100 + i;
        input["edges"][numEdges]["to"] = 100 + j;

        if (graphType == "WeightedGraph")
          input["edges"][numEdges]["weight"] = 1;

        numEdges++;
      }
    }

    int expectedSize = BruteForceEdmondsSize(numVertices, adjacency);

    auto res = cli->Post("/Edmonds", input.dump(),
        "application/json");

    if (!res) {
      REQUIRE(false);
    }

    nlohmann::json output = nlohmann::json::parse(res->body);

    size_t size = CheckMatchingValidity(input, output);

    REQUIRE_EQUAL(static_cast<int>(size), expectedSize);
  }
}
