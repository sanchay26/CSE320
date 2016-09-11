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
    printf("choice%d\n", choice);
    //int number_of_files = nfiles("/home/sanchay/Documents/CSE320/saagrawal/hw1/rsrc/ana_light");
    
    //For Stats 
    // int r = map("/home/sanchay/Documents/CSE320/saagrawal/hw1/rsrc/stats_light",stats_space,sizeof(struct Stats),stats);
    // if(r==0){
    //     stats_reduce(nfiles("/home/sanchay/Documents/CSE320/saagrawal/hw1/rsrc/stats_light"), stats_space);
    // }

    //For Analysis 
    int (*funcp)(FILE*,void*,char*);
    funcp = analysis;
    int hist = 1;
    int map_return = map("/home/sanchay/Documents/CSE320/saagrawal/hw1/rsrc/ana_light",analysis_space,sizeof(struct Analysis),funcp);
    //printf("********number of bytes****%d\n",map_return );
    struct Analysis final = analysis_reduce(nfiles("/home/sanchay/Documents/CSE320/saagrawal/hw1/rsrc/ana_light"), analysis_space);
    analysis_print(final, map_return, hist);
    //printf("%d\n",r );
    //printf("number of files%d\n",number_of_files);
    return EXIT_SUCCESS;
}
