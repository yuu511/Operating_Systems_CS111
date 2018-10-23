#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>

extern char ** get_args();
int status_exit = 0;
char **arg_tree[1024];      
int command_iterations = 0;      
char *file_input [1024];
char *file_output [1024];
char *sp_char[1024];
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
      if (arguments[i][1]){
        if (arguments[i][1]=='>'){
	  sp_char[command_iterations] = ">>";
	}
      } 
      else {
	sp_char[command_iterations] = ">";
      }
      arguments[i] = NULL;
      file_output[command_iterations] = arguments[i+1];
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
    // printf ("%s",sp_char[i]);
    // printf ("%d",strcmp (sp_char[i],">")==0);
    if (sp_char[i] != NULL){
      printf ("%s",sp_char[i]);
      if (strcmp (sp_char[i],">")==0){
	printf ("1!");
        int file_o = open (file_output[i],O_WRONLY|O_CREAT|O_TRUNC,0644);
        int old_stdout = dup(1);
        dup2(file_o,1);
        printf("%d",file_o);
        execvp(arg_tree[i][0],arg_tree[i]);
        fflush(stdout);
        dup2(old_stdout,1);
        close(old_stdout);
        close(file_o);
      }  
      else if (strcmp (sp_char[i],">>")==0){
	printf ("2!");
        int file_o = open (file_output[i],O_WRONLY|O_CREAT|O_APPEND,0644);
        int old_stdout = dup(1);
        dup2(file_o,1);
        printf("%d",file_o);
        execvp(arg_tree[i][0],arg_tree[i]);
        fflush(stdout);
        dup2(old_stdout,1);
        close(old_stdout);
        close(file_o);
      }
    }
    else {
      execvp(arg_tree[i][0],arg_tree[i]);
    }
  }
}

void clean_buffers(){
  memset(arg_tree,0,sizeof(arg_tree));
  memset(file_input,0,sizeof(file_input));
  memset(file_output,0,sizeof(file_output));
  memset(sp_char,0,sizeof(sp_char));
  memset(pipes,0,sizeof(pipes));
}

int
main()
{
    char **     args;
    while (1) {
	printf ("Command ('exit' to quit): ");
	command_iterations = 0 ;
	clean_buffers();
	args = get_args();
	read_arguments(args);
        // open_pipe();
        for (int i=0; i < command_iterations+1 ; i++){
	  arg_exec(i);
          while (1){
            if(wait(NULL) == - 1 )
              break;
          }
        }
        while (1){
          if(wait(NULL) == - 1 )
            break;
        }
    }
}
