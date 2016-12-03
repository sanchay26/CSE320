#include "lott.h"

static void* map(void*);
static void* reduce(void*);

pthread_mutex_t lock_mutex;

int fd;

FILE *readp;

static void cleanup_handler(void *arg)
{
    printf("%s\n","in clean" );
   double *max = (double*) arg;
   printf("Called clean-up handler\n");
   printf("%f\n",*max);
}   

int part3(size_t nthreads){

    pthread_t tid[nthreads];
    pthread_t read_thread;

    pthread_mutex_init(&lock_mutex, NULL);

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

    //helpmap3((void*)directory);

    pthread_create(&read_thread,NULL,reduce,NULL);
    //reduce(NULL);
    for(int i=0; i<nthreads;i++)
    {   
        pthread_join(tid[i],NULL);
    }

    printf("%s\n","I am here");

    pthread_cancel(read_thread);

    pthread_join(read_thread,NULL);
    //unlink("mapred.tmp");

    printf(
        "Part: %s\n"
        "Query: %s\n",
        PART_STRINGS[current_part], QUERY_STRINGS[current_query]);

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
    FILE *fp; 
    fp = fopen(filepath,"r");
    if(fp == NULL) 
    {
      perror("Error opening file");
    }


    while( fgets (line, 100, fp)!=NULL ) 
    {
      
      numoflines++;
      linedup = line;
      
      //-----------------------------Dont change the order ----------------------
      timestamp = strsep (&linedup,",");    
      ip = strsep (&linedup,",");    
      totalduration =totalduration + atoi(strsep (&linedup, ",")); 
      cc = strsep (&linedup,"\n");
      //-----------------------------Dont change the order ----------------------
    }
    werrorchut((void*)ip);
    werrorchut((void*)timestamp);
    werrorchut((void*)cc);
    double averageduration = totalduration/numoflines;
    char tmp[30];
    sprintf(tmp,"%f",averageduration);
    pthread_mutex_lock(&lock_mutex);
    write(fd,tmp,strlen(tmp));
    write(fd,",",1);
    write(fd,web->filename,strlen(web->filename));
    write(fd,"\n",1);
    pthread_mutex_unlock(&lock_mutex);
    //sem_post(&w);

    return NULL;
}

static void* reduce(void* v){

    double maxAvgDuration = -1; 
    pthread_cleanup_push(cleanup_handler, (void*)&maxAvgDuration);

    while(1){
        double averageduration;
        char *filename;
        //char *maxAvgDurationfilename = "";
        char line[100];
        char *linedup;

        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        pthread_mutex_lock(&lock_mutex);
        while(fgets(line, 100, readp)!=NULL ) {

            linedup = line;

            averageduration = atoi(strsep (&linedup,","));   
            filename = strsep(&linedup,"\n"); 
            printf("%s\n",filename);
            
            if(averageduration > maxAvgDuration){
                maxAvgDuration = averageduration;
                //strcpy(maxAvgDurationfilename,filename);
            }
        }
        pthread_mutex_unlock(&lock_mutex);
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