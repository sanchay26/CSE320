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
void tokenisePipe(char *cmddup);
int checkbuiltin(char *param);
void redirection();
int file_exist(char *filename);
void revertfiledescriptor();
void execute(char **tokenisedpipe, char* inputfile, char* outputfile, int count, int numofPipe);


typedef struct process
{
  struct process *next;       /* next process in pipeline */
  char *completearg;
  char *argument[25];                /* for exec */
  pid_t pid;                  /* process ID */
  char completed;             /* true if process has completed */
  char stopped;               /* true if process has stopped */
  int status;                 /* reported status value */
} process;

typedef struct job
{
  struct job *next;           /* next active job */
  char *command;              /* command line, used for messages */
  process *first_process;     /* list of processes in this job */
  process *last_process;
  pid_t pgid;                 /* process group ID */
  char notified;              /* true if user told about stopped job */
  //struct termios tmodes;      /* saved terminal modes */
  int stdin, stdout, stderr;  /* standard i/o channels */
} job;

job* createnewjob();

void addjobtolist(job *newjob);
void tokeniseProcess(job *jobprocess, char *command, char * inputfile, char* outputfile);
void addprocesstolist(job* existingjob , process* newprocess);
process* createnewprocess(job *jobprocess);

void printjobs();
void removespaces(char* s);