#include "socperfsim/simulator.hpp"

#include <cmath>
#include <iomanip>
#include <sstream>

namespace socperfsim {

double Metrics::l1_hit_rate() const {
    return accesses == 0 ? 0.0 : static_cast<double>(l1_hits) / accesses;
}

double Metrics::l2_hit_rate() const {
    const auto l2_accesses = l2_hits + l2_misses;
    return l2_accesses == 0 ? 0.0 : static_cast<double>(l2_hits) / l2_accesses;
}

double Metrics::amat_cycles() const {
    return accesses == 0 ? 0.0 : static_cast<double>(memory_cycles) / accesses;
}

Simulator::Simulator(SimulatorConfig config)
    : config_(config), l1_(config_.l1, 11), l2_(config_.l2, 29) {}

std::uint64_t Simulator::dram_transfer_cycles(std::size_t bytes) const {
    const double cpu_hz = config_.cpu_frequency_ghz * 1e9;
    const double bytes_per_second = config_.dram_bandwidth_gbps * 1e9;
    const double bytes_per_cycle = bytes_per_second / cpu_hz;
    if (bytes_per_cycle <= 0.0) return 0;
    return static_cast<std::uint64_t>(std::ceil(bytes / bytes_per_cycle));
}

void Simulator::dram_read_line() {
    ++metrics_.dram_reads;
    metrics_.bytes_from_dram += config_.l2.line_size;
    metrics_.memory_cycles += config_.dram_latency_cycles + dram_transfer_cycles(config_.l2.line_size);
}

void Simulator::dram_write_line() {
    ++metrics_.dram_writes;
    metrics_.bytes_to_dram += config_.l2.line_size;
    metrics_.memory_cycles += config_.dram_latency_cycles + dram_transfer_cycles(config_.l2.line_size);
}

void Simulator::install_in_l2(std::uint64_t address, bool dirty) {
    auto installed = l2_.install(address, dirty);
    if (installed.eviction && installed.eviction->dirty) {
        dram_write_line();
    }
}

void Simulator::writeback_l1_line(const EvictedLine& eviction) {
    auto probe = l2_.access(eviction.block_address, true);
    metrics_.memory_cycles += config_.l2.hit_latency_cycles;

    if (probe.hit) {
        return;
    }

    // A full-line L1 writeback can be installed directly into L2; no DRAM read is required.
    install_in_l2(eviction.block_address, true);
}

void Simulator::process(const TraceEntry& entry) {
    ++metrics_.accesses;
    entry.is_write ? ++metrics_.writes : ++metrics_.reads;
    metrics_.compute_cycles += entry.compute_cycles;

    // L1 lookup.
    metrics_.memory_cycles += config_.l1.hit_latency_cycles;
    auto l1_result = l1_.access(entry.address, entry.is_write);

    if (l1_result.hit) {
        ++metrics_.l1_hits;
        metrics_.total_cycles = metrics_.compute_cycles + metrics_.memory_cycles;
        return;
    }

    ++metrics_.l1_misses;

    // L2 lookup.
    metrics_.memory_cycles += config_.l2.hit_latency_cycles;
    auto l2_result = l2_.access(entry.address, false);

    if (l2_result.hit) {
        ++metrics_.l2_hits;
    } else {
        ++metrics_.l2_misses;
        dram_read_line();
        install_in_l2(entry.address, false);
    }

    // Bring line into L1 and mark it dirty if the original CPU operation was a write.
    auto installed_l1 = l1_.install(entry.address, entry.is_write);
    if (installed_l1.eviction && installed_l1.eviction->dirty) {
        writeback_l1_line(*installed_l1.eviction);
    }

    metrics_.total_cycles = metrics_.compute_cycles + metrics_.memory_cycles;
}

Metrics Simulator::run(const std::vector<TraceEntry>& entries) {
    for (const auto& entry : entries) process(entry);
    return metrics_;
}

double Simulator::simulated_time_ns() const {
    if (config_.cpu_frequency_ghz <= 0.0) return 0.0;
    return static_cast<double>(metrics_.total_cycles) / config_.cpu_frequency_ghz;
}

std::string Simulator::to_json() const {
    std::ostringstream out;
    out << std::fixed << std::setprecision(6);
    out << "{";
    out << "\"accesses\":" << metrics_.accesses << ",";
    out << "\"reads\":" << metrics_.reads << ",";
    out << "\"writes\":" << metrics_.writes << ",";
    out << "\"l1_hits\":" << metrics_.l1_hits << ",";
    out << "\"l1_misses\":" << metrics_.l1_misses << ",";
    out << "\"l1_hit_rate\":" << metrics_.l1_hit_rate() << ",";
    out << "\"l2_hits\":" << metrics_.l2_hits << ",";
    out << "\"l2_misses\":" << metrics_.l2_misses << ",";
    out << "\"l2_hit_rate\":" << metrics_.l2_hit_rate() << ",";
    out << "\"dram_reads\":" << metrics_.dram_reads << ",";
    out << "\"dram_writes\":" << metrics_.dram_writes << ",";
    out << "\"bytes_from_dram\":" << metrics_.bytes_from_dram << ",";
    out << "\"bytes_to_dram\":" << metrics_.bytes_to_dram << ",";
    out << "\"compute_cycles\":" << metrics_.compute_cycles << ",";
    out << "\"memory_cycles\":" << metrics_.memory_cycles << ",";
    out << "\"total_cycles\":" << metrics_.total_cycles << ",";
    out << "\"amat_cycles\":" << metrics_.amat_cycles() << ",";
    out << "\"simulated_time_ns\":" << simulated_time_ns();
    out << "}";
    return out.str();
}

std::string Simulator::to_text() const {
    std::ostringstream out;
    out << std::fixed << std::setprecision(3);
    out << "Accesses:             " << metrics_.accesses << "\n";
    out << "L1 hit rate:          " << 100.0 * metrics_.l1_hit_rate() << "%\n";
    out << "L2 hit rate:          " << 100.0 * metrics_.l2_hit_rate() << "%\n";
    out << "DRAM reads:           " << metrics_.dram_reads << "\n";
    out << "DRAM writes:          " << metrics_.dram_writes << "\n";
    out << "DRAM bytes read:      " << metrics_.bytes_from_dram << "\n";
    out << "DRAM bytes written:   " << metrics_.bytes_to_dram << "\n";
    out << "AMAT:                 " << metrics_.amat_cycles() << " cycles\n";
    out << "Memory stall cycles:  " << metrics_.memory_cycles << "\n";
    out << "Compute cycles:       " << metrics_.compute_cycles << "\n";
    out << "Total cycles:         " << metrics_.total_cycles << "\n";
    out << "Simulated time:       " << simulated_time_ns() << " ns\n";
    return out.str();
}

}  // namespace socperfsim
