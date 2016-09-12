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
void help(){
printf("%s\n\t", "Usage: ./mapreduce [h|v] FUNC DIR");
printf("%s\t","FUNC");
printf("%s\n\t\t","Which operation you would like to run on the data:");
printf("%s\n\t\t","ana - Analysis of various text files in a directory.");
printf("%s\n\t","stats - Calculates stats on files which contain only numbers.");
printf("%s\t","DIR");
printf("%s\n\n\t","The directory in which the files are located.");
printf("%s\n\t","Options:");
printf("%s\t","-h");
printf("%s\n\t","Prints this help menu.");
printf("%s\t","-v");
printf("%s\n","Prints the map function’s results, stating the file it’s from.");
//return 0;
}

int main(int argc, char** argv) {
    int choice = validateargs(argc,argv);
    if(choice==-1){
        help();
        return EXIT_FAILURE;
    }
    if(choice==0){
        help();
        return EXIT_SUCCESS;
    }
    //For Stats
    if(choice==1){
        
        argv++;
        argv++;
        //int number_of_files = nfiles(*argv);
        int (*funcp)(FILE*,void*,char*);
        funcp = analysis;
        int hist = 1;
        int map_return = map(*argv,analysis_space,sizeof(struct Analysis),funcp);
        if(map_return==-1){
            return EXIT_FAILURE;
        }
        struct Analysis final = analysis_reduce(nfiles(*argv), analysis_space);
        analysis_print(final, map_return, hist);
        return EXIT_SUCCESS;
    }
    
    if(choice==2){
        argv++;
        argv++;
        //int number_of_files = nfiles(*argv);
        int (*funcp)(FILE*,void*,char*);
        funcp = stats;
        //int hist = 1;
        int map_return = map(*argv,stats_space,sizeof(struct Stats),funcp);
        struct Stats final =  stats_reduce(nfiles(*argv), stats_space);
        //printf("%d\n",map_return );
        stats_print(final, map_return+1);
        return EXIT_SUCCESS;
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
       // printf("%d\n",map_return );
        for(int i=0;i<nfiles(*argv);i++){
        analysis_print(analysis_space[i],map_return,0);    
        }
        analysis_print(final, map_return, hist);
        return EXIT_SUCCESS;
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
       // printf("%d\n",map_return );
        for(int i=0;i<nfiles(*argv);i++){
             stats_print(stats_space[i],map_return);    
        }
        stats_print(final, hist);
        return EXIT_SUCCESS;
        
    }
    
    return EXIT_FAILURE;
}
