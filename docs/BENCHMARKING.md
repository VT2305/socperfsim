# Benchmarking plan

The repository includes synthetic workloads so the simulator can be exercised without proprietary traces.

A stronger portfolio study should report results only after the sweep scripts have actually been run.

Suggested experiments:

1. **Capacity sweep** — L1 16/32/64 KiB on compute-like and streaming traces.
2. **Associativity sweep** — 1/2/4/8 ways on a conflict-heavy trace.
3. **Replacement policy** — LRU vs FIFO vs random.
4. **DRAM sensitivity** — vary fixed latency and bandwidth independently.
5. **Workload comparison** — compare locality and memory traffic across compute-like, streaming, and AR-like synthetic patterns.

Recommended outputs:

- hit rate vs cache size
- AMAT vs cache size
- DRAM traffic vs line size
- total cycles vs configuration
- Pareto plot of cache capacity vs AMAT

Do not label synthetic traces as real application measurements.
