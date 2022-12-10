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
#define SERV_IP "192.168.123.102"
#define QNUM 3 //문제 갯수

void *handle_clnt(void *arg); 
void msgsend(int ns, char* buf);
void showrule(int ns);

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

char showRule[BUFSIZE]="\n========================================\n"
"알고리즘 대결을 위한 규칙을 설명해드리겠습니다.\n"
"1. 참가자는 터미널 창을 하나 더 띄워서 src 편집을 준비하세요.\n"
"2. 새로 띄운 창을 cd 명령을 통해 실행시킨 client 프로그램과 같은 디렉터리로 이동하세요.\n"
"3. 이동한 디렉터리에 작성한 C파일을 저장하세요.\n\n"
"제출방법은 다음과 같습니다!\n"
"1. 문제가 출력되고 s를 입력하여 파일을 제출하세요! 라는 문구가 출력되었을때!\n"
"2. s를 입력한뒤 엔터를 누릅니다.\n"
"3. 제출할 파일이름을 입력해주세요 라는 문구가 출력되면 작성한 C파일을 정확하게 입력해주세요.\n"
"이제 상대방보다 빠르게 문제를 제출해봅시다! good luck!\n"
"========================================\n";

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
	    msgsend(winner, "You win!!");
	    msgsend(clnt_socks[1], "You lose..");
	}
	else {
	    msgsend(clnt_socks[0], "You lose..");
	    msgsend(clnt_socks[1], "You win!!");
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
    msgsend(ns, "서버와 연결되었습니다.\n상대방의 입장을 기다리는 중입니다..");
    while (clnt_cnt < 2); //busy wating
    msgsend(ns,"상대방이 입장했습니다");
    sleep(1);
    msgsend(ns, "서버가 문제를 설정 중입니다...");
    sleep(1);
    msgsend(ns, "문제가 설정 되었습니다.");
    sleep(1);
    msgsend(ns,"10초 후에 알고리즘 배틀을 시작합니다.");
    sleep(1);
    showrule(ns);
    sleep(10);
    //문제 전송
    if ((fp = fopen(qpath, "r")) == NULL) {
	perror("question fopen");
	exit(1);
    }
    

    while(fgets(buf, BUFSIZE, fp) != NULL) {
	
	usleep(500000);
	msgsend(ns,buf);
	
    }
    msgsend(ns,"s를 입력하여 파일을 제출하세요!"); 
    fclose(fp);

    //승자가 나올 때 까지 유저의 제출을 기다리며 제출했다면 채점 시작
    while(winner==0) {

	FILE *ufp;
	remove("usr.c");
	remove("usr");
	//유저의 제출을 기다린다.
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
//	printf("ret : %d\n",ret);
	//컴파일 실패
	if (ret != 0) {
	    strcpy(buf, "컴파일 에러 발생\ns를 입력하여 파일을 제출하세요!");
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
			msgsend(ns,"정답입니다!");
			pthread_mutex_lock(&mutx);
			winner = ns;
			jg_inuse = 0;
			pthread_mutex_unlock(&mutx);

		    }
		    else {
			msgsend(ns,"오답입니다...\ns를 입력하여 파일을 제출하세요!");
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

void showrule(int ns){
	char *showRule[11];
	showRule[0]="\n========================================\n";
	showRule[1]="알고리즘 대결을 위한 규칙을 설명해드리겠습니다.\n";
        showRule[2]="1. 참가자는 터미널 창을 하나 더 띄워서 src 편집을 준비하세요.(해당 터미널에서는 파일 제출을 위해 서버와 통신을 유지하고 있기 때문에 추가적인 터미널이 필요합니다!)\n";
        showRule[3]="2. 새로 띄운 창을 cd 명령을 통해 실행시킨 client 프로그램과 같은 디렉터리로 이동하세요.(src파일의 경로가 해당 위치여야만 제출이 원활하게 이루어집니다!)\n";
        showRule[4]="3. 이동한 위치에서 문제를 마음껏 풀어주세요!\n\n";
        showRule[5]="제출방법은 다음과 같습니다!\n";
        showRule[6]="1. 문제가 출력되고 s를 입력하여 파일을 제출하세요! 라는 문구가 출력되었을때!\n";
        showRule[7]="2. s를 입력한뒤 엔터를 누릅니다.\n";
        showRule[8]="3. 제출할 파일이름을 입력해주세요 라는 문구가 출력되면 작성한 C파일을 정확하게 입력해주세요.\n";
        showRule[9]="이제 상대방보다 빠르게 문제를 제출해봅시다! good luck!\n";
        showRule[10]="========================================\n";
	
	for(int i=0; i<11; i++){
		msgsend(ns,showRule[i]);
		usleep(500000);
	}
}
