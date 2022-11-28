#define _GNU_SOURCE

#include <time.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <openssl/md5.h>
#include <errno.h>
#include <linux/limits.h>


char * getTimeAndDate(){

	time_t current_time;
    char* c_time_string;

    /* Obtain current time. */
    current_time = time(NULL);

    if (current_time == ((time_t)-1))
    {
        (void) fprintf(stderr, "Failure to obtain the current time.\n");
        exit(EXIT_FAILURE);
    }

    /* Convert to local time format. */
	/* ctime() has already added a terminating newline character. */
    c_time_string = ctime(&current_time);

    if (c_time_string == NULL)
    {
        (void) fprintf(stderr, "Failure to convert the current time.\n");
        exit(EXIT_FAILURE);
    }
    
	return c_time_string;
}

char * calculate_file_md5(FILE *original_fopen_ret) {
	char *filemd5 = (char*) malloc(33 *sizeof(char));

	// create file fingerprint
	unsigned char fingerprint[MD5_DIGEST_LENGTH];
	int length = 0;

	int current_pos = ftell( original_fopen_ret );
	fseek( original_fopen_ret , 0 , SEEK_END );
	length = ftell( original_fopen_ret );

	// read file
	fseek( original_fopen_ret , 0 , SEEK_SET );
	char buf[length];
	fread( buf , 1 , length , original_fopen_ret );

	// reset seek
	fseek( original_fopen_ret , 0 , current_pos );
	
	// create fingerprint
	MD5_CTX mdContext;
	MD5_Init( &mdContext );
	MD5_Update( &mdContext , buf , length );
	MD5_Final( fingerprint , &mdContext );
	for(int i = 0; i < MD5_DIGEST_LENGTH; i++) {
		sprintf(&filemd5[i*2], "%02x", (unsigned int)fingerprint[i]);
	}
	return filemd5;
}

char * recover_filename(FILE * f) {
  	int fd;
  	char fd_path[0xFFF];
  	char * filename = malloc(0xFFF);
  	ssize_t n;

  	fd = fileno(f);
  	sprintf(fd_path, "/proc/self/fd/%d", fd);
  	n = readlink(fd_path, filename, 0xFFF);
  	if (n < 0)
      	return NULL;
  	filename[n] = '\0';
  	return filename;
}


FILE* fopen( const char* path , const char* mode ) 
{
	//Get UID
	uid_t user_id = getuid();

	//Get access type
	int acc_type = 0;
	if(access(path,F_OK)==0){
	//File exists
	if(*mode == 'r'|| strcmp(mode, "r+") == 0 || strcmp(mode, "rb") == 0 || strcmp(mode, "r+b") == 0 || strcmp(mode, "rb+") == 0)
		acc_type = 1;
	//If the file exists and we open it with write permissions, its content are cleared, so the file is considered a new file
	else if( *mode == 'w' || strcmp(mode, "w+") == 0 || strcmp(mode, "wb") == 0 || strcmp(mode, "w+b") == 0 || strcmp(mode, "wb+") == 0)
		acc_type = 3;
	else if( *mode == 'a' || strcmp(mode, "a+") == 0 || strcmp(mode, "ab") == 0 || strcmp(mode, "a+b") == 0 || strcmp(mode, "ab+") == 0)					
		acc_type = 2;
	else
		acc_type = -1;
	}
	else{
	//If the file doesnt already exist and we try to open with write permissions, the file will be created.Same with append permissions
	if( *mode == 'w' || strcmp(mode, "w+") == 0 || strcmp(mode, "wb") == 0 || strcmp(mode, "w+b") == 0 || strcmp(mode, "wb+") == 0)
		acc_type = 0;
	else if( *mode == 'a' || strcmp(mode, "a+") == 0 || strcmp(mode, "ab") == 0 || strcmp(mode, "a+b") == 0 || strcmp(mode, "ab+") == 0)					
		acc_type = 0;
	else
		acc_type = -1;
	}

	//Call the original fopen function
	FILE* original_fopen_ret;
	FILE* (*original_fopen)( const char* , const char* );
	original_fopen = dlsym( RTLD_NEXT , "fopen" );
	original_fopen_ret = (*original_fopen)( path , mode );

	//Check if action denied
	int isDeniedFlag = 0;
	if( !original_fopen_ret && (errno == EACCES || errno == EBADF || errno == EPERM) )
		isDeniedFlag = 1;

	//Get actual path
	char * actualpath = realpath(path,NULL);

	//Open log file in append mode
	FILE* log = (*original_fopen)( "file_logging.log" , "a" );
	//Change permissions
	chmod( "file_logging.log"  , 0777 );

	//Write to file
	fprintf( log , "%d %d %d %s %s %s" , user_id , acc_type , isDeniedFlag , actualpath,calculate_file_md5(original_fopen_ret),getTimeAndDate());

	fclose(log);

	return original_fopen_ret;
}


size_t fwrite( const void* ptr , size_t size , size_t nmemb , FILE* stream ) 
{
	//Get UID
	uid_t user_id = getuid();

	//Get access type
	int acc_type = 2;

	/* call the original fwrite function */
	size_t original_fwrite_ret;
	size_t (*original_fwrite)( const void* , size_t , size_t , FILE* );
	original_fwrite = dlsym( RTLD_NEXT, "fwrite" );
	original_fwrite_ret = (*original_fwrite)( ptr , size , nmemb , stream );

	//Check if action denied
	int isDeniedFlag = 0;
	if( !original_fwrite_ret && (errno == EACCES || errno == EBADF || errno == EPERM) )
		isDeniedFlag = 1;

	//Open log file in append mode
	FILE* (*original_fopen)( const char* , const char* );
	original_fopen = dlsym( RTLD_NEXT , "fopen" );
	FILE* log = (*original_fopen)( "file_logging.log" , "a" );
	//Change permissions
	chmod( "file_logging.log"  , 0777 );

	//Write to log file
	fprintf( log , "%d %d %d %s %s %s" , user_id , acc_type , isDeniedFlag ,recover_filename(stream),calculate_file_md5(stream),getTimeAndDate());

	fclose(log);

	return original_fwrite_ret;
}


