#include "lott.h"
#include <sys/types.h>   
#include <sys/socket.h>
#include <sys/epoll.h>


static void* map(void*);
static void* reduce(void*);

pthread_mutex_t lock_mutex;

DIR *directoryp;

double maxAvgDuration5; 
double minAvgDuration5;
double maxUserCount5; 
double minUserCount5;
char *resultmaxfilename5;
char *resultminfilename5;
char finalcc5[2];
int count5 = 0;
countrystruct *head5 = NULL;
struct epoll_event events[50];      //epoll event 
struct epoll_event event;
int epfd;
static int flag = 0;

static void cleanup_handler(void *arg)
{

}  
int part5(size_t nthreads){
    pthread_t tid[nthreads];
    pthread_t read_thread;

    pthread_mutex_init(&lock_mutex, NULL);

    directoryp = opendir(DATA_DIR);

    epfd = epoll_create(nthreads);

    char index[30]="";

    for(int i=0;i<nthreads;i++){
        int fd[2];                                              //file descriptors one end is given to reduce and one to map
        socketpair(PF_LOCAL, SOCK_STREAM, 0,fd);                ///Sockets are created 
        event.data.fd = fd[0];                                    //registering file desciptor to epoll for reading
        event.events = EPOLLIN | EPOLLET;                       
        int s = epoll_ctl(epfd, EPOLL_CTL_ADD, fd[0], &event);
        if (s < 0)
        {
          perror("epoll_ctl");              //error if too many fd
          abort();
        }

        fdss *fds = malloc(sizeof(fdss));

        fds->filedes = fd[1];

        pthread_create(&tid[i],NULL,helpmap5,(void*)fds);
        char name[20] = "map";
        sprintf(index,"%d",i);
        strcat(name,index);
        pthread_setname_np(tid[i],name);
    }

    pthread_create(&read_thread,NULL,reduce,NULL);      // read is created as a thread 


    for(int i=0; i<nthreads;i++)
    {   
        pthread_join(tid[i],NULL);
    }

    pthread_cancel(read_thread);            // cancelling read when map joins 

    pthread_join(read_thread,NULL);

    printf(
        "Part: %s\n"
        "Query: %s\n",
        PART_STRINGS[current_part], QUERY_STRINGS[current_query]);
    
    if(current_query == A){
        printf("%s ","Result:");
        printf("%f, ",maxAvgDuration5);
        printf("%s",resultmaxfilename5);  
    }

    if(current_query == B){
        printf("%s ","Result:");
        printf("%f, ",minAvgDuration5);
        printf("%s",resultminfilename5);  
    }

    if(current_query == C){
        printf("%s ","Result:");
        printf("%f, ",maxUserCount5);
        printf("%s",resultmaxfilename5);  
    }

    if(current_query == D){
        printf("%s ","Result:");
        printf("%f, ",minUserCount5);           //prints out the final results for each query 
        printf("%s",resultminfilename5);  
    }

    if(current_query == E){
        
        while(head5!=NULL){
            if(head5->count > count5){
                count5 = head5->count;
                strcpy(finalcc5,head5->ccode);
            }
            head5 = head5->next;
        }
        printf("%s ","Result:");
        printf("%d, ",count5);
        printf("%s",finalcc5);
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
      usleep(0.000000001);
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
        //usleep(2000);
        double averageduration = totalduration/numoflines;
        char tmp[30];
        sprintf(tmp,"%f",averageduration);
        pthread_mutex_lock(&lock_mutex);
        //sem_wait(&w);
        
        write(web->writefd,tmp,strlen(tmp));
        write(web->writefd,",",1);
        write(web->writefd,web->filename,strlen(web->filename));            // writing to one end of socket pair 
        write(web->writefd,"\n",1);
        //sem_post(&w);
        // int* slow = malloc(sizeof(int));
        // int* slowmore = malloc(sizeof(int));
        // free(slow);
        // free(slowmore);
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
        write(web->writefd,web->filename,strlen(web->filename));    // writing to one end of socket pair 
        write(web->writefd,"\n",1);
        //sem_post(&w);
        pthread_mutex_unlock(&lock_mutex);
    }

    if(current_query == E){
        pthread_mutex_lock(&lock_mutex);
        countrystruct* maxCC = findmaxccodes(countryduplicate);
        char tmp[30];
        sprintf(tmp,"%d",maxCC->count);
        
        //sem_wait(&w);
        write(web->writefd,tmp,strlen(tmp));
        write(web->writefd,",",1);
        write(web->writefd,maxCC->ccode,strlen(maxCC->ccode));      // writing to one end of socket pair 
        write(web->writefd,"\n",1);
        //sem_post(&w);
        pthread_mutex_unlock(&lock_mutex);
        freecountry(countryduplicate);
        free(maxCC);
        usleep(500);                    //Sleeping so that epoll doesn't exit without reading each file in directory 
    }

    return NULL;
}

static void* reduce(void* v){
    pthread_cleanup_push(cleanup_handler,NULL);
    int f = 0;
    //int fdr =0;
    
    while(1){

        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

        f = epoll_wait(epfd,events,1,1000);             //epoll wait if any fd is being ready to be read
        
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

                if(flag == 0){
                    maxAvgDuration5 = averageduration;
                    minAvgDuration5 = averageduration;
                    resultmaxfilename5 = strdup(filename);
                    resultminfilename5 = strdup(filename);
                    flag++;
                }
                
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
                if(flag == 0){
                    maxUserCount5 = avgusercountperyear;
                    minUserCount5 = avgusercountperyear;
                    resultmaxfilename5 = strdup(filename);
                    resultminfilename5 = strdup(filename);
                    flag++;
                }
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
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);            //Enable set cancel state 
    usleep(5);
    
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