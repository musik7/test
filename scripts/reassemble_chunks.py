#!/usr/bin/env python3
import sys
import re
import base64

def reassemble(log_file, output_file):
    chunks = {}
    total_expected = -1
    pattern = re.compile(r"PROFILE_CHUNK:(\d+)/(\d+):(.*)")
    
    with open(log_file, 'r') as f:
        for line in f:
            match = pattern.search(line)
            if match:
                idx = int(match.group(1))
                total = int(match.group(2))
                b64_data = match.group(3)
                
                if total_expected == -1:
                    total_expected = total
                elif total_expected != total:
                    print("Error: chunk total mismatch")
                    return
                
                chunks[idx] = base64.b64decode(b64_data)
                
    if len(chunks) != total_expected:
        print(f"Error: missing chunks. Expected {total_expected}, got {len(chunks)}")
        return
        
    print(f"[INFO] Loading chunks... Reassembled {total_expected} chunks ({sum(len(c) for c in chunks.values())//1024}KB total).")
    
    with open(output_file, 'wb') as f:
        for i in range(total_expected):
            f.write(chunks[i])
            
    print(f"Data saved to {output_file}")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: reassemble_chunks.py <input.log> <output.bin>")
        sys.exit(1)
    reassemble(sys.argv[1], sys.argv[2])
