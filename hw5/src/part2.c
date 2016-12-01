#include "lott.h"

static void* map(void*);
static void* reduce(void*);

int part2(size_t nthreads) {

    //numfiles = nfiles();
    pthread_t tid[nthreads];
    DIR *directory = opendir(DATA_DIR);

    for(int i=0;i<nthreads;i++){

        pthread_create(&tid[i],NULL,helpmap,(void*)directory);
    }

    for(int i=0; i<nthreads;i++)
    {   
        pthread_join(tid[i],NULL);
    }
    
    reduce((void*)firststatshead);

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
    //printf("%s\n", "did you come here");
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
      
      if(current_query == C || current_query == D )
      {
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
    }
    
    fclose(fp);
    
    werrorchut((void*)ip);
    web->avgduration = totalduration/numoflines;
    addStatToList(web);
    return NULL;
}

static void* reduce(void* v){
    Stats *reduction = (Stats*)v;
    double maxAvgDuration = firststatshead->avgduration;
    double minAvgDuration = firststatshead->avgduration;
    double maxAvgUserCount = firststatshead->avgusercountperyear;
    double minAvgUserCount = firststatshead->avgusercountperyear;
    int maxCCcount = firststatshead->maxCCcount;
    char cc[2] = "";


    for(reduction = firststatshead; reduction!=NULL;reduction= reduction->next){

        if(reduction->avgduration > maxAvgDuration){
            maxAvgDuration = reduction->avgduration;
        }
        else if(reduction->avgduration < minAvgDuration){
            minAvgDuration = reduction->avgduration;
        }

        if(current_query == C || current_query == D){

            if(reduction->avgusercountperyear > maxAvgUserCount){
                maxAvgUserCount = reduction->avgusercountperyear;
            }
            else if(reduction->avgusercountperyear < minAvgUserCount){
                minAvgUserCount = reduction->avgusercountperyear;
            }
        }
        if(current_query == E){
            if(reduction->maxCCcount > maxCCcount){
                maxCCcount = reduction->maxCCcount;
                strcpy(cc,reduction->maxCC);
            }
        }
    }

    printf("maxAvgDuration: %f\n",maxAvgDuration);
    printf("minAvgDuration: %f\n",minAvgDuration);
    printf("maxAvgUserCount: %f\n",maxAvgUserCount);
    printf("minAvgUserCount: %f\n",minAvgUserCount);
    printf("maxCCcount: %d\n",maxCCcount);
    printf("maxCCcode: %s\n",cc);

    return NULL;
}


void* helpmap(void* dir){

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