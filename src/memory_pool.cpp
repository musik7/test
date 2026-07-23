#include "memory_pool.hpp"

MemoryPool::MemoryPool() : offset(0) {}

uint8_t* MemoryPool::allocate(size_t size) {
    // 8-byte alignment
    size_t aligned_size = (size + 7) & ~7;
    if (offset + aligned_size > POOL_SIZE) {
        return nullptr; // Out of memory in pool
    }
    uint8_t* ptr = &buffer[offset];
    offset += aligned_size;
    return ptr;
}

void MemoryPool::reset() {
    offset = 0;
}
