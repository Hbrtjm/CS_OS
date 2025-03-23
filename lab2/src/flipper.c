#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

#define PATH_SIZE 30
#define NAME_SIZE 10

void slice(const char* str, char* result, size_t start, size_t end) {
    strncpy(result, str + start, end - start);
}

int is_text_file(const char *filename) {
    const char *ext = strrchr(filename, '.');
    return (ext && strcmp(ext, ".txt") == 0);  
}

int reverse(char *dirname)
{
	DIR* mydir = opendir(dirname);
        if (mydir == NULL)
        {
                perror("Error opening directory");
                return 1;
        }
	char* newdirname = (char*)malloc(sizeof(char) * (PATH_SIZE + NAME_SIZE));
        strcpy(newdirname, dirname);
	strcat(newdirname, "_reversed");
	mkdir(newdirname,0777);
        struct dirent *file;
	int file_size = 0;
	char content[1000000];
	int sum = 0;
	while((file = readdir(mydir)) != NULL)
        {
		file_size = 0;
                sum = 0;
		if(strcmp(file->d_name, ".") != 0 && strcmp(file->d_name, "..") != 0)
		{
                	char file_path[256];
                	strcpy(file_path, file->d_name);
        		chdir(dirname);
                	FILE *infile = fopen(file_path,"r");
                	
			if(infile && is_text_file(file->d_name))
                	{
                	        char *c = (char*)malloc(sizeof(char));
                	        while(fread(c,sizeof(char),1,infile))
                	        {
                	        	content[file_size++] = *c;
					sum += (int) *c;
                	        }
                	        free(c);
			
				chdir("..");
				chdir(newdirname);
				char newfile[40];
				strcpy(newfile, "reversed_");
				strcat(newfile, file->d_name);
				FILE *outfile = fopen(newfile,"w");
				printf("\nWriting into %s...\n",newfile);
			
				for(int i = file_size-1;i >= 0;i--)
				{
					fwrite((void*)&content[i],sizeof(char),1,outfile);
				}
			
				printf("\nSum of the bytes in the file %s is %d\n", newfile, sum);
                	}
                	else
                	{
                	        printf("\nProblem with opening the file or the file is a directory: %s\n", file->d_name);
                	}
			chdir("..");
        	}
	}
        chdir("..");
	free(newdirname);		
	return 0;
}

int main(int argc, char *argv[])
{
	char* dirname;
	dirname = (char*) malloc(sizeof(char) * PATH_SIZE);
	
	if(argc < 2)
	{
		printf("Relative path to the directory name: ");
		scanf("%s",dirname);
	}
	else
	{
		strcpy(dirname,argv[1]);
		// Makes the program secure, but then it limits the usage to 
		// directories that have less than 29 characters in the name 
		dirname[PATH_SIZE - 1] = '\0'; 
	}
	
	printf("Reversing the directory .txt files %s\n", dirname);
	if(reverse(dirname))
	{
		printf("\nAn error has occured while reversing the content of %s\n", dirname);
	}
	else
	{
		printf("\nThe files in %s have been reversed!\n", dirname);
	}
	
	return 0;
}
