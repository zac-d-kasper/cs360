// Zac Kasper
// cs360 - Final Project
// type.h for final CS360 Project
#ifndef TYPE_H
#define TYPE_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

typedef struct ext2_super_block SUPER;
typedef struct ext2_group_desc GD;
typedef struct ext2_inode INODE;
typedef struct ext2_dir_entry_2 DIR;

SUPER *sp;
GD *gp;
INODE *ip;
DIR *dp; 

#define BLKSIZE 1024
#define NMINODES 100
#define NFD 16
#define NPROC 2

#define INODE_START_BLK 10 
#define INODES_PER_BLOCK (BLKSIZE/sizeof(INODE))

typedef struct Minode{
  INODE INODE;
  int dev;
	int ino;
  int refCount;
  int dirty;
  int mounted;
  struct mount *mptr; 
} MINODE;

typedef struct oft{
  int mode;
  int refCount;
	MINODE *mptr;
  int offset;
} OFT;

typedef struct proc{
  struct PROC *next;
  int pid;
  int uid;
  MINODE *cwd;
  OFT *fd[NFD]; 
} PROC;

typedef struct mount{
  int dev;
  int nblock;
  int ninodes;
  int bmap;
  int imap;
  int iblock;
	MINODE *mountDirPtr;
  char devName[64];
  char mntName[64]; 
} MOUNT; 

//GLOBAL VARIABLES 
PROC P[2]; 
MINODE minodes[NMINODES]; 
int dev;  
MINODE *root; 
PROC *running, *queue; 

char parameters[256], pathname[256], command[256], *path[256];  
char buf[BLKSIZE]; 

#endif 
