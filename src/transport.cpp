#include "transport.hpp"
#include <iostream>
#include <algorithm>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

static std::string base64_encode(const uint8_t* data, size_t len) {
    static const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    result.reserve((len + 2) / 3 * 4);
    for (size_t i = 0; i < len; i += 3) {
        uint32_t val = (data[i] << 16) | (i + 1 < len ? data[i+1] << 8 : 0) | (i + 2 < len ? data[i+2] : 0);
        result.push_back(chars[(val >> 18) & 0x3F]);
        result.push_back(chars[(val >> 12) & 0x3F]);
        result.push_back(i + 1 < len ? chars[(val >> 6) & 0x3F] : '=');
        result.push_back(i + 2 < len ? chars[val & 0x3F] : '=');
    }
    return result;
}

void ConsoleTransport::send(const uint8_t* data, size_t len) {
    std::string b64 = base64_encode(data, len);
#ifdef __EMSCRIPTEN__
    EM_ASM_({
        console.log("PROFILE_DATA:" + UTF8ToString($0));
    }, b64.c_str());
#else
    std::cout << "PROFILE_DATA:" << b64 << std::endl;
#endif
}

void ConsoleTransport::sendChunked(const uint8_t* data, size_t total_size, size_t chunk_size) {
    size_t total_chunks = (total_size + chunk_size - 1) / chunk_size;
    for (size_t i = 0; i < total_chunks; ++i) {
        size_t offset = i * chunk_size;
        size_t len = std::min(chunk_size, total_size - offset);
        std::string b64 = base64_encode(data + offset, len);
#ifdef __EMSCRIPTEN__
        EM_ASM_({
            console.log("PROFILE_CHUNK:" + $1 + "/" + $2 + ":" + UTF8ToString($0));
        }, b64.c_str(), i, total_chunks);
#else
        std::cout << "PROFILE_CHUNK:" << i << "/" << total_chunks << ":" << b64 << std::endl;
#endif
    }
}

void StringTransport::send(const uint8_t* data, size_t len) {
    data_store.assign(reinterpret_cast<const char*>(data), len);
}

void StringTransport::sendChunked(const uint8_t* data, size_t total_size, size_t chunk_size) {
    data_store.append(reinterpret_cast<const char*>(data), total_size);
}
