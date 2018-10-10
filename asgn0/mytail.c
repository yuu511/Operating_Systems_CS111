#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#define MAX_CHAR_BUFFER 2048
#define TAIL_SIZE 10

void tail_mode(int num_args,char **arg_names){
  for (int i=1 ; i<num_args ; i++){
    int fileopen = open(arg_names[i],O_RDONLY);
    if (fileopen < 0){
      perror ("error opening file"); 
    }
    char tl[MAX_CHAR_BUFFER];
    memset(tl,0,sizeof(tl));
    read (fileopen,tl,sizeof(tl)); 
    int reversed_index = 0;
    int current_number_lines = 0;
    int num_reversed = 0;
    for (int j = sizeof(tl)-1; j >= 0 ; j--){ 
      if (tl[j] == '\0')
        continue;
      if (tl[j] == '\n' || current_number_lines == 0)
        current_number_lines +=1;
      if(current_number_lines > TAIL_SIZE){
        reversed_index = j+1;   
        break;
      }
      num_reversed +=1;
    }
    int fileclose = close(fileopen);
    if (fileclose < 0){
      perror ("error closing file"); 
    }
    if (num_args < 3){
      write (1,tl+reversed_index,num_reversed);
      write (1,'\0',1);
    }
    else {
      write (1,"==> ",4);
      write (1,arg_names[i],strlen(arg_names[i]));
      write (1," <==",4); 
      write (1,"\n",1);
      write (1,tl+reversed_index,num_reversed);
      write (1,'\0',1);
      if (num_args != i+1){
        write (1,"\n",1);
      }
    }
  }
}

void echo_mode(){
    while(1){
      char data[MAX_CHAR_BUFFER];
      read (0,data,sizeof(data));
    }
}

int main(int argc, char **argv){
  if (argc > 1 )
    tail_mode(argc,argv);
  else
    echo_mode(); 
  return 0;
}
