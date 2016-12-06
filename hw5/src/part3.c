#include "lott.h"

static void* map(void*);
static void* reduce(void*);

pthread_mutex_t lock_mutex;
int readcnt = 0;
sem_t mutex,w;

int fd;

FILE *readp;

double maxAvgDuration = -1; 
double minAvgDuration = 1000;
double maxUserCount = -1; 
double minUserCount = 10000;
char *resultmaxfilename;
char *resultminfilename;
char finalcc[2];
int count = 0;
countrystruct *head = NULL;

static void cleanup_handler(void *arg)
{
   
}   

int part3(size_t nthreads){

    pthread_t tid[nthreads];
    pthread_t read_thread;

    pthread_mutex_init(&lock_mutex, NULL);
    sem_init(&mutex,0,1);
    sem_init(&w,0,1);

    fd = open("mapred.tmp",O_CREAT|O_TRUNC|O_RDWR,0666);

    readp = fopen("mapred.tmp","r");

    if(readp == NULL) 
    {
      perror("Error opening mapred.tmp file");
    }

    DIR *directory = opendir(DATA_DIR);

    for(int i=0;i<nthreads;i++){

       pthread_create(&tid[i],NULL,helpmap3,(void*)directory);
    }

    pthread_create(&read_thread,NULL,reduce,NULL);


    for(int i=0; i<nthreads;i++)
    {   
        pthread_join(tid[i],NULL);
    }

    pthread_cancel(read_thread);

    pthread_join(read_thread,NULL);
    
    //unlink("mapred.tmp");


    printf(
        "Part: %s\n"
        "Query: %s\n",
        PART_STRINGS[current_part], QUERY_STRINGS[current_query]);

    if(current_query == A){
        printf("%f\n",maxAvgDuration);
        printf("%s\n",resultmaxfilename);  
    }

    if(current_query == B){
        printf("%f\n",minAvgDuration);
        printf("%s\n",resultminfilename);  
    }

    if(current_query == C){
        printf("%f\n",maxUserCount);
        printf("%s\n",resultmaxfilename);  
    }

    if(current_query == D){
        printf("%f\n",minUserCount);
        printf("%s\n",resultminfilename);  
    }

    if(current_query == E){
        
        while(head!=NULL){
            if(head->count > count){
                count = head->count;
                strcpy(finalcc,head->ccode);
            }
            head = head->next;
        }
        printf("%d\n",count);
        printf("%s\n",finalcc);
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

    fclose(fp);
    
    werrorchut((void*)ip);
    

    if(current_query == A || current_query == B){

        double averageduration = totalduration/numoflines;
        char tmp[30];
        sprintf(tmp,"%f",averageduration);
        //pthread_mutex_lock(&lock_mutex);
        sem_wait(&w);
        write(fd,tmp,strlen(tmp));
        write(fd,",",1);
        write(fd,web->filename,strlen(web->filename));
        write(fd,"\n",1);
        sem_post(&w);
        //pthread_mutex_unlock(&lock_mutex);
    }

    if(current_query == C || current_query == D){
        distinctyears = calculateDistintYears(duplicate);
        freeyears(duplicate);
        double avgusercountperyear = numoflines/distinctyears;
        char tmp[30];
        sprintf(tmp,"%f",avgusercountperyear);
        //pthread_mutex_lock(&lock_mutex);
        sem_wait(&w);
        write(fd,tmp,strlen(tmp));
        write(fd,",",1);
        write(fd,web->filename,strlen(web->filename));
        write(fd,"\n",1);
        sem_post(&w);
        //pthread_mutex_unlock(&lock_mutex);
    }

    if(current_query == E){
        countrystruct* maxCC = findmaxccodes(countryduplicate);
        char tmp[30];
        sprintf(tmp,"%d",maxCC->count);
        //pthread_mutex_lock(&lock_mutex);
        sem_wait(&w);
        write(fd,tmp,strlen(tmp));
        write(fd,",",1);
        write(fd,maxCC->ccode,strlen(maxCC->ccode));
        write(fd,"\n",1);
        sem_post(&w);
        //pthread_mutex_unlock(&lock_mutex);
        freecountry(countryduplicate);
        free(maxCC);
    }

    return NULL;
}

static void* reduce(void* v){

    
    pthread_cleanup_push(cleanup_handler,NULL);

    while(1){

        double averageduration;
        double avgusercountperyear;
        char *filename;
        //int maxCCcount;
        char *cc;
        int ccount;
        
        char line[100];
        char *linedup;

        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        //pthread_mutex_lock(&lock_mutex);
        sem_wait(&mutex);
        readcnt++;
        if(readcnt == 1)
            sem_wait(&w);
        sem_post(&mutex);
        while(fgets(line, 100, readp)!=NULL ) {

            linedup = line;

            if(current_query == A || current_query == B){
                averageduration = atof(strsep (&linedup,","));   
                filename = strsep(&linedup,"\n"); 
                
                if(averageduration > maxAvgDuration){
                    maxAvgDuration = averageduration;
                    resultmaxfilename = strdup(filename);
                }
                else if(averageduration == maxAvgDuration){
                    if(strcmp(resultmaxfilename,filename) > 0){
                        maxAvgDuration = averageduration;
                        resultmaxfilename = filename;
                    }
                } 

                if(averageduration < minAvgDuration){
                    minAvgDuration = averageduration;
                    resultminfilename = strdup(filename);
                }
                else if(averageduration == minAvgDuration){
                    if(strcmp(resultminfilename,filename) > 0){
                        minAvgDuration = averageduration;
                        resultminfilename = filename;
                    }
                } 
            }
            
            if(current_query == C || current_query == D){
                avgusercountperyear = atof(strsep (&linedup,","));   
                filename = strsep(&linedup,"\n"); 
                if(avgusercountperyear > maxUserCount){
                    maxUserCount = avgusercountperyear;
                    resultmaxfilename = strdup(filename);
                }
                else if(avgusercountperyear == maxUserCount){
                    if(strcmp(resultmaxfilename,filename) > 0){
                        maxUserCount = avgusercountperyear;
                        resultmaxfilename = strdup(filename);
                    }
                } 
                if(avgusercountperyear < minUserCount){
                    minUserCount = avgusercountperyear;
                    resultminfilename = strdup(filename);
                }
                else if(avgusercountperyear == minUserCount){
                    if(strcmp(resultminfilename,filename) > 0){
                        minUserCount = avgusercountperyear;
                        resultminfilename = strdup(filename);
                    }
                } 
            }
            if(current_query == E){

                ccount = atoi(strsep (&linedup,","));   
                cc = strsep(&linedup,"\n"); 
                countrystruct *current2 = findcountry(head,cc);

                if(current2 == NULL){
                    pushcountrytolist(&head,createcountryforreduce(cc,ccount));
                }
                else{
                    current2->count = current2->count + ccount;
                }
            }
        }
        //pthread_mutex_unlock(&lock_mutex);
        sem_wait(&mutex);
        readcnt--;
        if(readcnt == 0)
            sem_post(&w);
        sem_post(&mutex);
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        usleep(0.5);
    }
    pthread_cleanup_pop(1);
    return NULL;
}

 


void* helpmap3(void* dir){

    DIR *directory = (DIR*)dir;
    int return_code;
    struct dirent entry;
    struct dirent *result;

    for (return_code = readdir_r(directory, &entry, &result);result != NULL && return_code == 0;return_code = readdir_r(directory, &entry, &result))
    {
       if(entry.d_type == DT_REG)
        {   
            Stats *insertStat = createStat();
            insertStat->filename = strdup(entry.d_name);
            map((void*)insertStat);
            //pthread_setname_np(&tid[i], (const char*)qwe);
        }
    }
    return NULL;
}