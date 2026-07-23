#pragma once
#include <cstdint>
#include "memory_pool.hpp"
#include "ring_buffer.hpp"
#include "profile_serializer.hpp"

class ProfilerAgent {
public:
    static ProfilerAgent& getInstance();
    
    void start(uint32_t sampling_interval_ms);
    void stop();
    bool isRunning() const { return running; }
    
    // Called by sampler
    void sample();

private:
    ProfilerAgent();
    ~ProfilerAgent() = default;

    bool running = false;
    uint32_t interval_ms = 0;
    MemoryPool memory_pool;
    
    struct RawSample {
        uint64_t timestamp;
        uint32_t pcs[32]; // Max depth 32
        size_t num_pcs;
    };
    RingBuffer<RawSample, 1024> sample_buffer;
};

extern "C" {
    void start_agent(uint32_t interval_ms);
    void stop_agent();
    bool is_agent_running();
}
