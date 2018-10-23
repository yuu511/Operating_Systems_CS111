#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

extern char ** get_args();
int status_exit = 0;
char **arg_tree[1024];      
int command_iterations = 0;      
char *file_input [1024];
char *file_output [1024];
char *sp_char[1024];
int print_error[1024];


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
      if (arguments[i][1] == '&'){
        print_error[command_iterations]=1;  
      }
    }
    if (cc == '|'){
      arguments[i] = NULL;
      sp_char[command_iterations]="|";    
      if (arguments[i][1] == '&'){
        print_error[command_iterations]=1;  
      }
    }
    if (cc== ';'){
     arguments[i] = NULL;
     command_iterations++; 
     arg_tree[command_iterations] = &arguments[i+1];
    }
  }
}


void exec_pipe(i){
  int fd[2];
  pipe (fd);
  int p_id = fork();
  if (p_id==-1){
    perror("pid = -1");
  }
  if (p_id == 0){
    if (sp_char[i] != NULL){
      if (strcmp (sp_char[i],"|")==0){
       int old_stdout = dup(1);
       close(fd[0]); 
       dup2(fd[1],1);
       close(fd[1]);
       execvp(arg_tree[i][1],arg_tree[i]);
       fflush(stdout);
       dup2(old_stdout,1);
       close(old_stdout);
      }
    }
  }
  else {
   if (sp_char[i] != NULL){
     if (strcmp (sp_char[i],"|")==0){
         int old_stdin = dup(0);
         close(fd[1]); 
         dup2(fd[0],0);
         close(fd[1]);
         execvp(arg_tree[i][0],arg_tree[i]);
         fflush(stdin);
         dup2(old_stdin,0);
         close(old_stdin);
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
//	int old_sterr = 0;
	if (strcmp (sp_char[i],">")==0)
          file_o = open (file_output[i],O_WRONLY|O_CREAT|O_TRUNC,0644);
	else if (strcmp (sp_char[i],">>")==0)
          file_o = open (file_output[i],O_WRONLY|O_CREAT|O_APPEND,0644);
        int old_stdout = dup(1);
        dup2(file_o,1);
        printf ("%d",print_error[i]);
//	if (print_error[i] == 1){
//          old_sterr = dup (2);
//	  dup2(file_o,2);
//	}
        execvp(arg_tree[i][0],arg_tree[i]);
        fflush(stdout);
        dup2(old_stdout,1);
        close(old_stdout);
        close(file_o);
//	if (print_error[i] == 1){
//          fflush(stderr);
//          dup2(old_sterr,2);
//	  close(old_sterr);
//	}
      }  
      else if (strcmp (sp_char[i],"<")==0){
        printf("%s",file_input[i]);
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
