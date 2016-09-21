#include "../include/utfconverter.h"
#include <sys/stat.h>

char* filename;
endianness source;
endianness conversion;

int main(int argc, char** argv)
{
	/* After calling parse_args(), filename and conversion should be set. */
	int fd;
	unsigned char buf[4];
	int rv;
	Glyph* glyph = malloc(sizeof(Glyph));
	
	parse_args(argc, argv);

	verbosity1();
	
	fd = open(filename, O_RDONLY); 
	 
	buf[0]=0;
	buf[1]=0;
	buf[2]=0;
	buf[3]=0;
	rv= 0;

	/*test = open("rsrc/test.txt", O_CREAT | O_WRONLY);*/
	/* Handle BOM bytes for UTF16 specially. 
         * Read our values into the first and second elements. */
	if((rv = read(fd, &buf[0], 1)) == 1 && (rv = read(fd, &buf[1], 1)) == 1){ 
		
		if(buf[0] == 0xff && buf[1] == 0xfe){
			/*file is little endian*/
			source = LITTLE;
			if (source == conversion){
				write(STDOUT_FILENO, &buf[0], 1);
				write(STDOUT_FILENO,&buf[1],1);
			}
			else{
				write(STDOUT_FILENO, &buf[1], 1);
				write(STDOUT_FILENO,&buf[0],1);
			}
		} 

	else if(buf[0] == 0xfe && buf[1] == 0xff){
			/*file is big endian*/
			source = BIG;
			if (source==conversion){
				write(STDOUT_FILENO, &buf[0], 1);
				write(STDOUT_FILENO,&buf[1],1);
			}
			else{
				write(STDOUT_FILENO, &buf[1], 1);
				write(STDOUT_FILENO,&buf[0],1);
			}
		} 

		else {
			/*file has no BOM*/
			free(&glyph->bytes); 
			fprintf(stderr, "File has no BOM.\n");
			quit_converter(NO_FD); 
		}

	}
	
	/* Now deal with the rest of the bytes.*/
	while((rv = read(fd, &buf[0], 1)) == 1 &&  (rv = read(fd, &buf[1], 1)) == 1){

		if(source == conversion){
			write_glyph(fill_glyph(glyph, buf, source, &fd));	
		}

		else {
			write_glyph(swap_endianness(fill_glyph(glyph, buf, source, &fd)));
		}
		
	}

	quit_converter(NO_FD);
	return 0;
}

Glyph* swap_endianness(Glyph* glyph) {
	/* Use XOR to be more efficient with how we swap values. */
	
	unsigned char temporary = glyph->bytes[0];
	glyph->bytes[0] = glyph->bytes[1];
	glyph->bytes[1]= temporary;

	if(glyph->surrogate){  /* If a surrogate pair, swap the next two bytes. */
	 	temporary = glyph->bytes[2];
		glyph->bytes[2] = glyph->bytes[3];
		glyph->bytes[3]= temporary;
	}
	glyph->end = conversion;
	return glyph;
}

Glyph* fill_glyph (Glyph* glyph,unsigned char data[4],endianness end,int* fd)  
{
	
	unsigned int bits = 0; 
	glyph->bytes[0] = data[0];
	glyph->bytes[1] = data[1];

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
}

int file_exist (char *filename)
{
  struct stat   buffer;   
  return (stat (filename, &buffer) == 0);
}

void write_glyph(Glyph* glyph)
{
	if(glyph->surrogate){
		write(STDOUT_FILENO, glyph->bytes, SURROGATE_SIZE);
	} else {
		write(STDOUT_FILENO, glyph->bytes, NON_SURROGATE_SIZE);
	}
}

void parse_args(int argc,char** argv)
{
	int option_index, c;
	char* endian_convert = NULL;

	static struct option long_options[] = {
		{"help", no_argument, 0, 'h'},
		{"h", no_argument, 0, 'h'},
		{"u",required_argument,NULL,'u'},
		{0, 0, 0, 0}
	};

	
	/* If getopt() returns with a valid (its working correctly) 
	 * return code, then process the args! */
	if((c = getopt_long(argc, argv, "hu:", long_options, &option_index)) != -1){
		switch(c){ 
			case 'h':
				print_help();
				break;
			case 'u':
				endian_convert = optarg;
				if(strcmp(endian_convert,"16BE")==0)
				{
					conversion = BIG;
				
				}
				else if(strcmp(endian_convert,"16LE")==0){
					conversion = LITTLE;
					
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
		if (!file_exist (filename))
		{
  			printf("%s\n","File Doesnt Exists" );
			print_help();
  		}
	} 
	else {
		fprintf(stderr, "Filename not given.\n");
		print_help();
	}

	if(endian_convert == NULL){
		fprintf(stderr, "Converson mode not given.\n");
		print_help();
		printf("%s\n","Something");
		quit_converter(NO_FD);
	}
	
}

void print_help(void) {
	int i;
	for(i = 0; i < 4; i++){
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

void verbosity1(void){

	struct utsname unameData;
	struct stat cat;
	size_t inputfilesize;
	
	char hostname[128];
	uname(&unameData);

	gethostname(hostname, sizeof (hostname));

	printf("OS NAME%s\n", unameData.sysname);
	printf("Hostname:%s\n", hostname);

	inputfilesize=0;
	if(stat(filename,&cat)!=0){
		printf("%s\n","ERROR" );
	}
	inputfilesize=cat.st_size;
	printf("FILE SIZE %zd\n",inputfilesize);
	char actualpath [300];
	char *ptr;
	ptr = realpath(filename, actualpath);
	printf("Absolute path%s\n",ptr);

}