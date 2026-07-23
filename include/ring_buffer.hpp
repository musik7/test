#pragma once
#include <atomic>
#include <array>
#include <cstdint>
#include <optional>

template <typename T, size_t Size>
class RingBuffer {
public:
    bool push(const T& item) {
        size_t current_tail = tail.load(std::memory_order_relaxed);
        size_t next_tail;
        do {
            next_tail = (current_tail + 1) % Size;
            if (next_tail == head.load(std::memory_order_acquire)) {
                dropped_samples.fetch_add(1, std::memory_order_relaxed);
                return false; // Full
            }
        } while (!tail.compare_exchange_weak(current_tail, next_tail, std::memory_order_release, std::memory_order_relaxed));
        
        buffer[current_tail] = item;
        return true;
    }

    std::optional<T> pop() {
        size_t current_head = head.load(std::memory_order_relaxed);
        if (current_head == tail.load(std::memory_order_acquire)) {
            return std::nullopt; // Empty
        }
        T item = buffer[current_head];
        head.store((current_head + 1) % Size, std::memory_order_release);
        return item;
    }

    uint32_t getDroppedCount() const {
        return dropped_samples.load(std::memory_order_relaxed);
    }
    
    void resetDroppedCount() {
        dropped_samples.store(0, std::memory_order_relaxed);
    }

private:
    std::array<T, Size> buffer;
    std::atomic<size_t> head{0};
    std::atomic<size_t> tail{0};
    std::atomic<uint32_t> dropped_samples{0};
};
