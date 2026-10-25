#include "socperfsim/trace.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace socperfsim {

namespace {

std::string trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::uint64_t parse_u64(const std::string& value) {
    std::size_t consumed = 0;
    const std::uint64_t parsed = std::stoull(value, &consumed, 0);
    if (consumed != value.size()) {
        throw std::runtime_error("invalid integer: " + value);
    }
    return parsed;
}

}  // namespace

std::vector<TraceEntry> load_trace(const std::string& path) {
    std::ifstream input(path);
    if (!input) throw std::runtime_error("unable to open trace: " + path);

    std::vector<TraceEntry> entries;
    std::string line;
    std::size_t line_number = 0;

    while (std::getline(input, line)) {
        ++line_number;
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        std::string op;
        std::string address;
        std::string compute;

        if (!std::getline(ss, op, ',') || !std::getline(ss, address, ',')) {
            throw std::runtime_error("invalid trace line " + std::to_string(line_number));
        }

        op = trim(op);
        address = trim(address);

        if (op == "op" || op == "OP") continue;

        std::getline(ss, compute, ',');
        compute = trim(compute);

        if (op != "R" && op != "W" && op != "r" && op != "w") {
            throw std::runtime_error("invalid operation on line " + std::to_string(line_number));
        }

        TraceEntry entry;
        entry.is_write = (op == "W" || op == "w");
        entry.address = parse_u64(address);
        entry.compute_cycles = compute.empty() ? 0 : parse_u64(compute);
        entries.push_back(entry);
    }

    return entries;
}

}  // namespace socperfsim
