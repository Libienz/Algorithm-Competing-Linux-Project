#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <fcntl.h>



#define PORTNUM 9000
#define BUFSIZE 4096
#define MAX_CLNT 2 // 최대 동시 접속 가능 수
#define SERV_IP "192.168.219.107"
#define QNUM 3 //문제 갯수

void *handle_clnt(void *arg); 
void myfileprint(char* path);
void msgsend(int ns, char* buf);


/********* Critical Section***************/

int clnt_cnt = 0; //연결된 클라이언트 수
int clnt_socks[MAX_CLNT]; // 클라이언트 배열
pthread_t t_ids[MAX_CLNT];//쓰레드 아이디 배열
int winner = 0;
int jg_inuse = 0;
pthread_mutex_t mutx; //mutex 선언

/********* Critical Section***************/

char qpath[BUFSIZE]; //랜덤으로 출제된 문제파일의 경로
char apath[BUFSIZE]; //답지 경로
int main() {
    
    int qnum;
    char buf[BUFSIZE];
    struct sockaddr_in sin, cli;
    pthread_t t_id;
    void *thread_return;
    int sd, ns, clientlen = sizeof(cli);

     
    //Socket 생성
    //AF_INET: IPv4 인터넷 프로토콜 사용
    //SOCK_STREAM: TCP 사용
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	perror("socket");
	exit(1);
    }

    //서버 IP 주소 지정과 포트 번호 설정
    //서버로 돌릴 컴퓨터 지정하고 IP 수정해야 함
    memset((char *)&sin, '\0', sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORTNUM); //HBO -> NBO
    sin.sin_addr.s_addr = inet_addr(SERV_IP); //str IP addr -> bin

    //bind: 소켓 파일 기술자를 지정된 IPaddr/Port와 결합
    if (bind(sd, (struct sockaddr *) &sin, sizeof(sin))) {
	perror("bind");
	exit(1);
    }
    //listen: 5로 지정했다고 5명만 할 수 있는 것은 아님
    //큐의 크기일 뿐 운영체제의 여유가 되면 알아서 accept
    if (listen(sd, 5)) {
	perror("listen");
	exit(1);
    }
	
    while (1) {

	while (clnt_cnt < 2) { 


	    if ((ns = accept(sd, (struct sockaddr *)&cli, &clientlen)) == -1) {
		perror("accept");
		exit(1);
	    }
	
	    pthread_mutex_lock(&mutx); //mutex lock
	    clnt_socks[clnt_cnt] = ns;
	    pthread_create(&t_id, NULL, handle_clnt,(void *)&ns);
	    t_ids[clnt_cnt++] = t_id;
	    pthread_mutex_unlock(&mutx); //mutex unlock
    
	}
    
	//문제 고르기
       srand(time(0));
       qnum = (rand() % QNUM) +1;
       sprintf(qpath,"./judge/questions/%d.txt",qnum);
       sprintf(apath,"./judge/answers/%d.sol",qnum);
       


	//게임끝날 때 까지 기달 
	while(winner == 0);


	if (clnt_socks[0] == winner) {
	    msgsend(winner, "You win..");
	    msgsend(clnt_socks[1], "You lose..");
	}
	else {
	    msgsend(clnt_socks[0], "You lose..");
	    msgsend(clnt_socks[1], "You win..");
	}


	//battle done 
	pthread_join(t_ids[0], &thread_return);
	pthread_join(t_ids[1], &thread_return);

	close(clnt_socks[--clnt_cnt]);
	close(clnt_socks[--clnt_cnt]);
	winner = 0;
    }

    close(sd);

}
void *handle_clnt(void *arg){

    char buf[BUFSIZE];
    int str_len,status;
    FILE* fp;
    size_t fsize;
    int ns = *((int *)arg);
    pid_t pid;
    msgsend(ns, "서버와 연결되었습니다\n상대방의 입장을 기다리는 중입니다..");
    while (clnt_cnt < 2); //busy wating
    msgsend(ns,"상대방이 입장했습니다");
    sleep(1);
    msgsend(ns, "서버가 random 문제를 설정 중입니다...");
    sleep(1);
    msgsend(ns, "문제가 설정 되었습니다.");
    sleep(1);
    msgsend(ns,"10초 후에 알고리즘 배틀을 시작합니다");
    sleep(10);
    //문제 전송
    if ((fp = fopen(qpath, "r")) == NULL) {
	perror("question fopen");
	exit(1);
    }
    

    while(fgets(buf, BUFSIZE, fp) != NULL) {
	
	sleep(1);
	msgsend(ns,buf);
	
  }
    fclose(fp);

    //승자가 나올 때 까지 유저의 제출을 기다리며 제출했다면 채점 시작
    while(winner==0) {

	FILE *ufp;
	remove("usr.c");
	remove("usr");
	//유저의 제출을 기다린다.
	//여기..?
	recv(ns,buf,sizeof(buf), 0);
	if (winner != 0 ) {
	    continue;
	}
	//유저가 제출을 시작했다면 심판프로세스에 락을 걸고 사용한다. 
	while(jg_inuse == 1); 
	if (jg_inuse = 0) {
	    pthread_mutex_lock(&mutx);
	    jg_inuse = 1;
	    pthread_mutex_unlock(&mutx);
	}
	if ((ufp = fopen("usr.c", "w")) == NULL) {
	    perror("ufp fopen");
	    exit(1);
	}

	//유저 제출물 소켓에서 읽어와서 usr.c에 저장
	str_len = read(ns,buf,BUFSIZE-1);
	buf[str_len] = '\0';
	if(winner != 0) {
	    continue;
	}
	fputs(buf,ufp);
	fclose(ufp);
	
	//유저가 제출한 소스코드는 이제 usr.c에 보관되어 있다.

	//채점
	int ret; 
	ret = system("gcc -o usr usr.c");
	printf("ret : %d\n",ret);
	//컴파일 실패
	if (ret != 0) {
	    strcpy(buf, "컴파일 에러임 ㅋ");
	    msgsend(ns, buf);
	    pthread_mutex_lock(&mutx);
	    jg_inuse--;
	    pthread_mutex_unlock(&mutx);
	    remove("usr.c");
	}
	//컴파일 성공
	else {
	    char* argv[4];

	    switch(pid = fork()) {
		case -1:
		    perror("fork");
		    exit(1);
		    break;
		case 0: 
		    argv[0] = "./judge/judgeproc";
		    argv[1] = "usr";
		    argv[2] = apath;
		    argv[3] = (char *)NULL;
		    execv("./judge/judgeproc",argv);
		    break;

		default:
		    wait(&status);
		    status = status >> 8;
		    if (status == 1) {
			msgsend(ns,"정답");
			pthread_mutex_lock(&mutx);
			winner = ns;
			jg_inuse = 0;
			pthread_mutex_unlock(&mutx);

		    }
		    else {
			msgsend(ns,"오답");
			jg_inuse = 0;
		    }
		    break;
	    }
	}

    }

}
    
void msgsend(int ns, char* buf) {

    if (send(ns, buf, strlen(buf) + 1, 0) == -1) {
	perror("send");
	exit(1);
    }
}



void myfileprint(char* path) {
    
    FILE* fp;
    char buf[BUFSIZE];
    int n;

    fp = fopen(path, "r");

    while(fgets(buf, BUFSIZE,fp) != NULL) {
	printf("%s",buf);
    }
    printf("\n");

    fclose(fp);
}
