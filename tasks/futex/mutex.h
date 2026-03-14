#pragma once

#include <atomic>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>

class Mutex {
    enum State {
        Unlocked = 0,
        LockedNoWaiters = 1,
        LockedWithWaiters = 2
    };

    std::atomic<int> state_ = Unlocked;

public:
    Mutex() : state_(Unlocked) {}

    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;

    void lock() {
        int c = Unlocked;

        if (state_.compare_exchange_strong(c, LockedNoWaiters, std::memory_order_acquire)) {
            return;
        }

        if (c != LockedWithWaiters) {
            c = state_.exchange(LockedWithWaiters, std::memory_order_acquire);
        }

        while (c != Unlocked) {
            syscall(SYS_futex, &state_, FUTEX_WAIT_PRIVATE, LockedWithWaiters, nullptr, nullptr, 0);
            c = state_.exchange(LockedWithWaiters, std::memory_order_acquire);
        }
    }

    void unlock() {
        if (state_.fetch_sub(1, std::memory_order_release) != LockedNoWaiters) {
            state_.store(Unlocked, std::memory_order_release);
            syscall(SYS_futex, &state_, FUTEX_WAKE_PRIVATE, LockedNoWaiters, nullptr, nullptr, 0);
        }
    }
};
