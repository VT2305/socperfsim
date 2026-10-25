#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace socperfsim {

struct TraceEntry {
    bool is_write = false;
    std::uint64_t address = 0;
    std::uint64_t compute_cycles = 0;
};

std::vector<TraceEntry> load_trace(const std::string& path);

}  // namespace socperfsim
