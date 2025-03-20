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
        char* newdirname = (char*)malloc(sizeof(char) * (PATH_SIZE + NAME_SIZE));
        strcpy(newdirname, dirname);
	strcat(newdirname, "_reversed");
	mkdir(newdirname,0777);
        struct dirent *file;
	int file_size = 0;
	char content[1000000];
	int sum = 0;
	do
        {
		file_size = 0;
                file = readdir(mydir);
                if(file == NULL)
                {
                        break;
                }
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
			char newdir[30];
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
			printf("\nSize of the file %s %d\n", newfile, sum);
                }
                else
                {
                        printf("Problem with opening the file or the file is a directory: %s\n", file->d_name);
                }
		chdir("..");
        	}
	}
        while(1);
        chdir("..");
	free(newdirname);		
	free(dirname);
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
	}
	printf("%s",dirname);
	reverse(dirname);
	return 0;
}
