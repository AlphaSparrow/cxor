# CXOR

A lightweight, pure C++20 prototype of MIT's COPE wireless network coding protocol that combines bidirectional network flows using bitwise XOR into single broadcast transmissions.

## Overview

In a classic two-way relay topology (`Node A <-> Relay R <-> Node B`):
- **Normal forwarding** requires 4 transmissions to exchange two packets (`A -> R`, `B -> R`, `R -> B`, `R -> A`).
- **XOR network coding** reduces this to 3 transmissions (`A -> R`, `B -> R`, `R -> broadcast (A ^ B)`), because `Node A` and `Node B` each retain the packet they originated and recover the missing packet by XORing the coded frame with their known side information.

## Project Structure

```text
cxor/
├── CMakeLists.txt
├── include/
│   ├── Packet.hpp     # Packet metadata, byte payload, constituent tracking
│   ├── Encoder.hpp    # Bytewise XOR engine (2-packet and N-packet coding)
│   ├── Decoder.hpp    # Side-information XOR recovery
│   ├── Node.hpp       # Outbox queue, known-packet cache, delivery handling
│   ├── Relay.hpp      # Bidirectional coding opportunity detection & forwarding
│   └── Network.hpp    # Topology simulation & Normal vs Coded metrics
├── src/
│   ├── Packet.cpp
│   ├── Encoder.cpp
│   ├── Decoder.cpp
│   ├── Node.cpp
│   ├── Relay.cpp
│   ├── Network.cpp
│   └── main.cpp       # Terminal comparison & CSV metrics exporter
├── tests/
│   └── tests.cpp      # Unit & integration tests via CTest
└── results/           # Generated CSV metrics (metrics.csv)
```

## Build & Run

Requires a C++20 compiler and CMake 3.20+.

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/CXOR
```

Running `./build/CXOR` executes symmetric, asymmetric, and mixed traffic scenarios, prints a Normal vs. Coded comparison table, and writes `results/metrics.csv`.

## License

See [LICENSE](LICENSE).
