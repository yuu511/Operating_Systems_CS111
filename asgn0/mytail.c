#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#define MAX_CHAR_BUFFER 128
#define TAIL_SIZE 10

void tail_mode(int num_args,char **arg_names){
  for (int i=1 ; i<num_args ; i++){
    int file = open(arg_names[i],O_RDONLY);
    char tl[MAX_CHAR_BUFFER]={0};
    char towrite[TAIL_SIZE][MAX_CHAR_BUFFER]={0};
    int current_number_lines = 0;
    int current_line_char = 0;

    read (file,tl,sizeof(tl)); 
    for (int j = sizeof(tl) - 2; j >= 0 ; j--){ 
      if(current_number_lines > TAIL_SIZE)
        break;
      if (tl[j] == '\n'){
        current_line_char = 0; 
	current_number_lines +=1;
      }
      towrite[TAIL_SIZE - current_number_lines][(MAX_CHAR_BUFFER - 1) - current_line_char] = tl[j];
      current_line_char += 1; 
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
