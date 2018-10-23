#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#define MAX_BUF 1024

extern char ** get_args();
int status_exit = 0;
char **arg_tree[MAX_BUF];      
int command_iterations = 0;      
char *file_input [MAX_BUF];
char *file_output [MAX_BUF];
char *sp_char[MAX_BUF];
int print_error[MAX_BUF];
char **fd2_args[MAX_BUF];


void read_arguments(char **arguments){
  arg_tree[0] = arguments;
  for (int i = 0 ; arguments[i] != NULL; i++){
    // printf ("argument :%s",arguments[i]);
    char cc = arguments[i][0];
    if (cc == '>'){
      if (arguments[i][1]=='>'){
        sp_char[command_iterations] = ">>";
        if (arguments[i][2] == '&'){
	  print_error[command_iterations]=1;  
	}
      }
      else {
	sp_char[command_iterations] = ">";
        if (arguments[i][1] == '&'){
	  print_error[command_iterations]=1;  
	}
      }
      arguments[i] = NULL;
      file_output[command_iterations] = arguments[i+1];
    }
    if (cc == '<'){
      sp_char[command_iterations]="<";    
      arguments[i] = NULL;
      file_input[command_iterations] = arguments[i+1];
    }
    if (cc == '|'){
      if (arguments[i][1] == '&'){
        print_error[command_iterations]=1;  
      }
      arguments[i] = NULL;
      sp_char[command_iterations]="|";    
      fd2_args[command_iterations] = &arguments[i+1];
    }
    if (cc== ';'){
     arguments[i] = NULL;
     command_iterations++; 
     arg_tree[command_iterations] = &arguments[i+1];
    }
  }
}


void exec_pipe(i){
  int old_stdin = dup(0);
  int old_stdout = dup(1);
  int old_stderr = dup(2);
  int fd[2];
  pipe (fd);
  int p_id = fork();
  if (p_id==-1){
    perror("pid = -1");
  }
  if (p_id == 0){
    if (sp_char[i] != NULL){
      if (strcmp (sp_char[i],"|")==0){
          close(fd[0]); 
          dup2(fd[1],0);
          execvp(fd2_args[i][0],fd2_args[i]);
          close(fd[1]);
          fflush(stdin);
          dup2(old_stdin,0);
          close(old_stdin);
          if (print_error[i] == 1) {
            fflush(stderr);
            dup2(old_stderr,2);
            close(old_stderr);
          }
      }
    }
  }
  else {
    if (sp_char[i] != NULL){
      if (strcmp (sp_char[i],"|")==0){
        close(fd[1]); 
        dup2(fd[0],1);
        if (print_error[i] == 1) {
          dup2(fd[0],2);
        }
        execvp(arg_tree[i][0],arg_tree[i]);
        close(fd[0]);
        fflush(stdout);
        dup2(old_stdout,1);
        close(old_stdout);
      }
    }
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
      if (strcmp (sp_char[i],">")==0 || strcmp (sp_char[i],">>")==0){
	int file_o=0;
	int old_stderr = 0;
	if (strcmp (sp_char[i],">")==0)
          file_o = open (file_output[i],O_WRONLY|O_CREAT|O_TRUNC,0644);
	else if (strcmp (sp_char[i],">>")==0)
          file_o = open (file_output[i],O_WRONLY|O_CREAT|O_APPEND,0644);
        int old_stdout = dup(1);
        dup2(file_o,1);
	if (print_error[i] == 1){
          old_stderr = dup (2);
	  dup2(file_o,2);
	}
        execvp(arg_tree[i][0],arg_tree[i]);
        fflush(stdout);
        dup2(old_stdout,1);
        close(old_stdout);
        close(file_o);
	if (print_error[i] == 1){
          fflush(stderr);
          dup2(old_stderr,2);
	  close(old_stderr);
	}
      }  
      else if (strcmp (sp_char[i],"<")==0){
        int file_i = open(file_input[i],O_RDONLY,0644);
        int old_stdin = dup(0);
        dup2(file_i,0);
        execvp(arg_tree[i][0],arg_tree[i]);
        fflush(stdin);
        dup2(old_stdin,0);
        close(old_stdin);
        close(file_i);
      }
      else if (strcmp (sp_char[i],"|")==0){
        exec_pipe(i);
          while (1){
            if(wait(NULL) == - 1 )
              break;
          }
	  exit(0);
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
  memset(fd2_args,0,sizeof(fd2_args));
  memset(sp_char,0,sizeof(sp_char));
  memset(print_error,0,sizeof(print_error));
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
        if (args[0] == NULL) {
            printf ("No arguments on line!\n");
        } else if ( !strcmp (args[0], "exit")) {
            printf ("Exiting...\n");
            exit(status_exit);
        }
	read_arguments(args);
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
