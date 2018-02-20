/*
Zac Kasper
Cs360 - Lab3
Server Side Code
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

#define  MAX 256

// Define variables:
struct sockaddr_in  server_addr, client_addr, name_addr;
struct hostent *hp;

int  mysock, client_sock;              // socket descriptors
int  serverPort;                     // server port number
int  r, length, n;                   // help variables

char *functArr[] = {"get", "put", "ls", "cd", "mkdir", "rmdir", "pwd", "rm"};
char myargv[12][MAX] = {""}; 

struct stat mystat, *sp; 
char *t1 = "xwrxwrxwr-------"; 
char *t2 = "----------------"; 

// Server initialization code:

int server_init(char *name)
{
   printf("==================== server init ======================\n");   
   // get DOT name and IP address of this host

   printf("1 : get and show server host info\n");
   hp = gethostbyname(name);
   if (hp == 0){
      printf("unknown host\n");
      exit(1);
   }
   printf("    hostname=%s  IP=%s\n",
               hp->h_name,  inet_ntoa(*(long *)hp->h_addr));
  
   //  create a TCP socket by socket() syscall
   printf("2 : create a socket\n");
   mysock = socket(AF_INET, SOCK_STREAM, 0);
   if (mysock < 0){
      printf("socket call failed\n");
      exit(2);
   }

   printf("3 : fill server_addr with host IP and PORT# info\n");
   // initialize the server_addr structure
   server_addr.sin_family = AF_INET;                  // for TCP/IP
   server_addr.sin_addr.s_addr = htonl(INADDR_ANY);   // THIS HOST IP address  
   server_addr.sin_port = 0;   // let kernel assign port

   printf("4 : bind socket to host info\n");
   // bind syscall: bind the socket to server_addr info
   r = bind(mysock,(struct sockaddr *)&server_addr, sizeof(server_addr));
   if (r < 0){
       printf("bind failed\n");
       exit(3);
   }

   printf("5 : find out Kernel assigned PORT# and show it\n");
   // find out socket port number (assigned by kernel)
   length = sizeof(name_addr);
   r = getsockname(mysock, (struct sockaddr *)&name_addr, &length);
   if (r < 0){
      printf("get socketname error\n");
      exit(4);
   }

   // show port number
   serverPort = ntohs(name_addr.sin_port);   // convert to host ushort
   printf("    Port=%d\n", serverPort);

   // listen at port with a max. queue of 5 (waiting clients) 
   printf("5 : server is listening ....\n");
   listen(mysock, 5);
   printf("===================== init done =======================\n");
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
  for (int i = 0; i < 8; i++)
    {
      if (strcmp(functArr[i], tempCmd) == 0)
	return i;
    }
  return -1; //command is not in function array 
}

// REMOTE SYSCALLS HERE
// ls 
	//each iteration of ls_file sends a buffer to the client and loops again 
char *ls_file(char *fname) //given by KC Wang, modified by myself 
{
  struct stat fstat, *sp;
  int r, i;
  char ftime[64];
  char temp[24] = {'\0'}; 
  char buffer[MAX] = {'\0'}; 

  //strcpy(buffer, "\0"); 
  sp = &fstat;
  //printf("name=%s\n", fname); getchar();

  if ( (r = lstat(fname, &fstat)) < 0){
     printf("can't stat %s\n", fname); 
     exit(1);
  }

  if ((sp->st_mode & 0xF000) == 0x8000)
     strcat(buffer, "-"); 
	 //printf("%c",'-');
  if ((sp->st_mode & 0xF000) == 0x4000)
     strcat(buffer, "d"); 
	 //printf("%c",'d');
  if ((sp->st_mode & 0xF000) == 0xA000)
     strcat(buffer, "l"); 
	 //printf("%c",'l');
 

  for (i=8; i >= 0; i--){
    if (sp->st_mode & (1 << i))
	{
		//strcpy(temp, t1[i]); 
		strncat(buffer, t1[i], 1); 	
	}
	//printf("%c", t1[i]);
    else
	{
		strcpy(temp, t2[i]); 
		strncat(buffer, t2[i], 1);
	} 	
	//printf("%c", t2[i]);
  }
  printf("buffer: %s\n", buffer);

  //printf("%4d ",sp->st_nlink);
  //strncpy(temp, sp->st_nlink, 4);
  snprintf(temp, 4, "%d", sp->st_nlink); 
  strncat(buffer, temp, 4); 
  strcat(buffer, " "); 
  //printf("%4d ",sp->st_gid);
  //strncpy(temp, sp->st_gid, 4); 
  snprintf(temp, 4, "%d", sp->st_gid); 
  strncat(buffer, temp, 4); 
  strcat(buffer, " "); 
  //printf("%4d ",sp->st_uid);
  //strncpy(temp, sp->st_uid, 4); 
  snprintf(temp, 4, "%d", sp->st_uid);
  strncat(buffer, temp, 4); 
  strcat(buffer, " "); 
  //printf("%8d ",sp->st_size);
  //strncpy(temp, sp->st_size, 8); 
  snprintf(temp, 8, "%d", sp->st_size);
  strncat(buffer, temp, 8); 
  strcat(buffer, " "); 

  // print time
  strcpy(ftime, ctime(&sp->st_ctime));
  ftime[strlen(ftime)-1] = 0;
  strcat(buffer, ftime); 
  strcat(buffer, "  "); 
  //printf("%s  ",ftime);

  // print name
  //printf("%s", basename(fname));  
  strcat(buffer, basename(fname)); 

  // print -> linkname if it's a symbolic file
  if ((sp->st_mode & 0xF000)== 0xA000){ // YOU FINISH THIS PART
     // use readlink() SYSCALL to read the linkname
     // printf(" -> %s", linkname);
  }
  printf("%s\n", buffer);
  return buffer; 
}


main(int argc, char *argv[])
{
   char *hostname;
   char line[MAX], buf[MAX] = {'\0'};
   char *cwd; 
   int test; 

   if (argc < 2)
      hostname = "localhost";
   else
      hostname = argv[1];
 
   server_init(hostname); 

   // Try to accept a client request
   while(1){
     printf("server: accepting new connection ....\n"); 

     // Try to accept a client connection as descriptor newsock
     length = sizeof(client_addr);
     client_sock = accept(mysock, (struct sockaddr *)&client_addr, &length);
     if (client_sock < 0){
        printf("server: accept error\n");
        exit(1);
     }
     printf("server: accepted a client connection from\n");
     printf("-----------------------------------------------\n");
     printf("        IP=%s  port=%d\n", inet_ntoa(client_addr.sin_addr.s_addr),
                                        ntohs(client_addr.sin_port));
     printf("-----------------------------------------------\n");

     // Processing loop: newsock <----> client
     while(1){
       n = read(client_sock, line, MAX);
       if (n==0){
           printf("server: client died, server loops\n");
           close(client_sock);
           break;
       }
       //PROCESS ACTION HERE
       parseInput(line);
       
       int functIndex = getFunctIndex(myargv[0]);
       if (functIndex < 0)
	 printf("Error: invalid instruction\n");
       else //remote function call 
	 {
	   printf("function index: %d\nRemote Function: %s\n", functIndex, functArr[functIndex]);
      printf("server: read  n=%d bytes; line=[%s]\n", n, line);

	  if (functIndex == 0) //get
	  {
		struct stat getstat, *sp; 
		sp = &getstat; 
		printf("getting lstat of %s\n", myargv[1]); 
		if ((r = lstat(myargv[1], &getstat)) < 0)
		{
			strcpy(line, "get filename BAD"); 
		}
		else 
		{
			sprintf(buf, "%d", sp->st_size); 
			strcpy(line, buf); 
			write(client_sock, line, MAX); //send filesize to client
			int fd = open(myargv[1], O_RDONLY); 
			if (fd > -1)
			{
				while (n = read(fd, buf, MAX))
				{
					write(client_sock, buf, n); 
				}
				close(myargv[1]); 
			} 
			else
				strcpy(line, "get readopen failed"); 
		}
	  }
	  if (functIndex == 1) //put
	  {
		n = read(client_sock, line, MAX); //line holds filesize from client HERE
		int SIZE = atoi(line); 
		int count = 0; 
		int fd = open(myargv[1], O_CREAT | O_WRONLY, 0666); 
		while (count < SIZE)
		{
			n = read(client_sock, buf, MAX); 
			count += n; 
			write(fd, buf, n); 
		}
		close(fd);
		strcpy(line, "put succeeded"); 
	  }

	  if (functIndex == 2) //ls
	  {
		/*cwd = getcwd(buf, sizeof(buf));		
		DIR *thisDir = opendir(cwd); 
		struct dirent *tempFile; 
		if (thisDir == NULL)
			printf("Error: failed to open directory\n"); 
		else 
		{	
			while ((tempFile = readdir(thisDir)) != NULL)
			{
				printf("ls_file: %s\n", tempFile->d_name); 
				strcpy(buf, "\0"); 
				strcpy(buf, ls_file(tempFile->d_name)); 
				strcat(buf, "\n"); 
				write(client_sock, buf, MAX);
			}
			closedir(thisDir); 
		}*/
		strcpy(line, "remote ls not currently implemented"); 
	  }
	  if (functIndex == 3) //cd
	  {
		printf("cd running\n"); 		
		test = chdir(myargv[1]); 
		if (test == 0)
			strcpy(line, "cd OK"); 
		else 
			strcpy(line, "cd FAILED"); 
	  }
	  if (functIndex == 4) //mkdir
	  {
		printf("mkdir running\n"); 		
		test = mkdir(myargv[1], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); 
		if (test == 0)
			strcpy(line, "mkdir OK"); 
		else 
			strcpy(line, "mkdir FAILED"); 
	  }
	  if (functIndex == 5) //rmdir
	  {
		printf("rmdir running\n"); 		
		test = rmdir(myargv[1]); 
		if (test == 0)
			strcpy(line, "rmdir OK"); 
		else 
			strcpy(line, "rmdir FAILED"); 
	  }
	  if (functIndex == 6) //pwd
	  {
		cwd = getcwd(buf, sizeof(buf)); 
		strcpy(line, cwd); 
	  }
	  if (functIndex == 7) //rm
	  {
		printf("rm running\n"); 
		test = remove(myargv[1]); 
		if (test == 0)
			strcpy(line, "rm OK"); 
		else 
			strcpy(line, "rm FAILED"); 
	  }

      // send the echo line to client 
      n = write(client_sock, line, MAX);

      printf("server: wrote n=%d bytes; REPLY=[%s]\n", n, line);
      printf("server: ready for next request\n");
	 } 
     }
   } 
}
