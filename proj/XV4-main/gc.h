#ifndef GC_H
#define GC_H

#include "types.h"

// Garbage collection functions
void gc_init(void);
int gc_run(void);
int gc_collect_versions(void);
int gc_collect_blocks(void);

// Version pruning
int gc_prune_old_versions(uint age_threshold);
int gc_prune_by_count(uint max_versions);

// Block reference counting
void bref_init(void);
int bref_inc(uint block_num);
int bref_dec(uint block_num);
uint bref_get(uint block_num);
void bref_set(uint block_num, uint count);

// Deduplication functions
uint dedup_hash(char *data, uint len);
uint dedup_find(uint checksum);
int dedup_insert(uint checksum, uint block_num);
int dedup_remove(uint block_num);
void dedup_init(void);

// GC statistics
struct gc_stats {
  uint blocks_freed;
  uint versions_pruned;
  uint last_run_time;
  uint total_runs;
};

extern struct gc_stats gc_statistics;

#endif // GC_H
