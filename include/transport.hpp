#pragma once
#include <cstdint>
#include <cstddef>
#include <string>

class ITransport {
public:
    virtual ~ITransport() = default;
    virtual void send(const uint8_t* data, size_t len) = 0;
    virtual void sendChunked(const uint8_t* data, size_t total_size, size_t chunk_size = 64 * 1024) = 0;
};

class ConsoleTransport : public ITransport {
public:
    void send(const uint8_t* data, size_t len) override;
    void sendChunked(const uint8_t* data, size_t total_size, size_t chunk_size = 64 * 1024) override;
};

class StringTransport : public ITransport {
public:
    void send(const uint8_t* data, size_t len) override;
    void sendChunked(const uint8_t* data, size_t total_size, size_t chunk_size = 64 * 1024) override;
    std::string getData() const { return data_store; }
    void clear() { data_store.clear(); }
private:
    std::string data_store;
};
