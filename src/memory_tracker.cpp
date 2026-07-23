#include "memory_tracker.hpp"
#include <cstdlib>

#if !defined(__EMSCRIPTEN__)
thread_local bool inside_malloc = false;
#else
// WebAssembly doesn't have true thread_local without pthreads, but we can simulate or use global
bool inside_malloc = false;
#endif

LinearMemoryGuard& LinearMemoryGuard::getInstance() {
    static LinearMemoryGuard instance;
    return instance;
}

void LinearMemoryGuard::recordAlloc(uintptr_t ptr, size_t size) {
    if (ptr == 0) return;
    buffer.push({ptr, size, false});
    size_t idx = (ptr >> 3) % HASH_SIZE;
    size_t start_idx = idx;
    while (allocations[idx].active) {
        idx = (idx + 1) % HASH_SIZE;
        if (idx == start_idx) break; // full
    }
    allocations[idx] = {ptr, size, true};
}

void LinearMemoryGuard::recordFree(uintptr_t ptr) {
    if (ptr == 0) return;
    buffer.push({ptr, 0, true});
    size_t idx = (ptr >> 3) % HASH_SIZE;
    size_t start_idx = idx;
    while (allocations[idx].active) {
        if (allocations[idx].ptr == ptr) {
            allocations[idx].active = false;
            break;
        }
        idx = (idx + 1) % HASH_SIZE;
        if (idx == start_idx) break;
    }
}

extern "C" {
    // Override malloc/free
    void* __wrap_malloc(size_t size) {
        extern void* __real_malloc(size_t);
        if (inside_malloc) return __real_malloc(size);
        inside_malloc = true;
        void* ptr = __real_malloc(size);
        if (ptr) {
            LinearMemoryGuard::getInstance().recordAlloc(reinterpret_cast<uintptr_t>(ptr), size);
        }
        inside_malloc = false;
        return ptr;
    }

    void __wrap_free(void* ptr) {
        extern void __real_free(void*);
        if (inside_malloc || !ptr) {
            __real_free(ptr);
            return;
        }
        inside_malloc = true;
        LinearMemoryGuard::getInstance().recordFree(reinterpret_cast<uintptr_t>(ptr));
        __real_free(ptr);
        inside_malloc = false;
    }
}
