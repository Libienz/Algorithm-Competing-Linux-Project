#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
int main(){
  int status; 
  pid_t pid;
  char *inputanswer[6];
  char *inputzip[10];
  char *solzip[10];
  char buffer[100];
  char *token;
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
	i=0;
	token = strtok(buffer,":");
	printf("%d %s",idx,token);
	while(token != NULL){
		if(i==0){
			inputzip[idx]=token;
		}
		else if(i==1){
	                solzip[idx]=token;
		}
		i++;
		token = strtok(NULL,":");
	        printf("%d %s",idx,token);

	}
	idx++;  
  }
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
}
