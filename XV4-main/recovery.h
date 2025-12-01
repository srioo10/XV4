#ifndef RECOVERY_H
#define RECOVERY_H

#include "types.h"
#include "fs.h"

// Recovery functions
int recover_file(char *path, uint timestamp);
int undelete_file(char *path);
int list_recoverable(struct recovery_entry *buf, int max);
int list_deleted(struct deleted_entry *buf, int max);

// Time-travel functions
struct inode* get_file_at_time(char *path, uint timestamp);
int list_versions(char *path, struct version_info *buf, int max);

// Deleted file registry management
void recovery_init(void);
int recovery_register_deletion(struct inode *ip, char *path);
struct deleted_entry* recovery_find_deleted(char *path);
int recovery_clear_entry(char *path);

// Version traversal
int version_count(struct inode *ip);
struct version_node* version_get_at_index(struct inode *ip, uint index);
struct version_node* version_get_at_time(struct inode *ip, uint timestamp);

#endif // RECOVERY_H
