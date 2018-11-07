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
int sz_args[MAX_BUF];
char *sp_char[MAX_BUF];
int print_error[MAX_BUF];
char **fd2_args[MAX_BUF];
char *cwd;

// read arguments and parse any special arguments (<,>>,>,&,|,;)
// define arguments (and possible flags) from eachother 
// order the arguments in sequential order
void read_arguments(char **arguments){
  arg_tree[0] = arguments;
  for (int i = 0 ; arguments[i] != NULL; i++){
    sz_args[command_iterations]++;
    char cc = arguments[i][0];
    if (cc == '>'){
      if (sz_args[command_iterations] >= 2){
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
      }
      arguments[i] = NULL;
      file_output[command_iterations] = arguments[i+1];
    }
    if (cc == '<'){
      if (sz_args[command_iterations] >= 2){
        sp_char[command_iterations]="<";    
        arguments[i] = NULL;
        file_input[command_iterations] = arguments[i+1];
      }
    }
    if (cc == '|'){
      if (sz_args[command_iterations] >= 2){
        if (arguments[i][1] == '&'){
          print_error[command_iterations]=1;  
        }
        arguments[i] = NULL;
        sp_char[command_iterations]="|";    
        fd2_args[command_iterations] = &arguments[i+1];
      }
    }
    if (cc== ';'){
     arguments[i] = NULL;
     command_iterations++; 
     arg_tree[command_iterations] = &arguments[i+1];
    }
  }
}

// create a new process and execute two commands
// wait for child process to complete before moving on
void exec_pipe(int i){
  int old_stdin = dup(0);
  int old_stdout = dup(1);
  int old_stderr = dup(2);
  if (old_stdin == -1 || old_stdout == -1 || old_stderr == -1)
    perror("");
  int fd[2];
  pipe (fd);
  int p_id = fork();
  if (p_id==-1){
    perror("pid = -1");
  }
  if (p_id == 0){
    if (sp_char[i] != NULL){
      if (strcmp (sp_char[i],"|")==0){
          int c1 = close(fd[0]); 
	  if (c1 == -1){perror(""); exit(1); return;}
          int d1 =dup2(fd[1],0);
	  if (d1 == -1){perror(""); exit(1); return;}
          int exece = execvp(fd2_args[i][0],fd2_args[i]);
	  wait(NULL);
	  if (exece == -1 ){
            fprintf(stderr,"could not find: %s !",fd2_args[i][0]);
	    perror("");
	    exit(1);
          }
          close(fd[1]);
          fflush(stdin);
          int d2 = dup2(old_stdin,0);
	  if (d2 == -1){perror(""); exit(1); return;}
          int c2 = close(old_stdin);
	  if (c2 == -1){perror(""); exit(1); return;}
          if (print_error[i] == 1) {
            fflush(stderr);

            int d3 = dup2(old_stderr,2);
	    if (d3 == -1){perror(""); exit(1); return;}

            int c3 =close(old_stderr);
	    if (c3 == -1){perror(""); exit(1); return;}
          }
      }
    }
    while (1){
      wait(NULL);
    }
  }
  else {
    if (sp_char[i] != NULL){
      if (strcmp (sp_char[i],"|")==0){
        int c1 = close(fd[1]); 
	if (c1 == -1){perror(""); exit(1); return;}
        int d4 = dup2(fd[0],1);
	if (d4 == -1){perror(""); exit(1); return;}
        if (print_error[i] == 1) {
          int d5 = dup2(fd[0],2);
	  if (d5 == -1){perror(""); exit(1); return;}
        }
        int exece = execvp(arg_tree[i][0],arg_tree[i]);
	wait(NULL);
	if (exece == -1 ){
          fprintf(stderr,"could not find: %s !",arg_tree[i][0]);
	  perror("");
	  exit(1);
        }
        int c2 = close(fd[0]);
	if (c2 == -1){perror(""); exit(1); return;}
        fflush(stdout);
        int d6 = dup2(old_stdout,1);
	if (d6 == -1){perror(""); exit(1); return;}
        int c3 = close(old_stdout);
	if (c3 == -1){perror(""); exit(1); return;}
      }
    }
    while (1){
      wait(NULL);
    }
  }
}

// execute a single command (with/without flags),
// i = order executed (1 = first command, 2= second command,etc)
// if a pipeline is detected, call function exec_pipe
void arg_exec(int i){
  int p_id = fork();
  if (p_id==-1){
    perror("pid = -1");
  }
  if (p_id == 0){
    if (sp_char[i] != NULL){
      if (strcmp (sp_char[i],">")==0 || strcmp (sp_char[i],">>")==0){
	int file_o=0;
	int old_stderr = 0;
	if (strcmp (sp_char[i],">")==0)
          file_o = open (file_output[i],O_WRONLY|O_CREAT|O_TRUNC,0644);
	else if (strcmp (sp_char[i],">>")==0)
          file_o = open (file_output[i],O_WRONLY|O_CREAT|O_APPEND,0644);
	if (file_o == -1){perror(""); exit(1); return;}
        int old_stdout = dup(1);
        int d1 =dup2(file_o,1);
	if (d1 == -1){perror(""); exit(1); return;}
	if (print_error[i] == 1){
          old_stderr = dup (2);
	  if (old_stderr == -1){perror(""); exit(1); return;}
	  int d2= dup2(file_o,2);
	  if (d2 == -1){perror(""); exit(1); return;}
        }
        int exece = execvp(arg_tree[i][0],arg_tree[i]);
	if (exece == -1 ){
          fprintf(stderr,"could not find: %s !",arg_tree[i][0]);
	  perror("");
	  return;
        }
        fflush(stdout);
        int d3 = dup2(old_stdout,1);
	if (d3 == -1){perror(""); exit(1); return;}
        int c1 = close(old_stdout);
	if (c1 == -1){perror(""); exit(1); return;}
        int c2 = close(file_o);
	if (c2 == -1){perror(""); exit(1); return;}
	if (print_error[i] == 1){
          fflush(stderr);
          dup2(old_stderr,2);
	  close(old_stderr);
	}
      }  
      else if (strcmp (sp_char[i],"<")==0){
        int file_i = open(file_input[i],O_RDONLY,0644);
	if (file_i == -1){perror(""); exit(1); return;}
        int old_stdin = dup(0);
        int d1 = dup2(file_i,0);
	if (d1 == -1){perror(""); exit(1); return;}
        int exece = execvp(arg_tree[i][0],arg_tree[i]);
	wait(NULL);
	if (exece == -1 ){
          fprintf(stderr,"could not find: %s !",arg_tree[i][0]);
	  perror("");
	  return;
        }
        fflush(stdin);
        int d2 = dup2(old_stdin,0);
	if (d2 == -1){perror(""); exit(1); return;}
        int c1 = close(old_stdin);
	if (c1 == -1){perror(""); exit(1); return;}
        int c2 = close(file_i);
	if (c2 == -1){perror(""); exit(1); return;}
      }
      else if (strcmp (sp_char[i],"|")==0){
        exec_pipe(i);
	exit(status_exit);
      }
   }
    else {
      int exece = execvp(arg_tree[i][0],arg_tree[i]);
      wait(NULL);
      if (exece == -1 ){
        fprintf(stderr,"could not find: %s !",arg_tree[i][0]);
        perror("");
        return;
      }
   }
   waitpid (-1,NULL,0);
  } 
}


void clean_buffers(){
  memset(arg_tree,0,sizeof(arg_tree));
  memset(file_input,0,sizeof(file_input));
  memset(file_output,0,sizeof(file_output));
  memset(fd2_args,0,sizeof(fd2_args));
  memset(sp_char,0,sizeof(sp_char));
  memset(print_error,0,sizeof(print_error));
  memset(sz_args,0,sizeof(sz_args));
}

int
main()
{
    cwd = calloc (MAX_BUF,sizeof(char));
    if (!cwd){
      perror ("MEMORY FAILURE,EXITING\n");
      status_exit = 1;
      exit(status_exit);
    }
    getcwd (cwd,MAX_BUF);
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
          // Original implementaiton of cd 
          if (strcmp (arg_tree[command_iterations][0],"cd")==0){
	    printf ("%d",sz_args[command_iterations]);
            if(sz_args[command_iterations] == 1){
              int change_1 = chdir(cwd);
	      if (change_1 == -1 ){
	        perror("\n");
	      }
            }
            if(sz_args[command_iterations] == 2){
              int change_2 = chdir(arg_tree[command_iterations][1]);
	      if (change_2 == -1 ){
                fprintf(stderr,"cd: %s does not exist!",arg_tree[command_iterations][1]);
	        perror("\n");
	      }
            }
	    if (sz_args[command_iterations] > 2){
              fprintf(stderr,"cd: too many arguments!");
	    }
          }
	  arg_exec(i);
          while (1){
            if(wait(NULL) == - 1 )
              break;
          }
        }
    }
    free(cwd);
}
