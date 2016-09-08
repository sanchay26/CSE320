//**DO NOT** CHANGE THE PROTOTYPES FOR THE FUNCTIONS GIVEN TO YOU. WE TEST EACH
//FUNCTION INDEPENDENTLY WITH OUR OWN MAIN PROGRAM.
#include "../include/map_reduce.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//Implement map_reduce.h functions here.
int validateargs(int argc, char** argv){
	printf("%d\t%s\n", argc,*argv);
	//*argv=*(argv+1);
	//printf("%s\n",*argv);
    // Invalidate Arguments 0 or 1 Argument 
	if (argc < 2){
	printf("%s\n","Invalidate Arguments");
	//print help menu 
	return -1;
	}
	
	else if(argc == 2){
		//validating first argument 
		if (strcmp("./map_reduce",*argv)){
			
			*argv = *(argv+1);

			
			if(strcmp(*argv,"-h")){
				// print menu here 
				printf("%s\n","user gave -h");
				return EXIT_SUCCESS;
			}
			else {
				printf("%s\n","Invalid Arguments in 2");
				return -1;
				//print Invalidate arguments menu 
			}
			
		}
		else{
			printf("%s\n",*(argv+1));
			return -1;
			//if first argument is not a ./mapreduce 
		}

	}
	else if (argc == 3){

		if(strcmp(*argv,"./map_reduce")){
			
			*argv = *(argv+1);
			
			if(strcmp(*argv,"-h")){
				// print menu here 
				printf("%s\n","user gave -h");
				return EXIT_SUCCESS;
			}

			else if (strcmp(*argv,"ana")){
				*argv = *(argv+1);
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
				*argv = *(argv+1);
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