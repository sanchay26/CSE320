#include "sfish.h"
#include <unistd.h> 
#include <sys/wait.h>


#define MAX_PARAMS 15

int numofParam=0;
int exitFlag =0;
char *param[MAX_PARAMS+1];
char prevDir[1024];
int firstcd = 0;

int main(int argc, char** argv) {
    //DO NOT MODIFY THIS. If you do you will get a ZERO.
    rl_catch_signals = 0;
    //This is disable readline's default signal handlers, since you are going
    //to install your own.

    char *cmd;
    

    while((cmd = readline("sfish> ")) != NULL) {
        
        if (strcmp(cmd,"quit")==0){
            break;
        }

        char* cmddup = strdup(cmd);
        
        tokenise(cmddup,param);

        if(numofParam == 0){
            continue;
        }

        if(strcmp(param[0],"exit")==0 && numofParam == 1){
            
            exit(3);
        }

        if(strcmp(param[0],"cd")==0 && numofParam<=2){

            cd();
        }
        // This is PWD
        if(strcmp(param[0],"pwd")==0 && numofParam==1){

            char cwd[1024];

            if (getcwd(cwd, sizeof(cwd)) != NULL){

            fprintf(stdout, "Current working dir: %s\n", cwd);

            }

        }
        
        if(fork() == 0){

            if (strcmp(param[0],"help")==0 && numofParam == 1){
                printf("%s\n","*******YO BITCH GET OUT DA WAY*********");
            }

            
          exit(0);  
        }

        //this is the parent processs......
        else{
            wait(NULL);
        }
           

        //printf("%s\n",cmd);

        //All your debug print statments should be surrounded by this #ifdef
        //block. Use the debug target in the makefile to run with these enabled.
        #ifdef DEBUG
        fprintf(stderr, "Length of command entered: %ld\n", strlen(cmd));
        #endif
        //You WILL lose points if your shell prints out garbage values.

    }

    //Don't forget to free allocated memory, and close file descriptors.
    free(cmd);
    //WE WILL CHECK VALGRIND!

    return EXIT_SUCCESS;
}


void tokenise(char *cmddup, char **param){

    char *init;
    int i = 1;
    numofParam = 0;
    init = strtok (cmddup," ");
    
    if(init == NULL){
        return;
    }
    param[0] = init;
    numofParam++;
    while (init != NULL)
    {
        param[i] = strtok (NULL, " ");
        if(param[i] == NULL) 
        break;
        
        i++;
        numofParam++;
    }

}

void cd(){

    char cwd[1024];
   
    if(numofParam == 1){

        getcwd(cwd, sizeof(cwd));
        strcpy(prevDir,cwd);
                    
        if(chdir(getenv("HOME"))!=0){
            perror("Error in the chdir");
        }
    }

    else if(numofParam == 2){

        if(strcmp(param[1],"-")==0){

            if(firstcd != 1){
                printf("%s\n","Old pwd not set" );
            }
            
            else{
                getcwd(cwd, sizeof(cwd));
                chdir(prevDir);
                strcpy(prevDir,cwd);
                return;
            }
        }
        
        getcwd(cwd, sizeof(cwd));
        strcpy(prevDir,cwd);
        
        if(chdir(param[1])!=0){
            perror("Error in the chdir");
            //return invalid
        }    
    }

    else{
        printf("%s\n","Error in the CD");
    }

    firstcd = 1;
}