#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#define BUFSIZE 257


/*
   getusrans -> get user answer
   입력 사례 한개에 대해서 유저의 정답을 output.txt라는 파일에 담는다.
	1. fork를 통해서 자식 프로세스가 인자로 받은 유저 제출물을 실행
	2. 입출력 redirection을 통해서 유저 제출 프로그램은 inject.txt를 통해 입력을 받게 됨
	3. 후에 정답인지 판별하게 될 유저의 출력은 output.txt에 담기게 된다.
   즉 getusrans 함수가 실행되는 시점에는 inject.txt가 사례 하나로 채워져 있어야 함
*/
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
	//fork error
	case -1 :
	    perror("fork");
	    exit(1);
	    break;
	//child proc: 입출력 redirection하고 제출물을 실행시켜 본다.
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
	//parent proc: 부모는 자식이 실행을 마칠 때 까지 기다림
	default:
	    waitpid(pid, &status, 0);
	    break;
	    
    }
}

/*
   main()에 대한 설명
   argv[1]: execfn (유저가 제출한 실행파일명)
   argv[2]: solfn (서버가 보관하고 있는 답안 파일명)
   유저 제출물을 1번 답지와 비교해서 채점하고 싶다면 요청은 다음과 같다
   ./judgeproc usrprog ./answers/1.sol
*/

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

    //유저 프로그램이 5가지 입력 예시에 대해서 모범답안을 출력하는지 확인해야 함
    //답지를 5번 반복해서 읽어와서 각 입력예시마다 정답인지 확인한다.
    while ((fgets(buf,BUFSIZE,fp)) != NULL) {
	
	if ((ifp = fopen("./temp/inject.txt","w")) == NULL) {
	    perror("fopen");
	    exit(1);
	}

	//userProg의 input으로 주입할 inject.txt 준비
	strcpy(model_input, strtok(buf,":"));
	strcat(model_input, "\n");
	printf("사례 입력: %s\n", model_input);
	fputs(model_input, ifp);
	fclose(ifp);
	
	//모범 답안 준비. 
	//후에 유저 답안과 비교할 예정
	strcpy(model_output, strtok(NULL,":"));

	//유저의 답안을 output.txt에 담는다.
	getusrans(argv[1]);


	if ((ofp = fopen("./temp/output.txt","r")) == NULL) {
	    perror("fopen");
	    exit(1);
	}

	//output.txt에서 usr 프로그램이 출력한 string 가져온다.
	fgets(usr_answer,BUFSIZE,ofp);
	printf("usr 정답: %s\n", usr_answer);
	printf("모범 정답: %s\n", model_output);
	
	//usr 정답과 모범 답안을 비교 다르면 오답!
	if(strcmp(usr_answer, model_output)!=0) { 
	    printf("오답\n");
	    return -1;
	}

	fclose(ofp);
    }
    //위의 반복문동안 모두 정답처리되어서 반복문을 탈출했다면 정답
    fclose(fp);
    printf("정답\n");
    return 0;

}
