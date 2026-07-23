#pragma once
#include <cstddef>
#include <cstdint>
#include "ring_buffer.hpp"

struct AllocRecord {
    uintptr_t ptr;
    size_t size;
    bool is_free;
};

class LinearMemoryGuard {
public:
    static LinearMemoryGuard& getInstance();
    void recordAlloc(uintptr_t ptr, size_t size);
    void recordFree(uintptr_t ptr);
    RingBuffer<AllocRecord, 1024>& getBuffer() { return buffer; }
    
    // Fixed size hash table for active allocations
    struct HashNode {
        uintptr_t ptr;
        size_t size;
        bool active;
    };
    static constexpr size_t HASH_SIZE = 4096;
    HashNode allocations[HASH_SIZE];
    
private:
    LinearMemoryGuard() {
        for (size_t i=0; i<HASH_SIZE; ++i) {
            allocations[i].active = false;
        }
    }
    RingBuffer<AllocRecord, 1024> buffer;
};
