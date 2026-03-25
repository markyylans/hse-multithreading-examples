#pragma once

#include <boost/coroutine2/all.hpp>

#include <unordered_map>
#include <unordered_set>
#include <vector>

// граф представлен как список смежности
using Graph = std::unordered_map<int, std::vector<int>>;

using DfsCoroutine = boost::coroutines2::coroutine<int>;

inline void DfsStep(DfsCoroutine::push_type& yield, const Graph& graph,
                    int node, std::unordered_set<int>& visited) {
    if (visited.contains(node)) {
        return;
    }
    visited.insert(node);

    yield(node);

    if (auto it = graph.find(node); it != graph.end()) {
        for (int neighbour : it->second) {
            DfsStep(yield, graph, neighbour, visited);
        }
    }
}

inline DfsCoroutine::pull_type MakeDfsGenerator(const Graph& graph, int start) {
    return DfsCoroutine::pull_type{
        [graph, start] (DfsCoroutine::push_type& yield) {
            std::unordered_set<int> visited;
            DfsStep(yield, graph, start, visited);
        }
    };
}
