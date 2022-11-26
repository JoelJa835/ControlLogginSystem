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

#ifndef LOGFILE
#define LOGFILE "//home/jogopogo/Projects/Archive/src_corpus/file_logging.log"   
#endif




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
	unsigned char c[MD5_DIGEST_LENGTH];
	int i;
	MD5_CTX mdContext;
	int bytes;
	unsigned char data[1024];
	char *filemd5 = (char*) malloc(33 *sizeof(char));

	MD5_Init (&mdContext);

	while ((bytes = fread (data, 1, 1024, original_fopen_ret)) != 0)

	MD5_Update (&mdContext, data, bytes);

	MD5_Final (c,&mdContext);

	for(i = 0; i < MD5_DIGEST_LENGTH; i++) {
		sprintf(&filemd5[i*2], "%02x", (unsigned int)c[i]);
	}

	//printf("calculated md5:%s ", filemd5);
	return filemd5;
}






FILE *
fopen(const char *path, const char *mode) 
{
	char *filemd5 = (char*) malloc(33 *sizeof(char));
	uid_t user_id = getuid();
	int isDeniedFlag = 0;
	int acc_type = 0;
	
	fprintf(stdout,"Went into custom fopen\n");
	//Check if the file already exists
	//Note both stat and access have the same TOCTOU vulnerability.So using x mode while opening would be prefered to avoid the TOCTOU.
	//However it exceeds the boundaries of the project, so we continue with access.
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
	
  	FILE *original_fopen_ret;
	FILE *(*original_fopen)(const char*, const char*);
	/* call the original fopen function */
	original_fopen = dlsym(RTLD_NEXT, "fopen");
	original_fopen_ret = (*original_fopen)(path, mode);

	if(!original_fopen_ret && (errno == EACCES || errno == EBADF || errno == EPERM))
		isDeniedFlag = 1;

	char * actualpath = realpath(path,NULL);
	if (actualpath == NULL)
		printf("Errno: %d\n", errno);

	if(isDeniedFlag){
		original_fopen_ret = (*original_fopen)( path , "r" );
		filemd5 = calculate_file_md5(original_fopen_ret);
		fclose(original_fopen_ret);
	}
	else
		filemd5 = calculate_file_md5(original_fopen_ret);
	
	FILE* log = (*original_fopen)( "file_logging.log" , "a" );
	chmod( "file_logging.log" , 0777 );


	fprintf(stdout,"Before writing\n");
	fprintf(log,"%d %s %d %d %s %s",user_id,actualpath,acc_type,isDeniedFlag,filemd5,getTimeAndDate());
           
	fclose(log);
	

	free(actualpath);
	return original_fopen_ret;
}


size_t 
fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) 
{

	size_t original_fwrite_ret;
	size_t (*original_fwrite)(const void*, size_t, size_t, FILE*);

	/* call the original fwrite function */
	original_fwrite = dlsym(RTLD_NEXT, "fwrite");
	original_fwrite_ret = (*original_fwrite)(ptr, size, nmemb, stream);

	



	return original_fwrite_ret;
}


