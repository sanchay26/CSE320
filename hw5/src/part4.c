#include "lott.h"

static void* map(void*);
static void* reduce(void*);


pthread_mutex_t lock_mutex;

//double maxAvgDuration = -1;
int flag =0;
static void cleanup_handler(void *arg)
{

} 
int part4(size_t nthreads){

   
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

    //reduce((void*)firststatshead);
    pthread_cancel(read_thread);

    pthread_join(read_thread,NULL);

    printf(
        "Part: %s\n"
        "Query: %s\n",
        PART_STRINGS[current_part], QUERY_STRINGS[current_query]);

    //printf("%f\n",maxAvgDuration);
    printf("maxdur%f\n",final_max_avg_duration);
    printf("mindur%f\n",final_min_avg_duration);
    printf("minuser%f\n",final_max_user_count);
    printf("maxuser%f\n",final_min_user_count);

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
    printf("%f\n",web->avgduration);
    addStatToList(web);

    return NULL;
}

static void* reduce(void* v){

    pthread_cleanup_push(cleanup_handler,NULL);
    Stats *reduction = (Stats*)v;
    int maxCCcount = -1;
    char cc[2] = "";
    countrystruct *head = NULL;
    //char *resultmaxfilename = NULL;
    //char *resultminfilename = NULL;

    while(1){
        
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        pthread_mutex_lock(&lock_mutex);
        
        for(reduction = firststatshead; reduction!=NULL;reduction= reduction->next){

            if(flag == 0){          // Initialize once 
                final_max_avg_duration = firststatshead->avgduration;
                final_min_avg_duration = firststatshead->avgduration;
                final_max_user_count = firststatshead->avgusercountperyear;
                final_min_user_count = firststatshead->avgusercountperyear;
                flag++;
            }
            
            if(current_query == A || current_query == B){
                
                if(reduction->avgduration > final_max_avg_duration){
                    final_max_avg_duration = reduction->avgduration;
                    //resultmaxfilename = strdup(reduction->filename);
                }
                else if(reduction->avgduration < final_min_avg_duration){
                    final_min_avg_duration = reduction->avgduration;
                    //resultminfilename = strdup(reduction->filename);
                }  
            }

            if(current_query == C || current_query == D){

                if(reduction->avgusercountperyear > final_max_user_count){
                    final_max_user_count = reduction->avgusercountperyear;
                    //resultmaxfilename = strdup(reduction->filename);
                }
                else if(reduction->avgusercountperyear < final_min_user_count){
                    final_min_user_count = reduction->avgusercountperyear;
                    //resultminfilename = strdup(reduction->filename);
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
        pthread_mutex_unlock(&lock_mutex);
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        usleep(0.5);
    }

    
    if(current_query == E){
       while(head!=NULL){
            if(head->count > maxCCcount){
                maxCCcount = head->count;
                strcpy(cc,head->ccode);
            }
            head = head->next;
        } 
        printf("maxCCcount: %d\n",maxCCcount);
        printf("maxCCcode: %s\n",cc);
    }
    pthread_cleanup_pop(1);
    return NULL;
}


void* helpmap4(void* dir){

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