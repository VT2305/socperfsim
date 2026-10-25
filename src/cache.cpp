#include "socperfsim/cache.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace socperfsim {

Cache::Cache(CacheConfig config, std::uint32_t seed)
    : config_(config), rng_(seed) {
    if (config_.capacity_bytes == 0 || config_.associativity == 0 || config_.line_size == 0) {
        throw std::invalid_argument("cache capacity, associativity, and line size must be non-zero");
    }
    if (config_.capacity_bytes % (config_.associativity * config_.line_size) != 0) {
        throw std::invalid_argument("cache capacity must be divisible by associativity * line size");
    }
    const std::size_t set_count =
        config_.capacity_bytes / (config_.associativity * config_.line_size);
    if (set_count == 0) {
        throw std::invalid_argument("cache must contain at least one set");
    }
    sets_.assign(set_count, std::vector<Line>(config_.associativity));
}

Cache::DecodedAddress Cache::decode(std::uint64_t byte_address) const {
    const std::uint64_t block = byte_address / config_.line_size;
    const std::size_t set_index = static_cast<std::size_t>(block % sets_.size());
    const std::uint64_t tag = block / sets_.size();
    return {block, set_index, tag};
}

std::uint64_t Cache::line_base_address(std::uint64_t tag, std::size_t set_index) const {
    const std::uint64_t block = tag * sets_.size() + set_index;
    return block * config_.line_size;
}

std::size_t Cache::choose_victim(std::size_t set_index) {
    auto& set = sets_.at(set_index);
    for (std::size_t way = 0; way < set.size(); ++way) {
        if (!set[way].valid) return way;
    }

    if (config_.policy == ReplacementPolicy::RANDOM) {
        std::uniform_int_distribution<std::size_t> dist(0, set.size() - 1);
        return dist(rng_);
    }

    std::size_t victim = 0;
    std::uint64_t best = std::numeric_limits<std::uint64_t>::max();
    for (std::size_t way = 0; way < set.size(); ++way) {
        const std::uint64_t value =
            config_.policy == ReplacementPolicy::LRU ? set[way].last_used : set[way].inserted_at;
        if (value < best) {
            best = value;
            victim = way;
        }
    }
    return victim;
}

CacheAccessResult Cache::access(std::uint64_t byte_address, bool is_write) {
    ++tick_;
    const auto decoded = decode(byte_address);
    auto& set = sets_.at(decoded.set_index);

    for (auto& line : set) {
        if (line.valid && line.tag == decoded.tag) {
            line.last_used = tick_;
            if (is_write) line.dirty = true;
            return {true, std::nullopt};
        }
    }
    return {false, std::nullopt};
}

CacheAccessResult Cache::install(std::uint64_t byte_address, bool dirty) {
    ++tick_;
    const auto decoded = decode(byte_address);
    auto& set = sets_.at(decoded.set_index);

    for (auto& line : set) {
        if (line.valid && line.tag == decoded.tag) {
            line.last_used = tick_;
            line.dirty = line.dirty || dirty;
            return {true, std::nullopt};
        }
    }

    const std::size_t victim_index = choose_victim(decoded.set_index);
    auto& victim = set[victim_index];

    std::optional<EvictedLine> eviction;
    if (victim.valid) {
        eviction = EvictedLine{
            line_base_address(victim.tag, decoded.set_index),
            victim.dirty
        };
    }

    victim.valid = true;
    victim.dirty = dirty;
    victim.tag = decoded.tag;
    victim.last_used = tick_;
    victim.inserted_at = tick_;

    return {false, eviction};
}

}  // namespace socperfsim
