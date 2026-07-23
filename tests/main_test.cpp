#include <gtest/gtest.h>
#include "ring_buffer.hpp"
#include "stack_unwinder.hpp"
#include "memory_tracker.hpp"
#include "transport.hpp"
#include "profile_serializer.hpp"
#include "memory_pool.hpp"
#include "profiler_agent.hpp"

TEST(RingBufferTest, PushPopDrop) {
    RingBuffer<int, 4> rb;
    EXPECT_TRUE(rb.push(1));
    EXPECT_TRUE(rb.push(2));
    EXPECT_TRUE(rb.push(3));
    EXPECT_FALSE(rb.push(4));
    EXPECT_EQ(rb.getDroppedCount(), 1);

    auto v1 = rb.pop();
    EXPECT_TRUE(v1.has_value());
    EXPECT_EQ(v1.value(), 1);
}

TEST(StackUnwinderTest, CaptureNative) {
    uint32_t buffer[10];
    size_t count = StackUnwinder::captureStackTrace(buffer, 10);
    EXPECT_GT(count, 0); // Should capture at least something native
}

TEST(MemoryTrackerTest, HashRecord) {
    auto& tracker = LinearMemoryGuard::getInstance();
    tracker.recordAlloc(0xDEADBEEF, 128);
    auto record = tracker.getBuffer().pop();
    EXPECT_TRUE(record.has_value());
    EXPECT_EQ(record.value().ptr, 0xDEADBEEF);
    EXPECT_EQ(record.value().size, 128);
    EXPECT_FALSE(record.value().is_free);
}

TEST(TransportTest, ChunkedStringTransport) {
    StringTransport transport;
    std::vector<uint8_t> data(100 * 1024, 0xAA); // 100KB
    transport.sendChunked(data.data(), data.size(), 64 * 1024);
    EXPECT_EQ(transport.getData().size(), 100 * 1024); // Should have appended fully
}

TEST(SerializerTest, BasicSerialization) {
    ProfileSerializer serializer;
    SampleData sample;
    sample.timestamp = 1000;
    sample.frames = {{0x1000, 1, "main"}};
    serializer.addSample(sample);
    
    auto data = serializer.serialize();
    EXPECT_FALSE(data.empty());
}

TEST(MemoryPoolTest, Allocation) {
    MemoryPool pool;
    uint8_t* p1 = pool.allocate(10);
    EXPECT_NE(p1, nullptr);
    uint8_t* p2 = pool.allocate(20);
    EXPECT_NE(p2, nullptr);
    // 8-byte aligned check
    EXPECT_EQ(reinterpret_cast<uintptr_t>(p2) - reinterpret_cast<uintptr_t>(p1), 16);
}

TEST(ProfilerAgentTest, Lifecycle) {
    auto& agent = ProfilerAgent::getInstance();
    EXPECT_FALSE(agent.isRunning());
    agent.start(10);
    EXPECT_TRUE(agent.isRunning());
    agent.sample();
    agent.stop();
    EXPECT_FALSE(agent.isRunning());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
