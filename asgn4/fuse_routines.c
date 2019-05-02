// ==
// fuse_routines.c : mounts the filesystem and defines routines that can be performed on the filesystem
// ==

#define FUSE_USE_VERSION 26

#include <errno.h>
#include <fuse.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include "fs.h"

#define F32_NAME "F32"

int fd,magic_number,total_blocks,k,block_size,starting_block;
int root_block_size = 1;
time_t c_time;
//int32_t *fat_table;
void initroot();
int fd_open;

int32_t read_fat_entry(int block) {
	int32_t addr;
	lseek(fd,block_size+(W_SIZE*(block)),SEEK_SET);
	read(fd,&addr,W_SIZE);
	return addr;
}

int new_block(){//returns block index for next free block in FAT
	int32_t addr;
	int index = starting_block;
	lseek(fd,block_size+index*W_SIZE,SEEK_SET);
	read(fd,&addr,W_SIZE);
	while(addr != 0) {
		read(fd,&addr,W_SIZE);
		index++;
	}
	return index;
}

int32_t path_to_addr(const char *path) {//returns addr of path or 0 if not found
	char *sub_path;
	char *path_copy;
	char dir_entry[FILENAME_SIZE];
	int s_block = starting_block;
	int32_t s_addr = starting_block*block_size;
//  fprintf(stdout,"\n%s\n",path);
	path_copy = strdup(path);
	sub_path = strtok(path_copy,"/");
	if(sub_path == NULL)
		return s_addr;
	while(1) {
		lseek(fd,s_addr,SEEK_SET);
		read(fd,&dir_entry,FILENAME_SIZE);
//        fprintf(stdout,"\n#######%s########\n",dir_entry);
		while(strcmp(sub_path,dir_entry)!=0){//loop thru dir entries
			s_addr = s_addr + DIR_SIZE;
			lseek(fd,s_addr,SEEK_SET);
			read(fd,&dir_entry,FILENAME_SIZE);
//        fprintf(stdout,"#########%s##########\n",dir_entry);
			if(strcmp(dir_entry,"\0") == 0)
				return 0;
			if(s_addr >= (s_block+1)*block_size){//if at end of block
				s_addr = read_fat_entry(s_block);
				s_block = s_addr/block_size;
				if (s_block < starting_block)
					return 0;
				lseek(fd,s_addr,SEEK_SET);
				read(fd,&dir_entry,FILENAME_SIZE);
			}
		}
		//when sub_path is found
		lseek(fd,7*W_SIZE,SEEK_CUR);//seek to starting block entry
		read(fd,&s_block,W_SIZE);//read starting block
		sub_path = strtok(NULL,"/");//get next path entry
		if(sub_path == NULL)
			return s_addr;
		s_addr = s_block*block_size;//set s_addr to start of block
	}	
}

static int f32_getattr(const char *path, struct stat *stbuf){
  memset(stbuf,0,sizeof(struct stat));
  int idx,response,current_block,exists,next; 
  int32_t seeki;
  char entry_name[FILENAME_SIZE];
  int flen,sblock,flag,null;
  time_t ctime,mtime,atime;
  char *cpy;
  char *ppath;
  cpy = strdup(path);
  if (strcmp(path,"/")==0){
    stbuf -> st_mode = S_IFDIR | 0755;
    stbuf -> st_nlink = 2;
    return 0;
  }
  seeki = path_to_addr(path);
  if (seeki == 0 ) { return -ENOENT; }
  response = 0;
  lseek (fd,seeki,SEEK_SET);
  read(fd,&entry_name,FILENAME_SIZE);
  read (fd,&ctime,2*W_SIZE);  
  read (fd,&mtime,2*W_SIZE);  
  atime = time(NULL);
  // alter acess time in place of read (fd,&atime,2*W_SIZE);  
  if (write(fd,&atime,2*W_SIZE)==-1) {perror("root name");}
  read (fd,&flen,W_SIZE);  
  read (fd,&sblock,W_SIZE);  
  read (fd,&flag,W_SIZE);  
  read (fd,&null,W_SIZE);  
  if (flag == FLAG_F){
    stbuf -> st_mode  = S_IFREG | 0444; 
    stbuf -> st_nlink = 1;
    stbuf -> st_size = flen;
  }
  else if(flag == FLAG_D){
    stbuf -> st_mode  = S_IFDIR | 0777; 
    stbuf -> st_nlink = 1;
    stbuf -> st_size = flen;
  }
  return response;
}

static int f32_read (const char *path ,char *buf, size_t size, off_t offset,
struct fuse_file_info *fi){
//  fprintf (stdout,"\n******F32 READ******\n");
  (void) fi;
  int seeki,dirnum,idx,current_block,exists,next;
  char *cpy;
  char *ppath;
  void *stuff;
  int flen,sblock,flag,null;
  time_t ctime,mtime,atime;
  char entry_name[FILENAME_SIZE];
  cpy = strdup(path);
  ppath = strtok (cpy,"/");
  dirnum = 0;
  if (strcmp(ppath,"/")==0) {return -ENOENT;}
  seeki = path_to_addr(path);
  if (seeki == 0 ) { return -ENOENT; }
  lseek (fd,seeki,SEEK_SET);
  read (fd,&entry_name,FILENAME_SIZE);
  read (fd,&ctime,2*W_SIZE);  
  read (fd,&mtime,2*W_SIZE);  
  atime = time(NULL);
  // alter acess time in place of read (fd,&atime,2*W_SIZE);  
  if (write(fd,&atime,2*W_SIZE)==-1) {perror("root name");}
  read (fd,&flen,W_SIZE);  
  read (fd,&sblock,W_SIZE);  
  read (fd,&flag,W_SIZE);  
  read (fd,&null,W_SIZE);  
  size_t fsz=flen;
  int num_blocks,cnt;
  num_blocks = flen / block_size; // number of blocks that a piece of data occupies
  if (flen % block_size > 0) 
	  num_blocks++;
  current_block = sblock; 
  char databuffer[num_blocks * block_size];
  next = 0;
  cnt = 0;
//  fprintf (stdout,"\n******READ_WHILE 2******\n");
  next = read_fat_entry(current_block);
//  fprintf (stdout,"SBLOCK  %d\n NEXT: %d\n",sblock,next);
  while (1){
    lseek(fd,current_block * block_size , SEEK_SET); 
    read(fd,&databuffer + (cnt * block_size),block_size);
    next = read_fat_entry(current_block);
    if (next == -2) { break; }
    current_block = next;
    cnt++;
  }
//  fprintf (stdout,"DATABUFFER %s: \n ",databuffer);
//  fprintf (stdout,"fsz %zu: \n ",fsz);
//  fprintf (stdout,"offset %zu: \n ",offset);
//  fprintf (stdout,"size %zu: \n ",size);
  if (offset < fsz){
    fprintf (stdout,"invoked");
    if (offset + size > fsz ){
      size = fsz-offset;
      fprintf (stdout,"invoked 2");
    }
    memcpy(buf, databuffer+offset,fsz);
  fprintf (stdout,"buf %s: \n ",buf);
  } else
      return 0;
  return size;
}

static int f32_write (const char *path, const char *buf, size_t size, off_t offset,
  struct fuse_file_info *fi){
  int res;
  int flen,sblock,flag,null;
  time_t ctime,mtime,atime;
  char entry_name[FILENAME_SIZE];
//  fprintf (stdout,"\n******WRITE******\n");
  int bufl = fi -> fh;
//  fprintf(stdout,"starting location: %d\n",bufl);
  lseek(fd,bufl, SEEK_SET); 
  read (fd,&entry_name,FILENAME_SIZE);  
  ctime = time(NULL);
  mtime = time(NULL);
  atime = time(NULL);
  // alter created / modified /acess/ time in place of read (fd,&ntime,2*W_SIZE);  
  if (write(fd,&ctime,2*W_SIZE)==-1) {perror("root name");}
  if (write(fd,&mtime,2*W_SIZE)==-1) {perror("root name");}
  if (write(fd,&atime,2*W_SIZE)==-1) {perror("root name");}
  read (fd,&flen,W_SIZE);  
  read (fd,&sblock,W_SIZE);  
  read (fd,&flag,W_SIZE);  
  if (flag == FLAG_D) {fprintf(stdout,"Cannot Write to a directory!");}
  read (fd,&null,W_SIZE);  
//  fprintf (stdout,"entry name %s",entry_name);
//  fprintf (stdout,"flen %d",flen);
//  fprintf (stdout,"\n******size: %zu*****\n",size);
//  fprintf (stdout,"\n******thing: %s*****\n",buf);
//  fprintf (stdout,"\n******sblock: %d*****\n",sblock);
//  fprintf (stdout,"\n******offset: %zu*****\n",offset);
//  fprintf (stdout,"\n******offset: %zu*****\n",offset);
  char *thing = strdup(buf);
  lseek(fd,(sblock*block_size)+offset, SEEK_SET); 
  res = write (fd,buf,size);
  lseek(fd,bufl+FILENAME_SIZE+(6*W_SIZE), SEEK_SET); 
  flen = flen + size; 
//  fprintf(stdout,"thisthing %d\n\n",flen);
  if (write(fd,&flen,W_SIZE)==-1) {perror("f32write 252");}
  lseek(fd,bufl, SEEK_SET); 
  read (fd,&entry_name,FILENAME_SIZE);  
  read (fd,&ctime,2*W_SIZE);  
  read (fd,&mtime,2*W_SIZE);  
  read (fd,&atime,2*W_SIZE);  
  read (fd,&flen,W_SIZE);  
  read (fd,&sblock,W_SIZE);  
  read (fd,&flag,W_SIZE);  
  read (fd,&null,W_SIZE);  
//  fprintf (stdout,"entry name %s",entry_name);
//  fprintf (stdout,"flen %d",flen);
  return res;
}

static int f32_open (const char *path, struct fuse_file_info *fi){
//  fprintf (stdout,"\n******OPEN******\n");
  (void) fi;
  fi -> fh = -1;
  int seeki,dirnum,idx,current_block,exists,next,location;
  char *cpy;
  char *ppath;
  void *stuff;
  int flen,sblock,flag,null;
  time_t ctime,mtime,atime;
  char entry_name[FILENAME_SIZE];
  cpy = strdup(path);
  ppath = strtok (cpy,"/");
  dirnum = 0;
  idx = 0;
  current_block = starting_block;
//  fprintf (stdout,"\n******OPEN_WHILE 1******\n");
  while (1){
    exists = 0;
    lseek (fd,(current_block * block_size)+(idx*DIR_SIZE),SEEK_SET);
    read(fd,&entry_name,FILENAME_SIZE);
    if (strcmp(entry_name,"\0")==0){break;}
    if (strcmp(ppath,entry_name)==0){ 
      exists = 1;
      read (fd,&ctime,2*W_SIZE);  
      read (fd,&mtime,2*W_SIZE);  
      atime = time(NULL);
      // alter acess time in place of read (fd,&atime,2*W_SIZE);  
      if (write(fd,&atime,2*W_SIZE)==-1) {perror("root name");}
      read (fd,&flen,W_SIZE);  
      read (fd,&sblock,W_SIZE);  
      read (fd,&flag,W_SIZE);  
      if (flag == FLAG_D) {fprintf(stdout,"Cannot call open() to a directory!");}
      read (fd,&null,W_SIZE);  
      ppath = strtok(NULL,"/");//check for a subdirectory in path
      if (ppath != NULL){
        current_block = sblock;
        continue;
      }
      location = ((current_block * block_size)+(idx*DIR_SIZE));
      break;
    }
    idx++;
    if (idx >= block_size/DIR_SIZE && read_fat_entry(current_block)!=-2) {
      idx = 0;
      next = read_fat_entry(current_block);
      current_block = next;
    } 
  }
//  fprintf(stdout,"LOCATION : %d \n ",location); 
  if (exists==1){
    fi->fh=location;
  } 
  else{
      char newpath[FILENAME_SIZE];
      strcpy (newpath,ppath);
      int fatidx;
      for (fatidx=0;read_fat_entry(fatidx)!=-2;fatidx++);
//      fprintf (stdout,"*************FATIDX %d",fatidx);
  }
  return 0;
}

static int f32_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
  off_t offset, struct fuse_file_info *fi) {//adapted from fusexmp.c
//  fprintf (stdout,"\n******READING FOLDER AT PATH %s******\n" , path);
  (void) offset;
  (void) fi;
  char entry_name[FILENAME_SIZE];
  char *sub_path;
  char endof[FILENAME_SIZE];
  int current_block;
  int next;
  int32_t seeki;
  filler(buf,".",NULL,0);
  //filler(buf,"..",NULL,0);
  if (strcmp(path,"/") == 0 ){
	current_block = starting_block;
  	seeki = starting_block*block_size+DIR_SIZE;//seeki to root block
					//(skipping over header for root dir)
  } else {
	seeki = path_to_addr(path);//address of path directory entry
	if (seeki == 0) {return -ENOENT;}//path could not be found
	seeki = seeki + 13*W_SIZE; //seek to starting block entry
	lseek(fd,seeki,SEEK_SET);
	read(fd,&current_block,W_SIZE);//read starting block
	seeki = current_block*block_size;//set seeki to start of block
  }
  while (1){
    lseek(fd,seeki,SEEK_SET);
    read (fd,&entry_name,FILENAME_SIZE);
//    fprintf (stdout,"\n***ADDING %s TO BUFFER***\n" , entry_name);
    if (strcmp(entry_name,"\0")==0){break;}
    fprintf(stdout,"\n\n*** ADDDING %s to BUFFER ********\n\n",entry_name);
    filler(buf, entry_name, NULL, 0);
    seeki = seeki + DIR_SIZE;
    if (seeki >= (current_block+1)*block_size) {
      seeki = read_fat_entry(current_block);
      if (seeki <= 0) {perror("block does not have continuing entry in FAT");}
      current_block = seeki/block_size;
    }
  }
  return 0;
}

static int f32_mkdir (const char* path, mode_t mode){ //which path is given
//  fprintf (stdout,"\n******CREATING FOLDER AT PATH %s******\n", path);
  int parent_blocknum;
  char *short_path;
  char *newdir_name;
  int32_t seeki;
  char endof[FILENAME_SIZE];
  char entry_name[FILENAME_SIZE];
  int w;
  // remove newdir_name from the path so that the call to path_to_addr
  //   doesn't try to enter it
  short_path = strdup(path);
  newdir_name = strdup(strrchr(short_path,'/') + 1);
  // short_path is path to parent directory
  short_path[strlen(short_path)-strlen(newdir_name)] = '\0';
      //fprintf (stdout,"\n***short_path:%s***\n",short_path);
      //fprintf (stdout,"\n***newdir_name:%s***\n",newdir_name);
      //int cmpval = strcmp(short_path, "/");
      //fprintf (stdout,"%d\n",cmpval);
  if (strcmp(short_path,"/") == 0){
      // fprintf(stdout,"in root%d\n",cmpval);
    parent_blocknum = starting_block;
    seeki = starting_block*block_size; // seeks to address of root
  } else {
    seeki = path_to_addr(short_path);
    if (seeki == 0) {return -ENOENT;} // the directory is not found
    parent_blocknum = seeki/block_size;
  }
  int newblock;
  newblock = new_block();
  while (1){
//  fprintf(stdout,"\n  SEEK:%d  \n",seeki);
    lseek(fd,seeki,SEEK_SET);
    read (fd,&entry_name,FILENAME_SIZE);
    //fprintf (stdout,"\n***ADDING %s TO BUFFER***\n" , entry_name);
    if (strcmp(entry_name,"\0")==0){break;}
    //filler(buf, entry_name, NULL, 0);
    seeki = seeki + DIR_SIZE;
    if (seeki >= (parent_blocknum+1)*block_size) {
      seeki = read_fat_entry(parent_blocknum);
      if (seeki <= 0) {perror("block does not have continuing entry in FAT");}
      parent_blocknum = seeki/block_size;
    }
  }
  strcpy(entry_name,newdir_name);
  lseek(fd,seeki,SEEK_SET);
//  fprintf(stdout,"\n  SEEK:%d  \n",seeki);
//  fprintf(stdout,"\n %s \n",entry_name);
  //  0-5     file name = newdir_name
  if(write(fd,&entry_name,FILENAME_SIZE)==-1) {perror("mkdir file name error");}
  c_time = time(NULL);
  //  6-7     creation_time = current_time
  //  8-9     modification_time = current_time
  //  10-11   access_time = current_time
  if (write(fd,&c_time,2*W_SIZE)==-1) {perror("mkdir file name error");}
  if (write(fd,&c_time,2*W_SIZE)==-1) {perror("mkdir file name error");}
  if (write(fd,&c_time,2*W_SIZE)==-1) {perror("mkdir file name error");}
  //  12      file length in bytes
  w = DIR_SIZE;
  if (write(fd,&w,W_SIZE)==-1) {perror("name");}
  //  13      start block
  if (write(fd,&newblock,W_SIZE) ==-1) {perror("name");}
  //  14      flags
  w = FLAG_D;
  if (write(fd,&w,W_SIZE) ==-1) {perror("name");}
  //allocate a new block
  lseek(fd,W_SIZE,SEEK_CUR);
  if (write(fd,"\0",FILENAME_SIZE)==-1) {perror(".. name");}
  return 0;
}

static struct fuse_operations f32_oper = {
	.getattr	= f32_getattr,
	.mkdir		= f32_mkdir,
//	.unlink		= f32_unlink,//remove a file
//	.rmdir		= f32_rmdir,
//	.truncate	= f32_truncate,//change the size of a file
	.open		= f32_open,
	.read		= f32_read,
	.write		= f32_write,
//	.statfs		= f32_statfs,//needed?
//	.flush		= f32_flush,//needed? used for writeback on close()
//	.release	= f32_release,
//	.opendir	= f32_opendir,
	.readdir	= f32_readdir,
//	.releasedir	= f32_releasedir,//needed?
//	.create		= f32_create,
//	.utimens	= f32_utimens, //change access and mod times, see man utimensat
//	.write_buf	= f32_write_buf,
//	.read_buf	= f32_read_buf,
//	.fallocate	= f32_fallocate
};

int main(int argc, char *argv[]){
  fd = open ("NEWFS", O_RDWR,S_IRWXU);
  if (fd < 0 ) {perror("unable to open file system \n");}
  lseek (fd,0,SEEK_SET);
  // load information from the superblock
  // read magic num
  read (fd,&magic_number,W_SIZE); 
//  fprintf (stdout,"magic_number %u\n" , magic_number);
  // total blocks
  read (fd,&total_blocks,W_SIZE);
//  fprintf (stdout,"totalblocks %d\n" , total_blocks);
  // number of blocks for fat
  read (fd,&k,W_SIZE);
//  fprintf (stdout,"numblocks %d\n" , k);
  // block size
  read (fd,&block_size,W_SIZE);
//  fprintf (stdout,"blk size %d\n" , block_size);
  // starting block of root
  read (fd,&starting_block,W_SIZE);
//  fprintf (stdout,"root %d\n" , starting_block); 

  initroot();
  // Load FAT table into memory
//  fat_table= (int*) calloc((((k)*block_size)),sizeof(int));
//  int idx = 0;
//  for (int i=block_size*(SB_OFFSET+1) ; i< ((k+1)*block_size) ; i+=4 ){
//	  lseek(fd,i,SEEK_SET);
//	  read(fd,&fat_table[idx],W_SIZE);
//	  fprintf(stdout,"%d : %d\n",idx,fat_table[idx]);
//	  idx++;
//  }
//  // close(fd);
  return fuse_main(argc, argv, &f32_oper, NULL);
}

void initroot(){
  if (fd < 0 ) {perror("unable to open file system \n");}
  // make directory named a.....
  lseek(fd, (k+1)* block_size,SEEK_SET);
  char root_name[] = "aaaaaaAAAAAAAAaaaaaaaa\0";
  // name of random directory
  if (write(fd,root_name,FILENAME_SIZE)==-1) {perror("root name");}
  c_time = time(NULL);
  // creation mod and access time
  if (write(fd,&c_time,2*W_SIZE)==-1) {perror("root name");}
  if (write(fd,&c_time,2*W_SIZE)==-1) {perror("root name");}
  if (write(fd,&c_time,2*W_SIZE)==-1) {perror("root name");}
  // file length
  int w;
  w= DIR_SIZE;
  if (write(fd,&w,W_SIZE)==-1) {perror("root name");}
  // start block of directory
  w = k+1;
  if (write(fd,&w,W_SIZE)==-1) {perror("root name");}
  // FLAG
  w = FLAG_D;
  if (write(fd,&w,W_SIZE)==-1) {perror("root name");}

  char thing[] = "helllllllllllooooo";
  // hello object (data located at k+2)
  lseek(fd, (k+2)* block_size,SEEK_SET);
  if (write(fd,thing,block_size)==-1) {perror("root name");}

  // make hello file header
  lseek(fd, (k+1)*block_size+(1*DIR_SIZE),SEEK_SET);
  // file name
  if (write(fd,"hello\0",FILENAME_SIZE)==-1) {perror(".. name");}
  // creation mod and access time
  c_time = time(NULL);
  if (write(fd,&c_time,2*W_SIZE)==-1) {perror(".. name");}
  if (write(fd,&c_time,2*W_SIZE)==-1) {perror(".. name");}
  if (write(fd,&c_time,2*W_SIZE)==-1) {perror(".. name");}
  // file length
  w= strlen(thing);
  if (write(fd,&w,W_SIZE)==-1) {perror("root name");}
  // start block of thing
  w = k+2;
  if (write(fd,&w,W_SIZE)==-1) {perror("root name");}
  // FLAG
  w = FLAG_F;
  if (write(fd,&w,W_SIZE)==-1) {perror("root name");}

  // make empty_sub_dir directory entry
  lseek(fd, (k+1)*block_size+(2*DIR_SIZE),SEEK_SET);
  // file name
  if (write(fd,"empty_sub_dir\0",FILENAME_SIZE)==-1) {perror(".. name");}
  // creation mod and access time
  c_time = time(NULL);
  if (write(fd,&c_time,2*W_SIZE)==-1) {perror(".. name");}
  if (write(fd,&c_time,2*W_SIZE)==-1) {perror(".. name");}
  if (write(fd,&c_time,2*W_SIZE)==-1) {perror(".. name");}
  // file length
  w= DIR_SIZE;
  if (write(fd,&w,W_SIZE)==-1) {perror("root name");}
  // start block of thing
  w = 0;
  if (write(fd,&w,W_SIZE)==-1) {perror("root name");}
  // FLAG
  w = FLAG_D;
  if (write(fd,&w,W_SIZE)==-1) {perror("root name");}

  // make sub_dir directory entry
  lseek(fd, (k+1)*block_size+(3*DIR_SIZE),SEEK_SET);
  // file name
  if (write(fd,"sub_dir\0",FILENAME_SIZE)==-1) {perror(".. name");}
  // creation mod and access time
  c_time = time(NULL);
  if (write(fd,&c_time,2*W_SIZE)==-1) {perror(".. name");}
  if (write(fd,&c_time,2*W_SIZE)==-1) {perror(".. name");}
  if (write(fd,&c_time,2*W_SIZE)==-1) {perror(".. name");}
  // file length
  w= DIR_SIZE;
  if (write(fd,&w,W_SIZE)==-1) {perror("root name");}
  // start block of thing
  w = k+3;
  if (write(fd,&w,W_SIZE)==-1) {perror("root name");}
  // FLAG
  w = FLAG_D;
  if (write(fd,&w,W_SIZE)==-1) {perror("root name");}

  //insert a zeroed name field to indicate end of directory entries
  lseek(fd, (k+1)*block_size+(4*DIR_SIZE),SEEK_SET);
  if (write(fd,"\0",FILENAME_SIZE)==-1) {perror(".. name");}

  // make .. directory (root) 
  lseek(fd, (k+3)*block_size,SEEK_SET);
  // file name
  if (write(fd,"..\0",FILENAME_SIZE)==-1) {perror(".. name");}
  // creation mod and access time
  c_time = time(NULL);
  if (write(fd,&c_time,2*W_SIZE)==-1) {perror(".. name");}
  if (write(fd,&c_time,2*W_SIZE)==-1) {perror(".. name");}
  if (write(fd,&c_time,2*W_SIZE)==-1) {perror(".. name");}
  // file length
  w= DIR_SIZE;
  if (write(fd,&w,W_SIZE)==-1) {perror("root name");}
  // start block of thing
  w = k+1;
  if (write(fd,&w,W_SIZE)==-1) {perror("root name");}
  // FLAG
  w = FLAG_D;
  if (write(fd,&w,W_SIZE)==-1) {perror("root name");}

  // make sub_sub_dir directory 
  lseek(fd, (k+3)*block_size+(1*DIR_SIZE),SEEK_SET);
  // file name
  if (write(fd,"sub_sub_dir\0",FILENAME_SIZE)==-1) {perror(".. name");}
  // creation mod and access time
  c_time = time(NULL);
  if (write(fd,&c_time,2*W_SIZE)==-1) {perror(".. name");}
  if (write(fd,&c_time,2*W_SIZE)==-1) {perror(".. name");}
  if (write(fd,&c_time,2*W_SIZE)==-1) {perror(".. name");}
  // file length
  w= DIR_SIZE;
  if (write(fd,&w,W_SIZE)==-1) {perror("root name");}
  // start block of thing
  w = 0;
  if (write(fd,&w,W_SIZE)==-1) {perror("root name");}
  // FLAG
  w = FLAG_D;
  if (write(fd,&w,W_SIZE)==-1) {perror("root name");}

  //insert a zeroed name field to indicate end of directory entries
  lseek(fd, (k+3)*block_size+(2*DIR_SIZE),SEEK_SET);
  int zero = 0;
  if (write(fd,&zero,FILENAME_SIZE)==-1) {perror(".. name");}

  // making fat block entries
  lseek(fd,block_size+(W_SIZE*(k+1)),SEEK_SET);
  //w = k+2; // this is the hello file data
  w = FLAG_LAST;
  if (write(fd,&w,W_SIZE)==-1) {perror("root name");} //root dir
  if (write(fd,&w,W_SIZE)==-1) {perror("root name");} //hello file data
  if (write(fd,&w,W_SIZE)==-1) {perror("root name");} //sub_dir dir
}
