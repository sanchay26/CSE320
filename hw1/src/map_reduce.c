//**DO NOT** CHANGE THE PROTOTYPES FOR THE FUNCTIONS GIVEN TO YOU. WE TEST EACH
//FUNCTION INDEPENDENTLY WITH OUR OWN MAIN PROGRAM.
#include "../include/map_reduce.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//Implement map_reduce.h functions here.
// Help Menu Print Funtion 
int help(){
printf("%s\n", "Usage: ./mapreduce [h|v] FUNC DIR");
printf("%s\n","FUNC");
printf("%s\n","Which operation you would like to run on the data:");
printf("%s\n","ana - Analysis of various text files in a directory.");
printf("%s\n","stats - Calculates stats on files which contain only numbers.");
printf("%s\n","DIR");
printf("%s\n","The directory in which the files are located.");
printf("%s\n","Options:");
printf("%s\n","-h");
printf("%s\n","Prints this help menu.");
printf("%s\n","-v");
printf("%s\n","Prints the map function’s results, stating the file it’s from.");
return 0;
}
// Validate Agrument Function Defination 
int validateargs(int argc, char** argv){
	
	// Invalidate Arguments 0 or 1 Argument 
	if (argc < 2){
	printf("%s\n","Invalid Arguments");
	help();
	return -1;
	}
	
	else if(argc == 2){
		//validating first argument 
		if (strcmp("./map_reduce",*argv) == 0){
			
			argv++;

			if(strcmp(*argv,"-h") == 0){
				// print menu here 
				help();
				return EXIT_SUCCESS;
			}
			else {
				//print menu here 
				help();
				return -1;
				}
		}
		else{
			printf("%s\n","Invalid Arguments");
			help();
			return -1;
			//if first argument is not a ./mapreduce 
		}

	}
	else if (argc == 3){

		if(strcmp(*argv,"./map_reduce")==0){
			
			argv++;
			
			if(strcmp(*argv,"-h")==0){
				// print menu here 
				help();
				return EXIT_SUCCESS;
			}

			else if (strcmp(*argv,"ana")==0){
				printf("%s\n",*argv );
				argv++;
				printf("%s\n%s\n",*argv,"this" );
				DIR* dir = opendir(*argv);
				if (dir)
				{
    				/* Directory exists. */
    				closedir(dir);
    				return 1;
				}
				else 
				{
					printf("%s\n","Invalid Directory");
					return -1;
    			/* Directory does not exist. */
				}
			}

			else if (strcmp(*argv,"stats")){
				argv++;
				DIR* dir = opendir(*argv);
				if (dir)
				{
    				/* Directory exists. */
    				closedir(dir);
    				return 2;
				}
				else 
				{
					printf("%s\n","Invalid Directory");
					return -1;
    			/* Directory does not exist. */
				}
			}
			else {
				printf("%s\n","Invalid Arguments");
				return -1;
			}
		}
	}
	else if( argc==4){

	}
	return -1;
}

// counting number of files 
int nfiles(char* dir){
	int number_of_files = 0;
	DIR * directory;
	struct dirent * entrance;

	directory = opendir(dir); 
	while ((entrance = readdir(directory)) != NULL) {
    	// Checking if it is a file 
    	if (entrance->d_type == DT_REG) { 
         number_of_files++;
    	}
	}
	closedir(directory);
	return number_of_files;
}

int map(char* dir, void* results, size_t size, int (*act)(FILE* f, void* res, char* fn)){

return 0;
}