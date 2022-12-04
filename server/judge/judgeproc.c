#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#define BUFSIZE 257


int getusrans(char* execfn) {

    
    
    int fd[2];
    int ifd, ofd;
    pid_t pid;
    char buf[BUFSIZE];
    int len, status;

    ifd = open("./temp/inject.txt",O_RDONLY | O_CREAT, 0644);
    ofd = open("./temp/output.txt",O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (ifd == -1) {
	perror("open");
	exit(1);
    }
    if (ofd == -1) {
	perror("open");
	exit(1);
    }


    switch (pid= fork()) {
	case -1 :
	    perror("fork");
	    exit(1);
	    break;
	case 0:

	    if(ifd != 0) {
		dup2(ifd, 0);
		close(ifd);
	    }

	    if (ofd != 1) {
		dup2(ofd,1);
		close(ofd);
	    }
	    //유저 제출물 실행
	    char *argv[1];
	    argv[0] = NULL;
	    if (execv(execfn,argv) == -1) {
		perror("execlp");
		exit(1);
	    }
	    exit(1);
	    break;
	default:
	    waitpid(pid, &status, 0);
	    break;
	    
    }
}

//argv[1]: execfn
//argv[2]: solfn
int main(int argc, char* argv[]) {

    FILE *fp, *ifp, *ofp;
    char buf[BUFSIZE];
    int c;
    char model_input[BUFSIZE];
    char model_output[BUFSIZE];
    char usr_answer[BUFSIZE];
    
    if ((fp = fopen(argv[2],"r")) == NULL) {
	perror("fopen");
	exit(1);
    }

    //우선 한줄만 .. 근데 이거 여러줄로도 할 수 있음
    while ((fgets(buf,BUFSIZE,fp)) != NULL) {
	
	if ((ifp = fopen("./temp/inject.txt","w")) == NULL) {
	    perror("fopen");
	    exit(1);
	}

	//inject.txt 준비
	strcpy(model_input, strtok(buf,":"));
	strcat(model_input, "\n");
	printf("사례 입력: %s\n", model_input);
	fputs(model_input, ifp);
	fclose(ifp);
	
	//모범 답안 준비
	strcpy(model_output, strtok(NULL,":"));

	//get_user_answer
	getusrans(argv[1]);


	if ((ofp = fopen("./temp/output.txt","r")) == NULL) {
	    perror("fopen");
	    exit(1);
	}

	fgets(usr_answer,BUFSIZE,ofp);
	printf("usr 정답: %s\n", usr_answer);
	printf("모범 정답: %s\n", model_output);
	
	if(strcmp(usr_answer, model_output)!=0) { 
	    printf("오답\n");
	    return -1;
	}

	fclose(ofp);
    }
    fclose(fp);
    printf("정답\n");
    return 0;

}
