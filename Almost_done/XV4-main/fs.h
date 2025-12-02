// On-disk file system format.
// Both the kernel and user programs use this header file.


#define ROOTINO 1  // root i-number
#define BSIZE 512  // block size

// ChronoFS Configuration
#define MAX_VERSIONS_PER_FILE 10    // Maximum versions to keep per file
#define MAX_SNAPSHOTS 100           // Maximum number of snapshots
#define SNAPSHOT_INODE_START 100    // Starting inode for snapshots
#define SNAPSHOT_INODE_END 199      // Ending inode for snapshots
#define JOURNAL_BLOCKS 100          // Number of blocks for journal
#define JOURNAL_MAGIC 0x4A4F524E    // "JORN" magic number
#define MAX_DELETED_TRACK 1000      // Max deleted files to track

// Version node structure (stored in data blocks)
#define VNODE_DATA_BLOCKS 10
// Disk layout:
// [ boot block | super block | log | inode blocks |
//   free bit map | journal | data blocks]
//
// mkfs computes the super block and builds an initial file system. The
// super block describes the disk layout:
struct superblock {
  uint size;         // Size of file system image (blocks)
  uint nblocks;      // Number of data blocks
  uint ninodes;      // Number of inodes.
  uint nlog;         // Number of log blocks
  uint logstart;     // Block number of first log block
  uint inodestart;   // Block number of first inode block
  uint bmapstart;    // Block number of first free map block
  uint journalstart; // Block number of first journal block (ChronoFS)
  uint njournalblocks; // Number of journal blocks (ChronoFS)
};

#define NDIRECT 10
#define NINDIRECT (BSIZE / sizeof(uint))
#define MAXFILE (NDIRECT + NINDIRECT)

// On-disk inode structure
struct dinode {
  short type;           // File type
  short major;          // Major device number (T_DEV only)
  short minor;          // Minor device number (T_DEV only)
  short nlink;          // Number of links to inode in file system
  
  uint size;            // Size of file (bytes)
  uint addrs[10];   // Data block addresses
  uint indirect;
  //added these
  uint create_time;     // Creation timestamp
  uint version_head;
};

//new flags
#define COW_ENABLED   0x01
#define IMMUTABLE     0x02
#define VERSIONED     0x04

// Inodes per block.
#define IPB           (BSIZE / sizeof(struct dinode))

// Block containing inode i
#define IBLOCK(i, sb)     ((i) / IPB + sb.inodestart)

// Bitmap bits per block
#define BPB           (BSIZE*8)

// Block of free map containing bit for block b
#define BBLOCK(b, sb) (b/BPB + sb.bmapstart)

// Directory is a file containing a sequence of dirent structures.
#define DIRSIZ 14

struct dirent {
  ushort inum;
  char name[DIRSIZ];
};

struct version_node {
  uint timestamp;           // When this version was created
  uint prev_version;        // Block number of previous version (0 if first)
  uint data_blocks[VNODE_DATA_BLOCKS]; // Data blocks for this version
  uint nblocks;             // Number of blocks used
  uint file_size;           // File size at this version
  uint refcount;            // Reference count
  char description[32];     // Optional description
  uint checksum;            // Simple integrity check
  uint snapshot_id;         // ID of snapshot this version belongs to (0 if none)
};

// Snapshot metadata structure (stored in snapshot inodes)
struct snapshot_metadata {
  uint valid;               // Is this snapshot valid?
  uint timestamp;           // When snapshot was created
  char name[32];            // User-provided name
  uint id;                  // Unique ID
  uint root_inum;           // Root inode at snapshot time
  uint file_count;          // Number of files in snapshot
  uint total_blocks;        // Total blocks referenced
  uint creator_pid;         // Process that created snapshot
  char reserved[32];        // Reserved for future use
};

// Journal header (at start of journal region)
struct journal_header {
  uint magic;               // JOURNAL_MAGIC
  uint sequence;            // Sequence number
  uint count;               // Number of entries in this transaction
  uint commit;              // 1 if committed, 0 if in progress
  uint checksum;            // Header checksum
};

// Journal entry (follows header)
struct journal_entry {
  uint block_num;           // Block being modified
  uint checksum;            // Checksum of data
  char data[BSIZE];         // Block data
};

// Block reference counting (in-memory structure)
struct block_refcount {
  uint block_num;           // Block number
  uint refcount;            // Reference count
  uint checksum;            // For deduplication
  uint valid;               // Is this entry valid?
};

// Deduplication hash table entry
struct dedup_entry {
  uint checksum;            // Block content checksum
  uint block_num;           // Block number with this content
  uint refcount;            // How many references
  uint valid;               // Is this entry valid?
};

// Deleted file tracking (for recovery)
struct deleted_entry {
  char name[DIRSIZ];        // Original filename
  uint inum;                // Inode number  
  uint version_head;        // Head of version chain
  uint delete_time;         // When it was deleted
  int valid;                // Is this entry valid?
};

// Version information (for user queries)
struct version_info {
  uint version_num;         // Version number (0 = oldest)
  uint timestamp;           // When created
  uint file_size;           // File size at this version
  uint block_count;         // Number of blocks
  char description[32];     // Optional description
};

// Recovery entry (for listing recoverable files)
// struct deleted_entry is already defined above

#define MAX_DELETED_FILES 32

struct recovery_entry {
  char path[128];           // File path
  uint deletion_time;       // When deleted
  uint version_head;        // Head of version chain
};

// System-wide snapshot metadata
// struct snapshot_metadata is already defined above

// Update the existing snapshot_metadata struct if needed, but for now we'll use the one at line 94
// However, the one at line 94 has different fields. Let's unify them.
// The one at line 94 is better. We just need to add 'id' to it if it's missing.
// Actually, let's just remove the duplicate here and update the original one if needed.


