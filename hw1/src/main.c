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
    printf("%d\n", choice);
    printf("Welcome to CSE 320!\n");
    int number_of_files = nfiles("/home/sanchay/Documents/CSE320/saagrawal/hw1/rsrc/ana_heavy");
    printf("%d\n",number_of_files);
    return EXIT_SUCCESS;
}
