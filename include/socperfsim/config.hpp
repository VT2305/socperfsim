#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <stdexcept>

namespace socperfsim {

enum class ReplacementPolicy { LRU, FIFO, RANDOM };

inline ReplacementPolicy parse_policy(const std::string& value) {
    if (value == "lru") return ReplacementPolicy::LRU;
    if (value == "fifo") return ReplacementPolicy::FIFO;
    if (value == "random") return ReplacementPolicy::RANDOM;
    throw std::invalid_argument("unknown replacement policy: " + value);
}

inline const char* policy_name(ReplacementPolicy policy) {
    switch (policy) {
        case ReplacementPolicy::LRU: return "lru";
        case ReplacementPolicy::FIFO: return "fifo";
        case ReplacementPolicy::RANDOM: return "random";
    }
    return "unknown";
}

struct CacheConfig {
    std::size_t capacity_bytes = 32 * 1024;
    std::size_t associativity = 8;
    std::size_t line_size = 64;
    std::uint64_t hit_latency_cycles = 4;
    ReplacementPolicy policy = ReplacementPolicy::LRU;
};

struct SimulatorConfig {
    CacheConfig l1{};
    CacheConfig l2{256 * 1024, 8, 64, 12, ReplacementPolicy::LRU};
    std::uint64_t dram_latency_cycles = 200;
    double dram_bandwidth_gbps = 25.0;
    double cpu_frequency_ghz = 3.0;
};

}  // namespace socperfsim
