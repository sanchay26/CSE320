#include "sfish.h"
#include <unistd.h> 
#include <sys/wait.h>
#include <sys/stat.h>


#define MAX_PARAMS 15

int numofParam=0;
int numofPath = 0;
int exitFlag =0;
char *param[MAX_PARAMS+1];
char *patharray[4096];
char prevDir[1024];
int firstcd = 0;
char hostname[1024];
char username[1024];
char prompt[4098];
char *dash = "-";
char *colon = ":";
char *attherate = "@";
char *sfish = "sfish";
char *usertoggle ="1";
char *hosttoggle="1";
int usercolor = 2 ;
int hostcolor = 0 ;
int userbold = 0;
int hostbold = 0;

char *builtin[] = {"exit","cd","chpmt","chclr","help","pwd","prt"};

char *promptcolor[] ={"red","green","blue","yellow","black","white","magenta","cyan"};

char *colorcodes[] ={"\x1b[31m","\x1b[32m","\x1b[34m","\x1b[33m","\x1b[30m","\x1b[37m","\x1b[35m","\x1b[36m"};

char * colorBoldCodes[]  = {"\033[1m\033[31m","\033[1m\033[32m","\033[1m\033[34m","\033[1m\033[33m", "\033[1m\033[30m","\033[1m\033[37m" ,"\033[1m\033[35m","\033[1m\033[36m" };




int main(int argc, char** argv) {
    //DO NOT MODIFY THIS. If you do you will get a ZERO.
    rl_catch_signals = 0;
    //This is disable readline's default signal handlers, since you are going
    //to install your own.
    hostname[1023] = '\0';
    gethostname(hostname, 1023);
   // username = getenv("USER");
   // strcpy(username,getenv("USER"));
    getPrompt("1","1");
    char *cmd;
    tokenisePath();
    //printf("%s\n",getenv );
    
    while((cmd = readline(prompt)) != NULL) {

        
        char* cmddup = strdup(cmd);
        
        tokenise(cmddup,param);


        if(numofParam == 0){
            continue;
        }

        if (strcmp(cmd,"quit")==0){
            break;
        }

        if(checkbuiltin(param[0])==1){

            if(strcmp(param[0],"exit")==0 && numofParam == 1){
                
                exit(3);
            }

            if(strcmp(param[0],"cd")==0 && numofParam<=2){
                cd();
            }

            if(strcmp(param[0],"chpmt")==0 && numofParam == 3){

                chpmt();
            }

            if(strcmp(param[0],"chclr")==0 && numofParam == 4){
               
                chclr();

            }
            
            getPrompt(usertoggle,hosttoggle);

        }
        
        else{

            char *temp = malloc(4096);
            strcpy(temp,"");
            for(int i=0;i<numofPath;i++){

                strcpy(temp,patharray[i]);
                strcat(temp,"/");
                strcat(temp,param[0]);

                if (file_exist (temp))
                {
                  
                    if(fork()==0){
                        execvp(temp,param);  
                        break;
                        exit(0);
                    }

                    wait(NULL);
                }
                
            }
            char exec[1024]="";
            getcwd(exec, sizeof(exec));
            strcat(exec,"/");
            strcat(exec,param[0]);
            if (file_exist (exec))
            {
              
                if(fork()==0){
                    execvp(exec,param);  
                    exit(0);
                }

                wait(NULL);
            }
            free(temp);
        }
        
        // if(fork() == 0){

        //     if (strcmp(param[0],"help")==0 && numofParam == 1){
        //         printf("%s\n","*******YO BITCH GET OUT DA WAY*********");
        //     }

        //     // This is PWD

        //     if(strcmp(param[0],"pwd")==0 && numofParam==1){

        //         char cwd[1024];

        //         if (getcwd(cwd, sizeof(cwd)) != NULL){

        //         fprintf(stdout, "Current working dir: %s\n", cwd);

        //         }
        //     }
            
        //     if(strcmp(param[0],"prt")==0){
        //         prt();
                
        //     }

        //     exit(0);  
        // }

        // //this is the parent processs......
        // else{
        //     wait(NULL);
        // }
           

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

int checkbuiltin(char *param){

    for(int i=0;i<7;i++){

        if(strcmp(param,builtin[i])==0){
            return 1;
        }
    }

return 0;
}

int file_exist (char *filename)
{
  struct stat   buffer;   
  return (stat (filename, &buffer) == 0);
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

void tokenisePath(){
    //patharray = malloc(1024*strlen(getenv("PATH")));
    char *init;
    int i = 1;
    numofPath = 0;
    init = strtok (getenv("PATH"),":");
    
    if(init == NULL){
        return;
    }
    patharray[0] = init;
    numofPath++;
    while (init != NULL)
    {
        patharray[i] = strtok (NULL, ":");
        
        if(patharray[i] == NULL) 
            break;
            
            i++;
            numofPath++;
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


void prt(){
    char *name[4];
    name[0] = "sh";
    name[1] = "-c";
    name[2] = "echo $?";
    name[3] = NULL;
    execvp("/bin/sh", name);
}


void getPrompt(char *user, char *host){

    strcpy(prompt,"");

    strcat(prompt,sfish); 
    char filename[1024] ="";
    size_t homelen = strlen(getenv("HOME"));


    if( strcmp(user,"1")==0 || strcmp(host,"1")==0){
        strcat(prompt,dash);
        if(userbold == 0){
            strcat(prompt,colorcodes[usercolor]);
        }
        
        else{
            strcat(prompt,colorBoldCodes[usercolor]);
        } 
            
    }

    if(strcmp(user,"1")==0){
        strcat(prompt,username);
        strcat(prompt,colorcodes[5]);
        
        if(strcmp(host,"1")==0){
            
            strcat(prompt,attherate);
        }
    }

    if(strcmp(host,"1")==0){
        if(hostbold == 0){
            //printf("%s\n","HERE 0" );
          strcat(prompt,colorcodes[hostcolor]);  
        }
        
        else{
            //printf("%s\n","HERE 1" );
         strcat(prompt,colorBoldCodes[hostcolor]);   
        }
        
        strcat(prompt,hostname);
        strcat(prompt,colorcodes[5]);
    }

    strcat(prompt,colon);
    strcat(prompt,"[");
    
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));

    size_t totalsize = strlen(cwd);

    strncpy(filename,cwd+homelen,totalsize-homelen);
    strcat(prompt,filename);
    strcat(prompt,"]");
    strcat(prompt,">");
}

void chpmt(){


    if(strcmp(param[1],"user") ==0 && strcmp(param[2],"0")==0){
        usertoggle = "0";
        getPrompt("0",hosttoggle);
    }
    if(strcmp(param[1],"user") ==0 && strcmp(param[2],"0")==1){
        usertoggle = "1";
        getPrompt("1",hosttoggle);
    }
    if(strcmp(param[1],"machine") ==0 && strcmp(param[2],"0")==0){
        hosttoggle = "0";
        getPrompt(usertoggle,"0");
    }
    if(strcmp(param[1],"machine") ==0 && strcmp(param[2],"0")==1){
        hosttoggle = "1";
        getPrompt(usertoggle,"1");
    }
    //**********************check if other than 0 or 1 
    //printf("%s\n","ERROR IN CHPMT" );

}

void chclr(){


    if(strcmp(param[1],"user") ==0){

        for(int i=0;i<8;i++){
            
            if(strcmp(param[2],promptcolor[i])==0){
                usercolor = i;
            }

        }
        if(strcmp(param[3],"0")==0){
            userbold = 0 ;
        }
        else if(strcmp(param[3],"1")==0){
            userbold =1;
        }
    }
    if(strcmp(param[1],"machine") ==0){

        for(int i=0;i<8;i++){
            
            if(strcmp(param[2],promptcolor[i])==0){
                hostcolor = i;
            }
        }
        if(strcmp(param[3],"0")==0){
            hostbold = 0 ;
        }
        else if(strcmp(param[3],"1")==0){
            hostbold =1;
        }
    }
}
