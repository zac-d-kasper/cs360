//Zac Kasper
//Cs360 - Running Main Program

#include "util.h"
#include "functions.h" 

char *cmd_strings[] = {
	"quit    |  exits the program\n",
	"ls      |  prints detailed contents of files in given directory\n", 
	"cd      |  changes cwd to given directory\n", 
	"pwd     |  prints the full name of the cwd\n", 
	"mkdir   |  creates a new directory at the given pathname\n", 
	"rmdir   |  removes a specified EMPTY directory\n", 
	"creat   |  creates an empty file at the given pathname\n", 
	"link    |  creates a hard-link to a given source file at the given pathname\n",
	"unlink  |  deletes a given hard-link pathname\n", 
	"symlink |  creates a symbolic link file to a given source at the given pathname\n", 
	"utime   |  updates the access time of the given pathname\n", 
	"stat    |  prints the detailed stats of a given file's pathname\n", 
	"chmod   |  changes the given file's permissions with an octal value\n", 
	"help    |  prints a detailed list of the functions that are currently implemented\n"
};

void print_help()
{
	for (int i = 0; i < 14; i++)
	{
		printf("%s", cmd_strings[i]); 
	}
}

int find_command()
{
  if (!strcmp(command, "quit"))
    return 0;
  else if (!strcmp(command, "ls"))
    return 1;
  else if (!strcmp(command, "cd"))
    return 2;
  else if (!strcmp(command, "pwd"))
    return 3;
	else if (!strcmp(command, "mkdir"))
		return 4; 
	else if (!strcmp(command, "creat"))
		return 5; 
	else if (!strcmp(command, "rmdir"))
		return 6; 
	else if (!strcmp(command, "link"))
		return 7; 
	else if (!strcmp(command, "unlink"))
		return 8; 
	else if (!strcmp(command, "symlink"))
		return 9; 
	else if (!strcmp(command, "utime"))
		return 10; 
	else if (!strcmp(command, "stat"))
		return 11; 
	else if (!strcmp(command, "chmod"))
		return 12; 
	else if (!strcmp(command, "help"))
		return 13; 
  return -1; 
}

char *device = "kasperdisk"; 
int main(int argc, char *argv[])
{
  int cmdtype;
  char line[128];
  if (argc > 1)
    {
      device = argv[1];
      printf("Target Device: %s\n", device); 
    }
		
  initialize(device);

  printf("%s ", running->cwd->mptr->devName); 
  pwd(running->cwd); 
  printf(" $ "); 
  while(fgets(line, 128, stdin) != NULL)
    {
      if (strcmp(line, "") != 0 && strcmp(line, "\n") != 0)
			{
				line[strlen(line) - 1] = 0; //removes '\n' char
				sscanf(line, "%s %s %256c", command, pathname, parameters);
				cmdtype = find_command();
				switch(cmdtype)
					{
						case 0: exit(0); break;
						case 1: ls(pathname); break;
						case 2: cd(pathname); break;
						case 3: pwd(running->cwd); printf("\n"); break; 
						case 4: makedir(); break; 
						case 5: creation(); break;
						case 6: removedir(); break; 
						case 7: hardlink(); break; 
						case 8: unlink(); break;  
						case 9: symlink(); break; 
						case 10: touchfile(); break; 
						case 11: get_stat(); break; 
						case 12: changemod(); break; 
						case 13: print_help(); break; 
						
						default:
							printf("ERROR: invalid command name %s\n", command); 
							break; 
					}
			}
      memset(line, 0, sizeof(line));
      printf("%s ", device); 
  		pwd(running->cwd); 
  		printf(" $ "); 
      memset(command, 0, sizeof(command));
      memset(pathname, 0, sizeof(pathname)); 
      memset(parameters, 0, sizeof(parameters)); 
    }

  return 0; 
}
