#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>


int main(int argc, char *argv[])
{
	// Variable Declaration Section
	DIR *directory;
	struct dirent *directory_pointer;
	char path[1024];
	char buffer[1024];
	char line[1024];
	char only_directory[1024];
	char command[2];
	int length = 0;
	int c1;
	int count = 0;

	// Clears all garbage from string variables
	memset(path, '\0', sizeof(path));
	memset(only_directory, '\0', sizeof(only_directory));

	// Gets the current directory
	getcwd(only_directory, sizeof(only_directory));
	length = strlen(only_directory);
	//printf("%s\n", only_directory);
	//printf("%d\n", length);
	strcpy(path, only_directory);

	while(1)
	{
		// Changes current working directory
		chdir(path);
		// Puts the current directory path to variable "path"
		getcwd(path, sizeof(path));
		// Prints the current path
		printf("Current Path: %s\n", path);

		// Checks if the new path can be accessed
		for (c1 = 0; c1 < length; c1++)
		{
			if (path[c1] == only_directory[c1])
			{
				count++;
			}
		}
		// Path can be accessed
		if (count == length)
		{
			count = 0;
			if ((directory = opendir(path)) == NULL)
			{
				perror("Cannot open directory");
				exit(1);
			}

			// Displays the directories in current folder
			while ((directory_pointer = readdir(directory)) != NULL)
			{
				if (directory_pointer->d_type == DT_DIR)
				{
					printf("%s\n", directory_pointer->d_name);
				}
			}
			printf("Save on this directory[Y/N]?: ");
			fgets(line, sizeof(line), stdin);
			sscanf(line, "%s", command);
			if (strcmp(command, "Y") == 0)
			{
				break;
			}
			else
			{
				// Gets user input
				printf("Enter directory to go next: ");
				fgets(line, sizeof(line), stdin);
				sscanf(line, "%s", buffer);
				sprintf(path, "%s/%s", path, buffer);
			}
		}
		// Path cannot be acccesed
		else
		{
			printf("Not allowed to access!\n");
			// Changes current path to original path
			strcpy(path, only_directory);
			count = 0;
		}
	}
	
	
	// Create a directory, need "<sys/stat.h>
	//mkdir()
	closedir(directory);
	return 0;
}
