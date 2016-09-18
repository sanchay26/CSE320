#include "../include/utfconverter.h"
//#include "struct.txt" 
char* filename;
endianness source;
endianness conversion;

int test=0;
int main(int argc, char** argv)
//char** argv;
{
	/* After calling parse_args(), filename and conversion should be set. */

	parse_args(argc, argv);
	
	int fd = open("rsrc/utf16le.txt", O_RDONLY); 

	unsigned int buf[4]; 
	buf[0]=0;
	buf[1]=0;
	buf[2]=0;
	buf[3]=0;
	int rv = 0;
	Glyph* glyph = malloc(sizeof(Glyph)); 

	
	/* Handle BOM bytes for UTF16 specially. 
         * Read our values into the first and second elements. */
	if((rv = read(fd, &buf[0], 1)) == 1 && (rv = read(fd, &buf[1], 1)) == 1){ 
		
		if(buf[0] == 0xff && buf[1] == 0xfe){
			/*file is little endian*/
			//printf("*********%x\n",buf[0]);
			source = LITTLE; 
		} else if(buf[0] == 0xfe && buf[1] == 0xff){
			/*file is big endian*/
			source = BIG;
		} else {
			/*file has no BOM*/
			free(&glyph->bytes); 
			fprintf(stderr, "File has no BOM.\n");
			quit_converter(NO_FD); 
		}

		void* memset_return = memset(glyph, 0, sizeof(Glyph)+1);
		/* Memory write failed, recover from it: */
		if(memset_return == NULL){
			/* tweak write permission on heap memory. */
			/* Now make the request again. */
			memset(glyph, 0, sizeof(Glyph)+1);
		}
	}
	test = open("rsrc/test.txt", O_CREAT | O_WRONLY);
	//write();
	/* Now deal with the rest of the bytes.*/
	while((rv = read(fd, &buf[0], 1)) == 1 &&  (rv = read(fd, &buf[1], 1)) == 1){

		write_glyph(fill_glyph(glyph, buf, source, &fd));
		
		void* memset_return = memset(glyph, 0, sizeof(Glyph)+1);
	        /* Memory write failed, recover from it: */
	        if(memset_return == NULL){
		        /* tweak write permission on heap memory. */
		        /* Now make the request again. */
		        memset(glyph, 0, sizeof(Glyph)+1);
	        }
	}
	quit_converter(NO_FD);
	return 0;
}

Glyph* swap_endianness(Glyph* glyph) {
	/* Use XOR to be more efficient with how we swap values. */
	glyph->bytes[0] ^= glyph->bytes[1];
	glyph->bytes[1] ^= glyph->bytes[0];
	if(glyph->surrogate){  /* If a surrogate pair, swap the next two bytes. */
		glyph->bytes[2] ^= glyph->bytes[3];
		glyph->bytes[3] ^= glyph->bytes[2];
	}
	glyph->end = conversion;
	return glyph;
}

Glyph* fill_glyph (Glyph* glyph,unsigned int data[4],endianness end,int* fd)  
{
	glyph->bytes[0] = data[0];
	glyph->bytes[1] = data[1];
	
	unsigned int bits = 0; 


	if(end == LITTLE){
		bits |= (data[FIRST] + (data[SECOND] << 8));	

	}
	else if (end == BIG){
		bits |= ((data[FIRST]<<8) + data[SECOND]);
	}

	if(bits > 0xD800 && bits < 0xDBFF){ 
		
		if(read(*fd, &data[THIRD], 1) == 1 && read(*fd, &data[FOURTH], 1) == 1){
					
			bits = 0;
					
			if(end == LITTLE){
				
				bits |= (data[THIRD] + (data[FOURTH] << 8)) ;	
			} 
			else if(end == BIG){
				
				bits |= ((data[THIRD]<<8) + data[FOURTH]) ;	
			} 
			
			if(bits > 0xDC00 && bits < 0xDFFF){ /* Check low surrogate pair.*/
			glyph->surrogate = true; 
			} 
			
			else {
				lseek(*fd, -OFFSET, SEEK_CUR); 
				glyph->surrogate = false;
			}
		}
	}	

	else{
		
		glyph->surrogate = false;
	}
	if(!glyph->surrogate){
		glyph->bytes[THIRD] = 0;
		glyph->bytes[FOURTH]= 0;
	} else {
		glyph->bytes[THIRD] = data[THIRD]; 
		glyph->bytes[FOURTH] = data[FOURTH];
	}
	glyph->end = end;

	return glyph;

	//*****************************************
	//bits |= (data[FIRST] + (data[SECOND] << 8));
	/* Check high surrogate pair using its special value range.*/
	//if(bits > 0xD800 && bits < 0xDBFF){ 
	//	if(read(*fd, &data[SECOND], 1) == 1 && 
	//		read(*fd, &data[FIRST], 1) == 1){
	//		bits = 0; /* bits |= (bytes[FIRST] + (bytes[SECOND] << 8)) */
	//		if(bits > 0xDC00 && bits < 0xDFFF){ /* Check low surrogate pair.*/
	//			glyph->surrogate = false; 
	//		} else {
	//			//lseek(*fd, -OFFSET, SEEK_CUR); 
	//			glyph->surrogate = true;
	//		}
	//	}
	//}
	//******************************
}

void write_glyph(Glyph* glyph)
{
	if(glyph->surrogate){
		//printf("%s\n", "Somewhere");
		write(test, glyph->bytes, SURROGATE_SIZE);
	} else {
		write(test, glyph->bytes, NON_SURROGATE_SIZE);
	}
}

void parse_args(int argc,char** argv)
{
	int option_index, c;
	char* endian_convert = NULL;

	// Copied from struct.txt
	static struct option long_options[] = {
		{"help", no_argument, 0, 'h'},
		{"h", no_argument, 0, 'h'},
		{"u",required_argument,NULL,'u'},
		{0, 0, 0, 0}
	};

	
	/* If getopt() returns with a valid (its working correctly) 
	 * return code, then process the args! */
	if((c = getopt_long(argc, argv, "hu:", long_options, &option_index)) 
			!= -1){
		switch(c){ 
			case 'h':
				print_help();
				break;
			case 'u':
				//printf("optarg%s\n",argv[optind]);
				endian_convert = optarg;
				printf("***endian***%s\n",endian_convert );
				if(strcmp(endian_convert,"16BE")==0)
				{
					//Big Endian Condition
					//printf("%s\n","****Big Endian****");
				}
				else if(strcmp(endian_convert,"16LE")==0){
					//Little Endian Condition
				//printf("%s\n","****Little Endian****");	
				}
				else {
					printf("%s\n","wrong arguments" );
				}
				break;
			default:
				fprintf(stderr, "Unrecognized argument.\n");
				quit_converter(NO_FD);
				break;
		}

	}

	if(optind < argc){
		filename= malloc(strlen(argv[optind])*sizeof(char));
		strcpy(filename, argv[optind]);
	} else {
		fprintf(stderr, "Filename not given.\n");
		print_help();
	}

	if(endian_convert == NULL){
		fprintf(stderr, "Converson mode not given.\n");
		print_help();
	}

	if(strcmp(endian_convert, "LE")){ 
		conversion = LITTLE;
	} else if(strcmp(endian_convert, "BE")){
		conversion = BIG;
	} else {
		quit_converter(NO_FD);
	}
}

void print_help(void) {
	for(int i = 0; i < 4; i++){
		printf("%s",USAGE[i]); 
	}
	quit_converter(NO_FD);
}

void quit_converter(int fd)
{
	close(STDERR_FILENO);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	if(fd != NO_FD)
		close(fd);
	exit(0);
	/* Ensure that the file is included regardless of where we start compiling from. */
}
