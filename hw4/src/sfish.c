#include "sfish.h"
#include <unistd.h> 
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>


#define MAX_PARAMS 15

int numofParam=0;
int numofParamcpy = 0;
int numofPath = 0;
int exitFlag =0;
char *param[MAX_PARAMS+1];
char *paramcpy[MAX_PARAMS+1];
char *pipetoken[MAX_PARAMS+1];
char *patharray[1024];
char prevDir[1024];
int  firstcd = 0;
char hostname[500];
char username[500];
char prompt[1024];
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
int outputredirect = 0;
int inputredirect = 0;
int dfd;
int fd;
int found = 0;
int enabledpipe = 0;
int bg;

char *builtin[] = {"exit","cd","chpmt","chclr","help","pwd","prt"};

char *promptcolor[] ={"red","green","blue","yellow","black","white","magenta","cyan"};

char *colorcodes[] ={"\x1b[31m","\x1b[32m","\x1b[34m","\x1b[33m","\x1b[30m","\x1b[37m","\x1b[35m","\x1b[36m"};

char * colorBoldCodes[]  = {"\033[1m\033[31m","\033[1m\033[32m","\033[1m\033[34m","\033[1m\033[33m", "\033[1m\033[30m","\033[1m\033[37m" ,"\033[1m\033[35m","\033[1m\033[36m" };

job *first_job = NULL;
job *last_job = NULL;


int main(int argc, char** argv) {
    //DO NOT MODIFY THIS. If you do you will get a ZERO.
    rl_catch_signals = 0;
    //This is disable readline's default signal handlers, since you are going
    //to install your own.
    
    char *cmd;

    bg = 0;
    
    getPrompt("1","1");

    tokenisePath();

    while((cmd = readline(prompt)) != NULL) {

        found = 0;
        enabledpipe = 0;
        bg = 0;

        for(int i =0; i<MAX_PARAMS;i++){
            
            param[i] = NULL;
        }
        
        char* cmddup = strdup(cmd);

        //strcpy(cmddup,cmd);

        //job* newjob = createnewjob();

        //newjob->command = cmd;

       
        //tokeniseProcess(newjob,cmddup,inputfile, outputfile);

        //printjobs();
        

        testtokenise(cmddup,param);
        

        if(numofParam == 0){
            continue;
        }

        if (strcmp(cmd,"quit")==0){
            break;
        }
        //It's a built-in command 
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

            if (strcmp(param[0],"help")==0 && numofParam == 1){
                help();
            }

            if(strcmp(param[0],"pwd")==0 && numofParam==1){
                pwd();
            }

            if(strcmp(param[0],"prt")==0){
                prt();
            }
            
            getPrompt(usertoggle,hosttoggle);
        }
        //Not a built in command 
        else {

          tokenisePipe(cmddup);

        }
        
        revertfiledescriptor();

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

void execute(char **tokenisedpipe, char* inputfile, char* outputfile, int count, int numofPipe){

    char temp[4096] = "";
    
    if(count == 0 && inputfile != NULL)
    {
        fd = open(inputfile,O_RDONLY,0666);
        dfd = dup(STDIN_FILENO);
        dup2(fd,STDIN_FILENO);
        close(fd);
    }
    
    if(count == numofPipe && outputfile != NULL)
    {

        fd = open(outputfile,O_CREAT | O_WRONLY,0666);
        dfd = dup(STDOUT_FILENO);
        dup2(fd,STDOUT_FILENO);
        close(fd);
    }
    
    for(int i=0;i<numofPath;i++){

        strcpy(temp,patharray[i]);
        strcat(temp,"/");
        strcat(temp,tokenisedpipe[0]);
        if (file_exist (temp))
        {
            found = 1;
            execvp(temp,tokenisedpipe);
        }
    }

    // Comes here if not found in path. It checks in current working directory if executable is present.
    
    if(found == 0){

        char exec[1024]="";
        getcwd(exec, sizeof(exec));
        strcat(exec,"/");
        strcat(exec,tokenisedpipe[0]);
        if (file_exist (exec))
        {
            execvp(exec,tokenisedpipe);  
        }
        //No command fouund 
        else {

            fprintf(stderr,"%s%s\n",param[0],": command not found");
        }
    }

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


void tokenise(char *cmddup, char **tokenisedpipe){

    char *init;
    int i = 1;
    numofParam = 0;
    init = strtok (cmddup," ");

    if(init == NULL){
        return;
    }
    tokenisedpipe[0] = init;
    while (init != NULL)
    {
        tokenisedpipe[i] = strtok (NULL, " ");
        if(tokenisedpipe[i] == NULL) 
        break;
        i++;
    }
}

void testtokenise(char *cmd, char **param){

    char *init;
    char *tok;
    char *cmddup = strdup(cmd);
    int i = 1;
    int k = 1;
    
    numofParam = 0;
    numofParamcpy = 0;
    
    init = strtok (cmddup," ");

    if(init == NULL){
        return;
    }
    param[0] = init;
    // if(strcmp(param[0],"<")==0 || strcmp(param[0],">")==0 || strcmp(param[0],"|")==0){
    //     return;
    // }
    paramcpy[0] = init;
    numofParam++;
    while (init != NULL)
    {
        tok = strtok (NULL, " ");
        
        if(tok == NULL) 
        break;

        if(strcmp(tok,">")==0){
            outputredirect = 1;
        }

        if(strcmp(tok,"<")==0){
            inputredirect = 1;
        }

        if(strcmp(tok,"|")==0){
            enabledpipe = 1;
        }

        
        if(outputredirect !=1 && inputredirect !=1 ){
            param[i] = tok;
            paramcpy[k] = tok;
            i++; k++;
        }
        else{
            paramcpy[k] = tok;
            k++;
        }
        numofParam = i;
        numofParamcpy = k;
    }
    // for (int m = 0; m < numofParam; m++)
    // {
    //     printf("HIII%s ",param[m]);
    // }
    // printf("\n");
    // for (int n = 0; n < numofParamcpy; n++)
    // {
    //     printf("%s ************",paramcpy[n]);
    // }
}
void tokenisePipe(char *cmd){
    char *init;
    int numofPipe = 0;
    char *final = NULL;
    char *cmddup; 
    int i = 1;
    //char *parsedtoken;
    char *inputfile = NULL;
    char *outputfile = NULL;
    //char *outputfile = NULL;
    cmddup = cmd;
    if(strstr(cmd,"&")!=NULL){
        cmddup = strsep(&cmd,"&");
        bg = 1;
    } 
    
    init = strtok (cmddup,"|");

    if(init == NULL){
       return;
    }

    pipetoken[0] = init;

    while (init != NULL){

        pipetoken[i] = strtok (NULL, "|");

        if(pipetoken[i] == NULL) 
        break;
        
        i++;
    }
    numofPipe = i-1;
    int pipefds[numofPipe*2];

    //int outdup = dup(STDOUT_FILENO);

    for(int i = 0; i<numofPipe; i++){
        pipe(pipefds+(i*2));
    }
    
    char *tokenisedpipe[MAX_PARAMS+1];
    int status;
    
    for (int count = 0; count <= numofPipe; count++)
    {
        memset(tokenisedpipe, 0, sizeof(tokenisedpipe));
        //---------------------------------------------------------------------
        if(count == 0){

            if(strstr(pipetoken[count],"<")!=NULL){
                
                final = strsep(&pipetoken[count],"<");
                
                if(strstr(pipetoken[count], ">") != NULL && numofPipe == 0)
                {
                    inputfile = strsep(&pipetoken[count], ">");
                    //check for spaces 
                    removespaces(inputfile);
                    outputfile = strsep(&pipetoken[count], ">");
                    //check for spaces
                    removespaces(outputfile);
                }
                else
                {
                    inputfile = strsep(&pipetoken[count], "<");
                    //check for spaces
                    removespaces(inputfile);
                }

            }

            else if(strstr(pipetoken[count], ">") != NULL)
            {
                final = strsep(&pipetoken[count], ">");
                outputfile = strsep(&pipetoken[count], ">");
                //check for spaces
                removespaces(outputfile);
            }
            else{
                
                final = pipetoken[count];
            }

            tokenise(final,tokenisedpipe);
        }
        
        else if(count == numofPipe)
        {
            if(strstr(pipetoken[count], ">") != NULL)
            {
                final = strsep(&pipetoken[count], ">");
                outputfile = strsep(&pipetoken[count], ">");
                //check for spaces
                removespaces(outputfile);
            }
            else{
                final = pipetoken[count];
            }

            tokenise(final,tokenisedpipe);
        }
        else{

            tokenise(final,tokenisedpipe);
        }

        pid_t pid = fork();

        if(pid == 0)
        {
            //printf("Process:%dInfile:%dOutfile:%d\n",count,pipefds[(count-1)*2],pipefds[count*2+1]);
            // signal(SIGQUIT, SIG_IGN);
            // signal(SIGTTOU, SIG_IGN);
            // signal(SIGTTIN, SIG_IGN);
            // signal(SIGTSTP, SIG_IGN);
            // signal(SIGINT, SIG_IGN);

            if(count!=0){
                dup2(pipefds[(count-1)*2],STDIN_FILENO);
            }
            
           // dup2(outdup,1);
            
            if(count != numofPipe && numofPipe > 0){
                dup2(pipefds[count*2+1],STDOUT_FILENO);
            }
            
            //printf("%s\n",inputfile);
            execute(tokenisedpipe,inputfile,outputfile,count,numofPipe);

            exit(0);
        }
                
        else if (pid == -1) 
        {
            perror("Error forking");
            // return -1;
        }

        
        else
        {
            if(count == numofPipe)
            {
                waitpid(pid, &status, WUNTRACED);
            }
            if(count < numofPipe && numofPipe > 0)
            {
                close(pipefds[count*2+1]);
            }
            if(count > 0 && numofPipe > 0)
            {
                close(pipefds[(count - 1)*2]);
            }
        }
       
    }
}

void tokenisePath(){

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

void tokeniseProcess(job *jobprocess, char *command, char * inputfile, char* outputfile){

    int i;
    int j;
    i=0;  
    char* token;
    char* token1;
    char* infile = NULL;

    printf("%s\n",infile);
    //char* outfile = strdup(outputfile);
    char* cmd;

    if(strstr(command,"&")!=NULL){
        cmd = strsep(&cmd,"&");
        bg = 1;
    } 
    while ((token = strsep(&cmd,"|")) != NULL)
    {

        if(strcmp(token, "") != 0)
        {       
            process* pro = createnewprocess(jobprocess);

            pro->completearg = token;

            if(i == 0){
                if(strstr(pro->completearg,"<")!=NULL){
                    
                    token = strsep(&pro->completearg,"<");
                    infile = strdup(strsep(&token,"<"));
                    printf("%s\n",infile);
                }
            }
        
            j=0;  
            
            while ((token1 = strsep(&token," ")) != NULL)
            {
                if(strcmp(token1, "") != 0)
                {       
                    pro->argument[j] = token1;
                    j++;    
                }       
            }

            i++;    
        }       
    }
}



void help(){

    pid_t pid = fork();
                
    if (pid == -1) 
    {
        perror("Error forking");
        // return -1;
    }
    
    else if (pid > 0)
    {
        wait(NULL); 
    }
    
    else 
    {   
        printf("%s\n","USAGE:");
        printf("%s\n","cd[DIR|[-[.[..]");
        printf("%s\n","chpmt[SETTING][TOGGLE]");
        printf("%s\n","chclr[SETTING][COLOR][BOLD]");
        printf("%s\n","pwd");
        printf("%s\n","prt");
        printf("%s\n","exit");
        exit(0);
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
void pwd(){

    pid_t pid = fork();
                
    if (pid == -1) 
    {
        perror("Error forking");
        // return -1;
    }
    
    else if (pid > 0)
    {
        wait(NULL); 
    }
    
    else 
    {
        char cwd[1024];
        
        if (getcwd(cwd, sizeof(cwd)) != NULL){
        printf("%s\n", cwd);
        exit(0);
        }
    }
}

void prt(){
    
    pid_t pid = fork();
                
    if (pid == -1) 
    {
        perror("Error forking");
        // return -1;
    }
    else if (pid > 0)
    {
         
         wait(NULL); 
    }
    else 
    {
        char *name[4];
        name[0] = "sh";
        name[1] = "-c";
        name[2] = "echo $?";
        name[3] = NULL;
        execvp("/bin/sh", name);
        exit(0);
    }
    
}


void getPrompt(char *user, char *host){

    hostname[1023] = '\0';
    gethostname(hostname, 1023);
    strcpy(username,getenv("USER"));
    strcpy(prompt,"");

    strcat(prompt,sfish); 
    char filename[1024] ="";
    size_t homelen = strlen(getenv("HOME"));

    if( strcmp(user,"1")==0 || strcmp(host,"1")==0){
        strcat(prompt,dash);
        if(userbold == 0){
            strcat(prompt,colorcodes[usercolor]);
        }
        
        else if(userbold == 1){
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
          strcat(prompt,colorcodes[hostcolor]);  
        }
        
        else if(hostbold == 1){
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
    
    if(strcmp(cwd,getenv("HOME"))==0){
        strcpy(filename,"~");
    }
    else if(strstr(cwd,getenv("HOME"))==NULL){
        strcpy(filename,cwd);
    }
    
    else{
        strncpy(filename,cwd+homelen,totalsize-homelen);
    }
    
    strcat(prompt,filename);
    strcat(prompt,"]");
    strcat(prompt,"> ");
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


void redirection(){

    for (int i = 0; i < numofParamcpy; i++)
    {
        if(strcmp(paramcpy[i],">")==0){

            fd = open(paramcpy[i+1],O_CREAT | O_WRONLY,0666);
            dfd = dup(STDOUT_FILENO);
            dup2(fd,STDOUT_FILENO);
            close(fd);
        }
        if(strcmp(paramcpy[i],"<")==0){

            fd = open(paramcpy[i+1],O_RDONLY,0666);
            dfd = dup(STDIN_FILENO);
            dup2(fd,STDIN_FILENO);
            close(fd);
        }
    }
}

void revertfiledescriptor(){


    if(outputredirect == 1){

        dup2(dfd,STDOUT_FILENO);
    }

    if(inputredirect == 1){

        dup2(dfd,STDIN_FILENO);
    }

    outputredirect = 0;
    inputredirect = 0;
}




job* createnewjob(){

    job* newjob = malloc(sizeof(job));

    newjob->first_process = NULL;

    addjobtolist(newjob);

return newjob;
}


void addjobtolist(job* newjob){

    if(first_job== NULL){
        first_job = newjob;
        last_job = newjob;
        first_job->next = NULL;
        last_job->next = NULL;
    }
    else{
        last_job->next = newjob;
        last_job =last_job->next;
        last_job->next = NULL;
    }

}

process* createnewprocess(job *jobprocess){

    process* newprocess = malloc(sizeof(process));

    addprocesstolist(jobprocess,newprocess);

    return newprocess;
}

void addprocesstolist(job* existingjob , process* newprocess){

    if(existingjob->first_process == NULL){
        printf("%s\n","first_process");
        existingjob->first_process = newprocess;
        existingjob->last_process = newprocess;

    }
    else{
        printf("second process");
        existingjob->last_process->next = newprocess;
        existingjob->last_process = existingjob->last_process->next;
        existingjob->last_process->next = NULL;
    }

}

void printjobs(){
    

    for(job *j=first_job; j!=NULL; j=j->next){

        printf("jobscommand%s\n",j->command);
        for (process *p = j->first_process; p!=NULL ; p = p->next){
            printf("complete arg%s\n",p->completearg);
        }
    }
}

void removespaces(char* s)
{
    char *p = s;
    int len = strlen(p);
    while(isspace(p[len - 1]))
    {
        p[--len] = 0;
    }
    while(*p && isspace(*p))
    {
        ++p;
        --len;
    }
    memmove(s, p, len + 1);
}
