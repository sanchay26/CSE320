#include "lott.h"

#include <sys/types.h>
#include <dirent.h>


static void* map(void*);
static void* reduce(void*);

int numfiles = 0;
Stats* statshead = NULL;
Stats* firststatshead = NULL;
int part1(){

    
    numfiles = nfiles();
    pthread_t tid[numfiles];
    int return_code;
    DIR *directory = opendir(DATA_DIR);
    struct dirent entry;
    struct dirent *result;
    int i; 
    for (return_code = readdir_r(directory, &entry, &result),i=0;result != NULL && return_code == 0;return_code = readdir_r(directory, &entry, &result),i++)
    {
       if(entry.d_type == DT_REG)
        {
            Stats *insertStat = createStat();
            insertStat->filename = strdup(entry.d_name);
            pthread_create(&tid[i],NULL,map,(void*)insertStat);
            
            //map((void*)insertStat);
            //pthread_setname_np(&tid[i], (const char*)qwe);
            //stats = stats->next;
        }
    }
    
    if (return_code != 0)
    {
       perror("readdir_r() error"); 
    } 
    
    closedir(directory);
   
   
    for(int i=0; i<numfiles;i++)
    {
        pthread_join(tid[i],NULL);
    }
    //printstats();
    reduce((void*)firststatshead);
    
    printf("Number of files: %d\n",numfiles);
    /* DELETE THIS: THIS IS TO QUIET COMPILER ERRORS */

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
    struct tm *info;
    char year[80];

    FILE *fp; 
    fp = fopen(filepath,"r");
    if(fp == NULL) 
    {
      perror("Error opening file");
    }
    
    while( fgets (line, 100, fp)!=NULL ) 
    {
      numoflines++;
      linedup = strdup(line);
      
      //-----------------------------Dont change the order ----------------------
      timestamp = strsep (&linedup,",");    
      ip = strsep (&linedup,",");    
      totalduration =totalduration + atoi(strsep (&linedup, ",")); 
      cc = strsep (&linedup,"\n");
      //-----------------------------Dont change the order ----------------------

      time_t rawtime = atoi(timestamp);
      info = localtime(&rawtime);
      strftime(year,80,"%Y",info);
      //printf("year%s\n",year);
    }
    //printf("%s %s\n",ip,cc);
    werrorchut((void*)ip);
    werrorchut((void*)cc);
    //printf("%s\n",web->filename);
    web->avgduration = totalduration/numoflines;
    addStatToList(web);
    
    return NULL;
}

static void* reduce(void* v){

    Stats *reduction = (Stats*)v;
    double maxAvgDuration = firststatshead->avgduration;
    double minAvgDuration = firststatshead->avgduration;
    for(reduction = firststatshead; reduction!=NULL;reduction= reduction->next){

        if(reduction->avgduration > maxAvgDuration){
            maxAvgDuration = reduction->avgduration;
        }
        else if(reduction->avgduration < minAvgDuration){
            minAvgDuration = reduction->avgduration;
        }

    }
    printf("maxAvgDuration:%f\n",maxAvgDuration);
    printf("minAvgDuration:%f\n",minAvgDuration);

    return NULL;
}

int nfiles(){
    int number_of_files = 0;
    DIR * directory;
    struct dirent * entrance;
    directory = opendir(DATA_DIR); 
    
    while ((entrance = readdir(directory)) != NULL) {
        if (entrance->d_type == DT_REG) { 
         number_of_files++;
        }
    }
    
    closedir(directory);
return number_of_files;
}

Stats* createStat(){
    Stats *newStat = malloc(sizeof(Stats));
    newStat->filename = NULL;
    newStat->avgduration = 0;
    newStat->avgusercount = 0;
    newStat->maxCC = NULL;
    newStat->maxCCcount = 0;
    newStat->next = NULL;
return newStat;
}

void addStatToList(Stats* add){

    if(firststatshead == NULL)
    {
        
        firststatshead = add;
        statshead = firststatshead;
    }
    else
    {
        statshead->next = add;
        statshead = statshead->next;
    }
}

void printstats(){
    printf("%s\n","--------------------------------------------------------" );
    Stats* first;
    //printf("%s",firststatshead->filename);
    for(first = firststatshead; first!=NULL;first=first->next){
        
        printf("%s\n",first->filename);
        printf("%f\n",first->avgduration);
    }
    printf("%s\n","--------------------------------------------------------" );
}

void werrorchut(void *p){

}