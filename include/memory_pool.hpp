#pragma once
#include <cstddef>
#include <cstdint>

class MemoryPool {
public:
    static constexpr size_t POOL_SIZE = 2 * 1024 * 1024; // 2MB
    
    MemoryPool();
    ~MemoryPool() = default;

    uint8_t* allocate(size_t size);
    void reset();
    size_t getUsed() const { return offset; }
    
private:
    uint8_t buffer[POOL_SIZE];
    size_t offset;
};
