#include <stdio.h>
#include <string.h>
#include <unistd.h>

void tail_mode(char **args){
}

void echo_mode(){
    while(1){
      char data [128];
      memset(data,0,sizeof(data));
      read (0,data,sizeof(data));
      write (1,data,sizeof(data));
    }
}

int main(int argc, char **argv){
  if (argc > 1 )
    tail_mode(argv);
  else
    echo_mode(); 
  return 0;
}
