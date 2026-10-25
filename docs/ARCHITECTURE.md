# Architecture notes

## Cache model

Each cache is set-associative and stores:

- valid bit
- dirty bit
- tag
- last-use timestamp
- insertion timestamp

These timestamps support LRU and FIFO replacement.

## Hierarchy behaviour

CPU accesses probe L1 first. An L1 miss probes L2. An L2 miss fetches one cache line from DRAM.

The simulator uses write-back + write-allocate:

- writes that hit in L1 mark the line dirty
- a write miss fetches the line and installs it dirty in L1
- dirty L1 evictions write a full line to L2
- dirty L2 evictions generate DRAM writes

## Timing model

Memory-service cycles include cache lookup latency, DRAM fixed latency, and line-transfer time. Compute-cycle gaps from the trace are tracked separately and combined with memory cycles for total simulated execution time.

## Deliberate simplifications

The current model is blocking and does not model:

- out-of-order overlap
- multiple outstanding misses
- prefetchers
- coherence
- TLBs
- bank-level DRAM timing
- row-buffer locality
- cache inclusion policies

Those are suitable future extensions rather than hidden assumptions.
