#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
void tokenise(char *cmddup, char **param);
void cd();
void prt();
void getPrompt();
void chpmt();
void chclr();
void pwd();
void help();
void tokenisePath();
int checkbuiltin(char *param);

int file_exist(char *filename);