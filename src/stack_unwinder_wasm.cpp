#include "stack_unwinder.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <cstring>
#include <string>

// String table is defined in stack_unwinder.cpp

size_t StackUnwinder::captureStackTrace(uint32_t* pc_buffer, size_t max_depth) {
    // In emscripten, emscripten_get_callstack can get a string representation.
    int flags = EMSCRIPTEN_CALL_STACK;
    size_t bytes = emscripten_get_callstack(flags, NULL, 0);
    if (bytes == 0) return 0;
    
    // Use a static buffer to avoid malloc during critical sampling
    static char stack_buffer[8192];
    if (bytes >= sizeof(stack_buffer)) bytes = sizeof(stack_buffer) - 1;
    
    emscripten_get_callstack(flags, stack_buffer, bytes + 1);
    
    // Parse stack string. Example line: "    at func_name (file.js:10:5)"
    size_t count = 0;
    char* line = strtok(stack_buffer, "\n");
    while (line != NULL && count < max_depth) {
        // Simple hash of line to act as a fake PC
        uint32_t hash = 5381;
        int c;
        char* str = line;
        while ((c = *str++))
            hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
            
        pc_buffer[count++] = hash;
        if (string_table.find(hash) == string_table.end()) {
            string_table[hash] = std::string(line);
        }
        line = strtok(NULL, "\n");
    }
    
    return count;
}
#endif
