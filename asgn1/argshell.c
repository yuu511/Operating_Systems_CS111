#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

extern char ** get_args();
int status_exit = 0;
char **arg_tree[1024];      
int command_iterations = 0;      
char *file_input [1024];
char *file_output [1024];
int *pipes [1024]; 


void read_arguments(char **arguments){
  if (arguments[0] == NULL) {
      printf ("No arguments on line!\n");
  } else if ( !strcmp (arguments[0], "exit")) {
      printf ("Exiting...\n");
      exit(status_exit);
  }
  arg_tree[0] = arguments;
  for (int i = 0 ; arguments[i] != NULL; i++){
    // printf ("argument :%s",arguments[i]);
    char cc = arguments[i][0];
    if (cc == '>'){
      arguments[i] = NULL;
      file_output[i] = arguments[i+1];
    }
    if (cc== ';'){
     arguments[i] = NULL;
     command_iterations++; 
     arg_tree[command_iterations] = &arguments[i+1];
    }
  }
}

void open_pipe(){
  for( int i=0 ;i< command_iterations; i++){
    pipes[i] = calloc(2,sizeof(int));
    if (pipe(pipes[i]) == -1 ) perror("failed to create pipe!");
  }
}

void arg_exec(i){
  int p_id = fork();
  if (p_id==-1){
    perror("pid = -1");
  }
  if (p_id == 0){
    execvp(arg_tree[i][0],arg_tree[i]);
    while (1){
      if(wait(NULL) == - 1 )
        break;
    }
  }
}

int
main()
{
    char **     args;
    while (1) {
	printf ("Command ('exit' to quit): ");
	args = get_args();
	read_arguments(args);
        open_pipe();
        for (int i=0; i < command_iterations+1 ; i++){
	  arg_exec(i);
        }
    }
}
