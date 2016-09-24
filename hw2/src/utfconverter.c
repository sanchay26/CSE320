#include "../include/utfconverter.h"
#include <sys/stat.h>
#include <sys/times.h>

char* filename;
char* outfile=NULL;
endianness source;
endianness conversion;
int utf8;
int outfd = STDOUT_FILENO;
int rv;
int verbosity = 0;
int someflag = 0;
int visited = 0;
int totalascii = 0;
int totalsurrogate = 0;
int totalglyphs = 0;
static clock_t st_read_time;
static clock_t en_read_time;
static clock_t st_convert_time;
static clock_t en_convert_time;
static clock_t st_write_time;
static clock_t en_write_time;
static struct tms st_read;
static struct tms en_read;
static struct tms st_write;
static struct tms en_write;
static struct tms st_convert;
static struct tms en_convert;
int main(int argc, char** argv)
{
	/* After calling parse_args(), filename and conversion should be set. */
	int fd;
	unsigned char buf[4];
	unsigned char buf1[4];
	unsigned char newline[2];
	Glyph* glyph = malloc(sizeof(Glyph));
	newline[0]= 0x0a;
	newline[1]= 0x00;

	buf1[0]=0;
	buf1[1]=0;

	visited =0;
	parse_args(argc, argv);

	
	fd = open(filename, O_RDONLY); 

	/*Output Ridirection stuff*/

	if(outfile!=NULL){
		
		outfd = open(outfile,O_CREAT|O_RDWR|O_APPEND,0666); 
		
		if((rv = read(outfd, &buf1[0], 1)) == 1){

			if((rv = read(outfd, &buf1[1], 1)) == 1){

					if(buf1[0] == 0xff && buf1[1] == 0xfe){

						visited =1;
						if(conversion == BIG){
							printf("%s\n","Mismatch BOM" );
							quit_converter(NO_FD);
							return EXIT_FAILURE;
						}
						else{
							write(outfd,&newline[0],1);
							write(outfd,&newline[1],1);
						}
						
					}
					else if(buf1[0] == 0xfe && buf1[1] == 0xff){
						/*BOM BIG and conversion little break*/
						visited =1;
						if(conversion == LITTLE){
							printf("%s\n","Mismatch BOM" );
							quit_converter(NO_FD);
							return EXIT_FAILURE;
						}
						else{
							write(outfd,&newline[1],1);
							write(outfd,&newline[0],1);
						}
					}					

			}			
			else{
				/*Only one byte in the file EXIT PROGRAMME*/
				quit_converter(NO_FD);
				return EXIT_FAILURE;
			}
		}
	}

	buf[0]=0;
	buf[1]=0;
	buf[2]=0;
	buf[3]=0;
	rv= 0;
	utf8=0;
	/*test = open("rsrc/test.txt", O_CREAT | O_WRONLY);*/
	/* Handle BOM bytes for UTF16 specially. 
         * Read our values into the first and second elements. */
	if((rv = read(fd, &buf[0], 1)) == 1 && (rv = read(fd, &buf[1], 1)) == 1){ 
		
		/********Here starts UTF-8 Stuff ********/

		if(buf[0]==0xef && buf[1]==0xbb){

			if((rv = read(fd,&buf[2],1)==1)){

				if(buf[2]==0xbf){
					utf8 = 1;
				}
			}
		}
		/*************************/

		else if(buf[0] == 0xff && buf[1] == 0xfe){
			/*file is little endian*/
			source = LITTLE;
			if(visited == 0){
				if (source == conversion){
					write(outfd, &buf[0], 1);
					write(outfd,&buf[1],1);
				}
				else{
					write(outfd, &buf[1], 1);
					write(outfd,&buf[0],1);
				}	
			}
			
		} 

		else if(buf[0] == 0xfe && buf[1] == 0xff){
			/*file is big endian*/
			source = BIG;

			if(visited == 0){
				if (source==conversion){
					write(outfd, &buf[0], 1);
					write(outfd,&buf[1],1);
				}
				else{
					write(outfd, &buf[1], 1);
					write(outfd,&buf[0],1);
				}
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
	if(utf8==1){
		unsigned char bom[2];
		bom[0]= 0xff;
		bom[1]= 0xfe;
		if(conversion== LITTLE && visited==0){
			write(outfd, &bom[0], 1);
			write(outfd, &bom[1], 1);
		}
		else if (conversion== BIG && visited==0){
			write(outfd, &bom[1], 1);
			write(outfd, &bom[0], 1);
		}
		while((rv = read(fd, &buf[0], 1)) == 1){
			write_glyph(mock_glyph(glyph,buf,source,&fd));
		}

	}
	else
	{
		while((rv = read(fd, &buf[0], 1)) == 1 &&  (rv = read(fd, &buf[1], 1)) == 1 && someflag==0){

			if(source == LITTLE){
				st_read_time = times(&st_read);
				fill_glyph(glyph, buf, source, &fd);
				en_read_time = times(&en_read);
				st_write_time = times(&st_write);
				write_glyph(glyph);	
				en_write_time = times(&en_write);
			}

			else if(source == BIG) {
				st_read_time = times(&st_read);
				fill_glyph(glyph, buf, source, &fd);
				en_read_time = times(&en_read);
				st_convert_time = times(&st_convert);
				swap_endianness(glyph);
				en_convert_time = times(&en_convert);
				st_write_time = times(&st_write);
				write_glyph(glyph);
				en_write_time = times(&en_write);
			}
		}
	}
	if(verbosity>1){
		verbosity1();
		verbosity2();
	}
	if(verbosity==1){
		verbosity1();
	}
	free(&glyph->bytes); 
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
	
	if(conversion == LITTLE){

		totalglyphs = totalglyphs+1;
		
		if(glyph->surrogate){

		totalsurrogate = totalsurrogate +1;
		write(outfd, glyph->bytes, SURROGATE_SIZE);
		} 
		
		else {
			if(glyph->bytes[1]==0x00){
				totalascii = totalascii+1;
			}
		write(outfd, glyph->bytes, NON_SURROGATE_SIZE);
		}
	}
	if(conversion == BIG){
		totalglyphs= totalglyphs+1;
		if(glyph->surrogate){
		totalsurrogate = totalsurrogate +1;
		write(outfd, &glyph->bytes[1],1);
		write(outfd, &glyph->bytes[0],1);
		write(outfd, &glyph->bytes[3],1);
		write(outfd, &glyph->bytes[2],1);
		} 
		
		else {
			if(glyph->bytes[1]==0x00){
				totalascii = totalascii+1;
			}
		write(outfd, &glyph->bytes[1], 1);
		write(outfd, &glyph->bytes[0], 1);
		}
	}
}

void parse_args(int argc,char** argv)
{
	int option_index, c;
	char* endian_convert = NULL;

	static struct option long_options[] = {
		{"help", no_argument, 0, 'h'},
		{"h", no_argument, 0, 'h'},
		{"v",no_argument,0,'v'},
		{"UTF",required_argument,NULL,'u'},
		{0, 0, 0, 0}
	};

	/* If getopt() returns with a valid (its working correctly) 
	 * return code, then process the args! */
	while((c = getopt_long(argc, argv, "hu:v", long_options, &option_index)) != -1){
		
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
					printf("%s\n","Wrong Arguments");
					print_help();
				}
				break;
			case 'v':
				verbosity++;
				break;
			default:
				fprintf(stderr, "Unrecognized argument.\n");
				quit_converter(NO_FD);
				break;
		}

	}

	if(optind < argc){
		filename= malloc((strlen(argv[optind])+1)*sizeof(char));
		strcpy(filename, argv[optind]);
		if(argv[optind+1]!=NULL){
			outfile= malloc((strlen(argv[optind+1])+1)*sizeof(char));
			strcpy(outfile,argv[optind+1]);
			if(strcmp(filename,outfile)==0){
				printf("%s\n","Same File");
				quit_converter(NO_FD);
			}
		}
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
		quit_converter(NO_FD);
	}
	
}

void print_help(void) {
	int i;
	for(i = 0; i < 9; i++){
		printf("%s",USAGE[i]); 
	}
	quit_converter(NO_FD);
}

void quit_converter(int fd)
{	free(outfile);
	free(filename);
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
	char actualpath [300];
	char *ptr;
	char hostname[128];
	char *buffer = filename;
	char *input;
	char *output;
	
	uname(&unameData);
	

	if(source == LITTLE){
		input = "UTF-16LE";
	}
	
	if (source == BIG){
		input = "UTF-16BE";
	}
	if(utf8 ==1){
		input = "UTF-8";
	}

	if(conversion == LITTLE){
		output = "UTF-16LE";
	}
	if(conversion == BIG){
		output = "UTF-16BE";
	}

	gethostname(hostname, sizeof (hostname));
	inputfilesize=0;
	 if(stat(filename,&cat)!=0){
	printf("%s\n","ERROR" );
	 }
	inputfilesize=cat.st_size;
	ptr = realpath(buffer, actualpath);

	printf("	Input file size: %d kb\n",(int)inputfilesize);
	printf("	Input file path: %s\n",ptr);
	printf("	Input file encoding: %s\n",input);
	printf("	Output file encoding: %s\n",output);
	printf("	Hostmachine: %s\n", hostname);
	printf("	Operating System: %s\n", unameData.sysname);

}
void verbosity2(){
	int asciipercent=0;
	int surrogatepercent=0;

	printf("	Reading: real=%f, user=%f, sys%f\n",
        (float)(en_read_time - st_read_time),
        (float)(en_read.tms_utime - st_read.tms_utime),
        (float)(en_read.tms_stime - st_read.tms_stime));
	printf("	Converting: real=%f, user=%f, sys%f\n",
        (float)(en_convert_time - st_convert_time),
        (float)(en_convert.tms_utime - st_convert.tms_utime),
        (float)(en_convert.tms_stime - st_convert.tms_stime));
	printf("	Writing: real=%f, user=%f, sys%f\n",
        (float)(en_write_time - st_write_time),
        (float)(en_write.tms_utime - st_write.tms_utime),
        (float)(en_write.tms_stime - st_write.tms_stime));
	asciipercent = ((float)totalascii/(float)(totalglyphs)) *100;
	surrogatepercent = ((float)totalsurrogate/(float)totalglyphs)*100;
	printf("	ASCII:%d%%\n",asciipercent);
	printf("	Surrogates: %d%%\n",surrogatepercent);
	printf("	Glyphs: %d\n",totalglyphs);

}
Glyph* mock_glyph (Glyph* glyph,unsigned char data[4],endianness end,int* fd){
	
	unsigned int  bits = 0; 
	unsigned char byte0 = 0;
	unsigned char byte1 = 0;
	unsigned char byte2 = 0;
	unsigned int  merge = 0;
	unsigned int merge1 = 0;
	unsigned int mergeh = 0;
	unsigned int mergel = 0;
	unsigned char final0 = 0;
	unsigned char final1 = 0;
	unsigned char final2 = 0;
	unsigned char final3 = 0;

	if(end==LITTLE){

	}
	bits |= (data[0] >> 7) ;
	
	if(bits==0x00){
		glyph->bytes[0] = data[0];
		glyph->bytes[1] = 0x00;
		glyph->surrogate = false;
		/*Glyph is encoded in one byte*/
	}
	
	else {
		bits=0;
		
		bits |= (data[0] >> 5);
		if(bits == 0x06){
				/*Glyph is encoded in two bytes*/
			if((rv=read(*fd, &data[1], 1)) == 1){   
				glyph->bytes[0]=((data[0]<<6)+(data[1] & 0x3f));
				glyph->bytes[1]=(((data[0]&0x1f)>>2) & 0x07); 
				glyph->surrogate = false;
				return glyph;
			}
			else{
				printf("%s\n", "ERROR");
			}
		}
		bits=0;
		bits |= (data[0] >> 4);
		if(bits == 0x0e){
			/*Glyph is encoded in three bytes*/
			if((rv=read(*fd, &data[1], 1)) == 1 && (rv=read(*fd, &data[2], 1))==1){  
				glyph->bytes[0] = ((data[1]&0x3f)<<6)+(data[2]&0x3f);
				glyph->bytes[1] = ((data[0]&0x0f)<<4)+((data[1]&0x3c)>>2);
				glyph->surrogate = false;
				return glyph;
			}
			
		}
		bits=0;
		bits |= (data[0] >> 3);
		if(bits == 0x1e){
			/*Glyph is encoded in four bytes*/
			if(read(*fd, &data[1], 1) == 1 && read(*fd, &data[2], 1)==1 && read(*fd, &data[3], 1)==1){
				byte0= (data[3]&0x3f) + (data[2]<<6);
				byte1= ((data[2]&0x3c)>>2)+(data[1]<<4);
				byte2= ((data[1]& 0x30)>>4)+((data[0]&0x07)<<2);
				merge |=((byte0)+(byte1<<8))+(byte2<<16);
				if(merge>0x10000){

					merge1 = merge - 0x10000;
					mergeh = merge1>>10;
					mergel = merge1 & 0x3ff;

					mergeh = 0xd800 + mergeh;
					mergel = 0xdc00 + mergel;

					final0 = mergeh & 0xff;
					final1 = (mergeh >>8) & 0xff;
					final2 = mergel & 0xff;
					final3 = (mergel >>8) & 0xff;

					glyph->bytes[0]=final0;
					glyph->bytes[1]=final1;
					glyph->bytes[2]=final2;
					glyph->bytes[3]=final3;
					glyph->surrogate = true;
					return glyph;
				}
				else {
					printf("%s\n","Wrong UTF8 Encoding" );
				}

			}
			

		}
		else{
			printf("%s\n","Wrong UT8-8 Encoding " );
		}
	}

return glyph;
}