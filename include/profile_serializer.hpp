#pragma once
#include <cstdint>
#include <vector>
#include <string>

struct StackFrameData {
    uint32_t pc;
    uint16_t module_id;
    std::string func_name;
};

struct SampleData {
    uint64_t timestamp;
    std::vector<StackFrameData> frames;
};

class ProfileSerializer {
public:
    void addSample(const SampleData& sample);
    std::vector<uint8_t> serialize();
private:
    std::vector<SampleData> samples;
};
