#include "lott.h"
#include <sys/types.h>   
#include <sys/socket.h>
#include <sys/epoll.h>


static void* map(void*);
static void* reduce(void*);

pthread_mutex_t lock_mutex;

DIR *directoryp;

double maxAvgDuration5 = -1; 
double minAvgDuration5 = 1000;
double maxUserCount5 = -1; 
double minUserCount5 = 10000;
char *resultmaxfilename5;
char *resultminfilename5;
char finalcc5[2];
int count5 = 0;
countrystruct *head5 = NULL;
struct epoll_event events[50];
struct epoll_event event;
int epfd;

static void cleanup_handler(void *arg)
{

}  
int part5(size_t nthreads){
    pthread_t tid[nthreads];
    pthread_t read_thread;

    pthread_mutex_init(&lock_mutex, NULL);

    directoryp = opendir(DATA_DIR);

    epfd = epoll_create(nthreads);

    for(int i=0;i<nthreads;i++){
        int fd[2];
        socketpair(PF_LOCAL, SOCK_STREAM, 0,fd);
        event.data.fd = fd[0];
        event.events = EPOLLIN;
        int s = epoll_ctl(epfd, EPOLL_CTL_ADD, fd[0], &event);
        if (0>s)
        {
          perror("epoll_ctl");
          abort();
        }

        fdss *fds = malloc(sizeof(fdss));

        fds->filedes = fd[1];

        pthread_create(&tid[i],NULL,helpmap5,(void*)fds);
    }

    pthread_create(&read_thread,NULL,reduce,NULL);


    for(int i=0; i<nthreads;i++)
    {   
        pthread_join(tid[i],NULL);
    }

    pthread_cancel(read_thread);

    pthread_join(read_thread,NULL);

    printf(
        "Part: %s\n"
        "Query: %s\n",
        PART_STRINGS[current_part], QUERY_STRINGS[current_query]);
    if(current_query == A){
        printf("%f\n",maxAvgDuration5);
        printf("%s\n",resultmaxfilename5);  
    }

    if(current_query == B){
        printf("%f\n",minAvgDuration5);
        printf("%s\n",resultminfilename5);  
    }

    if(current_query == C){
        printf("%f\n",maxUserCount5);
        printf("%s\n",resultmaxfilename5);  
    }

    if(current_query == D){
        printf("%f\n",minUserCount5);
        printf("%s\n",resultminfilename5);  
    }

    if(current_query == E){
        
        while(head5!=NULL){
            if(head5->count > count5){
                count5 = head5->count;
                strcpy(finalcc5,head5->ccode);
            }
            head5 = head5->next;
        }
        printf("%d\n",count5);
        printf("%s\n",finalcc5);
    }
    return 0;
}

static void* map(void* v){

    Stats *web = (Stats*)v;
    char filepath[1024];
    strcpy(filepath,DATA_DIR);strcat(filepath,"/");strcat(filepath,web->filename);

    char line[100];
    char *linedup;
    char *timestamp;
    char *ip;
    double totalduration = 0;
    char *cc;
    double numoflines = 0;
    struct tm info;
    char year[80];
    int intyear;
    double distinctyears;
    yearstruct *duplicate = NULL;
    countrystruct *countryduplicate = NULL;

    FILE *fp; 
    fp = fopen(filepath,"r");
    if(fp == NULL) 
    {
      perror("Error opening file");
    }

    while( fgets (line, 100, fp)!=NULL ) {
      
      numoflines++;
      linedup = line;
      
      //-----------------------------Dont change the order ----------------------
      timestamp = strsep (&linedup,",");    
      ip = strsep (&linedup,",");    
      totalduration =totalduration + atoi(strsep (&linedup, ",")); 
      cc = strsep (&linedup,"\n");
      //-----------------------------Dont change the order ----------------------

        if(current_query == C || current_query == D ){
            time_t rawtime = atoi(timestamp);
            localtime_r(&rawtime,&info);
            strftime(year,80,"%Y",&info);
            intyear = atoi(year);
            yearstruct *current = findyear(duplicate,intyear);
            
            if(current == NULL){
                pushyeartolist(&duplicate,createyear(intyear));
            }
            else{
                current->count++;
            }
        }

        if(current_query == E){
        
            countrystruct *current1 = findcountry(countryduplicate,cc);

            if(current1 == NULL){
                pushcountrytolist(&countryduplicate,createcountry(cc));
            }
            else{
                current1->count++;
            }
        }
    }

    //fclose(fp);
    
    werrorchut((void*)ip);
    

    if(current_query == A || current_query == B){

        double averageduration = totalduration/numoflines;
        char tmp[30];
        sprintf(tmp,"%f",averageduration);
        pthread_mutex_lock(&lock_mutex);
        //sem_wait(&w);
        
        write(web->writefd,tmp,strlen(tmp));
        write(web->writefd,",",1);
        write(web->writefd,web->filename,strlen(web->filename));
        write(web->writefd,"\n",1);
        //sem_post(&w);
        
        pthread_mutex_unlock(&lock_mutex);
    }

    if(current_query == C || current_query == D){
        distinctyears = calculateDistintYears(duplicate);
        freeyears(duplicate);
        double avgusercountperyear = numoflines/distinctyears;
        char tmp[30];
        sprintf(tmp,"%f",avgusercountperyear);
        pthread_mutex_lock(&lock_mutex);
        //sem_wait(&w);
        write(web->writefd,tmp,strlen(tmp));
        write(web->writefd,",",1);
        write(web->writefd,web->filename,strlen(web->filename));
        write(web->writefd,"\n",1);
        //sem_post(&w);
        pthread_mutex_unlock(&lock_mutex);
    }

    if(current_query == E){
        countrystruct* maxCC = findmaxccodes(countryduplicate);
        char tmp[30];
        sprintf(tmp,"%d",maxCC->count);
        pthread_mutex_lock(&lock_mutex);
        //sem_wait(&w);
        write(web->writefd,tmp,strlen(tmp));
        write(web->writefd,",",1);
        write(web->writefd,maxCC->ccode,strlen(maxCC->ccode));
        write(web->writefd,"\n",1);
        //sem_post(&w);
        pthread_mutex_unlock(&lock_mutex);
        freecountry(countryduplicate);
        free(maxCC);
    }

    return NULL;
}

static void* reduce(void* v){
    pthread_cleanup_push(cleanup_handler,NULL);
    int f = 0;
    //int fdr =0;
    
    while(1){

        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

        f = epoll_wait(epfd,events,1,1000);
        
        if(f>0){

        double averageduration;
        double avgusercountperyear;
        char *filename;
        char *cc;
        int ccount;
        
        char line[100];
        char *linedup;
        FILE* fp = fdopen(events[f-1].data.fd, "r");
        
        pthread_mutex_lock(&lock_mutex);

        
        if(fgets(line,100,fp)!=NULL ) {

            linedup = line;
            if(current_query == A || current_query == B){

                averageduration = atof(strsep (&linedup,","));   
                filename = strsep(&linedup,"\n"); 
                
                if(averageduration > maxAvgDuration5){
                    maxAvgDuration5 = averageduration;
                    resultmaxfilename5 = strdup(filename);
                }
                else if(averageduration == maxAvgDuration5){
                    if(strcmp(resultmaxfilename5,filename) > 0){
                        maxAvgDuration5 = averageduration;
                        resultmaxfilename5 = filename;
                    }
                } 

                if(averageduration < minAvgDuration5){
                    minAvgDuration5 = averageduration;
                    resultminfilename5 = strdup(filename);
                }
                else if(averageduration == minAvgDuration5){
                    if(strcmp(resultminfilename5,filename) > 0){
                        minAvgDuration5 = averageduration;
                        resultminfilename5 = filename;
                    }
                } 

            }
            
            if(current_query == C || current_query == D){
                avgusercountperyear = atof(strsep (&linedup,","));   
                filename = strsep(&linedup,"\n"); 
                if(avgusercountperyear > maxUserCount5){
                    maxUserCount5 = avgusercountperyear;
                    resultmaxfilename5 = strdup(filename);
                }
                else if(avgusercountperyear == maxUserCount5){
                    if(strcmp(resultmaxfilename5,filename) > 0){
                        maxUserCount5 = avgusercountperyear;
                        resultmaxfilename5 = strdup(filename);
                    }
                } 
                if(avgusercountperyear < minUserCount5){
                    minUserCount5 = avgusercountperyear;
                    resultminfilename5 = strdup(filename);
                }
                else if(avgusercountperyear == minUserCount5){
                    if(strcmp(resultminfilename5,filename) > 0){
                        minUserCount5 = avgusercountperyear;
                        resultminfilename5 = strdup(filename);
                    }
                } 
            }
            if(current_query == E){

                ccount = atoi(strsep (&linedup,","));   
                cc = strsep(&linedup,"\n"); 
                countrystruct *current2 = findcountry(head5,cc);

                if(current2 == NULL){
                    pushcountrytolist(&head5,createcountryforreduce(cc,ccount));
                }
                else{
                    current2->count = current2->count + ccount;
                }
            }
        }

        pthread_mutex_unlock(&lock_mutex);
    }
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    usleep(0.5);
    
}
    pthread_cleanup_pop(1);
    return NULL;
}


void* helpmap5(void *fd){

    fdss *fds = (fdss*)fd;
    int return_code;
    struct dirent entry;
    struct dirent *result;

    for (return_code = readdir_r(directoryp, &entry, &result);result != NULL && return_code == 0;return_code = readdir_r(directoryp, &entry, &result))
    {
       if(entry.d_type == DT_REG)
        {   
            pthread_mutex_lock(&lock_mutex);
            Stats *insertStat = createStat();
            insertStat->filename = strdup(entry.d_name);
            insertStat->writefd = fds->filedes;
            pthread_mutex_unlock(&lock_mutex);
            map((void*)insertStat);
            //pthread_setname_np(&tid[i], (const char*)qwe);
        }
    }
    return NULL;
}