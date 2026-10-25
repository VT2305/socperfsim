#pragma once

#include "socperfsim/cache.hpp"
#include "socperfsim/config.hpp"
#include "socperfsim/trace.hpp"

#include <cstdint>
#include <string>

namespace socperfsim {

struct Metrics {
    std::uint64_t accesses = 0;
    std::uint64_t reads = 0;
    std::uint64_t writes = 0;

    std::uint64_t l1_hits = 0;
    std::uint64_t l1_misses = 0;
    std::uint64_t l2_hits = 0;
    std::uint64_t l2_misses = 0;

    std::uint64_t dram_reads = 0;
    std::uint64_t dram_writes = 0;
    std::uint64_t bytes_from_dram = 0;
    std::uint64_t bytes_to_dram = 0;

    std::uint64_t compute_cycles = 0;
    std::uint64_t memory_cycles = 0;
    std::uint64_t total_cycles = 0;

    double l1_hit_rate() const;
    double l2_hit_rate() const;
    double amat_cycles() const;
};

class Simulator {
public:
    explicit Simulator(SimulatorConfig config);

    void process(const TraceEntry& entry);
    Metrics run(const std::vector<TraceEntry>& entries);

    const Metrics& metrics() const { return metrics_; }
    double simulated_time_ns() const;
    std::string to_json() const;
    std::string to_text() const;

private:
    std::uint64_t dram_transfer_cycles(std::size_t bytes) const;
    void dram_read_line();
    void dram_write_line();
    void install_in_l2(std::uint64_t address, bool dirty);
    void writeback_l1_line(const EvictedLine& eviction);

    SimulatorConfig config_;
    Cache l1_;
    Cache l2_;
    Metrics metrics_;
};

}  // namespace socperfsim
