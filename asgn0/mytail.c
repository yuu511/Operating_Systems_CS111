#include <stdio.h>
#include <unistd.h>

void tail_mode(char **args){
}

void echo_mode(){
    char data [128];
    read (0,data,128);
    write (1,data,128);
}

int main(int argc, char **argv){
  if (argc > 1 )
    tail_mode(argv);
  else
    echo_mode(); 
  return 0;
}
