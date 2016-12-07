#include "lott.h"

static void* map(void*);
static void* reduce(void*);


pthread_mutex_t lock_mutex;

int readcount = 0;
int writecount = 0;
sem_t rmutex,wmutex,readTry,resource;
static int flag =0;
static void cleanup_handler(void *arg)
{
    //Not required 
} 

int part4(size_t nthreads){

    sem_init(&rmutex,0,1);
    sem_init(&wmutex,0,1);
    sem_init(&readTry,0,1);
    sem_init(&resource,0,1);
       
    pthread_t tid[nthreads];
    pthread_t read_thread;

    pthread_mutex_init(&lock_mutex, NULL);

    DIR *directory = opendir(DATA_DIR);

    for(int i=0;i<nthreads;i++){

       pthread_create(&tid[i],NULL,helpmap4,(void*)directory);
    }

    pthread_create(&read_thread,NULL,reduce,(void*)firststatshead);

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
        printf("%s ","Result:");
        printf("%f, ",final_max_avg_duration);
        printf("%s",final_max_filename);  
    }

    if(current_query == B){
        printf("%s ","Result:");
        printf("%f, ",final_min_avg_duration);
        printf("%s",final_min_filename);  
    }

    if(current_query == C){
        printf("%s ","Result:");
        printf("%f, ",final_max_user_count);
        printf("%s",final_max_filename);  
    }

    if(current_query == D){
        printf("%s ","Result:");
        printf("%f, ",final_min_user_count);
        printf("%s",final_min_filename);  
    }

    if(current_query == E){
        printf("%s ","Result:");
        printf("%d, ",final_max_ccount);
        printf("%s",final_max_ccode);
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

    sem_wait(&wmutex);
    writecount++;
    if(writecount == 1)
        sem_wait(&readTry);
    sem_post(&wmutex);
    sem_wait(&resource);
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

    if(current_query == C || current_query == D){
        distinctyears = calculateDistintYears(duplicate);
        freeyears(duplicate);
        web->avgusercountperyear = numoflines/distinctyears;
    }

    if(current_query == E){
        countrystruct* maxCC = findmaxccodes(countryduplicate);
        strcpy(web->maxCC,maxCC->ccode);
        web->maxCCcount = maxCC->count;
        freecountry(countryduplicate);
        free(maxCC);
    }

    fclose(fp);
    werrorchut((void*)ip);
    web->avgduration = totalduration/numoflines;
    addStatToList(web);
    sem_post(&resource);

    sem_wait(&wmutex);
    writecount--;
    if(writecount == 0)
        sem_post(&readTry);
    sem_post(&wmutex);

    return NULL;
}

static void* reduce(void* v){

    pthread_cleanup_push(cleanup_handler,NULL);
    Stats *reduction = (Stats*)v;
    countrystruct *head = NULL;
    
    while(1){
        
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        //pthread_mutex_lock(&lock_mutex);
        sem_wait(&readTry);
        sem_wait(&rmutex);
        readcount++;
        if(readcount == 1)
            sem_wait(&resource);
        sem_post(&rmutex);
        sem_post(&readTry);
        
        for(reduction = firststatshead; reduction!=NULL;reduction= reduction->next){

            if(flag == 0){          // Initialize once 
                final_max_avg_duration = firststatshead->avgduration;
                final_min_avg_duration = firststatshead->avgduration;
                final_max_user_count = firststatshead->avgusercountperyear;
                final_min_user_count = firststatshead->avgusercountperyear;
                final_max_filename = firststatshead->filename;
                final_min_filename = firststatshead->filename;
                flag++;
            }
            
            if(current_query == A || current_query == B){
                
                if(reduction->avgduration > final_max_avg_duration){
                    final_max_avg_duration = reduction->avgduration;
                    final_max_filename = strdup(reduction->filename);
                }
                else if(reduction->avgduration < final_min_avg_duration){
                    final_min_avg_duration = reduction->avgduration;
                    final_min_filename = strdup(reduction->filename);
                }  
            }

            if(current_query == C || current_query == D){

                if(reduction->avgusercountperyear > final_max_user_count){
                    final_max_user_count = reduction->avgusercountperyear;
                    final_max_filename = strdup(reduction->filename);
                }
                else if(reduction->avgusercountperyear == final_max_user_count){
                    if(strcmp(final_max_filename,reduction->filename) > 0){
                        final_max_user_count = reduction->avgusercountperyear;
                        final_max_filename = strdup(reduction->filename);
                    }
                }
                if(reduction->avgusercountperyear < final_min_user_count){
                    final_min_user_count = reduction->avgusercountperyear;
                    final_min_filename = strdup(reduction->filename);
                }
                else if(reduction->avgusercountperyear == final_min_user_count){
                    if(strcmp(final_min_filename,reduction->filename) > 0){
                        final_min_user_count = reduction->avgusercountperyear;
                        final_min_filename = strdup(reduction->filename);
                    }
                }
            }
        
            if(current_query == E){
                
                countrystruct *current2 = findcountry(head,reduction->maxCC);

                if(current2 == NULL){
                    pushcountrytolist(&head,createcountryforreduce(reduction->maxCC,reduction->maxCCcount));
                }
                else{
                    current2->count = current2->count + reduction->maxCCcount;
                }
            }
        }
        if(current_query == E){
            while(head!=NULL){
                if(head->count > final_max_ccount){
                    final_max_ccount = head->count;
                    strcpy(final_max_ccode,head->ccode);
                }
                head = head->next;
            } 
        }
        //pthread_mutex_unlock(&lock_mutex);
        sem_wait(&rmutex);
        readcount--;
        if(readcount == 0)
            sem_post(&resource);
        sem_post(&rmutex);
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        usleep(0.5);
    }

    
    
    pthread_cleanup_pop(1);
    return NULL;
}


void* helpmap4(void* dir){

    DIR *directory = (DIR*)dir;     //Cast void* directory to DIR* 
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