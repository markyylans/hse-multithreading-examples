#pragma once

#include <chrono>
#include <condition_variable>
#include <exception>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>

template <typename T>
class Future;

template <typename T>
struct SharedState {
    std::mutex mutex;
    std::condition_variable cv;

    std::optional<T> result;
    std::exception_ptr exception;
    bool ready = false;

    void set_value(T value) {
        std::lock_guard lock{mutex};
        result = std::move(value);
        ready = true;
        cv.notify_all();
    }

    void set_exception(std::exception_ptr e) {
        std::lock_guard lock{mutex};
        exception = std::move(e);
        ready = true;
        cv.notify_all();
    }

    T get() {
        std::unique_lock lock{mutex};
        cv.wait(lock, [this] () { return ready; });
        if (exception) {
            std::rethrow_exception(exception);
        }
        return std::move(*result);
    }

    void wait() {
        std::unique_lock lock{mutex};
        cv.wait(lock, [this] () { return ready; });
    }

    template <typename TimeoutType = std::chrono::milliseconds>
    bool wait_for(const TimeoutType& timeout) {
        std::unique_lock lock{mutex};
        return cv.wait_for(lock, timeout, [this] () { return ready; });
    }
};

template <>
struct SharedState<void> {
    std::mutex mutex;
    std::condition_variable cv;
    std::exception_ptr exception;
    bool ready = false;

    void set_value() {
        std::lock_guard lock{mutex};
        ready = true;
        cv.notify_all();
    }

    void set_exception(std::exception_ptr e) {
        std::lock_guard lock{mutex};
        exception = std::move(e);
        ready = true;
        cv.notify_all();
    }

    void get() {
        std::unique_lock lock{mutex};
        cv.wait(lock, [this] { return ready; });
        if (exception) {
            std::rethrow_exception(exception);
        }
    }

    void wait() {
        std::unique_lock lock{mutex};
        cv.wait(lock, [this] { return ready; });
    }

    template <typename TimeoutType = std::chrono::milliseconds>
    bool wait_for(const TimeoutType& timeout) {
        std::unique_lock lock{mutex};
        return cv.wait_for(lock, timeout, [this] () { return ready; });
    }
};

template <typename T>
class Future {
public:
    Future() = default;

    explicit Future(std::shared_ptr<SharedState<T>> state)
        : state_(std::move(state)) { }

    Future(Future&&) noexcept = default;
    Future& operator=(Future&&) noexcept = default;
    Future(const Future&) = delete;
    Future& operator=(const Future&) = delete;

    bool valid() const { return state_ != nullptr; }

    T get() {
        if (!valid()) {
            throw std::future_error(std::future_errc::no_state);
        }
        return state_->get();
    }

    void wait() const {
        if (!valid()) {
            throw std::future_error(std::future_errc::no_state);
        }
        state_->wait();
    }

    template <typename TimeoutType = std::chrono::milliseconds>
    bool wait_for(const TimeoutType& timeout) const {
        if (!valid()) {
            throw std::future_error(std::future_errc::no_state);
        }
        return state_->wait_for(timeout);
    }

    std::shared_ptr<SharedState<T>> state() const { return state_; }

private:
    std::shared_ptr<SharedState<T>> state_;
};
