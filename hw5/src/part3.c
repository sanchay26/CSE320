#include "lott.h"

static void* map(void*);
static void* reduce(void*);

pthread_mutex_t lock_mutex;
int readcnt = 0;
sem_t mutex,w;

int fd;

static int flag = 0;

FILE *readp;

double maxAvgDuration; 
double minAvgDuration;
double maxUserCount; 
double minUserCount;
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

    fd = open("mapred.tmp",O_CREAT|O_TRUNC|O_RDWR,0666);        //opens file to write for each map 

    readp = fopen("mapred.tmp","r");                    //reads mapred.tmp to calculate final output 

    if(readp == NULL) 
    {
      perror("Error opening mapred.tmp file");
    }

    DIR *directory = opendir(DATA_DIR);

    char index[30]="";
    for(int i=0;i<nthreads;i++){

       pthread_create(&tid[i],NULL,helpmap3,(void*)directory);
        char name[20] = "map";
        sprintf(index,"%d",i);
        strcat(name,index);
        pthread_setname_np(tid[i],name);
    }

    pthread_create(&read_thread,NULL,reduce,NULL);              // Readuce is also created as a thread 


    for(int i=0; i<nthreads;i++)
    {   
        pthread_join(tid[i],NULL);
    }

    pthread_cancel(read_thread);                // cancels the reuce thread once all maps are joined 

    pthread_join(read_thread,NULL);
    
    unlink("mapred.tmp"); //delete mapred.tmp


    printf(
        "Part: %s\n"
        "Query: %s\n",
        PART_STRINGS[current_part], QUERY_STRINGS[current_query]);

    if(current_query == A){
        printf("%s ","Result:");
        printf("%f, ",maxAvgDuration);
        printf("%s",resultmaxfilename);  
    }

    if(current_query == B){
        printf("%s ","Result:");
        printf("%f, ",minAvgDuration);
        printf("%s",resultminfilename);  
    }

    if(current_query == C){
        printf("%s ","Result:");
        printf("%f, ",maxUserCount);
        printf("%s",resultmaxfilename);  
    }

    if(current_query == D){
        printf("%s ","Result:");
        printf("%f, ",minUserCount);
        printf("%s",resultminfilename);  
    }

    if(current_query == E){
        
        while(head!=NULL){
            if(head->count > count){
                count = head->count;
                strcpy(finalcc,head->ccode);
            }
            head = head->next;
        }
        printf("%s ","Result:");
        printf("%d, ",count);
        printf("%s",finalcc);
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
        sem_wait(&w);                                   //Semaphores used to give locks while writing to avoid race 
        write(fd,tmp,strlen(tmp));                  //writes info to the filedescriptor for mapred.tmp
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
        sprintf(tmp,"%f",avgusercountperyear);      //Semaphores used to give locks while writing to avoid race 
        //pthread_mutex_lock(&lock_mutex);          //writes info to the filedescriptor for mapred.tmp
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
        write(fd,",",1);                                    //Semaphores used to give locks while writing to avoid race 
        write(fd,maxCC->ccode,strlen(maxCC->ccode));        //writes info to the filedescriptor for mapred.tmp
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

        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);       // disabling cancel state for thread to ensure reduce 
                                                                    // is not cancelled in between
        //pthread_mutex_lock(&lock_mutex);
        sem_wait(&mutex);
        readcnt++;
        if(readcnt == 1)                            // Gives reader preference to read from a file using Semaphores 
            sem_wait(&w);
        sem_post(&mutex);
        while(fgets(line, 100, readp)!=NULL ) {

            linedup = line;

            if(current_query == A || current_query == B){
                averageduration = atof(strsep (&linedup,","));   
                filename = strsep(&linedup,"\n"); 
                if(flag == 0){
                    maxAvgDuration = averageduration;
                    minAvgDuration = averageduration;
                    resultmaxfilename = strdup(filename);
                    resultminfilename = strdup(filename);
                    flag++;
                }
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

                if(flag == 0){
                    maxUserCount = avgusercountperyear;
                    minUserCount = avgusercountperyear;
                    resultmaxfilename = strdup(filename);
                    resultminfilename = strdup(filename);
                    flag++;
                }
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
        if(readcnt == 0)                                        // Semaphores used to give reader preference and avoid race
            sem_post(&w);
        sem_post(&mutex);
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);            // thread is cancelled once all the map finishes 
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
            map((void*)insertStat);                             // for each file in directory map is called 
            //pthread_setname_np(&tid[i], (const char*)qwe);
        }
    }
    return NULL;
}