#include "../include/map_reduce.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

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
    int r = map("/home/sanchay/Documents/CSE320/saagrawal/hw1/rsrc/ana_light",analysis_space,sizeof(struct Analysis),analysis);
    printf("%d\n",r );
    //printf("number of files%d\n",number_of_files);
    return EXIT_SUCCESS;
}
