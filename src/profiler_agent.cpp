#include "profiler_agent.hpp"
#include "stack_unwinder.hpp"
#include "transport.hpp"
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

ProfilerAgent& ProfilerAgent::getInstance() {
    static ProfilerAgent instance;
    return instance;
}

ProfilerAgent::ProfilerAgent() {}

void ProfilerAgent::start(uint32_t sampling_interval_ms) {
    if (running) return;
    interval_ms = sampling_interval_ms;
    running = true;
    memory_pool.reset();
    sample_buffer.resetDroppedCount();
    
#ifdef __EMSCRIPTEN__
    // We would start a JS interval or use asyncify
    EM_ASM({
        console.log("Agent started with interval " + $0 + "ms");
    }, interval_ms);
#else
    std::cout << "Agent started with interval " << interval_ms << "ms" << std::endl;
#endif
}

void ProfilerAgent::sample() {
    if (!running) return;
    
    RawSample rs;
    // timestamp could be from emscripten_get_now() or native clock
    rs.timestamp = 0; // Placeholder
    rs.num_pcs = StackUnwinder::captureStackTrace(rs.pcs, 32);
    
    if (!sample_buffer.push(rs)) {
        // buffer full, dropped handled in RingBuffer
    }
}

void ProfilerAgent::stop() {
    if (!running) return;
    running = false;
    
    ProfileSerializer serializer;
    
    auto rs_opt = sample_buffer.pop();
    while (rs_opt.has_value()) {
        auto rs = rs_opt.value();
        SampleData sd;
        sd.timestamp = rs.timestamp;
        for (size_t i = 0; i < rs.num_pcs; ++i) {
            StackFrameData sf;
            sf.pc = rs.pcs[i];
            sf.module_id = 0;
            sf.func_name = StackUnwinder::getFuncName(rs.pcs[i]);
            sd.frames.push_back(sf);
        }
        serializer.addSample(sd);
        rs_opt = sample_buffer.pop();
    }
    
    auto fbs_data = serializer.serialize();
    
    ConsoleTransport transport;
    transport.sendChunked(fbs_data.data(), fbs_data.size());
    
    uint32_t dropped = sample_buffer.getDroppedCount();
    if (dropped > 0) {
#ifdef __EMSCRIPTEN__
        EM_ASM({
            console.warn("Dropped " + $0 + " samples (buffer full).");
        }, dropped);
#else
        std::cout << "[WARN] Dropped " << dropped << " samples (buffer full)." << std::endl;
#endif
    }
}

extern "C" {
    void start_agent(uint32_t interval_ms) {
        ProfilerAgent::getInstance().start(interval_ms);
    }
    void stop_agent() {
        ProfilerAgent::getInstance().stop();
    }
    bool is_agent_running() {
        return ProfilerAgent::getInstance().isRunning();
    }
}
