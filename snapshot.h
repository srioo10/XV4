#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#include "types.h"
#include "fs.h"

// Snapshot management functions
int snapshot_create(char *name);
int snapshot_restore(char *name);
int snapshot_delete(char *name);
int snapshot_list(struct snapshot_metadata *buf, int max);

// Snapshot helper functions
struct snapshot_metadata* snapshot_find(char *name);
struct snapshot_metadata* snapshot_find_by_inum(uint inum);
int snapshot_validate(struct snapshot_metadata *snap);
void snapshot_init(void);
uint snapshot_alloc_inum(void);
void snapshot_free_inum(uint inum);

// Snapshot internal functions
int snapshot_save_metadata(uint inum, struct snapshot_metadata *meta);
int snapshot_load_metadata(uint inum, struct snapshot_metadata *meta);

#endif // SNAPSHOT_H
