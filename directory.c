#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>


int main(int argc, char *argv[])
{
	DIR *directory;
	struct dirent *directory_pointer;
	char path[1024];
	char buffer[1024];
	char line[1024];
	char only_directory[1024];
	int length = 0;
	int c1;
	int count = 0;
	memset(path, '\0', sizeof(path));
	memset(only_directory, '\0', sizeof(only_directory));
	getcwd(only_directory, sizeof(only_directory));
	length = strlen(only_directory);
	printf("%s\n", only_directory);
	printf("%d\n", length);
	strcpy(path, only_directory);

	while(1)
	{
		for (c1 = 0; c1 < length; c1++)
		{
			if (path[c1] == only_directory[c1])
			{
				count++;
			}
		}
		if (count == length)
		{
			count = 0;
			if ((directory = opendir(path)) == NULL)
			{
				perror("Cannot open directory");
				exit(1);
			}

			// Search for directory
			while ((directory_pointer = readdir(directory)) != NULL)
			{
				if (directory_pointer->d_type == DT_DIR)
				{
					printf("%s\n", directory_pointer->d_name);
				}
			}
			
			printf("Enter directory to go next: ");
			fgets(line, sizeof(line), stdin);
			sscanf(line, "%s", buffer);
			sprintf(path, "%s/%s", path, buffer);
			printf("Path: %s\n", path);
		}
		else
		{
			break;
		}
	}
	
	
	// Create a directory, need "<sys/stat.h>
	mkdir("./example", 0700);
	closedir(directory);
	return 0;
}
