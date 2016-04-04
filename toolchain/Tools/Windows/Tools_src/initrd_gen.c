#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILENAME_LENGTH 64

struct {
	unsigned int header_size;
	unsigned int file_count;
	unsigned int * offset, * length;
	char ** filename;
} __attribute__((packed)) initrd_header;

unsigned int filesize(char * filename) {
	FILE * f = fopen(filename, "rb");
	fseek(f, 0, SEEK_END);
	unsigned int size = (unsigned int)ftell(f);
	fclose(f);
	return size;
}

char * fileread(int buffer_size, char * filename) {
	char * buff = (char*)malloc(buffer_size);
	FILE * f = fopen(filename, "rb");
	fread(buff, sizeof(char), buffer_size, f);
	fclose(f);
	return buff;
}

void create_initrd(char ** argv) {
	printf("- Creating initrd.img ...\n");
	FILE * initrd_file = fopen("iso/initrd.img", "wb");

	/* Write the entire struct into the file: */
	fwrite((unsigned int*)&initrd_header.header_size, sizeof(unsigned int), 2, initrd_file);
	fwrite(initrd_header.offset, sizeof(unsigned int), initrd_header.file_count, initrd_file);
	fwrite(initrd_header.length, sizeof(unsigned int), initrd_header.file_count, initrd_file);

	int i;
	for(i = 0; i < initrd_header.file_count; i++)
		fwrite(initrd_header.filename[i], sizeof(char), FILENAME_LENGTH, initrd_file);

	/* Now append the files: */
	for(i = 0; i < initrd_header.file_count; i++) {
		char * file_buff = fileread(initrd_header.length[i], argv[i+1]);
		fwrite(file_buff, sizeof(char), initrd_header.length[i], initrd_file);
		free(file_buff);
	}

	fseek(initrd_file, 0, SEEK_END);
	printf("- initrd.img size: %d bytes", ftell(initrd_file));

	fclose(initrd_file);
}

int main(char argc, char **argv) {
	if(!(argc-1)) {
		printf("ERROR: You did not provide files!\nUsage: initrd.exe mod1.mod mod2.mod mod3.mod ...");
		getch();
		return 1;
	}
	printf("- Module count: %d\n", argc - 1);

	/* Allocate everything: */
	initrd_header.file_count = argc - 1;
	initrd_header.offset = (unsigned int*)malloc(sizeof(unsigned int) * initrd_header.file_count);
	initrd_header.length = (unsigned int*)malloc(sizeof(unsigned int) * initrd_header.file_count);
	initrd_header.filename = (char**)malloc(sizeof(char*) * initrd_header.file_count);
	
	/* Calculate table size: */
	initrd_header.header_size = 
		sizeof(unsigned int) * 2
		+ (sizeof(unsigned int) * initrd_header.file_count * 2) 
		+ (initrd_header.file_count * FILENAME_LENGTH);

	/* Allocate the rest of the table (with respect to the files): */
	printf("- Header size: %d bytes\n- Adding modules:\n", initrd_header.header_size);
	int i;
	for(i = 0; i < initrd_header.file_count; i++) {
		initrd_header.filename[i] = (char*) malloc(sizeof(unsigned char) * FILENAME_LENGTH);
		initrd_header.length[i] = filesize(argv[i+1]);
		initrd_header.offset[i] = i ? initrd_header.offset[i-1]+initrd_header.length[i-1] : initrd_header.header_size;
		memset(initrd_header.filename[i], 0, FILENAME_LENGTH);
		strcpy(initrd_header.filename[i], strrchr(argv[i+1],'\\')+1);

		printf("  %d > %s (offset: %d size: %d bytes)\n", i+1, initrd_header.filename[i], initrd_header.offset[i], initrd_header.length[i]);
	}
		
	/* Now generate the initrd based on the table that we allocated: */
	create_initrd(argv);

	/* Cleanup everything: */
	free(initrd_header.offset);
	free(initrd_header.length);
	for(i = 0; i < initrd_header.file_count; i++)
		free(initrd_header.filename[i]);
	free(initrd_header.filename);

	printf("\n\n- Done!");
	return 0;
}