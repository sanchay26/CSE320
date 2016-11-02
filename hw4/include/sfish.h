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
void testtokenise(char *cmddup, char **param);
int checkbuiltin(char *param);
void redirection();
int file_exist(char *filename);
void revertfiledescriptor();