#include "socperfsim/config.hpp"
#include "socperfsim/simulator.hpp"
#include "socperfsim/trace.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

using socperfsim::ReplacementPolicy;
using socperfsim::Simulator;
using socperfsim::SimulatorConfig;
using socperfsim::load_trace;
using socperfsim::parse_policy;

namespace {

std::size_t to_size(const std::string& value) {
    return static_cast<std::size_t>(std::stoull(value));
}

std::uint64_t to_u64(const std::string& value) {
    return static_cast<std::uint64_t>(std::stoull(value));
}

void print_usage() {
    std::cout <<
        "Usage: socperfsim TRACE.csv [options]\n"
        "Options:\n"
        "  --l1-size BYTES\n"
        "  --l1-assoc N\n"
        "  --l1-line BYTES\n"
        "  --l1-policy lru|fifo|random\n"
        "  --l1-latency CYCLES\n"
        "  --l2-size BYTES\n"
        "  --l2-assoc N\n"
        "  --l2-line BYTES\n"
        "  --l2-policy lru|fifo|random\n"
        "  --l2-latency CYCLES\n"
        "  --dram-latency-cycles CYCLES\n"
        "  --dram-bandwidth-gbps GBPS\n"
        "  --cpu-ghz GHZ\n"
        "  --json\n";
}

}  // namespace

int main(int argc, char** argv) {
    try {
        if (argc < 2) {
            print_usage();
            return 1;
        }

        const std::string trace_path = argv[1];
        SimulatorConfig config;
        bool json = false;

        for (int i = 2; i < argc; ++i) {
            const std::string arg = argv[i];
            auto require_value = [&](const char* name) -> std::string {
                if (i + 1 >= argc) throw std::runtime_error(std::string("missing value for ") + name);
                return argv[++i];
            };

            if (arg == "--l1-size") config.l1.capacity_bytes = to_size(require_value("--l1-size"));
            else if (arg == "--l1-assoc") config.l1.associativity = to_size(require_value("--l1-assoc"));
            else if (arg == "--l1-line") config.l1.line_size = to_size(require_value("--l1-line"));
            else if (arg == "--l1-policy") config.l1.policy = parse_policy(require_value("--l1-policy"));
            else if (arg == "--l1-latency") config.l1.hit_latency_cycles = to_u64(require_value("--l1-latency"));
            else if (arg == "--l2-size") config.l2.capacity_bytes = to_size(require_value("--l2-size"));
            else if (arg == "--l2-assoc") config.l2.associativity = to_size(require_value("--l2-assoc"));
            else if (arg == "--l2-line") config.l2.line_size = to_size(require_value("--l2-line"));
            else if (arg == "--l2-policy") config.l2.policy = parse_policy(require_value("--l2-policy"));
            else if (arg == "--l2-latency") config.l2.hit_latency_cycles = to_u64(require_value("--l2-latency"));
            else if (arg == "--dram-latency-cycles") config.dram_latency_cycles = to_u64(require_value("--dram-latency-cycles"));
            else if (arg == "--dram-bandwidth-gbps") config.dram_bandwidth_gbps = std::stod(require_value("--dram-bandwidth-gbps"));
            else if (arg == "--cpu-ghz") config.cpu_frequency_ghz = std::stod(require_value("--cpu-ghz"));
            else if (arg == "--json") json = true;
            else if (arg == "--help" || arg == "-h") {
                print_usage();
                return 0;
            } else {
                throw std::runtime_error("unknown option: " + arg);
            }
        }

        auto trace = load_trace(trace_path);
        Simulator simulator(config);
        simulator.run(trace);

        std::cout << (json ? simulator.to_json() : simulator.to_text());
        if (json) std::cout << "\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "error: " << ex.what() << "\n";
        return 2;
    }
}
