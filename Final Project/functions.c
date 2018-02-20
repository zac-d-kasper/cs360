//Zac Kasper
//Cs360 - Final Project functions.c

#include "functions.h"

void printfile(MINODE *mip, char *filename)
{
	char *time; 
	char *t1 = "xwrxwrxwr-------"; 
	char *t2 = "----------------"; 
	int mode, type; 
	
	mode = mip->INODE.i_mode; 
	//file type
	if (S_ISDIR(mode))
		printf("d"); 
	else if (S_ISLNK(mode))
		printf("l"); 
	else if (S_ISREG(mode))
		printf("-"); 
		
	//file permissions - Alg given by KC
	for (int i=8; i >= 0; i--)
	{
    if (mode & (1 << i))
			printf("%c", t1[i]);
    else
			printf("%c", t2[i]);
  }
  
  //file info
  printf("%4d ", mip->INODE.i_links_count); 
  printf("%4d ", mip->INODE.i_uid); 
  printf("%8d ", mip->INODE.i_size); 
  time = ctime(&(mip->INODE.i_atime)); 
  time[strlen(time) - 1] = 0; 
  printf("%s ", time); 
  printf("%s ", filename); 
  
  if (S_ISLNK(mode))
  	printf("-> %s", (char *)mip->INODE.i_block); //prints name of link source
  printf("\n"); 
}
void printdirectory(MINODE *mip)
{   
	char *cp, tempname[100]; 
	int ino, length; 
	MINODE *tempM; 
	DIR *tempD; 
	
	for (int i = 0; i < 12; i++) //cycle direct blocks
	{
		if (mip->INODE.i_block[i] != 0)
		{ 
			get_block(dev, mip->INODE.i_block[i], buf);
			cp = buf;
			tempD = (DIR *)buf;
			
			while (cp < buf + BLKSIZE)
				{
				  strncpy(tempname, tempD->name, tempD->name_len);
				  tempname[tempD->name_len] = 0;
				  
				  ino = tempD->inode;  
				  tempM = iget(dev, ino); 
				  printfile(tempM, tempname);
				  iput(tempM); 
				  
				  cp += tempD->rec_len; 
				  tempD = (DIR *)cp; 
				}
		}
  }   
}
int ls(char *givenpath)
{
  int ino;
  MINODE *mip; 
  if (pathname[0] != 0) //given pathname to file outside cwd
  { 
		ino = getino(givenpath); 
		
		if (ino == 0)
		{
			printf("ERROR: given path does not exist\n"); 
			return 0; 
		}
		mip = iget(dev, ino); 
  }
    
  else
  {
  	mip = iget(dev, running->cwd->ino); 
  } 
  
  if (!S_ISDIR(mip->INODE.i_mode))
  {
  	printf("ERROR: gave path to file, not directory\n"); 
  	printf("Note: try 'stat %s' to print a single file's stats\n", pathname); 
  	return 0; 
  }
  printdirectory(mip); 
	iput(mip);
	return 1;  
}

int cd(char *pathname)
{
  int ino;
  MINODE *mip;
  
  if (pathname[0] == 0) //no input -> set cwd to root
  {
  	iput(running->cwd); 
  	running->cwd = root; 
  	root->refCount++; 
  	return 1; 
  }
   
  ino = getino(pathname); 
  if (ino == 0) //directory doesn't exist 
  	return 0; 
  mip = iget(dev, ino); 
  
  if (!S_ISDIR(mip->INODE.i_mode))
  {
  	printf("ERROR: %s is not a directory\n", pathname); 
  	iput(mip); 
  	return 0; 
  }
  iput(running->cwd); 
  running->cwd = mip; 
  return 1; 
}

void rpwd(MINODE *wd)
{
  if (wd == root)
    return;
  //else find parent inode number
  char *cp;
  char currName[256];
  MINODE *parent; 
  
  get_block(dev, wd->INODE.i_block[0], buf);
  dp = (DIR *)buf;
  cp = buf;
  int currInode = dp->inode; 
  
  cp += dp->rec_len;
  dp = (DIR *)cp;
  int parentInode = dp->inode; 
  parent = iget(dev, parentInode);

  if (parentInode == currInode)
    return; //stops circular reference to "." file at the root

  for (int i = 0; i < 12; i++)
    {
      if (parent->INODE.i_block[i] != 0)
			{
				get_block(dev, parent->INODE.i_block[i], buf);
				dp = (DIR *)buf;
				cp = buf;
				while(cp < buf + BLKSIZE)
				{
					  if (dp->inode == currInode) //find current dir inode in parent
						{
							strncpy(currName, dp->name, dp->name_len);
							currName[dp->name_len] = 0; 
						}
					  cp += dp->rec_len;
					  dp = (DIR *)cp; 
				}
			}
    }
  rpwd(parent); 
  iput(parent); 
  
  printf("/%s", currName);
}
void pwd(MINODE *wd)
{
  if (wd == root)
    printf("/");
  else
    rpwd(wd); 
}

int makedir()
{
	MINODE *pip; 
	int ino, result; 
	char *parent, *child; 

	if (pathname[0] == 0)
	{
		printf("ERROR: No pathname given\n"); 
		return 0; 
	}
	if (is_parent(pathname)) //mkdir outside of cwd
	{
		parent = dirname(pathname); 
		child = basename(pathname); 
		ino = getino(parent); 
		if (ino == 0)
			return -1; //return error
		pip = iget(dev, ino); 
	}
	else //mkdir in cwd
	{
		pip = iget(dev, running->cwd->ino); 
		child = (char *)malloc((strlen(pathname) + 1) * sizeof(char)); 
		strcpy(child, pathname); 
	}

	if (!S_ISDIR(pip->INODE.i_mode))
	{
		printf("ERROR: not DIR type\n"); 
		iput(pip); 
		return -1; 
	}
	
	if (search(pip, child))
	{
		printf("%s already exists\n", child); 
		iput(pip); 
		return 1; //return misfire
	}

	result = my_mkdir(pip, child); 
	iput(pip); 
	return result; 
}
int my_mkdir(MINODE *pip, char *name)
{
	int inumber, bnumber; 
	int need_len, ideal_len, rec_length, remain; 
	int newdblock; 
	char *cp; 
	char tempbuf[BLKSIZE]; 
	DIR *dptr; 
	MINODE *mip; 

	inumber = ialloc(dev); 
	bnumber = balloc(dev); 
 
	mip = iget(dev, inumber); 

	mip->INODE.i_mode = 0x41ED; //DIR 
	mip->INODE.i_uid = running->uid;  
	mip->INODE.i_size = BLKSIZE; 
	mip->INODE.i_links_count = 2; 
	mip->INODE.i_atime = mip->INODE.i_ctime = mip->INODE.i_mtime = time(0L); 
	mip->INODE.i_blocks = 2; 
	mip->dirty = 1; 

	for (int i = 1; i < 15; i++) //clear blocks to start 
		mip->INODE.i_block[i] = 0; 
	mip->INODE.i_block[0] = bnumber; 
	iput(mip); //write base framework to disk 

	//create . and .. entries 
	memset(buf, 0, BLKSIZE);
	dp = (DIR *)buf; 
	// "." entry
	dp->inode = inumber; 
	dp->name[0] = '.'; 
	dp->name_len = 1; 
	dp->rec_len = 12; //dir entry length 

	cp = buf + dp->rec_len; 
	dp = (DIR *)cp; 
	// ".." entry
	dp->inode = pip->ino; 
	dp->name_len = 2; 
	dp->name[0] = dp->name[1] = '.'; 
	dp->rec_len = BLKSIZE - 12; //includes the rest of the block 

	put_block(dev, bnumber, buf); //write block to disk 

	//enter name into parent dir
	int i = 0; 
	while (pip->INODE.i_block[i] != 0)
		i++; 
	i--; 
	
	get_block(dev, pip->INODE.i_block[i], buf); 
	dp = (DIR *)buf; 
	cp = buf; 
	rec_length = 0; 
	//walk to end of parent directory 
	while(dp->rec_len + rec_length < BLKSIZE)
	{
		rec_length += dp->rec_len; 
		cp += dp->rec_len; 
		dp = (DIR *)cp; 
	}
	
	need_len = 4 * ((8 + strlen(name) + 3) / 4); //create multiple of 4 
	ideal_len = 4 * ((8 + dp->name_len + 3) / 4); 

	remain = dp->rec_len - ideal_len; 
	
	//check if it can add new entry to current data block 
	if (remain >= need_len)
	{	
		dp->rec_len = ideal_len; 
		cp += dp->rec_len; 
		dp = (DIR *)cp; 
		dp->rec_len = remain; 
		dp->name_len = strlen(name); 
		strncpy(dp->name, name, dp->name_len); 
		dp->inode = inumber; 

		put_block(dev, pip->INODE.i_block[i], buf); 
	}
	else //allocate new data block 
	{
		i++; 
		newdblock = balloc(dev); 
		pip->INODE.i_block[i] = newdblock; 
		get_block(dev, newdblock, tempbuf);
		
		dptr = (DIR *)tempbuf; 
		dptr->rec_len = BLKSIZE; 
		dptr->name_len = strlen(name); 
		strncpy(dptr->name, name, dptr->name_len); 
		dptr->inode = inumber; 
		pip->INODE.i_size += BLKSIZE; 
		put_block(dev, pip->INODE.i_block[i], tempbuf); 
	}
	//increment parent's link count by 1, touch it's time, mark dirty
	pip->INODE.i_links_count++; 
	pip->INODE.i_atime = time(0L); 
	pip->dirty = 1; 
	return 0; 
}

int creation()
{
	MINODE *pip; 
	int ino, result; 
	char *parent, *child; 

	if (pathname[0] == 0)
	{
		printf("ERROr: No pathname given\n"); 
		return 0; 
	}
	if (is_parent(pathname)) //creat outside of cwd
	{
		parent = dirname(pathname); 
		child = basename(pathname); 
		ino = getino(parent); 
		if (ino == 0)
			return -1; //return error
		pip = iget(dev, ino); 
	}
	else //creat in cwd
	{
		pip = iget(dev, running->cwd->ino); 
		child = (char *)malloc((strlen(pathname) + 1) * sizeof(char)); 
		strcpy(child, pathname); 
	}

	if (!S_ISDIR(pip->INODE.i_mode))
	{
		printf("ERROR: not DIR type\n"); 
		iput(pip); 
		return -1; 
	}
	
	if (search(pip, child))
	{
		printf("%s already exists\n", child); 
		iput(pip); 
		return 1; //return misfire
	}

	result = my_creat(pip, child); 
	return result; 
}
int my_creat(MINODE *pip, char *name)
{
	int inumber, bnumber; 
	int need_len, ideal_len, rec_length, remain; 
	int newdblock; 
	char *cp; 
	char tempbuf[BLKSIZE]; 
	DIR *dptr; 
	MINODE *mip; 

	inumber = ialloc(pip->dev); 
	bnumber = balloc(pip->dev); 
 
	mip = iget(dev, inumber); 

	mip->INODE.i_mode = 0x81A4; //REG
	mip->INODE.i_uid = running->uid;  
	mip->INODE.i_size = 0; 
	mip->INODE.i_links_count = 1; 
	mip->INODE.i_atime = mip->INODE.i_ctime = mip->INODE.i_mtime = time(0L); 
	mip->INODE.i_blocks = 2; 
	mip->dirty = 1; 

	for (int i = 1; i < 15; i++)
		mip->INODE.i_block[i] = 0; 
	iput(mip); 

	//enter name into parent dir
	int i = 0; 
	while (pip->INODE.i_block[i] != 0)
		i++; 
	i--;  
	
	get_block(dev, pip->INODE.i_block[i], buf); 
	dp = (DIR *)buf; 
	cp = buf; 
	rec_length = 0; 
	//walk to end of parent directory 
	while(dp->rec_len + rec_length < BLKSIZE)
	{
		rec_length += dp->rec_len; 
		cp += dp->rec_len; 
		dp = (DIR *)cp; 
	}
	
	need_len = 4 * ((8 + strlen(name) + 3) / 4); //create multiple of 4
	ideal_len = 4 * ((8 + dp->name_len + 3) / 4);  
	remain = dp->rec_len - ideal_len; 
	//check if it can add new entry to current data block 
	if (remain >= need_len)
	{ 		
		dp->rec_len = ideal_len; 
		cp += dp->rec_len; 
		dp = (DIR *)cp; 
		dp->rec_len = remain; 
		dp->name_len = strlen(name); 
		strncpy(dp->name, name, dp->name_len); 
		dp->inode = inumber; 

		put_block(dev, pip->INODE.i_block[i], buf); 
	}
	else //allocate new data block 
	{
		i++; 
		newdblock = balloc(dev); 
		pip->INODE.i_block[i] = newdblock; 
		get_block(pip->dev, newdblock, tempbuf); //potentially not use tempbuf
		
		dptr = (DIR *)tempbuf; 
		dptr->rec_len = BLKSIZE; 
		dptr->name_len = strlen(name); 
		strncpy(dptr->name, name, dptr->name_len); 
		dptr->inode = inumber; 
		pip->INODE.i_size += BLKSIZE; 
		put_block(dev, pip->INODE.i_block[i], tempbuf); 
	}
	//touch parent's time, mark dirty
	pip->INODE.i_atime = time(0L); 
	pip->dirty = 1; 
	iput(pip); 
	return 0;
}

int removedir()
{
	int pino, curino; 
	MINODE *mip, *pip; 
	char *targetname; 
	
	if (pathname[0] == 0)
	{
		printf("ERROR: no pathname given\n"); 
		return 0; 
	}
 
	curino = getino(pathname); 
	
	if (curino == 0)
		return 0; //pathname doesn't exist
	
	mip = iget(dev, curino); 
	if (!S_ISDIR(mip->INODE.i_mode))
	{
		printf("ERROR: file is not dir type\n"); 
		iput(mip); 
		return 0; 
	} 
	
	if (mip->refCount > 1)
	{
		printf("ERROR: directory is currently being used\n"); 
		iput(mip); 
		return 0; 
	}
	
	if (!mino_empty(mip))
	{
		printf("ERROR: directory is not empty\n"); 
		iput(mip); 
		return 0; 
	} 
	//deallocate block/inode
	for (int i = 0; i < 12; i++)
	{
		if (mip->INODE.i_block[i] == 0)
			continue; 
		bdealloc(mip->dev, mip->INODE.i_block[i]); 
	}
	idealloc(mip->dev, mip->ino); 
	iput(mip); 
	
	get_inos(mip, &curino, &pino);  
	pip = iget(dev, pino); 
	
	if (is_parent(pathname))
		targetname = basename(pathname); 
	else
	{
		targetname = (char *)malloc((strlen(pathname) + 1) * sizeof(char));
		strcpy(targetname, pathname); 
	}
	rm_child(pip, targetname); 
	pip->INODE.i_links_count--; 
	pip->INODE.i_atime = pip->INODE.i_mtime = time(0L); 
	pip->dirty = 1; 
	iput(pip); 
	return 1; //success 
}
int rm_child(MINODE *pip, char *name)
{
	int total_len, next_len, remove_len, prev_len, first_check; 
	DIR *nextD; 
	char tempname[256], *cp, *nextC; 
	
	for (int i = 0; i < 12; i++) //searching direct blocks 
	{
		if (pip->INODE.i_block[i] > 0)
		{
			get_block(dev, pip->INODE.i_block[i], buf); 
			dp = (DIR *)buf; 
			cp = buf; 
			first_check = 0; 
			total_len = 0; 
			while(cp < buf + BLKSIZE)
			{
				strncpy(tempname, dp->name, dp->name_len); 
				tempname[dp->name_len] = 0; 
				total_len += dp->rec_len;  
				if (strcmp(tempname, name) == 0) //item found
				{
					if (first_check > 0) //if item is not the first entry in data block
					{
						if (total_len == BLKSIZE) //if item is LAST ENTRY
						{
							remove_len = dp->rec_len; 
							cp -= prev_len; //goes back to second to last entry
							dp = (DIR *)cp; 
							dp->rec_len += remove_len; //extends second to last to end of block 
							put_block(dev, pip->INODE.i_block[i], buf); 
							pip->dirty = 1; 
							return 1; 
						}
						
						//Else have to move all following entries left 
						remove_len = dp->rec_len; 
						nextC = cp + dp->rec_len; 
						nextD = (DIR *)nextC; 
						while (total_len + nextD->rec_len < BLKSIZE)
						{ //handshake each adjacent DIR to the left to the end of the block 
							total_len += nextD->rec_len; 
							next_len = nextD->rec_len; 
							dp->inode = nextD->inode; 
							dp->rec_len = nextD->rec_len; 
							dp->name_len = nextD->rec_len; 
							strncpy(dp->name, nextD->name, nextD->name_len); 
							//overwrite current DIR with following DIR2 
							nextC += next_len; 
							nextD = (DIR *)nextC; 
							//increment DIR2 to next dir
							cp += next_len; 
							dp = (DIR *)cp; 
							//increment DIR to DIR2 
						}
						//overwrite targeted dir entry 
						dp->inode = nextD->inode; 
						dp->rec_len = nextD->rec_len + remove_len; 
						dp->name_len = nextD->name_len; 
						strncpy(dp->name, nextD->name, nextD->name_len); 
						put_block(dev, pip->INODE.i_block[i], buf); 
						pip->dirty = 1; 
						return 1; 
					}
					else //item is only entry in data block 
					{
						bdealloc(dev, pip->INODE.i_block[i]); 
						memset(buf, 0, BLKSIZE); 
						put_block(dev, pip->INODE.i_block[i], buf); 
						pip->INODE.i_size -= BLKSIZE; 
						pip->INODE.i_block[i] = 0; 
						pip->dirty = 1; 
						return 1; 
					}
				}
				first_check++; 
				prev_len = dp->rec_len; 
				cp += dp->rec_len; 
				dp = (DIR *)cp; 
			}
			
		}
	}
	return 0; //failed
}

int hardlink()
{
	int ino, oldino, targetdev; 
	int rec_length, need_length, ideal_length, remain, dblock; 
	char tempbuffer[BLKSIZE], *cp;
	char parent[256], child[256];  
	MINODE *mip; 
	
	if (pathname[0] == 0)
	{
		printf("ERROR: No pathname for source file given\n"); 
		return 0; 
	}
	if (parameters[0] == 0)
	{
		printf("ERROR: No pathname for new hardlink file given\n"); 
		return 0; 
	}
		
	oldino = getino(pathname); 
	if (oldino == 0)
		return 0; 
		
	mip = iget(dev, oldino); 
	
	if (!S_ISREG(mip->INODE.i_mode))
	{
		printf("ERROR: %s is not a regular file\n", pathname); 
		iput(mip); 
		return 0; 
	}	
	iput(mip); 
		
	if (is_parent(parameters))
	{
		strcpy(parent, dirname(parameters)); 
		strcpy(child, basename(parameters)); 
		
		ino = getino(parent); 
		if (ino == 0)
			return 0; 
		
		//check if parent is directory 
		mip = iget(dev, ino); 
		if (!S_ISDIR(mip->INODE.i_mode))
		{
			printf("ERROR: %s is not a directory\n", parent); 
			iput(mip); 
			return 0; 
		}
		if (search(mip, child))
		{
			printf("ERROR: %s already exists\n", child); 
			iput(mip); 
			return 0; 
		}
	}
	else //insert link inside current directory 
	{
		strcpy(parent, "."); 
		strcpy(child, parameters); 

		if (search(running->cwd, child))
		{
			printf("ERROR: %s already exists\n", child); 
			return 0; 
		}
		
		mip = iget(dev, running->cwd->ino); 
	} 
	
	int i = 0; 
	while (mip->INODE.i_block[i] != 0)
		i++; 
	i--;  //find open data block for new entry 
	
	get_block(dev, mip->INODE.i_block[i], buf); 
	dp = (DIR *)buf; 
	cp = buf; 
	rec_length = 0; 
	while (dp->rec_len + rec_length < BLKSIZE)
	{
		rec_length += dp->rec_len; 
		cp += dp->rec_len; 
		dp = (DIR *)cp; 
	}
	
	need_length = 4 * ((8 + strlen(child) + 3) / 4); 
	ideal_length = 4 * ((8 + dp->name_len + 3) / 4); 
	remain = dp->rec_len - ideal_length;  
	
	if (remain >= need_length) //space found, resize last entry, add new one
	{
		dp->rec_len = ideal_length; 
		cp += dp->rec_len; 
		dp = (DIR *)cp; 
		dp->rec_len = remain; 
		dp->name_len = strlen(child); 
		strncpy(dp->name, child, dp->name_len); 
		dp->inode = oldino; 
		
		put_block(dev, mip->INODE.i_block[i], buf); 
	}
	else //allocate new data block 
	{
		i++; 
		dblock = balloc(dev); 
		mip->INODE.i_block[i] = dblock; 
		get_block(dev, dblock, tempbuffer); 
		dp = (DIR *)tempbuffer; 
		dp->rec_len = BLKSIZE; 
		dp->name_len = strlen(child); 
		strncpy(dp->name, child, dp->name_len); 
		dp->inode = oldino; 
		mip->INODE.i_size += BLKSIZE; 
		
		put_block(dev, mip->INODE.i_block[i], tempbuffer); 
	}
	
	mip->INODE.i_atime = time(0L); 
	mip->dirty = 1; 
	iput(mip); 
	mip = iget(dev, oldino); 
	mip->INODE.i_links_count++; 
	mip->INODE.i_atime = mip->INODE.i_mtime = time(0L); 
	mip->dirty = 1; 
	iput(mip); 
	
	return 1; 
}

int unlink() //removes file specified in pathname
{ 
	MINODE *mip, *pip; 
	int tempblocks[15], symcheck; 
	int ino; 
	char tbuf[BLKSIZE], parent[256], child[256];  
	
	if (pathname[0] == 0)
	{
		printf("ERROR: no pathname given\n"); 
		return 0; 
	} 
		
	ino = getino(pathname); 
	if (ino == 0)
		return 0; //error 

	mip = iget(dev, ino); 

	if (S_ISDIR(mip->INODE.i_mode))
	{
		printf("ERROR: %s is a directory\n", pathname); 
		iput(mip); 
		return 0; 
	}
	
	if (S_ISLNK(mip->INODE.i_mode))
	{
		symcheck = 1; 
		iput(mip); 
	}
		
	if (is_parent(pathname) != 0)
	{
		strcpy(parent, dirname(pathname)); 
		strcpy(child, basename(pathname)); 
		ino = getino(parent); 
		pip = iget(dev, ino); //get parent mip
	} 
	else
	{
		strcpy(parent, "."); 
		strcpy(child, pathname); 
		pip = iget(dev, running->cwd->ino); 
	}
 
	rm_child(pip, child); 
	pip->INODE.i_atime = pip->INODE.i_mtime = time(0L); 
	pip->dirty = 1; 
	iput(pip); 
	
	mip->INODE.i_links_count--; 

	if (mip->INODE.i_links_count == 0)
	{
		//allocate temp blocks indicies for block deallocation
		for (int i = 0; i < 15; i++)
			tempblocks[i] = mip->INODE.i_block[i]; 
			
		if (symcheck == 0) //file is not a symlink 
		{
			for (int i = 0; i < 12; i++) //deallocate direct blocks 
			{
				if (tempblocks[i] != 0)
					bdealloc(mip->dev, tempblocks[i]); 
			}
			
			if (tempblocks[12] != 0) //deallocate indirect 
			{
				get_block(mip->dev, tempblocks[12], buf); 
				int *k = (int *)buf; 
				for (int i = 0; i < 256; i++)
				{
					if (*k != 0)
						bdealloc(mip->dev, *k); 
					k++; 
				}
			}
			
			if (tempblocks[13] != 0) //deallocate doubleindirect
			{
				get_block(mip->dev, tempblocks[13], buf); 
				int *k = (int *)buf; 
				for (int i = 0; i < 256; i++)
				{
					if (*k != 0)
					{
						get_block(mip->dev, *k, tbuf); 
						int *j = (int *)tbuf; 
						for (int l = 0; l < 256; l++)
						{
							if (*j != 0)
								bdealloc(mip->dev, *j); 
							j++; 
						}
					}
					k++; 
				}
			}
		}
		//deallocate INODE
		idealloc(mip->dev, mip->ino); 
	}
	mip->dirty = 1; 
	iput(mip); 
	
	return 1; 
}

int symlink()
{
	int ino, result; 
	MINODE *mip, *pip; 
	char *cp, *parent, *child; 
	
	if (pathname[0] == 0)
	{
		printf("ERROR: no pathname for original file given\n"); 
		return 0; 
	}
	if (parameters[0] == 0)
	{
		printf("ERROR: no pathname for symlink file given\n"); 
		return 0; 
	}
		
	ino = getino(pathname); 
	if (ino == 0)
		return 0; 
	mip = iget(dev, ino); 
		
	if (!S_ISREG(mip->INODE.i_mode) && !S_ISDIR(mip->INODE.i_mode)) 
	{
		printf("ERROR: %s is not a regular file or a directory\n", pathname); 
		iput(mip); 
		return 0; 
	} 
	iput(mip); 
	
	if (is_parent(parameters) != 0)
	{
		parent = dirname(parameters); 
		child = basename(parameters); 
		ino = getino(parent); 
		if (ino == 0)
			return 0; 
		
		pip = iget(dev, ino); 
	}
	else 
	{
		pip = iget(dev, running->cwd->ino); 
		child = (char *)malloc((strlen(parameters) + 1) * sizeof(char)); 
		strcpy(child, parameters); 
		parent = (char *)malloc(2 * sizeof(char)); 
		strcpy(parent, "."); 
	}
	
	//check that pip is a directory 
	if (!S_ISDIR(pip->INODE.i_mode))
	{
		printf("ERROR: %s is not a directory\n", parent); 
		iput(pip); 
		return 0; 
	}
	if (search(pip, child) != 0)
	{
		printf("ERROR: %s already exists\n", child); 
		iput(pip); 
		return 0; 
	}
	
	result = my_creat(pip, child); //create file, filemode changes below
	
	ino = getino(parameters); 
	if (ino == 0)
		return 0; 
		
	mip = iget(dev, ino); //get newly created inode
	mip->INODE.i_mode = 0xA1FF; //symlink mode
	strcpy((char *)(mip->INODE.i_block), pathname); 
	mip->INODE.i_size = strlen(pathname); 
	mip->dirty = 1; 
	iput(mip); 
	
	return result; 
}

int touchfile()
{
	int ino; 
	MINODE *mip; 
	
	if (pathname[0] == 0)
	{
		printf("ERROR: no pathname given\n"); 
		return 0; 
	}
		
	ino = getino(pathname); 
	if (ino == 0)
		return 0; 
		
	mip = iget(dev, ino); 
	
	mip->INODE.i_atime = time(0L); 
	mip->dirty = 1; 
	
	iput(mip); 
	return 1; 
}

int get_stat()
{
	struct stat fstat; 
	int ino; 
	MINODE *mip; 
	
	if (pathname[0] == 0)
	{
		printf("ERROR: no pathname given\n"); 
		return 0; 
	} 
		
	ino = getino(pathname); 
	if (ino == 0)
		return 0; 
	
	mip = iget(dev, ino); 
	
	fstat.st_dev = dev; 
	fstat.st_ino = ino; 
	fstat.st_mode = mip->INODE.i_mode; 
	fstat.st_uid = mip->INODE.i_uid; 
	fstat.st_nlink = mip->INODE.i_links_count; 
	fstat.st_size = mip->INODE.i_size; 
	fstat.st_atime = mip->INODE.i_atime; 
	fstat.st_mtime = mip->INODE.i_mtime; 
	fstat.st_ctime = mip->INODE.i_ctime; 
	
	printf("Device: %d   ", fstat.st_dev); 
	printf("Ino: %d   ", fstat.st_ino); 
	printf("Mode: %4x\n", fstat.st_mode); 
	printf("uid: %d   ", fstat.st_uid);  
	printf("Num Links: %d   ", fstat.st_nlink); 
	printf("Size: %d\n", fstat.st_size); 
	printf("atime: %s", ctime(&fstat.st_atime)); 
	printf("mtime: %s", ctime(&fstat.st_mtime)); 
	printf("ctime: %s", ctime(&fstat.st_ctime)); 
	
	iput(mip); 
	
	return 1; 
}

int changemod()
{
	int mode, ino; 
	MINODE *mip; 
	
	if (pathname[0] == 0)
	{
		printf("ERROR: No pathname given\n"); 
		return 0; 
	}
	if (parameters[0] == 0)
	{
		printf("ERROR: No permissions given\n"); 
		return 0; 
	}
	
	sscanf(parameters, "%o", &mode);  
	ino = getino(pathname); 
	
	if (ino == 0)
		return 0; 
		
	mip = iget(dev, ino); 
	
	if (S_ISREG(mip->INODE.i_mode))
		mip->INODE.i_mode = 0100000 + mode; 
	else if (S_ISDIR(mip->INODE.i_mode))
		mip->INODE.i_mode = 0040000 + mode; 
	else //symbolic link file 
		mip->INODE.i_mode = 0120000 + mode; 
		
	mip->dirty = 1; 
	iput(mip); 
	return 1; 
}
















