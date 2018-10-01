#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#define MAX_CHAR_BUFFER 128
#define TAIL_SIZE 10

void tail_mode(int num_args,char **arg_names){
  for (int i=1 ; i<num_args ; i++){
    int file = open(arg_names[i],O_RDONLY);
    char tl[MAX_CHAR_BUFFER]={0};
    char tl_reversed[MAX_CHAR_BUFFER]={0};
    int current_number_lines = 0;

    read (file,tl,sizeof(tl)); 
    for (int j = sizeof(tl) - 1; j >= 0 ; j--){ 
      if(current_number_lines > TAIL_SIZE)
        break;
      if (tl[j] == '\n')
        current_number_lines +=1;
      tl_reversed[j] = tl[j];     
    }
    write (1,tl_reversed,sizeof(tl_reversed));
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
