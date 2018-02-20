//Zac Kasper
//Cs360 - Final Project Code
//Main Functions

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "type.h"
#include "util.h"

void printfile(MINODE *mip, char *filename); 
void printdirectory(MINODE *mip); 
int ls(char *givenpath);

int cd(char *pathname); 

void rpwd(MINODE *wd);
void pwd(MINODE *wd); 

int makekdir();
int my_mkdir(MINODE *pip, char *name); 

int creation(); 
int my_creat(MINODE *pip, char *name); 

int removedir();
int rm_child(MINODE *pip, char *name); 

int hardlink(); 
int unlink(); 
int symlink();

int touchfile(); 
int get_stat(); 
int changemod(); 

#endif
