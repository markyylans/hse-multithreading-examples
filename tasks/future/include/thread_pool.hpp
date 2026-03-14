#pragma once

#include "future.hpp"

#include <atomic>
#include <deque>
#include <functional>
#include <mutex>
#include <stop_token>
#include <thread>
#include <vector>

class ThreadPool {
public:
    explicit ThreadPool(size_t num_threads) {
        workers_.reserve(num_threads);
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this] (std::stop_token st) { worker(st); });
        }
    }

    ~ThreadPool() {
        for (auto& w : workers_) {
            if (w.joinable()) {
                w.request_stop();
            }
        }

        cv_.notify_all();

        for (auto& w : workers_) {
            if (w.joinable()) {
                w.join();
            }
        }
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    template <typename F>
    using ReturnType = std::invoke_result_t<std::decay_t<F>>;

    template <typename F>
    auto Submit(F&& f) -> Future<ReturnType<F>> {
        auto state = std::make_shared<SharedState<ReturnType<F>>>();
        Future<ReturnType<F>> future(state);

        auto task = [state, func = std::forward<F>(f)] () noexcept {
            try {
                if constexpr (std::is_void_v<ReturnType<F>>) {
                    func();
                    state->set_value();
                }
                else {
                    state->set_value(func());
                }
            }
            catch (...) {
                state->set_exception(std::current_exception());
            }
        };

        {
            std::lock_guard lock{mutex_};
            tasks_.push_back(std::move(task));
        }

        cv_.notify_one();

        return future;
    }

private:
    void worker(std::stop_token st) {
        while (!st.stop_requested()) {
            std::function<void()> task;
            {
                std::unique_lock lock{mutex_};
                cv_.wait(lock, [this, &st] () { return st.stop_requested() || !tasks_.empty(); });
                if (tasks_.empty()) {
                    if (st.stop_requested()) {
                        break;
                    }
                    continue;
                }
                task = std::move(tasks_.front());
                tasks_.pop_front();
            }

            task();
        }
    }

    std::mutex mutex_;
    std::condition_variable cv_;

    std::deque<std::function<void()>> tasks_;
    std::vector<std::jthread> workers_;
};
