#include "mutex.h"

#include <gtest/gtest.h>
#include <thread>
#include <vector>

TEST(MutexTest, BasicLockUnlock) {
    Mutex m;
    m.lock();
    SUCCEED();
    m.unlock();
}

TEST(MutexTest, ConcurrentIncrement) {
    Mutex m;
    auto counter = 0;
    auto num_threads = 100;
    auto num_iterations = 100000;
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    for (auto i = 0; i < num_threads; ++i) {
        threads.emplace_back([&] () {
            for (auto j = 0; j < num_iterations; ++j) {
                m.lock();
                ++counter;
                m.unlock();
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(counter, num_threads * num_iterations);
}
