#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/stat.h>
#include "fs.h"

// ==
//  USAGE: mkdisk [NUM_BLOCKS] [MODE]  
//  1. NUM_BLOCKS: number of blocks (integer)
//  2. MODE: (0) = 512B blocks (1) = 8KB blocks
// ==

char fs_name[]= "NEWFS";
time_t c_time;
int main(int argc, char* argv[]){
  int fd,num_blocks,mode,size_blocks,pointers_per_block;
  int zero = 0;
  int k = 0;
  // Formatting error checking
  if (argc != 3) {
    fprintf(stderr,"usage:mk_disk [NUM_BLOCKS] [MODE]\n");
    return 1;
  }
  char *wordptr;
  num_blocks = strtol(argv[1],&wordptr,10);
  if (strlen(wordptr) > 0 || num_blocks < 1){
    fprintf(stderr,"Invalid block number: please enter a number >0");
    return 1;
  }
  mode = strtol(argv[2],&wordptr,10);
  if (strlen(wordptr) > 0 || (mode != 0 && mode != 1 )){
    fprintf(stderr,"Incorrect mode number: \n Format:[0] or [1]\n");
    return 1;
  }

  // ==
  // Calculate the amount of allocated blocks.
  // ==
  if (mode == 0 ){size_blocks = 512;}
  else {size_blocks = 8192;}
  pointers_per_block = size_blocks / BLOCK_PTR_SIZE;
  // Calculate k FAT table blocks, round up if the number of blocks is not pefectly divisible by number of pointers
  k = num_blocks / pointers_per_block;
  if (num_blocks % pointers_per_block > 0)
    k +=1;
    
  fprintf(stdout,"Maximum amount of pointers:%d\n",pointers_per_block);
  fprintf(stdout,"Number of fat table Blocks:%d\n",k);
  fprintf(stdout,"Size of Blocks:%d\n",size_blocks);
  fprintf(stdout,"Size of data blocks:%d\n",(size_blocks * num_blocks ));
  fprintf(stdout,"k: %d\n",k);

  // ==
  // Initialize data blocks.
  // ==
  fd = open(fs_name, O_CREAT|O_RDWR|O_TRUNC,S_IRWXU);
  if (fd < 0 ){perror("unable to open file\n"); return 1; }
  lseek(fd,0,SEEK_SET);
  fprintf(stdout,"Creating your data blocks...\n:");
  for (int i=0; i< (size_blocks * num_blocks ) ;i++){
    if (write(fd,&zero,1)==-1) {perror("write error:init data blocks");}
  }
  fprintf(stdout,"finished!\n");

  // ==
  // Create Superblock
  // Word 0: Magic Number 
  // Word 1: Total number of blocks
  // Word 2: Number of blocks in FAT
  // Word 3: Block Size
  // Word 4: Starting Block
  // == 
  fprintf(stdout,"writing superblocks..\n");
  lseek(fd,SB_OFFSET * size_blocks ,SEEK_SET);
  // Insert Words in order above.
  // magic number
  int MAGIC_NUMBER = 0xfa19283e;
  if (write(fd,&MAGIC_NUMBER,W_SIZE)==-1) {perror("MAGIC_NUMBER SUPERBLOCK");}
  // total number blocks
  if (write(fd,&num_blocks,W_SIZE)==-1) {perror("total number blocks SUPERBLOCK");}
  // number of blocks in allocation table (k) 
  if (write(fd,&k,W_SIZE)==-1) {perror("number FAT blocks SUPERBLOCK");}
  // Block Size
  if (write(fd,&size_blocks,W_SIZE)==-1) {perror("block size SUPERBLOCK");}
  // Starting block for root
  int temp = k+1;
  if (write(fd,&temp,W_SIZE)==-1) {perror("root block SUPERBLOCK");}

  // sanity checks
  fprintf(stdout,"Sanity checks\n");
  int c;
  lseek(fd,0,SEEK_SET);
  read(fd,&c,4);
  fprintf(stdout,"magic num: %u\n",c);

  lseek(fd,4,SEEK_SET);
  read(fd,&c,4);
  fprintf(stdout,"total num blocks %d\n",c);

  lseek(fd,8,SEEK_SET);
  read(fd,&c,4);
  fprintf(stdout,"total blocks fat %d\n",c);


  lseek(fd,12,SEEK_SET);
  read(fd,&c,4);
  fprintf(stdout,"block sz %d\n",c);

  
  lseek(fd,16,SEEK_SET);
  read(fd,&c,4);
  fprintf(stdout,"start block for root: %d\n",c);
  fprintf(stdout,"finished superblocks!..\n");

  // ===
  // Mark non-usable / non-data block k+1 FAT entries
  // ==
  
  // Mark the FAT entries pointing to the superblock, the FAT itself as -1
  lseek (fd,(SB_OFFSET+1)*size_blocks,SEEK_SET);
  fprintf(stdout,"sb_offset+1*sz : %u\n",(SB_OFFSET+1)*size_blocks);
  int invalid = -1;
   for (int i=0; i < k+1; i++){
     if (write(fd,&invalid,W_SIZE)==-1) {perror("mark unused k+1");}
   }
   //  skim the usable blocks 
   int unusable_blocks = (k * pointers_per_block) - num_blocks;
   for (int i=0; i < (k*pointers_per_block) - unusable_blocks - (k+1);i++){
       if (write(fd,&zero,W_SIZE)==-1) {perror("mark unused k+1");}
   }
   // Mark the unusable block pointers as -1
   fprintf(stdout,"unusable_blocks %d\n",unusable_blocks);
   if (unusable_blocks > 0 ){
     for (int i = 0 ; i < unusable_blocks; i++ ){
       if (write(fd,&invalid,W_SIZE)==-1) {perror("mark unused k+1");}
     }
   }
  // sanity checks
  int bt;
  int zr=0;
  int negative=0;
  for (int i = (size_blocks*1); i < ((k+1)*pointers_per_block*BLOCK_PTR_SIZE) ; i+=4){
    lseek(fd,i,SEEK_SET);
    read(fd,&bt,W_SIZE);
    if (bt == 0) zr++;
    else negative++;
    fprintf(stdout,"%d : %d\n",i,bt);
  }
  fprintf (stdout,"available: %d\n",zr);
  fprintf (stdout,"unavailable: %d\n",negative);
  return 0;
}
