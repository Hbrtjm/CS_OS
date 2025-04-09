#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

#define RECURSION_LIMIT 1000
#define PATH_SIZE 512  

int count_size(const char* dirname, int depth)
{
	if(depth >= RECURSION_LIMIT)
	{
		printf("Recursion depth limit exceeded\n");
		return 0;
	}    
	DIR* mydir = opendir(dirname);
	if (mydir == NULL)
	{
		perror("Error opening directory");
		return 0;
	}
	struct dirent *file;
	int size = 0;
	while ((file = readdir(mydir)) != NULL)
	{
		if (strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0)
			continue;
		
		char file_path[PATH_SIZE];
		snprintf(file_path, sizeof(file_path), "%s/%s", dirname, file->d_name);
		
		struct stat file_stat;
	if (stat(file_path, &file_stat) == -1)
		{
			perror("stat error");
			continue;
		}
		
		if (S_ISDIR(file_stat.st_mode))
		{
			size += count_size(file_path, depth + 1);
		}
		else
		{
			FILE *fp = fopen(file_path, "rb");
			if (fp)
			{
				fseek(fp, 0L, SEEK_END);
				size += ftell(fp);
				fclose(fp);
			}
			else
			{
				perror("Error opening file, check your permissions");
			}
		}
	}
	closedir(mydir);
	return size;
}

int main(int argc, char *argv[])
{
	char dirname[PATH_SIZE];
	
	if (argc < 2)
	{
		printf("Relative path to the directory name: ");
		scanf("%s", dirname);
	}
	else
	{
		strncpy(dirname, argv[1], PATH_SIZE - 1);
		dirname[PATH_SIZE - 1] = '\0';
	}
	
	printf("Counting filesize of %s\n", dirname);
	printf("The directory takes up: %d bytes\n", count_size(dirname, 1));
	
	return 0;
}

