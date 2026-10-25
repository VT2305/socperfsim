# Trace format

SoCPerfSim consumes a simple CSV trace:

```text
op,address,compute_cycles
R,0x1000,12
W,0x1040,3
```

`op` is `R` or `W`.

`address` is a byte address and may be hexadecimal (`0x1000`) or decimal.

`compute_cycles` is optional and represents simulated CPU work performed since the previous memory operation. It allows total simulated cycles to include both compute gaps and memory service time.

The format is intentionally small and human-readable so traces can be generated from scripts or adapted from other sources.
