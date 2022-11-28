#include <stdio.h>
#include <string.h>

int main() 
{
	int i;
	size_t bytes;
	FILE *file;
	// char filenames[10][7] = {"file_0", "file_1", 
	// 		"file_2", "file_3", "file_4",
	// 		"file_5", "file_6", "file_7", 		
	// 		"file_8", "file_9"};
	char filenames[2][7] = { "file_0" , "file_1"  };


	/* example source code */

	for (i = 0; i < 2; i++) {

		file = fopen(filenames[i], "w+");
		if (file == NULL) 
			printf("fopen error\n");
		else {
			printf("Here %s\n",filenames[i]);
			fclose(file);
		}

	}

	file = fopen(filenames[0], "r"); // file_0 open once
	if (file == NULL) 
		printf("fopen error\n");
	else
	{	// all should fail because file was opened with wrong mode
		bytes = fwrite(filenames[0], strlen(filenames[0]), 1, file); // file modification 5 times
		bytes = fwrite(filenames[0], strlen(filenames[0]), 1, file);
		bytes = fwrite(filenames[0], strlen(filenames[0]), 1, file);
		bytes = fwrite(filenames[0], strlen(filenames[0]), 1, file);
		bytes = fwrite(filenames[0], strlen(filenames[0]), 1, file);
		fclose(file);
	}

	file = fopen(filenames[1], "r+"); // file_1 open once
	if (file == NULL) 
		printf("fopen error\n");
	else
	{	// all should work if user has write rights
		bytes = fwrite(filenames[1], strlen(filenames[1]), 1, file); // file modification 5 times
		bytes = fwrite(filenames[1], strlen(filenames[1]), 1, file);
		bytes = fwrite(filenames[1], strlen(filenames[1]), 1, file);
		bytes = fwrite(filenames[1], strlen(filenames[1]), 1, file);
		bytes = fwrite(filenames[1], strlen(filenames[1]), 1, file);
		fclose(file);
	}
	//fclose(file);
}
