#pragma once

#include <vector>
#include <functional>
#include <thread>
#include <algorithm>

template <typename T>
void ApplyFunction(std::vector<T>& data, const std::function<void(T&)>& transform, const int threadCount = 1) {
    if (data.empty()) {
        return;
    }

    size_t effectiveThreads = std::min(threadCount, data.size());

    if (effectiveThreads <= 1) {
        for (auto& elem : data) {
            transform(elem);
        }
        return;
    }

    size_t totalSize = data.size();
    size_t chunkSize = totalSize / effectiveThreads;
    size_t remainder = totalSize % effectiveThreads;

    std::vector<std::thread> threads;
    threads.reserve(effectiveThreads);

    size_t offset = 0;
    for (size_t i = 0; i < effectiveThreads; ++i) {
        size_t count = chunkSize + (i < remainder ? 1 : 0);
        threads.emplace_back([&data, &transform, offset, count] () {
            for (size_t j = offset; j < offset + count; ++j) {
                transform(data[j]);
            }
        });
        offset += count;
    }

    for (auto& t : threads) {
        t.join();
    }
}
