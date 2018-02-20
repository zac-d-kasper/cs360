// Zac Kasper
// cs360 - Final Project
// util.h for final CS360 Project

#ifndef UTIL_H
#define UTIL_H

#include "type.h"

void get_block(int fd, int blk, char *buf);
void put_block(int fd, int blk, char *buf);
int split_path(char *givenpath); 
int tst_bit(char *buf, int bit);
int set_bit(char *buf, int bit);
int clr_bit(char *buf, int bit); 
int decFreeInodes(int dev);
int incFreeInodes(int dev); 
int decFreeBlocks(int dev); 
int incFreeBlocks(int dev); 
int ialloc(int dev); 
int balloc(int dev); 
MINODE *iget(int dev, int ino);
void iput(MINODE *mip); 
void initialize(); 
void mount_root(char *device); 
int getino(char *name); 
int search(MINODE *mip, char *fname);
int is_parent(char *givenpath); 
int idealloc(int dev, int ino);
int bdealloc(int dev, int iblk); 
int mino_empty(MINODE *mip); 
int get_inos(MINODE *mip, int *curIno, int *pIno); 


#endif 
