#include "stack_unwinder.hpp"

#if !defined(__EMSCRIPTEN__)
#include <execinfo.h>
#include <cstdlib>
#endif

std::unordered_map<uint32_t, std::string> StackUnwinder::string_table;

#if !defined(__EMSCRIPTEN__)
size_t StackUnwinder::captureStackTrace(uint32_t* pc_buffer, size_t max_depth) {
    void* buffer[128];
    int nptrs = backtrace(buffer, max_depth > 128 ? 128 : max_depth);
    char** strings = backtrace_symbols(buffer, nptrs);
    
    size_t count = 0;
    if (strings != nullptr) {
        for (int j = 0; j < nptrs; j++) {
            uint32_t pc = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(buffer[j]) & 0xFFFFFFFF);
            pc_buffer[count++] = pc;
            if (string_table.find(pc) == string_table.end()) {
                string_table[pc] = std::string(strings[j]);
            }
        }
        free(strings);
    }
    return count;
}
#endif

const char* StackUnwinder::getFuncName(uint32_t pc) {
    auto it = string_table.find(pc);
    if (it != string_table.end()) {
        return it->second.c_str();
    }
    return "unknown";
}
