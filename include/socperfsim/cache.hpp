#pragma once

#include "socperfsim/config.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <random>
#include <vector>

namespace socperfsim {

struct EvictedLine {
    std::uint64_t block_address = 0;
    bool dirty = false;
};

struct CacheAccessResult {
    bool hit = false;
    std::optional<EvictedLine> eviction;
};

class Cache {
public:
    explicit Cache(CacheConfig config, std::uint32_t seed = 1);

    CacheAccessResult access(std::uint64_t byte_address, bool is_write);
    CacheAccessResult install(std::uint64_t byte_address, bool dirty);

    std::size_t num_sets() const { return sets_.size(); }
    const CacheConfig& config() const { return config_; }

private:
    struct Line {
        bool valid = false;
        bool dirty = false;
        std::uint64_t tag = 0;
        std::uint64_t last_used = 0;
        std::uint64_t inserted_at = 0;
    };

    struct DecodedAddress {
        std::uint64_t block = 0;
        std::size_t set_index = 0;
        std::uint64_t tag = 0;
    };

    DecodedAddress decode(std::uint64_t byte_address) const;
    std::size_t choose_victim(std::size_t set_index);
    std::uint64_t line_base_address(std::uint64_t tag, std::size_t set_index) const;

    CacheConfig config_;
    std::vector<std::vector<Line>> sets_;
    std::uint64_t tick_ = 0;
    std::mt19937 rng_;
};

}  // namespace socperfsim
