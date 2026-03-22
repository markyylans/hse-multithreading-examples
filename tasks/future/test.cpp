#include "thread_pool.hpp"

#include <gtest/gtest.h>
#include <stdexcept>

TEST(ThreadPool, SubmitReturnsResult) {
    ThreadPool pool{2};
    auto future = pool.Submit([] () { return 42; });
    ASSERT_TRUE(future.valid());
    EXPECT_EQ(future.get(), 42);
}

TEST(ThreadPool, VoidCorrectness) {
    ThreadPool pool{2};
    auto future = pool.Submit([] () { });
    ASSERT_TRUE(future.valid());
    future.wait();
    SUCCEED();
}

TEST(ThreadPool, WaitForCorrectness) {
    ThreadPool pool{2};
    auto future = pool.Submit([] () { std::this_thread::sleep_for(std::chrono::milliseconds(100)); return 42; });
    ASSERT_TRUE(future.valid());
    ASSERT_FALSE(future.wait_for(std::chrono::milliseconds(30)));
    EXPECT_EQ(future.get(), 42);
}

TEST(ThreadPool, SubmitPropagatesException) {
    ThreadPool pool{2};
    auto future = pool.Submit([] () {
        throw std::runtime_error("task error");
    });
    ASSERT_TRUE(future.valid());
    EXPECT_THROW(future.get(), std::runtime_error);
}
