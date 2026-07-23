#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <unordered_map>

class StackUnwinder {
public:
    static size_t captureStackTrace(uint32_t* pc_buffer, size_t max_depth);
    static const char* getFuncName(uint32_t pc);
private:
    static std::unordered_map<uint32_t, std::string> string_table;
};
