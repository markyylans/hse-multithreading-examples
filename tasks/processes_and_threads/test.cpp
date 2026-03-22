#include "apply_function.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <vector>
#include <string>
#include <cmath>

TEST(ApplyFunction, EmptyVector) {
    std::vector<int> data;
    ApplyFunction<int>(data, [] (int& x) { x *= 2; }, 4);
    ASSERT_TRUE(data.empty());
}

TEST(ApplyFunction, SingleElement) {
    std::vector<int> data = {5};
    ApplyFunction<int>(data, [] (int& x) { x *= 3; }, 1);
    ASSERT_EQ(data, (std::vector<int>{15}));
}

TEST(ApplyFunction, SingleThread) {
    std::vector<int> data = {1, 2, 3, 4, 5};
    ApplyFunction<int>(data, [] (int& x) { x *= x; }, 1);
    ASSERT_EQ(data, (std::vector<int>{1, 4, 9, 16, 25}));
}

TEST(ApplyFunction, MultipleThreads) {
    auto n = 100;
    std::vector<int> data(n, 0);

    ApplyFunction<int>(data, [] (int& x) { x += 10; }, 4);

    ASSERT_EQ(data.size(), n);
    ASSERT_TRUE(std::all_of(data.begin(), data.end(), [] (const auto& v) { return v == 10; }));
}

TEST(ApplyFunction, ManyThreadsSmallVector) {
    std::vector<int> data = {1, 2, 3};
    ApplyFunction<int>(data, [] (int& x) { x *= 2; }, 100);
    ASSERT_EQ(data, (std::vector<int>{2, 4, 6}));
}

TEST(ApplyFunction, DoubleType) {
    std::vector<double> data = {1.0, 4.0, 9.0, 16.0};
    ApplyFunction<double>(data, [] (double& x) { x = std::sqrt(x); }, 2);

    ASSERT_DOUBLE_EQ(data[0], 1.0);
    ASSERT_DOUBLE_EQ(data[1], 2.0);
    ASSERT_DOUBLE_EQ(data[2], 3.0);
    ASSERT_DOUBLE_EQ(data[3], 4.0);
}

TEST(ApplyFunction, StringType) {
    std::vector<std::string> data = {"hello", "world", "foo", "bar"};
    ApplyFunction<std::string>(data, [] (std::string& s) { s += "!"; }, 3);
    ASSERT_EQ(data, (std::vector<std::string>{"hello!", "world!", "foo!", "bar!"}));
}

TEST(ApplyFunction, LargeVectorCorrectness) {
    auto transformFunc = [] (int& x) { x = x * x + 1; };
    std::vector<int> data(100000, 0);

    auto expected = data;
    std::for_each(expected.begin(), expected.end(), transformFunc);

    ApplyFunction<int>(data, transformFunc, 8);
    ASSERT_EQ(data, expected);
}

TEST(ApplyFunction, AllElementsProcessed) {
    auto n = 997;
    auto threads = 7;
    std::vector<int> data(n, 0);

    ApplyFunction<int>(data, [] (int& x) { x = 42; }, threads);

    ASSERT_EQ(data.size(), n);
    ASSERT_TRUE(std::all_of(data.begin(), data.end(), [] (const auto& v) { return v == 42; }));
}

TEST(ApplyFunction, IdentityTransform) {
    std::vector<int> data = {1, 2, 3, 4, 5};
    auto original = data;
    ApplyFunction<int>(data, [] (int&) {}, 4);
    ASSERT_EQ(data, original);
}
