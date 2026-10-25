#include "socperfsim/cache.hpp"
#include "socperfsim/config.hpp"

#include <cstdlib>
#include <iostream>

using socperfsim::Cache;
using socperfsim::CacheConfig;
using socperfsim::ReplacementPolicy;

namespace {

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        std::exit(1);
    }
}

void test_basic_hit() {
    CacheConfig cfg{128, 1, 64, 1, ReplacementPolicy::LRU};
    Cache cache(cfg);

    expect(!cache.access(0x0000, false).hit, "cold access should miss");
    cache.install(0x0000, false);
    expect(cache.access(0x0000, false).hit, "installed line should hit");
    expect(cache.access(0x003F, false).hit, "same cache line should hit");
}

void test_lru_replacement() {
    // One set, two ways, 64 B lines.
    CacheConfig cfg{128, 2, 64, 1, ReplacementPolicy::LRU};
    Cache cache(cfg);

    cache.install(0x0000, false);  // A
    cache.install(0x0040, false);  // B
    expect(cache.access(0x0000, false).hit, "A should hit");  // A becomes MRU

    auto result = cache.install(0x0080, false);  // C should evict B
    expect(result.eviction.has_value(), "install should evict a line");
    expect(result.eviction->block_address == 0x0040, "LRU should evict B");
    expect(cache.access(0x0000, false).hit, "A should remain");
    expect(!cache.access(0x0040, false).hit, "B should be evicted");
    expect(cache.access(0x0080, false).hit, "C should be present");
}

void test_dirty_eviction() {
    CacheConfig cfg{64, 1, 64, 1, ReplacementPolicy::LRU};
    Cache cache(cfg);

    cache.install(0x0000, false);
    expect(cache.access(0x0000, true).hit, "write should hit and mark dirty");
    auto result = cache.install(0x0040, false);
    expect(result.eviction.has_value(), "replacement should evict");
    expect(result.eviction->dirty, "evicted line should be dirty");
}

}  // namespace

int main() {
    test_basic_hit();
    test_lru_replacement();
    test_dirty_eviction();
    std::cout << "All cache tests passed.\n";
    return 0;
}
