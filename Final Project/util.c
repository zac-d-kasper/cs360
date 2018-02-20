//Zac Kasper
//Cs360 - Final Project Code
//Utility Functions

#include "util.h"

void get_block(int fd, int blk, char *buffer)
{
  lseek(fd, (long)(blk * BLKSIZE), SEEK_SET);
  read(fd, buffer, BLKSIZE); 
}
void put_block(int fd, int blk, char *buffer)
{
  lseek(fd, (long)(blk * BLKSIZE), SEEK_SET);
  write(fd, buffer, BLKSIZE); 
}

int split_path(char *givenpath)
{
  int numSplits = 0;
  char *temp, *pathCopy;
  pathCopy = malloc(sizeof(char) * strlen(givenpath));
  strcpy(pathCopy, givenpath);
  temp = strtok(pathCopy, "/");
  while (temp != NULL)
    {
      path[numSplits] = temp;
      path[numSplits+1] = 0; //creates null buffer in pathNames array
      numSplits++;
      temp = strtok(0, "/"); 
    }
  return numSplits; 
}

int tst_bit(char *buffer, int bit)
{
	return buffer[bit/8] & (1 << (bit % 8)); 
}
int set_bit(char *buffer, int bit)
{
	return buffer[bit/8] |= (1 << (bit % 8)); 
}
int clr_bit(char *buffer, int bit)
{
	return buffer[bit/8] &= ~(1 << (bit % 8)); 
}
int decFreeInodes(int dev)
{
	char buffer[BLKSIZE]; 
	get_block(dev, 1, buffer); 
	sp = (SUPER *)buffer; 
	sp->s_free_inodes_count--; 
	put_block(dev, 1, buffer); 
	
	get_block(dev, 2, buffer); 
	gp = (GD *)buffer; 
	gp->bg_free_inodes_count--; 
	put_block(dev, 2, buffer); 
}
int incFreeInodes(int dev)
{
	char buffer[BLKSIZE]; 
	//increment inode count in superblock 
	get_block(dev, 1, buffer); 
	sp = (SUPER *)buffer; 
	sp->s_free_inodes_count++;
	put_block(dev, 1, buffer); 

	//increment inode count in group descriptor 
	get_block(dev, 2, buffer); 
	gp = (GD *)buffer; 
	gp->bg_free_inodes_count++; 
	put_block(dev, 2, buffer); 
}
int decFreeBlocks(int dev)
{
	char buffer[BLKSIZE]; 
	get_block(dev, 1, buffer); 
	sp = (SUPER *)buffer; 
	sp->s_free_blocks_count--; 
	put_block(dev, 1, buffer); 

	get_block(dev, 2, buffer); 
	gp = (GD *)buffer;
	gp->bg_free_blocks_count--; 
	put_block(dev, 2, buffer); 
}
int incFreeBlocks(int dev)
{
	char buffer[BLKSIZE]; 
	//super block
	get_block(dev, 1, buffer); 
	sp = (SUPER *)buffer; 
	sp->s_free_blocks_count++; 
	put_block(dev, 1, buffer); 
	
	//gd block 
	get_block(dev, 2, buffer); 
	gp = (GD *)buffer; 
	gp->bg_free_blocks_count++; 
	put_block(dev, 2, buffer); 
}

int ialloc(int dev)
{
	int i, ninodes; 
	char buffer[BLKSIZE]; 
	SUPER *stemp; 

	get_block(dev, 1, buffer); //get superblock 
	stemp = (SUPER *)buffer; 
	ninodes = stemp->s_inodes_count; 
	put_block(dev, 1, buffer); 

	get_block(dev, 9, buffer); //get imap 

	for( i = 0; i < ninodes; i++)
	{
		if (tst_bit(buffer, i) == 0)
		{
			set_bit(buffer, i); 
			put_block(dev, 9, buffer); //write back edited imap
			decFreeInodes(dev); 
			return i+1; 
		}
	}
	return 0; //no free inodes
}
int balloc(int dev)
{
	int i, nblocks; 
	char buffer[BLKSIZE]; 
	SUPER *stemp; 

	get_block(dev, 1, buffer); 
	stemp = (SUPER *)buffer; 
	nblocks = stemp->s_blocks_count; 
	put_block(dev, 1, buffer); //is this necessary? buffer is not edited

	get_block(dev, 8, buffer); 
	for (i = 0; i < nblocks; i++)
	{
		if (tst_bit(buffer, i) == 0)
		{
			set_bit(buffer, i); 
			put_block(dev, 8, buffer); 
			decFreeBlocks(dev); 
			return i+1; 
		}
	}
	return 0; 
}

MINODE *iget(int dev, int ino)
{
	int nIndex, nOffset; 
	INODE *tp; 
	char buffer[BLKSIZE]; 
	 
	//search current minodes 
	for(int i = 0; i < NMINODES; i++)
	{
		if (minodes[i].refCount > 0)
		{ 
			if (minodes[i].dev == dev && minodes[i].ino == ino)
			{
				minodes[i].refCount++; 
				return &minodes[i]; //return address of referenced MINODE
			}
		}
	}
	
	//allocate new minode 
	for (int i = 0; i < NMINODES; i++)
	{
		if (minodes[i].refCount == 0)
		{			
			nOffset = (ino - 1) % INODES_PER_BLOCK; 
			nIndex = (ino - 1) / INODES_PER_BLOCK + INODE_START_BLK; 
			//MAILMAN'S ALGORITHM 
			get_block(dev, nIndex, buffer); 
			tp = (INODE *)buffer; 
			tp += nOffset; 
			minodes[i].INODE = *tp; 
			minodes[i].dev = dev; 
			minodes[i].ino = ino; 
			minodes[i].refCount = 1; 
			minodes[i].dirty = 0; 
			minodes[i].mounted = 0; 
			minodes[i].mptr = 0; 
			return &minodes[i]; 
		}
	}
}
void iput(MINODE *mip)
{
  char buffer[BLKSIZE]; 
  int ipos, offset; 
  int ino = mip->ino;
  mip->refCount--;
  if (mip->refCount > 0) return;
  if (mip->dirty == 0) return;
  ipos = (ino - 1) / INODES_PER_BLOCK + INODE_START_BLK;
  offset = (ino - 1) % INODES_PER_BLOCK;
  get_block(dev, ipos, buffer);
  INODE *ip = (INODE *)buffer;
  ip += offset;
  *ip = mip->INODE;
  put_block(mip->dev, ipos, buffer); 
}

void initialize(char *device)
{
  root = malloc(sizeof(MINODE));
  P[0].uid = 0;
  P[0].pid = 1;
  P[1].pid = 2;
  P[1].uid = 1;
  P[0].cwd = 0;
  P[1].cwd = 0;

  for (int i = 0; i < 100; i++)
    {
      minodes[i].refCount = 0; 
    }

  root = 0; 
	mount_root(device); 
	running = &P[0]; 
	queue = &P[1]; 
}
void mount_root(char *device)
{
  dev = open(device, O_RDWR);
  if (dev < 0)
    {
      printf("Failed to open %s\n", device);
      exit(1); 
    }
  
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  if (sp->s_magic != 0xEF53)
    {
      printf("%s is not a valid EXT2 device\n", device);
      exit(0); 
    }
	int nblocks = sp->s_blocks_count; 
	int ninodes = sp->s_inodes_count; 
  get_block(dev, 2, buf);
  gp = (GD *)buf; 
  
	root = iget(dev, 2);
	root->mptr = (MOUNT *)malloc(sizeof(MOUNT)); 
  root->mptr->dev = dev;
  root->mptr->nblock = nblocks;
  root->mptr->ninodes = ninodes;
  root->mptr->bmap = gp->bg_block_bitmap;
  root->mptr->imap = gp->bg_inode_bitmap;
  root->mptr->iblock = gp->bg_inode_table;
  root->mptr->mountDirPtr = root;
  strcpy(root->mptr->devName, device);
  strcpy(root->mptr->mntName, "/");
	root->mounted = 1; 
  
  P[0].cwd = root;
  P[1].cwd = root;
	root->refCount = 3; 
}

int getino(char *name)
{
  int inodenum, blk, disp, n;
	char buffer[BLKSIZE]; 
  INODE *inp; 
	MINODE *mip;  

	if (strcmp(name, "/") == 0)
		return 2; //return root inode num
	if (name[0] == '/')
		mip = iget(dev, 2); 
	else
		mip = iget(dev, running->cwd->ino);  	

	strcpy(buffer, name); 
	n = split_path(buffer); 

	for (int i = 0; i < n; i++)
	{
		inodenum = search(mip, path[i]); 
		if (inodenum == 0)
		{
			iput(mip); 
			printf("ERROR: %s does not exist\n", path[i]); 
			return 0; 
		}
		iput(mip); 
		mip = iget(dev, inodenum); 
	}
	iput(mip); 

	return inodenum; 
}

int search(MINODE *mip, char *fname)
{
	char *cp, tempbuf[256], buffer[BLKSIZE]; 

	for (int i = 0; i < 12; i++) //search direct blocks 
	{
		if (mip->INODE.i_block[i])
		{
			get_block(dev, mip->INODE.i_block[i], buffer); 
			dp = (DIR *)buffer; 
			cp = buffer; 

			while(cp < buffer + BLKSIZE)
			{
				strncpy(tempbuf, dp->name, dp->name_len); 
				tempbuf[dp->name_len] = 0; 

				if (strcmp(tempbuf, fname) == 0)
				{
					return dp->inode; 
				}
				cp += dp->rec_len; 
				dp = (DIR *)cp; 
			}
		}
	}
	return 0; //not found
}

int is_parent(char *givenpath) //detects if there is a parent in pathname
{
	int i = 0; 
	while (i < strlen(givenpath))
	{
		if (givenpath[i] == '/') 
			return 1; 
		i++; 
	}
	return 0; 
}

int idealloc(int dev, int ino)
{
	char buffer[BLKSIZE]; 
	get_block(dev, 9, buffer); //get imap block 
	clr_bit(buffer, ino - 1); 
	put_block(dev, 9, buffer); //write modified inode back 
	incFreeInodes(dev); 
}
int bdealloc(int dev, int iblk)
{
	char buffer[BLKSIZE]; 
	get_block(dev, 8, buffer); 
	clr_bit(buffer, iblk - 1); 
	put_block(dev, 8, buffer); 
	incFreeBlocks(dev); 
}

int mino_empty(MINODE *mip)
{
	char tempname[256], *cp, buffer[BLKSIZE]; 

	//more than two links -> must have a file 
	if (mip->INODE.i_links_count > 2)
		return 0; 

	if (mip->INODE.i_links_count == 2)
	{
		//cycle through each direct block 
		for (int i = 0; i < 12; i++)
		{
			if (mip->INODE.i_block[i] > 0)
			{
				get_block(dev, mip->INODE.i_block[i], buffer); 
				cp = buffer; 
				dp = (DIR *)buffer; 
				
				while (cp < buf + BLKSIZE)
				{
					strncpy(tempname, dp->name, dp->name_len); 
					tempname[dp->name_len] = 0; 
					
					if (strcmp(tempname, ".") && strcmp(tempname, ".."))
						return 0; 
						//if tempname is neither "." or "..", if statement is true
						//therefore, tempname is something else -> dir isn't empty 
					cp += dp->rec_len; 
					dp = (DIR *)cp; 
				}
			}
		}
		return 1; //minode is empty 
	}
	return -1; //error, minode doesn't have correct internal references 
}

//given a DIR MINODE, returns inode num for itself and for it's parent 
	//gets from "." and ".." DIR's 
int get_inos(MINODE *mip, int *curIno, int *pIno)
{
	char *cp, buffer[BLKSIZE]; 
	get_block(dev, mip->INODE.i_block[0], buffer); 
	dp = (DIR *)buffer; 
	cp = buffer; 
	
	*curIno = dp->inode; 
	cp += dp->rec_len; 
	dp = (DIR *)cp; 
	*pIno = dp->inode; 
	return 0; 
}










