#include "profile_serializer.hpp"
#include "profile_generated.h"
#include "flatbuffers/flatbuffers.h"

void ProfileSerializer::addSample(const SampleData& sample) {
    samples.push_back(sample);
}

std::vector<uint8_t> ProfileSerializer::serialize() {
    flatbuffers::FlatBufferBuilder builder;

    std::vector<flatbuffers::Offset<Profiler::Sample>> fbs_samples;
    for (const auto& sample : samples) {
        std::vector<flatbuffers::Offset<Profiler::StackFrame>> fbs_frames;
        for (const auto& frame : sample.frames) {
            auto func_name_str = builder.CreateString(frame.func_name);
            auto fbs_frame = Profiler::CreateStackFrame(builder, frame.pc, frame.module_id, func_name_str);
            fbs_frames.push_back(fbs_frame);
        }
        auto frames_vector = builder.CreateVector(fbs_frames);
        auto fbs_sample = Profiler::CreateSample(builder, sample.timestamp, frames_vector);
        fbs_samples.push_back(fbs_sample);
    }

    auto samples_vector = builder.CreateVector(fbs_samples);
    auto profile_root = Profiler::CreateProfileRoot(builder, samples_vector);
    
    builder.Finish(profile_root);

    uint8_t* buf = builder.GetBufferPointer();
    int size = builder.GetSize();
    return std::vector<uint8_t>(buf, buf + size);
}
