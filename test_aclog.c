#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include<sys/wait.h>
#include <unistd.h>

int main() 
{
	int i;
	size_t bytes;
	FILE *file;
	char filenames[10][7] = {"file_0", "file_1", 
			"file_2", "file_3", "file_4",
			"file_5", "file_6", "file_7", 		
			"file_8", "file_9"};
	// char filenames[2][7] = { "file_0" , "file_1"  };


	/* example source code */

	for (i = 0; i < 10; i++) {

		file = fopen(filenames[i], "w+");
		if (file == NULL) 
			printf("fopen error\n");
		else {
			printf(" %s\n",filenames[i]);
			fclose(file);
		}

	}

	for(int i = 0; i < 10; i++){
	file = fopen(filenames[i], "r"); // trying to open files in read mode
		if (file == NULL) 
			printf("fopen error\n");
		else
		{	//All should fail because file was opened with wrong mode
			bytes = fwrite(filenames[i], 1, strlen(filenames[i]), file); 
			fclose(file);
		}
	}

	for(int i = 0; i < 10; i++){
	file = fopen(filenames[i], "r+"); 
		if (file == NULL) 
			printf("fopen error\n");
		else
		{	
			bytes = fwrite(filenames[i], 1, strlen(filenames[i]), file); 
			fclose(file);
		}
	}


	setuid(1000);

	for(int i = 0; i < 10; i++){
	file = fopen(filenames[i], "w"); // 
		if (file == NULL) 
			printf("fopen error\n");
		else
		{	// all should fail because file was opened with wrong mode
			bytes = fwrite(filenames[i], 1, strlen(filenames[i]), file); 
			fclose(file);
		}
	}



}
