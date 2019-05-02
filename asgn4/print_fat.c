
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "fs.h"

int main(int argc, char *argv[]){
	int fd,magic_number,total_blocks,k,block_size,starting_block;
	int32_t fat_entry;
	fd = open("NEWFS", O_RDONLY, S_IRWXU);
	if (fd < 0) {perror("unable to open file sys\n");}

	lseek(fd, 0, SEEK_SET);
	// load information from the superblock   
	// read magic num   
	read (fd,&magic_number,W_SIZE);   
	fprintf (stdout,"magic_number %u\n" , magic_number);   
	// total blocks   
	read (fd,&total_blocks,W_SIZE);   
	fprintf (stdout,"totalblocks %d\n" , total_blocks);   
	// number of blocks for fat   
	read (fd,&k,W_SIZE);   
	fprintf (stdout,"numblocks %d\n" , k);   
	// block size   
	read (fd,&block_size,W_SIZE);   
	fprintf (stdout,"blk size %d\n" , block_size);   
	// starting block of root   
	read (fd,&starting_block,W_SIZE);   
	fprintf (stdout,"root dir %d\n" , starting_block); 

	lseek(fd, block_size, SEEK_SET);
	fprintf (stdout,"READING FAT... [index] : [value]\n"); 
	for(int i = 0; i < (k*block_size)/W_SIZE; i = i + W_SIZE) {
		read(fd,&fat_entry,W_SIZE);
		int fat_i = i/W_SIZE;
		fprintf(stdout, "%d : %d\n", fat_i, fat_entry);
	}
	
	
	fprintf (stdout,"printing first block\n"); 
	for(int i = 0; i < (k*block_size)/W_SIZE; i = i + W_SIZE) {
		read(fd,&fat_entry,W_SIZE);
		fprintf(stdout, "%d ", fat_entry);
		if((i/W_SIZE)%16 == 15) {fprintf(stdout, "\n");}
	}
}
