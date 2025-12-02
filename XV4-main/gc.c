// Garbage collection, reference counting, and deduplication for ChronoFS
#include "types.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "gc.h"

// Block reference counting table
#define MAX_REFCOUNT_ENTRIES 10000
struct {
  struct spinlock lock;
  struct block_refcount entries[MAX_REFCOUNT_ENTRIES];
} refcount_table;

// Deduplication hash table
#define DEDUP_TABLE_SIZE 1000
struct {
  struct spinlock lock;
  struct dedup_entry entries[DEDUP_TABLE_SIZE];
} dedup_table;

// GC statistics
struct gc_stats gc_statistics;

// Initialize reference counting system
void
bref_init(void)
{
  initlock(&refcount_table.lock, "refcount");
  
  acquire(&refcount_table.lock);
  for(int i = 0; i < MAX_REFCOUNT_ENTRIES; i++){
    refcount_table.entries[i].valid = 0;
    refcount_table.entries[i].refcount = 0;
  }
  release(&refcount_table.lock);
}

// Find or create refcount entry for a block
static struct block_refcount*
bref_find_or_create(uint block_num)
{
  struct block_refcount *entry = 0;
  
  // First try to find existing entry
  for(int i = 0; i < MAX_REFCOUNT_ENTRIES; i++){
    if(refcount_table.entries[i].valid && 
       refcount_table.entries[i].block_num == block_num){
      return &refcount_table.entries[i];
    }
  }
  
  // Create new entry
  for(int i = 0; i < MAX_REFCOUNT_ENTRIES; i++){
    if(!refcount_table.entries[i].valid){
      entry = &refcount_table.entries[i];
      entry->valid = 1;
      entry->block_num = block_num;
      entry->refcount = 0;
      entry->checksum = 0;
      return entry;
    }
  }
  
  return 0; // Table full
}

// Increment block reference count
int
bref_inc(uint block_num)
{
  acquire(&refcount_table.lock);
  
  struct block_refcount *entry = bref_find_or_create(block_num);
  if(entry == 0){
    release(&refcount_table.lock);
    return -1; // Table full
  }
  
  entry->refcount++;
  release(&refcount_table.lock);
  return entry->refcount;
}

// Decrement block reference count
int
bref_dec(uint block_num)
{
  acquire(&refcount_table.lock);
  
  struct block_refcount *entry = 0;
  for(int i = 0; i < MAX_REFCOUNT_ENTRIES; i++){
    if(refcount_table.entries[i].valid && 
       refcount_table.entries[i].block_num == block_num){
      entry = &refcount_table.entries[i];
      break;
    }
  }
  
  if(entry == 0){
    release(&refcount_table.lock);
    return 0; // Not found
  }
  
  if(entry->refcount > 0)
    entry->refcount--;
  
  uint count = entry->refcount;
  
  // If refcount reaches 0, mark entry as invalid
  if(entry->refcount == 0){
    entry->valid = 0;
  }
  
  release(&refcount_table.lock);
  return count;
}

// Check if a block is tracked in the refcount table
int
bref_is_tracked(uint block_num)
{
  acquire(&refcount_table.lock);
  
  for(int i = 0; i < MAX_REFCOUNT_ENTRIES; i++){
    if(refcount_table.entries[i].valid && 
       refcount_table.entries[i].block_num == block_num){
      release(&refcount_table.lock);
      return 1; // Found
    }
  }
  
  release(&refcount_table.lock);
  return 0; // Not found
}


// Get block reference count
uint
bref_get(uint block_num)
{
  acquire(&refcount_table.lock);
  
  for(int i = 0; i < MAX_REFCOUNT_ENTRIES; i++){
    if(refcount_table.entries[i].valid && 
       refcount_table.entries[i].block_num == block_num){
      uint count = refcount_table.entries[i].refcount;
      release(&refcount_table.lock);
      return count;
    }
  }
  
  release(&refcount_table.lock);
  return 0; // Not found or refcount is 0
}

// Initialize deduplication system
void
dedup_init(void)
{
  initlock(&dedup_table.lock, "dedup");
  
  acquire(&dedup_table.lock);
  for(int i = 0; i < DEDUP_TABLE_SIZE; i++){
    dedup_table.entries[i].valid = 0;
    dedup_table.entries[i].refcount = 0;
  }
  release(&dedup_table.lock);
}

// Simple checksum function (djb2 hash)
uint
dedup_hash(char *data, uint len)
{
  uint hash = 5381;
  
  for(uint i = 0; i < len; i++){
    hash = ((hash << 5) + hash) + data[i]; // hash * 33 + c
  }
  
  return hash;
}

// Find block with matching checksum
uint
dedup_find(uint checksum)
{
  acquire(&dedup_table.lock);
  
  for(int i = 0; i < DEDUP_TABLE_SIZE; i++){
    if(dedup_table.entries[i].valid && 
       dedup_table.entries[i].checksum == checksum &&
       dedup_table.entries[i].refcount > 0){
      uint block = dedup_table.entries[i].block_num;
      release(&dedup_table.lock);
      return block;
    }
  }
  
  release(&dedup_table.lock);
  return 0; // Not found
}

// Insert block into dedup table
int
dedup_insert(uint checksum, uint block_num)
{
  acquire(&dedup_table.lock);
  
  // Check if already exists
  for(int i = 0; i < DEDUP_TABLE_SIZE; i++){
    if(dedup_table.entries[i].valid && 
       dedup_table.entries[i].block_num == block_num){
      dedup_table.entries[i].checksum = checksum;
      dedup_table.entries[i].refcount++;
      release(&dedup_table.lock);
      return 0;
    }
  }
  
  // Find empty slot
  for(int i = 0; i < DEDUP_TABLE_SIZE; i++){
    if(!dedup_table.entries[i].valid){
      dedup_table.entries[i].valid = 1;
      dedup_table.entries[i].checksum = checksum;
      dedup_table.entries[i].block_num = block_num;
      dedup_table.entries[i].refcount = 1;
      release(&dedup_table.lock);
      return 0;
    }
  }
  
  release(&dedup_table.lock);
  return -1; // Table full
}

// Remove block from dedup table
int
dedup_remove(uint block_num)
{
  acquire(&dedup_table.lock);
  
  for(int i = 0; i < DEDUP_TABLE_SIZE; i++){
    if(dedup_table.entries[i].valid && 
       dedup_table.entries[i].block_num == block_num){
      if(dedup_table.entries[i].refcount > 0)
        dedup_table.entries[i].refcount--;
      
      if(dedup_table.entries[i].refcount == 0){
        dedup_table.entries[i].valid = 0;
      }
      release(&dedup_table.lock);
      return 0;
    }
  }
  
  release(&dedup_table.lock);
  return -1; // Not found
}

// Initialize garbage collection
void
gc_init(void)
{
  bref_init();
  dedup_init();
  
  gc_statistics.blocks_freed = 0;
  gc_statistics.versions_pruned = 0;
  gc_statistics.last_run_time = 0;
  gc_statistics.total_runs = 0;
}

// Run garbage collection (placeholder - will be implemented later)
int
gc_run(void)
{
  // This will be implemented in Phase 8
  // For now, just update statistics
  gc_statistics.total_runs++;
  gc_statistics.last_run_time = get_timestamp();
  
  return 0;
}

