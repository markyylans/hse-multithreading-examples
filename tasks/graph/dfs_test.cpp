#include "graph_dfs.hpp"

#include <gtest/gtest.h>

#include <vector>

inline std::vector<int> CollectDfs(const Graph& graph, int start) {
    std::vector<int> result;
    for (int node : MakeDfsGenerator(graph, start)) {
        result.push_back(node);
    }
    return result;
}

TEST(DfsCoroutineTest, SingleNode) {
    Graph graph = {{0, {}}};
    EXPECT_EQ(CollectDfs(graph, 0), (std::vector<int>{0}));
}

TEST(DfsCoroutineTest, LinearChain) {
    Graph graph = {{0, {1}}, {1, {2}}, {2, {3}}, {3, {}}};
    EXPECT_EQ(CollectDfs(graph, 0), (std::vector<int>{0, 1, 2, 3}));
}

TEST(DfsCoroutineTest, BinaryTree) {
    Graph graph = {{0, {1, 2}}, {1, {3, 4}}, {2, {}}, {3, {}}, {4, {}}};
    EXPECT_EQ(CollectDfs(graph, 0), (std::vector<int>{0, 1, 3, 4, 2}));
}

TEST(DfsCoroutineTest, SimpleCycle) {
    Graph graph = {{0, {1}}, {1, {2}}, {2, {0}}};
    auto nodes = CollectDfs(graph, 0);
    EXPECT_EQ(nodes, (std::vector<int>{0, 1, 2}));
}

TEST(DfsCoroutineTest, SelfLoop) {
    Graph graph = {{0, {0, 1}}, {1, {}}};
    auto nodes = CollectDfs(graph, 0);
    EXPECT_EQ(nodes, (std::vector<int>{0, 1}));
}

TEST(DfsCoroutineTest, MultipleBackEdges) {
    Graph graph = {{0, {1, 2}}, {1, {2, 3}}, {2, {0}}, {3, {2}}};
    auto nodes = CollectDfs(graph, 0);

    std::unordered_set<int> unique(nodes.begin(), nodes.end());
    EXPECT_EQ(unique.size(), 4);
    EXPECT_EQ(nodes.size(), 4);
}

TEST(DfsCoroutineTest, StartFromMiddle) {
    Graph graph = {{0, {1, 2}}, {1, {3}}, {2, {}}, {3, {}}};
    EXPECT_EQ(CollectDfs(graph, 1), (std::vector<int>{1, 3}));
}

TEST(DfsCoroutineTest, IsolatedStartNode) {
    Graph graph = {{0, {1}}, {1, {}}, {2, {3}}, {3, {}}};
    auto nodes = CollectDfs(graph, 0);
    EXPECT_EQ(nodes, (std::vector<int>{0, 1}));

    nodes = CollectDfs(graph, 2);
    EXPECT_EQ(nodes, (std::vector<int>{2, 3}));
}

TEST(DfsCoroutineTest, YieldsOneNodeAtATime) {
    Graph graph = {{0, {1}}, {1, {2}}, {2, {}}};

    auto gen = MakeDfsGenerator(graph, 0);

    ASSERT_TRUE(bool(gen));
    EXPECT_EQ(gen.get(), 0);

    gen();

    ASSERT_TRUE(bool(gen));
    EXPECT_EQ(gen.get(), 1);

    gen();

    ASSERT_TRUE(bool(gen));
    EXPECT_EQ(gen.get(), 2);

    gen();

    EXPECT_FALSE(bool(gen));
}

TEST(DfsCoroutineTest, TwoWorkersRoundRobin) {
    Graph graph_a = {{0, {1}}, {1, {}}};
    Graph graph_b = {{10, {11}}, {11, {}}};

    auto gen_a = MakeDfsGenerator(graph_a, 0);
    auto gen_b = MakeDfsGenerator(graph_b, 10);

    std::vector<int> path;

    while (gen_a || gen_b) {
        if (gen_a) {
            path.push_back(gen_a.get());
            gen_a();
        }
        if (gen_b) {
            path.push_back(gen_b.get());
            gen_b();
        }
    }

    EXPECT_EQ(path, (std::vector<int>{0, 10, 1, 11}));
}
