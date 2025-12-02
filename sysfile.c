//
// File-system system calls.
// Mostly argument checking, since we don't trust
// user code, and calls into file.c and fs.c.
//

#include "types.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "mmu.h"
#include "proc.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "buf.h"
#include "file.h"
#include "fcntl.h"

// Fetch the nth word-sized system call argument as a file descriptor
// and return both the descriptor and the corresponding struct file.
static int
argfd(int n, int *pfd, struct file **pf)
{
  int fd;
  struct file *f;

  if(argint(n, &fd) < 0)
    return -1;
  if(fd < 0 || fd >= NOFILE || (f=myproc()->ofile[fd]) == 0)
    return -1;
  if(pfd)
    *pfd = fd;
  if(pf)
    *pf = f;
  return 0;
}

// Allocate a file descriptor for the given file.
// Takes over file reference from caller on success.
static int
fdalloc(struct file *f)
{
  int fd;
  struct proc *curproc = myproc();

  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd] == 0){
      curproc->ofile[fd] = f;
      return fd;
    }
  }
  return -1;
}

int
sys_dup(void)
{
  struct file *f;
  int fd;

  if(argfd(0, 0, &f) < 0)
    return -1;
  if((fd=fdalloc(f)) < 0)
    return -1;
  filedup(f);
  return fd;
}

int
sys_read(void)
{
  struct file *f;
  int n;
  char *p;

  if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
    return -1;
  return fileread(f, p, n);
}

int
sys_write(void)
{
  struct file *f;
  int n;
  char *p;

  if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
    return -1;
  return filewrite(f, p, n);
}

int
sys_close(void)
{
  int fd;
  struct file *f;

  if(argfd(0, &fd, &f) < 0)
    return -1;
  myproc()->ofile[fd] = 0;
  fileclose(f);
  return 0;
}

int
sys_fstat(void)
{
  struct file *f;
  struct stat *st;

  if(argfd(0, 0, &f) < 0 || argptr(1, (void*)&st, sizeof(*st)) < 0)
    return -1;
  return filestat(f, st);
}

// Create the path new as a link to the same inode as old.
int
sys_link(void)
{
  char name[DIRSIZ], *new, *old;
  struct inode *dp, *ip;

  if(argstr(0, &old) < 0 || argstr(1, &new) < 0)
    return -1;

  begin_op();
  if((ip = namei(old)) == 0){
    end_op();
    return -1;
  }

  ilock(ip);
  if(ip->type == T_DIR){
    iunlockput(ip);
    end_op();
    return -1;
  }

  ip->nlink++;
  iupdate(ip);
  iunlock(ip);

  if((dp = nameiparent(new, name)) == 0)
    goto bad;
  ilock(dp);
  if(dp->dev != ip->dev || dirlink(dp, name, ip->inum) < 0){
    iunlockput(dp);
    goto bad;
  }
  iunlockput(dp);
  iput(ip);

  end_op();

  return 0;

bad:
  ilock(ip);
  ip->nlink--;
  iupdate(ip);
  iunlockput(ip);
  end_op();
  return -1;
}

// Is the directory dp empty except for "." and ".." ?
static int
isdirempty(struct inode *dp)
{
  int off;
  struct dirent de;

  for(off=2*sizeof(de); off<dp->size; off+=sizeof(de)){
    if(readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
      panic("isdirempty: readi");
    if(de.inum != 0)
      return 0;
  }
  return 1;
}

//PAGEBREAK!
int
sys_unlink(void)
{
  struct inode *ip, *dp;
  struct dirent de;
  char name[DIRSIZ], *path;
  uint off;

  if(argstr(0, &path) < 0)
    return -1;

  begin_op();
  if((dp = nameiparent(path, name)) == 0){
    end_op();
    return -1;
  }

  ilock(dp);

  // Cannot unlink "." or "..".
  if(namecmp(name, ".") == 0 || namecmp(name, "..") == 0)
    goto bad;

  if((ip = dirlookup(dp, name, &off)) == 0)
    goto bad;
  ilock(ip);

  if(ip->nlink < 1)
    panic("unlink: nlink < 1");
  if(ip->type == T_DIR && !isdirempty(ip)){
    iunlockput(ip);
    goto bad;
  }
  
  // ChronoFS: If file has versions, track it before deletion
  if(ip->type == T_FILE && ip->version_head != 0){
    add_deleted_file(name, ip->inum, ip->version_head);
  }

  memset(&de, 0, sizeof(de));
  if(writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
    panic("unlink: writei");
  if(ip->type == T_DIR){
    dp->nlink--;
    iupdate(dp);
  }
  iunlockput(dp);

  ip->nlink--;
  iupdate(ip);
  iunlockput(ip);

  end_op();

  return 0;

bad:
  iunlockput(dp);
  end_op();
  return -1;
}

static struct inode*
create(char *path, short type, short major, short minor)
{
  struct inode *ip, *dp;
  char name[DIRSIZ];

  if((dp = nameiparent(path, name)) == 0)
    return 0;
  ilock(dp);

  if((ip = dirlookup(dp, name, 0)) != 0){
    iunlockput(dp);
    ilock(ip);
    if(type == T_FILE && ip->type == T_FILE)
      return ip;
    iunlockput(ip);
    return 0;
  }

  if((ip = ialloc(dp->dev, type)) == 0)
    panic("create: ialloc");

  ilock(ip);
  ip->major = major;
  ip->minor = minor;
  ip->nlink = 1;
  iupdate(ip);

  if(type == T_DIR){  // Create . and .. entries.
    dp->nlink++;  // for ".."
    iupdate(dp);
    // No ip->nlink++ for ".": avoid cyclic ref count.
    if(dirlink(ip, ".", ip->inum) < 0 || dirlink(ip, "..", dp->inum) < 0)
      panic("create dots");
  }

  if(dirlink(dp, name, ip->inum) < 0)
    panic("create: dirlink");

  iunlockput(dp);

  return ip;
}

int
sys_open(void)
{
  char *path;
  int fd, omode;
  struct file *f;
  struct inode *ip;

  if(argstr(0, &path) < 0 || argint(1, &omode) < 0)
    return -1;

  begin_op();

  if(omode & O_CREATE){
    ip = create(path, T_FILE, 0, 0);
    if(ip == 0){
      end_op();
      return -1;
    }
  } else {
    if((ip = namei(path)) == 0){
      end_op();
      return -1;
    }
    ilock(ip);
    if(ip->type == T_DIR && omode != O_RDONLY){
      iunlockput(ip);
      end_op();
      return -1;
    }
  }

  if((f = filealloc()) == 0 || (fd = fdalloc(f)) < 0){
    if(f)
      fileclose(f);
    iunlockput(ip);
    end_op();
    return -1;
  }
  iunlock(ip);
  end_op();

  f->type = FD_INODE;
  f->ip = ip;
  f->off = 0;
  f->readable = !(omode & O_WRONLY);
  f->writable = (omode & O_WRONLY) || (omode & O_RDWR);
  return fd;
}

int
sys_mkdir(void)
{
  char *path;
  struct inode *ip;

  begin_op();
  if(argstr(0, &path) < 0 || (ip = create(path, T_DIR, 0, 0)) == 0){
    end_op();
    return -1;
  }
  iunlockput(ip);
  end_op();
  return 0;
}

int
sys_mknod(void)
{
  struct inode *ip;
  char *path;
  int major, minor;

  begin_op();
  if((argstr(0, &path)) < 0 ||
     argint(1, &major) < 0 ||
     argint(2, &minor) < 0 ||
     (ip = create(path, T_DEV, major, minor)) == 0){
    end_op();
    return -1;
  }
  iunlockput(ip);
  end_op();
  return 0;
}

int
sys_chdir(void)
{
  char *path;
  struct inode *ip;
  struct proc *curproc = myproc();
  
  begin_op();
  if(argstr(0, &path) < 0 || (ip = namei(path)) == 0){
    end_op();
    return -1;
  }
  ilock(ip);
  if(ip->type != T_DIR){
    iunlockput(ip);
    end_op();
    return -1;
  }
  iunlock(ip);
  iput(curproc->cwd);
  end_op();
  curproc->cwd = ip;
  return 0;
}

int
sys_exec(void)
{
  char *path, *argv[MAXARG];
  int i;
  uint uargv, uarg;

  if(argstr(0, &path) < 0 || argint(1, (int*)&uargv) < 0){
    return -1;
  }
  memset(argv, 0, sizeof(argv));
  for(i=0;; i++){
    if(i >= NELEM(argv))
      return -1;
    if(fetchint(uargv+4*i, (int*)&uarg) < 0)
      return -1;
    if(uarg == 0){
      argv[i] = 0;
      break;
    }
    if(fetchstr(uarg, &argv[i]) < 0)
      return -1;
  }
  return exec(path, argv);
}

int
sys_pipe(void)
{
  int *fd;
  struct file *rf, *wf;
  int fd0, fd1;

  if(argptr(0, (void*)&fd, 2*sizeof(fd[0])) < 0)
    return -1;
  if(pipealloc(&rf, &wf) < 0)
    return -1;
  fd0 = -1;
  if((fd0 = fdalloc(rf)) < 0 || (fd1 = fdalloc(wf)) < 0){
    if(fd0 >= 0)
      myproc()->ofile[fd0] = 0;
    fileclose(rf);
    fileclose(wf);
    return -1;
  }
  fd[0] = fd0;
  fd[1] = fd1;
  return 0;
}

// ChronoFS system calls

int
sys_version_create(void)
{
  char *path;
  char *desc;
  struct inode *ip;
  int len;

  if(argstr(0, &path) < 0 || argstr(1, &desc) < 0)
    return -1;
  
  // Calculate description length
  len = 0;
  while(desc[len]) len++;

  begin_op();
  if((ip = namei(path)) == 0){
    end_op();
    return -1;
  }
  
  ilock(ip);
  uint vblock = version_create(ip, desc, len, 0);
  if(vblock == 0){
    iunlockput(ip);
    end_op();
    return -1;
  }
  
  iunlockput(ip);
  end_op();
  return 0;
}

int
sys_version_list(void)
{
  char *path;
  void *buf;
  int max_entries;
  struct inode *ip;
  int count;

  if(argstr(0, &path) < 0 || argptr(1, (void*)&buf, 0) < 0 || argint(2, &max_entries) < 0)
    return -1;

  begin_op();
  if((ip = namei(path)) == 0){
    end_op();
    return -1;
  }
  
  ilock(ip);
  count = version_list(ip, (struct version_info*)buf, max_entries);
  iunlockput(ip);
  end_op();
  return count;
}

int
sys_snapshot_create(void)
{
  char *desc;
  
  if(argstr(0, &desc) < 0)
    return -1;
    
  return snapshot_create(desc);
}

int
sys_snapshot_restore(void)
{
  char *desc;
  
  if(argstr(0, &desc) < 0)
    return -1;
    
  // Not implemented yet - placeholder
  return -1;
}

int
sys_recover_file(void)
{
  char *name;
  struct inode *ip;
  struct version_node *vnode;
  
  if(argstr(0, &name) < 0)
    return -1;
    
  // Find deleted file info
  uint vhead = recover_deleted_file(name);
  if(vhead == 0)
    return -1; // Not found or not recoverable
    
  begin_op();
  
  // Allocate new inode
  ip = ialloc(ROOTDEV, T_FILE);
  if(ip == 0){
    end_op();
    return -1;
  }
  
  ilock(ip);  // Lock the inode before modifying
  
  // Explicitly set inode fields
  ip->type = T_FILE;
  ip->nlink = 1;
  
  // Restore content from version by COPYING blocks (not sharing)
  vnode = version_get(vhead);
  if(vnode){
    ip->version_head = vhead;
    ip->size = vnode->file_size;
    
    // Copy blocks from version (create new blocks with same data)
    // This avoids reference counting and shared ownership issues
    for(int i = 0; i < vnode->nblocks && i < NDIRECT; i++){
      uint new_block = balloc(ip->dev);
      if(new_block == 0){
        iunlockput(ip);
        end_op();
        return -1;
      }
      
      // Copy data from version block to new block
      struct buf *src_bp = bread(ip->dev, vnode->data_blocks[i]);
      struct buf *dst_bp = bread(ip->dev, new_block);
      memmove(dst_bp->data, src_bp->data, BSIZE);
      log_write(dst_bp);
      brelse(dst_bp);
      brelse(src_bp);
      
      ip->addrs[i] = new_block;
    }
  }
  
  iupdate(ip);  // Write inode to disk
  
  // Create directory entry
  if(dirlink(namei("."), name, ip->inum) < 0){
    ip->nlink = 0;
    iupdate(ip);
    iunlockput(ip);
    end_op();
    return -1;
  }
  
  iunlockput(ip);
  end_op();
  return 0;
}

int
sys_version_restore(void)
{
  char *path;
  int version_num;
  struct inode *ip;
  
  if(argstr(0, &path) < 0 || argint(1, &version_num) < 0)
    return -1;
  
  begin_op();
  
  if((ip = namei(path)) == 0){
    end_op();
    return -1;
  }
  
  ilock(ip);
  
  // Get the target version from the version chain
  struct version_node *vnode = 0;
  uint vblock = ip->version_head;
  int current_version = 0;
  
  // Walk the version chain to find the target version
  while(vblock != 0){
    vnode = version_get(vblock);
    if(vnode == 0)
      break;
    
    if(current_version == version_num){
      // Found the target version - restore it
      
      // 1. Free current blocks
      for(int i = 0; i < NDIRECT; i++){
        if(ip->addrs[i]){
          if(bref_is_tracked(ip->addrs[i])){
             if(bref_dec(ip->addrs[i]) == 0) bfree(ip->dev, ip->addrs[i]);
          } else {
             bfree(ip->dev, ip->addrs[i]);
          }
          ip->addrs[i] = 0;
        }
      }
      
      // 2. Restore from version by COPYING blocks (safe)
      ip->size = vnode->file_size;
      
      for(int i = 0; i < vnode->nblocks && i < NDIRECT; i++){
        // Allocate new block
        uint newblock = balloc(ip->dev);
        if(newblock == 0) break; // Should handle error better but panic avoidance
        
        // Copy data
        struct buf *src = bread(ip->dev, vnode->data_blocks[i]);
        struct buf *dst = bread(ip->dev, newblock);
        memmove(dst->data, src->data, BSIZE);
        log_write(dst);
        brelse(src);
        brelse(dst);
        
        ip->addrs[i] = newblock;
      }
      
      iupdate(ip);
      iunlockput(ip);
      end_op();
      return 0;
    }
    
    current_version++;
    vblock = vnode->prev_version;
  }
  
  // Version not found
  iunlockput(ip);
  end_op();
  return -1;
}

