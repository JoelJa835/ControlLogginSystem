#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>

#define BUF_SIZE 65536

typedef struct entry {

	int uid; /* user id (positive integer) */
	int access_type; /* access type values [0-2] */
	int action_denied; /* is action denied values [0-1] */
	char *file; /* filename (string) */
	char *fingerprint; /* file fingerprint */
	char *date_and_time; /* file access date */

}entry;


void
usage(void)
{
	printf(
	       "\n"
	       "usage:\n"
	       "\t./monitor \n"
		   "Options:\n"
		   "-m, Prints malicious users\n"
		   "-i <filename>, Prints table of users that modified "
		   "the file <filename> and the number of modifications\n"
		   "-h, Help message\n\n"
		   );

	exit(1);
}

int count_lines(FILE* file)
{
    char buf[BUF_SIZE];
    int counter = 0;
    for(;;)
    {
        size_t res = fread(buf, 1, BUF_SIZE, file);
        if (ferror(file))
            return -1;

        int i;
        for(i = 0; i < res; i++)
            if (buf[i] == '\n')
                counter++;

        if (feof(file))
            break;
    }

    return counter;
}


void 
list_unauthorized_accesses(FILE *log)
{
	int num_of_entries = count_lines(log);
	fseek( log , 0 , SEEK_SET );
	struct entry entries[num_of_entries]; //Similar to Java lol
	// /printf("here %d\n",num_of_entries	);
	//Parse through log file to initialize entries
	for(int i = 0 ; i < num_of_entries ; i++ )
	{
		char* line_buf = NULL;
		size_t line_buf_length = 0;
		getline( &line_buf , &line_buf_length , log );
		entries[i].uid = atoi( strtok( line_buf , " " ) );
		entries[i].access_type = atoi( strtok( NULL , " " ) );
		entries[i].action_denied = atoi( strtok( NULL , " " ) );
		entries[i].file = strtok( NULL , " " );
		entries[i].fingerprint = strtok( NULL , " " );
		entries[i].date_and_time =  strtok( NULL , "\0" ) ;
	}
	int num_of_users = 0;
	int users[num_of_entries];
	users[num_of_users] = entries[num_of_users].uid;
	//Get the different users ignore duplicates
	for( int i = 1 ; i < num_of_entries ; i++ )
	{		
		if(entries[i].uid == entries[i-1].uid){;
			continue;
		}
		else{
			int flag=0;
			for(int k=0; k <=num_of_users; k++){
				if(entries[i].uid == users[k]){
					flag=1;
					break;
				}
			}
				if(flag!=1){
					users[num_of_users+1] = entries[i].uid;
					num_of_users++;
				}
		}	
	}

		int malicious_accesses[num_of_users];

		for( int i = 0 ; i < num_of_users ; i++ ) 
		malicious_accesses[i] = 0;

		for( int i = 0 ; i < num_of_entries ; i++ ){
			for( int k = 0 ; k < num_of_users ; k++ ){
				if( entries[i].uid == users[k] ){
					if( entries[i].action_denied == 1 ){
						malicious_accesses[k]++;
					}
				}
			}
			
		}
		//getpwuid retrieves the user name given the uid
		for(int i = 0 ; i < num_of_users ; i++ ){
			if( malicious_accesses[i] > 7 )
				printf("Malicious User:%s:%d",getpwuid(users[i])->pw_name,users[i]);
		}



	return;

}


void
list_file_modifications(FILE *log, char *file_to_scan)
{
	int num_of_entries = count_lines(log);
	//Carefull we are at the end of the file so we have to bring the cursor back in the start of the file.
	fseek( log , 0 , SEEK_SET );
	struct entry entries[num_of_entries]; //Similar to Java lol
	//Parse through log file to initialize entries
	for(int i = 0 ; i < num_of_entries ; i++ )
	{
		char* line_buf = NULL;
		size_t line_buf_length = 0;
		getline( &line_buf , &line_buf_length , log );
		entries[i].uid = atoi( strtok( line_buf , " " ) );
		entries[i].access_type = atoi( strtok( NULL , " " ) );
		entries[i].action_denied = atoi( strtok( NULL , " " ) );
		entries[i].file = strtok( NULL , " " );
		entries[i].fingerprint = strtok( NULL , " " );
		entries[i].date_and_time =  strtok( NULL , "\0" ) ;
	}

	int num_of_users = 0;
	int users[num_of_entries];
	users[num_of_users] = entries[num_of_users].uid;

	//Get the different users ignore duplicates
	for( int i = 1 ; i < num_of_entries ; i++ )
	{		
		if(entries[i].uid == entries[i-1].uid){;
			continue;
		}
		else{
			int flag=0;
			for(int k=0; k <=num_of_users; k++){
				if(entries[i].uid == users[k]){
					flag=1;
					break;
				}
			}
				if(flag!=1){
					users[num_of_users+1] = entries[i].uid;
					num_of_users++;
				}
		}	
	}

	int file_changes_per_user[num_of_users];
	for( int i = 0 ; i <=num_of_users-1 ; i++ ){
		file_changes_per_user[i] = 0;
		fprintf(stdout,"%d\n", file_changes_per_user[i]);
	}
	//Get actual path
	char * actualpath = realpath(file_to_scan,NULL);

	char prev_fingerprint[33] =" "; 

	//Check if the entries path matches the actualpath and if the previous fingerprint of the same file has changed or not.
		//If it has changed it means we have some modification on the file.
		//Note strcmp returns 0 if they match.So we have to use not operator.
	for( int i = 0 ; i < num_of_entries ; i++ )
	{
		if( !strcmp( entries[i].file , actualpath ) && strcmp( entries[i].fingerprint , prev_fingerprint ) != 0 )
		{
			//Check if the entries path matches the actualpath and if the previous fingerprint of the same file has changed or not.
			//If it has changed it means we have some modification on the file.
			//Note strcmp returns 0 if they match.So we have to use not operator.
			for( int k = 0 ; k <= num_of_users ; k++ )
			{	
				if( entries[i].uid == users[k] && entries[i].access_type != 1 && entries[i].action_denied != 1 )
				{
					file_changes_per_user[k]++;
					strcpy( prev_fingerprint , entries[i].fingerprint );
				}
			}
		}
	}
	//Getpwuid retrieves the user name given the uid
	for( int k = 0 ; k <=num_of_users-1 ; k++ ){
		printf("%d\n",file_changes_per_user[0]);
		printf("User:%s Id:%d  Modifications:%d\n",getpwuid(users[k])->pw_name,users[k], file_changes_per_user[k]);
	}	


	return;

}


int 
main(int argc, char *argv[])
{

	int ch;
	FILE *log;

	if (argc < 2)
		usage();

	log = fopen("./file_logging.log", "r");
	if (log == NULL) {
		printf("Error opening log file \"%s\"\n", "./log");
		return 1;
	}

	while ((ch = getopt(argc, argv, "hi:m")) != -1) {
		switch (ch) {		
		case 'i':
			list_file_modifications(log, optarg);
			break;
		case 'm':
			list_unauthorized_accesses(log);
			break;
		default:
			usage();
		}

	}


	fclose(log);
	argc -= optind;
	argv += optind;	
	
	return 0;
}
