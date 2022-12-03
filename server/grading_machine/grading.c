#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
int main(int argc, char* argv[]){


    int status; 
  pid_t pid;
  char *inputanswer[6];
  char *inputzip[10];
  char *solzip[10];
  char buffer[100];
  char* comp_input;
  char* comp_output;

  int i;
  int idx;
  int j; 
  FILE* fp;

  if ((fp = fopen("1.sol","r"))==NULL){
	perror("fopen: 1.sol");
	exit(1);
  }

  idx=0;
  while(fgets(buffer,100,fp) != NULL){
	comp_input = strtok(buffer,":");
	comp_output = strtok(NULL,":");
	printf("input: %s\n", comp_input);
	printf("input: %s\n", comp_output);
  }

  /*
  for (j=0; j<idx; j++){
	printf("%s %s \n",inputzip[j],solzip[j]);
  }
  fclose(fp);
  
  switch( pid= fork()){
   case -1:
	perror("fork");
	exit(1);
	break;
    case 0:
	printf("child\n");
	inputanswer[0] ="./test1";	
        inputanswer[1] ="3";
        inputanswer[2] ="asdf";
        inputanswer[3] ="qwer";
        inputanswer[4] ="qwer";
	inputanswer[5] =NULL;
	

	if(execvp("./test1",inputanswer)==-1){
		perror("execlp");


		exit(1);
	}
	exit(0);
	break;

    default :
	while(wait(&status) != pid)
  	  continue;
	printf("parent\n");
	break;


  }
  */

}
