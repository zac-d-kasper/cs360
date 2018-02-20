/*
Zac Kasper
Cs360 - Lab3
Client Side Code
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netdb.h>

#include <sys/stat.h>
#include <time.h> 
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>

#define MAX 256

// Define variables
struct hostent *hp;              
struct sockaddr_in  server_addr; 

int server_sock, r;
int SERVER_IP, SERVER_PORT; 

struct stat mystat, *sp; 
char *t1 = "xwrxwrxwr-------"; 
char *t2 = "----------------"; 

char myargv[12][MAX] = {""}; 

char *functArr[] = {"lcat", "lls", "lcd", "lmkdir", "lrmdir", "lpwd", "lrm", "get", "put"}; 

// clinet initialization code

int client_init(char *argv[])
{
  printf("======= client init ==========\n");

  printf("1 : get server info\n");
  hp = gethostbyname(argv[1]);
  if (hp==0){
     printf("unknown host %s\n", argv[1]);
     exit(1);
  }

  SERVER_IP   = *(long *)hp->h_addr;
  SERVER_PORT = atoi(argv[2]);

  printf("2 : create a TCP socket\n");
  server_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (server_sock<0){
     printf("socket call failed\n");
     exit(2);
  }

  printf("3 : fill server_addr with server's IP and PORT#\n");
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = SERVER_IP;
  server_addr.sin_port = htons(SERVER_PORT);

  // Connect to server
  printf("4 : connecting to server ....\n");
  r = connect(server_sock,(struct sockaddr *)&server_addr, sizeof(server_addr));
  if (r < 0){
     printf("connect failed\n");
     exit(1);
  }

  printf("5 : connected OK to \007\n"); 
  printf("---------------------------------------------------------\n");
  printf("hostname=%s  IP=%s  PORT=%d\n", 
          hp->h_name, inet_ntoa(SERVER_IP), SERVER_PORT);
  printf("---------------------------------------------------------\n");

  printf("========= init done ==========\n");
}

int parseInput(char *input)
{
  int i=0;
  char *temp;
  char tempIn[MAX];
  strcpy(tempIn, input);

  memset(myargv, NULL, sizeof(myargv)); 
  temp = strtok(tempIn, " ");
  while (temp != NULL)
    {
      //myargv[i] = temp; 
	  strcpy(myargv[i], temp); 
      temp = strtok(NULL, " ");
      i++; 
    }
}

int getFunctIndex(char *cmd)
{
  char tempCmd[8]; 
  strcpy(tempCmd, cmd);    
  for (int i = 0; i < 9; i++)
    { 
      if (strcmp(functArr[i], tempCmd) == 0)
	{ 
	  return i;
	}
    }
  return -1; //command is not in function array 
}

int ls_file(char *fname) //given by KC Wang 
{
  struct stat fstat, *sp;
  int r, i;
  char ftime[64];

  sp = &fstat;
  //printf("name=%s\n", fname); getchar();

  if ( (r = lstat(fname, &fstat)) < 0){
     printf("can't stat %s\n", fname); 
     exit(1);
  }

  if ((sp->st_mode & 0xF000) == 0x8000)
     printf("%c",'-');
  if ((sp->st_mode & 0xF000) == 0x4000)
     printf("%c",'d');
  if ((sp->st_mode & 0xF000) == 0xA000)
     printf("%c",'l');

  for (i=8; i >= 0; i--){
    if (sp->st_mode & (1 << i))
	printf("%c", t1[i]);
    else
	printf("%c", t2[i]);
  }

  printf("%4d ",sp->st_nlink);
  printf("%4d ",sp->st_gid);
  printf("%4d ",sp->st_uid);
  printf("%8d ",sp->st_size);

  // print time
  strcpy(ftime, ctime(&sp->st_ctime));
  ftime[strlen(ftime)-1] = 0;
  printf("%s  ",ftime);

  // print name
  printf("%s", basename(fname));  

  // print -> linkname if it's a symbolic file
  if ((sp->st_mode & 0xF000)== 0xA000){ // YOU FINISH THIS PART
     // use readlink() SYSCALL to read the linkname
     // printf(" -> %s", linkname);
  }
  printf("\n");
}
int ls_dir(char *dname)
{
	DIR *thisDir = opendir(dname); 
	struct dirent *tempFile; 
	if ((thisDir = opendir(dname)) == NULL)
		printf("Error: failed to open directory\n"); 
	else 
	{
		while ((tempFile = readdir(thisDir)) != NULL)
			ls_file(tempFile->d_name); 
		closedir(thisDir); 
	}
}

main(int argc, char *argv[ ])
{
  int n;
  int functIndex = 0; 
  char line[MAX], ans[MAX], buf[MAX];
  char *cwd; //used for local function calls 

  if (argc < 3){
     printf("Usage : client ServerName SeverPort\n");
     exit(1);
  }

  client_init(argv);
  // sock <---> server
  printf("********  processing loop  *********\n");
  while (1){
    printf("input a line : ");
    bzero(line, MAX);                // zero out line[ ]
    fgets(line, MAX, stdin);         // get a line (end with \n) from stdin

    line[strlen(line)-1] = 0;        // kill \n at end
    if (line[0]==0)                  // exit if NULL line
       exit(0);

    char tempLine[MAX];
    strcpy(tempLine, line);

    parseInput(tempLine); 
    
    functIndex = getFunctIndex(myargv[0]);
    if (functIndex < 0)
      {
	// Send ENTIRE line to server
	n = write(server_sock, line, MAX);
	printf("client: wrote n=%d bytes; line=(%s)\n", n, line); 

	// Read a line from sock and show it
	n = read(server_sock, ans, MAX);
	//printf("client: read  n=%d bytes; echo=(%s)\n",n, ans); 
	printf("%s\n", ans); 
      }
    else //local function call 
      {	
		if (functIndex == 1) //lls
		{
			cwd = getcwd(buf, sizeof(buf)); 			
			ls_dir(cwd); 
		}
		if (functIndex == 2) //lcd
		{
			chdir(myargv[1]); 
		}
		if (functIndex == 3) //lmkdir
		{
			//printf("myargv[1]: %s\n", myargv[1]); 			
			mkdir(myargv[1], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); 
		}
		if (functIndex == 4) //lrmdir
		{
			rmdir(myargv[1]); 
		}
		if (functIndex == 5) //lpwd
		{
			cwd = getcwd(buf, sizeof(buf)); 
			printf("%s\n", cwd);
		}
		if (functIndex == 6) //rm
		{
			remove(myargv[1]); 
		}
		if (functIndex == 7) //get
		{
			n = write(server_sock, line, MAX);
			n = read(server_sock, ans, MAX); //ans holds filesize from server HERE
			int SIZE = atoi(ans); 
			int count = 0; 
			int fd = open(myargv[1], O_CREAT | O_WRONLY, 0666); 
			while (count < SIZE)
			{
				n = read(server_sock, buf, MAX); 
				count += n; 
				write(fd, buf, n); 
			}
			close(fd); 
		}
		if (functIndex == 8) //put
		{
			n = write(server_sock, line, MAX);
			struct stat getstat, *sp; 
			sp = &getstat; 
			printf("getting lstat of %s\n", myargv[1]); 
			if ((r = lstat(myargv[1], &getstat)) < 0)
			{
				printf("put filename BAD\n"); 
			}
			else 
			{
				sprintf(buf, "%d", sp->st_size); 
				strcpy(line, buf); 
				write(server_sock, line, MAX); //send filesize to server
				int fd = open(myargv[1], O_RDONLY); 
				if (fd > -1)
				{
					while (n = read(fd, buf, MAX))
					{
						write(server_sock, buf, n); 
					}
					close(myargv[1]); 
					read(server_sock, ans, MAX); //recieve success message
					printf("%s\n", ans); 
				} 
				else
					printf("put readopen failed\n"); 
			}
		}

		else
			printf("funct index: %d\nLocal Function: %s\n", functIndex, functArr[functIndex]);
      }
  }
}
