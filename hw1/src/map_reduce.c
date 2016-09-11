//**DO NOT** CHANGE THE PROTOTYPES FOR THE FUNCTIONS GIVEN TO YOU. WE TEST EACH
//FUNCTION INDEPENDENTLY WITH OUR OWN MAIN PROGRAM.
#include "../include/map_reduce.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include "../include/const.h"
//Implement map_reduce.h functions here.
// Help Menu Print Funtion 
int help(){
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
return 0;
}
// Validate Agrument Function Defination 
int validateargs(int argc, char** argv){
	
	// Invalidate Arguments 0 or 1 Argument 
	if (argc < 2){
	//printf("%s\n","Invalid Arguments");
	help();
	return -1;
	}
	
	else if(argc == 2){
		//validating first argument 
			
			argv++;

			if(strcmp(*argv,"-h") == 0){
				// print menu here 
				help();
				return EXIT_SUCCESS;
			}
			else {
				//if second argument is not -h 
				help();
				return -1;
			}
	}
	else if (argc == 3){

			argv++;
			
			if(strcmp(*argv,"-h")==0){
				// print menu here 
				help();
				return EXIT_SUCCESS;
			}

			else if (strcmp(*argv,"ana")==0){
				//printf("%s\n",*argv );
				argv++;
				DIR* dir = opendir(*argv);
				if (dir)
				{
    				/* Directory exists. */
    				closedir(dir);
    				return 1;
				}
				else 
				{
					//printf("%s\n","Invalid Directory");
					help();
					return -1;

    			/* Directory does not exist. */
				}
			}

			else if (strcmp(*argv,"stats")==0){
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
					help();
					return -1;
    			/* Directory does not exist. */
				}
			}
			else {
				//printf("%s\n","Invalid Arguments");
				help();
				return -1;
			}
	}
	
	else if(argc==4){
			argv++;
			
			if(strcmp(*argv,"-h")==0){
				// print menu here 
				help();
				return EXIT_SUCCESS;
			}

			else if(strcmp(*argv,"-v")==0){
				argv++;
				
				if (strcmp(*argv,"ana")==0){
				//printf("%s\n",*argv );
					argv++;
					//printf("%s\n%s\n",*argv,"this" );
					DIR* dir = opendir(*argv);
					if (dir)
					{
	    				/* Directory exists. */
	    				closedir(dir);
	    				return 3;
					}
					else 
					{
						printf("%s\n","Invalid Directory");
						return -1;
	    			/* Directory does not exist. */
					}
				}
				
				else if (strcmp(*argv,"stats")==0){
					printf("**************in Stats" );
					argv++;
					DIR* dir = opendir(*argv);
					if (dir)
					{
	    				/* Directory exists. */
	    				closedir(dir);
	    				return 4;
					}
					else 
					{
						printf("%s\n","Invalid Directory");
						return -1;
	    			/* Directory does not exist. */
					}
				}
				
				else{
					printf("this%s\n","Invalid Arguments");
					return -1;
				}

			}
			else {
				printf("%s\n","Invalid Arguments");
				return -1;
			}
		}
		
	else {
		argv++;
		if(strcmp(*argv,"-h")==0){
				// print menu here 
				help();
				return EXIT_SUCCESS;
		}
		else {
			help();
			return -1;
		}
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

	DIR * directory;
	struct dirent * entrance;

	directory = opendir(dir); 
	memset(results,'\0',NFILES*size);
	void *res;
    res=results;
    int g;
	while ((entrance = readdir(directory)) != NULL) {
    	// Checking if it is a file 
    	if (entrance->d_type == DT_REG) { 
         char relativepath[strlen(dir)+strlen(entrance->d_name)+1];
		 strcpy(relativepath,dir);
         strcat(relativepath,"/"); 
		 strcat(relativepath,entrance->d_name);
         //printf("Sanchay%s\n",relativepath);
         char *filepath = relativepath;
         //Check if it is correct 
         char *duplicatefilepath = strdup(entrance->d_name);
         //printf("%s\n",duplicatefilepath);
         //printf("%s\n",filepath);
         FILE * fp;
		 fp = fopen (filepath, "r");
		 g=g+act(fp,res, duplicatefilepath);
		 fclose(fp);
		 res = res + size;
		}
	}
	closedir(directory);
return g;
}


//Analysis Function

int analysis(FILE* f, void* res, char* filename){
	char c;
	int n=0;
	int line_number = 1;
    int longest_line_length = 0;
    int longest_line_number = 0;
    int total_bytes = 0;
    int ascii_array[128];
    for(int i=0;i<128;i++){
		ascii_array[i]=0;
	}



    while((c = fgetc(f)) != EOF) {
    	//ascii_array = (int)c;
    	//printf("%d\n",ascii_code );
    	ascii_array[(int)c]++;
    	total_bytes = total_bytes + sizeof(c);
        if(c =='\n'){
        	if(longest_line_length < n){
        		longest_line_length = n;
        		longest_line_number = line_number;
        		}
        		n=0;
        		line_number++;    		
        }
        else if (c!='\n')
    	{
    		n++;
    	}
    }
	// printf("File: %s\n", filename);
    // printf("Longest line lenght: %d\n",longest_line_length);
    // printf("longest line number: %d\n",longest_line_number);
    // printf("total bytes in file %d\n", total_bytes);
    struct Analysis *pointer=(struct Analysis*)res;
    
    //Saving part 
    for(int i=0;i<128;i++){
		pointer->ascii[i]=ascii_array[i];
		//printf("%d\n",pointer->ascii[i] );
		}
    pointer->lnlen= longest_line_length;
    pointer->lnno= longest_line_number;
  	pointer->filename=filename; 
    //printf("Saved filename %s\n",pointer->filename );
	return total_bytes;
}


// int sort(FILE* f, int count){
// 	//printf("sort called\n" );
// 	int c;
//     int n = 0;
//     int array[count];
//     while((fscanf(f,"%d",&c)) != EOF){
//         array[n]=c;
//         n++;
//     }
//     for(int i=0;i<n;i++){
//     	printf("Array%d\n",array[i]);
//     }
//     printf("\n");
//     return n;
// return 0;
// }

int stats(FILE* f, void* res, char* filename){
	int c;
	int count=0;
	int total = 0;
	int min=0;
	int max=0;
	//float average;
	int array[NVAL];
	
	//intializing array to zero
	for(int i=0;i<NVAL;i++){
		array[i]=0;
	}

	while((fscanf(f,"%d",&c)) != EOF){
		//printf("%d\n",c );
		array[c]++;
		count=count+1;
		total=total+c;
		if(c>max)
           max=c;
        if(c<min)
           min=c;
	}
	//average = mean(total,count);
	//printf("Mean %f\n", average);
	//printf("minimum:%d\n",min );
	//printf("maximum:%d\n",max );
	struct Stats *pointer=(struct Stats*)res;
	pointer->sum=total;
	pointer->n=count;
	pointer->filename=filename;
	for(int i=0;i<NVAL;i++){
		pointer->histogram[i]=array[i];
	}
	//testing stuff
	//printf("%d\n",pointer->sum);
	//printf("%d\n",pointer->n);
	//printf("%s\n",pointer->filename);
	// for(int i=0;i<NVAL;i++){
	// 	printf("%d\n",pointer->histogram[i] );
	// }
	
	return 0;;
}

//Analysis reduce function defination 

struct Analysis analysis_reduce(int n, void* results){
	
	struct Analysis total;
	total.lnlen=0;
	total.lnno=0;
	for(int i=0;i<128;i++){
	total.ascii[i]=0;	
	}
	
	struct Analysis *temp=(struct Analysis*)results;
	for(int i=0;i<n;i++){
		for(int j=0;j<128;j++){
			total.ascii[j]=total.ascii[j]+temp->ascii[j];
		}
		if(temp->lnlen > total.lnlen) {
			total.lnlen = temp->lnlen;
			total.lnno = temp->lnno;
			total.filename = temp->filename;
		} 
		temp++;
	}
	return total; 
}

//Stats funtion defination 
Stats stats_reduce(int n, void* results){
	struct Stats total;
	total.sum=0;
	total.n=0;
	total.filename=NULL;
	for(int i=0;i< NVAL;i++){
	total.histogram[i]=0;	
	}

	struct Stats *temp=(struct Stats*)results;
	for(int j=0;j<n;j++){
		total.sum = total.sum + temp->sum;
		total.n = total.n + temp->n;
		for(int i=0;i< NVAL;i++){
		total.histogram[i]=total.histogram[i]+temp->histogram[i];	
		}
		temp++;
	}
	//Printing stuff
	// printf("*********total Sum********%d\n",total.sum);
	// printf("***********total count********%d\n",total.n);
	// for(int i=0;i<NVAL;i++){
	// 	//printf("***hist1***%d\n",total.histogram[i]);
	// 	//printf("%d\n",i);
	// }
return total;
}

//Analysis print function defination 

void analysis_print(struct Analysis res, int nbytes, int hist){
	
	printf("File: %s\n",res.filename);
	printf("Longest line length: %d\n",res.lnlen);
	printf("Longest line number: %d\n\n\n",res.lnno );
	if(hist!=0){
		printf("Total Bytes in directory: %d\n",nbytes );
		printf("Histogram:\n");
		for(int i=0;i<128;i++){
			if(res.ascii[i]!=0){
				printf("%d:",i);
				for (int j = 0; j < res.ascii[i]; j++)
				{
					printf("-");
				}
				printf("\n");
			}
		}
	}
}

 
//helper function for stats mean

float mean(int total, int count){
	float m =((float)total)/((float)count);
	return m ; 
}

// Stats Print Function


void stats_print(Stats res, int hist){
	
	// for(int i =0 ; i<NVAL;i++){
	// //printf("***hist***%d\n",res.histogram[i]);	
	// }
	
	if(hist!=0){
		printf("Histogram:\n");
		for(int i=0;i<NVAL;i++){
			if(res.histogram[i]!=0){
				printf("%d :",i);
				for (int j = 0; j < res.histogram[i]; j++)
				{
					printf("-");
				}
				printf("\n");
			}
		}printf("\n");

	}
	
	if(res.filename!=NULL){
		printf("File: %s\n",res.filename );
	}

	printf("Count: %d\n",res.n);
	
	float mean1 = mean(res.sum,res.n);
	printf("Mean: %f\n",mean1);
	
	int min=NVAL;
	int max=-1;
	int largest= -1;
	int count= res.n;
	int index=0;
	for(int i=0;i<NVAL;i++){
		if(res.histogram[i]>largest){
			largest=res.histogram[i];
		}
		if(res.histogram[i]!=0 && i<min){
			min=i;
		}
		if(res.histogram[i]!=0 && i>max){
			max=i;
		}		
	}
	
	printf("Mode: ");
	for(int i=0;i<NVAL;i++){
		if(res.histogram[i]==largest){
			printf("%d ",i);
		}
	}
	printf("\n");
	int fullarray[count];
	for(int i=0;i<NVAL;i++){
		if(res.histogram[i]!=0){
			for(int j=0;j<res.histogram[i];j++){
			fullarray[index]=i;
			index++;
			}
		}
	}
	
	if(count%2!=0){
		int place=(count+1)/2;
		printf("Median: %f\n",(float)fullarray[place] );
	}
	else{
		int place=(count+1)/2;
		printf("Median: %f\n", (float)(fullarray[place]+fullarray[place-1])/(float)2);
	}

	 //count*0.25;
	float random1 = count*0.25;
	if(random1-(int)random1==0){
		int r = (int)random1;
		printf("Q1: %f\n",(float)(fullarray[r]+fullarray[r+1])/(float)2);
	}
	else{
		int r= (int)random1+1;
		printf("Q1: %d\n",fullarray[r]);
	}

	float random2 = count*0.75;
	if(random2-(int)random2==0){
		int r = (int)random2;
		printf("Q3: %f\n",(float)(fullarray[r]+fullarray[r+1])/(float)2);
	}
	else{
		int r= (int)random2+1;
		printf("Q3: %d\n",fullarray[r]);
	}
	printf("Min: %d\n", min);
	printf("Max:%d\n\n", max);

}
