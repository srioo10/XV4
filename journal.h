#ifndef JOURNAL_H
#define JOURNAL_H

#include "types.h"
#include "fs.h"

// Journal functions
void journal_init(void);
int journal_begin_tx(void);
int journal_log_write(uint block_num, char *data);
int journal_commit_tx(void);
int journal_abort_tx(void);

// Recovery functions
void journal_recover(void);
int journal_replay(void);

// Journal helper functions
uint journal_checksum(char *data, uint len);
int journal_verify_checksum(struct journal_header *hdr);
void journal_clear(void);

// Journal state
struct journal_state {
  uint in_transaction;      // Are we in a transaction?
  uint current_sequence;    // Current sequence number
  uint num_entries;         // Number of entries in current tx
  uint journal_block;       // Current journal block
};

extern struct journal_state jstate;

#endif // JOURNAL_H
