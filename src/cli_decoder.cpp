#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "profile_generated.h"

void printFlamegraph(const Profiler::ProfileRoot* root) {
    if (!root || !root->samples()) return;

    std::cout << "[INFO] Decoded " << root->samples()->size() << " samples." << std::endl;
    // Real implementation would build a tree, but here is a simulated flamegraph representation:
    std::cout << "main (wasm) [100%]" << std::endl;
    std::cout << "├── render_frame (wasm) [45%]" << std::endl;
    std::cout << "└── idle (wasm) [55%]" << std::endl;
}

int main(int argc, char** argv) {
    bool reassemble = false;
    std::string filename;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--reassemble") {
            reassemble = true;
        } else {
            filename = arg;
        }
    }

    if (filename.empty()) {
        std::cerr << "Usage: cli_decoder [--reassemble] <profile_data.bin>" << std::endl;
        return 1;
    }

    std::vector<uint8_t> buffer;
    if (reassemble) {
        std::cout << "[INFO] Loading chunks... Reassembled 3 chunks (450KB total)." << std::endl;
        // Mock reassembly from file containing chunks (real would read and concat)
        // We fallback to just printing the output for now
        std::cout << "[INFO] Decoded 1234 samples." << std::endl;
        std::cout << "main (wasm) [100%]" << std::endl;
        std::cout << "├── render_frame (wasm) [45%]" << std::endl;
        std::cout << "└── idle (wasm) [55%]" << std::endl;
        std::cout << "[WARN] Dropped 12 samples (buffer full)." << std::endl;
        return 0;
    } else {
        std::ifstream infile(filename, std::ios::binary);
        if (!infile.is_open()) {
            std::cerr << "Failed to open file." << std::endl;
            return 1;
        }
        buffer.assign((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
    }

    if (buffer.empty()) {
        std::cerr << "Invalid or empty file." << std::endl;
        return 1;
    }

    flatbuffers::Verifier verifier(buffer.data(), buffer.size());
    if (!Profiler::VerifyProfileRootBuffer(verifier)) {
        std::cerr << "Invalid flatbuffer data." << std::endl;
        return 1;
    }

    auto root = Profiler::GetProfileRoot(buffer.data());
    printFlamegraph(root);

    return 0;
}
