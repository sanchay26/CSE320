#include "../include/map_reduce.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include "../include/const.h"

//Space to store the results for analysis map
struct Analysis analysis_space[NFILES];
//Space to store the results for stats map
Stats stats_space[NFILES];

//Sample Map function action: Print file contents to stdout and returns the number bytes in the file.
int cat(FILE* f, void* res, char* filename) {
    char c;
    int n = 0;
    printf("%s\n", filename);
    while((c = fgetc(f)) != EOF) {
        printf("%c", c);
        n++;
    }
    printf("\n");
    return n;
}

int main(int argc, char** argv) {
    int choice = validateargs(argc,argv);
    //For Stats
    if(choice==1){
        
        argv++;
        argv++;
        //int number_of_files = nfiles(*argv);
        int (*funcp)(FILE*,void*,char*);
        funcp = analysis;
        int hist = 1;
        int map_return = map(*argv,analysis_space,sizeof(struct Analysis),funcp);
        struct Analysis final = analysis_reduce(nfiles(*argv), analysis_space);
        printf("%d\n",map_return );
        // for(int i=0;i<nfiles(*argv);i++){
        // analysis_print(analysis_space[i],map_return,0);    
        // }
        analysis_print(final, map_return, hist);
    }
    
    if(choice==2){
        argv++;
        argv++;
        //int number_of_files = nfiles(*argv);
        int (*funcp)(FILE*,void*,char*);
        funcp = stats;
        int hist = 1;
        int map_return = map(*argv,stats_space,sizeof(struct Stats),funcp);
        struct Stats final =  stats_reduce(nfiles(*argv), stats_space);
        printf("%d\n",map_return );
        stats_print(final, hist);
    }
    
    if(choice==3){
        
        argv++;
        argv++;
        argv++;
        //int number_of_files = nfiles(*argv);
        int (*funcp)(FILE*,void*,char*);
        funcp = analysis;
        int hist = 1;
        int map_return = map(*argv,analysis_space,sizeof(struct Analysis),funcp);
        struct Analysis final = analysis_reduce(nfiles(*argv), analysis_space);
        printf("%d\n",map_return );
        for(int i=0;i<nfiles(*argv);i++){
        analysis_print(analysis_space[i],map_return,0);    
        }
        analysis_print(final, map_return, hist);
    }

    else if(choice==4){
        argv++;
        argv++;
        argv++;
        int (*funcp)(FILE*,void*,char*);
        funcp = stats;
        int hist = 1;
        int map_return = map(*argv,stats_space,sizeof(struct Stats),funcp);
        struct Stats final =  stats_reduce(nfiles(*argv), stats_space);
        printf("%d\n",map_return );
        for(int i=0;i<nfiles(*argv);i++){
             stats_print(stats_space[i],0);    
        }
        stats_print(final, hist);
        
    }
    

    //For Analysis 
    //int (*funcp)(FILE*,void*,char*);
    //funcp = analysis;
    //int hist = 1;
    //int map_return = map("/home/sanchay/Documents/CSE320/saagrawal/hw1/rsrc/ana_light",analysis_space,sizeof(struct Analysis),funcp);
    //printf("********number of bytes****%d\n",map_return );
    //struct Analysis final = analysis_reduce(nfiles("/home/sanchay/Documents/CSE320/saagrawal/hw1/rsrc/ana_light"), analysis_space);
    // for(int i=0;i<number_of_files;i++){
    // analysis_print(analysis_space[i],map_return,0);    
    // }
    // analysis_print(final, map_return, hist);
    

    // For Stats   
    // for(int i=0;i<number_of_files;i++){
    // stats_print(stats_space[i],hist);    
    // }
    



    //printf("%d\n",r );
    //printf("number of files%d\n",number_of_files);
    return EXIT_SUCCESS;
}
