#include "lott.h"
#include <sys/types.h>



static void* map(void*);
static void* reduce(void*);

int numfiles = 0;

int part1(){

    
    numfiles = nfiles();
    pthread_t tid[numfiles];
    int return_code;
    DIR *directory = opendir(DATA_DIR);
    struct dirent entry;
    struct dirent *result;
    int i=0;
    char index[30]="";
    for (return_code = readdir_r(directory, &entry, &result);result != NULL && return_code == 0;return_code = readdir_r(directory, &entry, &result))
    {
       if(entry.d_type == DT_REG)
        {
            Stats *insertStat = createStat();
            insertStat->filename = strdup(entry.d_name);
            pthread_create(&tid[i],NULL,map,(void*)insertStat);
            char name[20] = "map";
            sprintf(index,"%d",i);
            strcat(name,index);
            pthread_setname_np(tid[i],name);
            i++;
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


    reduce((void*)firststatshead);

    
    freestats(firststatshead);
    
    printf("Number of files: %d\n",numfiles);
    /* DELETE THIS: THIS IS TO QUIET COMPILER ERRORS */

    printf(
        "Part: %s\n"
        "Query: %s\n",
        PART_STRINGS[current_part], QUERY_STRINGS[current_query]);

    if(current_query == A){
        printf("%f\n",final_max_avg_duration);
        printf("%s\n",final_max_filename);  
    }

    if(current_query == B){
        printf("%f\n",final_min_avg_duration);
        printf("%s\n",final_min_filename);  
    }

    if(current_query == C){
        printf("%f\n",final_max_user_count);
        printf("%s\n",final_max_filename);  
    }

    if(current_query == D){
        printf("%f\n",final_min_user_count);
        printf("%s\n",final_min_filename);  
    }

    if(current_query == E){
        printf("%d\n",final_max_ccount);
        printf("%s\n",final_max_ccode);
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
        freecountry(countryduplicate);
        free(maxCC);
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
    countrystruct *head = NULL;
    char *resultmaxfilename = firststatshead->filename;
    char *resultminfilename = firststatshead->filename;


    for(reduction = firststatshead; reduction!=NULL;reduction= reduction->next){

        
        if(current_query == A || current_query == B){


            if(reduction->avgduration > maxAvgDuration){
                maxAvgDuration = reduction->avgduration;
                resultmaxfilename = reduction->filename;
            }
            else if(reduction->avgduration == maxAvgDuration){
                if(strcmp(resultmaxfilename,reduction->filename) > 0){
                    maxAvgDuration = reduction->avgduration;
                    resultmaxfilename = reduction->filename;
                }
            } 

            if(reduction->avgduration < minAvgDuration){
                minAvgDuration = reduction->avgduration;
                resultminfilename = reduction->filename;
            }
            else if(reduction->avgduration == minAvgDuration){
                if(strcmp(resultminfilename,reduction->filename) > 0){
                    minAvgDuration = reduction->avgduration;
                    resultminfilename = reduction->filename;
                }
            }
        }

        if(current_query == C || current_query == D){

            if(reduction->avgusercountperyear > maxAvgUserCount){
                maxAvgUserCount = reduction->avgusercountperyear;
                resultmaxfilename = reduction->filename;
            }
            else if(reduction->avgusercountperyear == maxAvgUserCount){
                if(strcmp(resultmaxfilename,reduction->filename) > 0){
                    maxAvgUserCount = reduction->avgusercountperyear;
                    resultmaxfilename = reduction->filename;
                }
            } 
            
            if(reduction->avgusercountperyear < minAvgUserCount){
                minAvgUserCount = reduction->avgusercountperyear;
                resultminfilename = reduction->filename;
            }
            else if(reduction->avgusercountperyear == minAvgUserCount){
                if(strcmp(resultminfilename,reduction->filename) > 0){
                    maxAvgUserCount = reduction->avgusercountperyear;
                    resultminfilename = reduction->filename;
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
    while(head!=NULL){
        if(head->count > maxCCcount){
            maxCCcount = head->count;
            strcpy(cc,head->ccode);
        }
        head = head->next;
    }

    final_max_avg_duration = maxAvgDuration;

    final_min_avg_duration = minAvgDuration;

    final_max_user_count = maxAvgUserCount;

    final_min_user_count = minAvgUserCount;

    final_max_filename = resultmaxfilename;

    final_min_filename = resultminfilename;

    final_max_ccount = maxCCcount;

    strcpy(final_max_ccode,cc);

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
    newStat->avgusercountperyear = 0;
    //newStat->maxCC = NULL;
    strcpy(newStat->maxCC,"");
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
    for(first = firststatshead; first!=NULL;first=first->next){
        
        printf("%s\n",first->filename);
        printf("%f\n",first->avgduration);
        printf("%f\n",first->avgusercountperyear);
    }
    printf("%s\n","--------------------------------------------------------" );
}

void werrorchut(void *p){

}

yearstruct* createyear(int value){

    yearstruct *newyearstruct = (yearstruct *)malloc(sizeof(yearstruct));
    newyearstruct->year = value;
    newyearstruct->count = 1;
    return newyearstruct;
}

yearstruct* findyear(yearstruct *head, int value){
    yearstruct *counter = head;

    while (counter != NULL && counter->year != value){
        counter = counter->next;
    }
    return counter;
}

void pushyeartolist(yearstruct **head, yearstruct *item){

    item->next = *head;
    *head = item;
}

double calculateDistintYears(yearstruct *head){
    double i = 0;
    while(head!= NULL){
        i++;
        head = head->next;
    }
    return i;
}

void freeyears(yearstruct *head){
    yearstruct *current;
    while((current = head)!= NULL){
        head = head->next;
        free(current);
    }
}

void freecountry(countrystruct *head){
    countrystruct *current;
    while((current = head)!= NULL){
        head = head->next;
        free(current);
    }
}

void freestats(Stats *head){
    Stats *current;
    while((current = head)!= NULL){
        head = head->next;
        free(current);
    }
}

countrystruct* createcountry(char *value){

    countrystruct *newcountrystruct = (countrystruct *)malloc(sizeof(countrystruct));
    strcpy(newcountrystruct->ccode,value);
    newcountrystruct->count = 1;
    return newcountrystruct;
}

countrystruct* createcountryforreduce(char *value, int count){

    countrystruct *newcountrystruct = (countrystruct *)malloc(sizeof(countrystruct));
    strcpy(newcountrystruct->ccode,value);
    newcountrystruct->count = count;
    return newcountrystruct;
}


countrystruct* findcountry(countrystruct *head, char* value){
    countrystruct *counter = head;

    while (counter != NULL && strcmp(counter->ccode,value)!=0){

        counter = counter->next;
    }
    return counter;
}

void pushcountrytolist(countrystruct **head, countrystruct *item){
    item->next = *head;
    *head = item;
}

countrystruct* findmaxccodes(countrystruct *head){

    countrystruct *maxCC = malloc(sizeof(countrystruct));
    maxCC->count = head->count;
    strcpy(maxCC->ccode,head->ccode);
    while(head!= NULL){
        
        if(maxCC->count < head->count){
            maxCC->count = head->count;
            strcpy(maxCC->ccode,head->ccode);
            //maxCC->ccode = head->ccode;
        }
        if(maxCC->count == head->count){
            
            if(strcmp(maxCC->ccode,head->ccode) > 0){
                maxCC->count = head->count;
                strcpy(maxCC->ccode,head->ccode);
            }
        }
        
        head = head->next;
    }
    return maxCC;
}