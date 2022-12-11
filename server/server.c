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
#define SLEEPTIME 100000
#define MAX_CLNT 2 // 최대 동시 접속 가능 수
#define SERV_IP "192.168.219.107"
#define QNUM 3 //문제 갯수

void *handle_clnt(void *arg); 
void msgsend(int ns, char* buf);
void showrule(int ns);

/********* Critical Section***************/

int clnt_cnt = 0; //연결된 클라이언트 수
int clnt_socks[MAX_CLNT]; // 클라이언트 배열
pthread_t t_ids[MAX_CLNT];//쓰레드 아이디 배열
int winner = 0; //승자의 소켓번호가 저장된다.
int jg_inuse = 0; //채점기 프로세스를 사용하고 있는지 체크하는 변수
pthread_mutex_t mutx; //mutex 선언

/********* Critical Section***************/

char qpath[BUFSIZE]; //문제파일의 경로 (후에 랜덤하게 지정)
char apath[BUFSIZE]; //답지 경로 (문제가 정해지면 해당 번호의 답지도 지정)


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
	
    //무한 반복 서버 (종료는 ctrl + c로)
    while (1) {

	//서버에 2명이 연결되면 추가적으로 연결을 받지 않는다.
	while (clnt_cnt < 2) { 

	    if ((ns = accept(sd, (struct sockaddr *)&cli, &clientlen)) == -1) {
		perror("accept");
		exit(1);
	    }
	    //클라이언트의 연결을 accept하면 쓰레드를 할당하고 
	    //임계영역에 있는 클라이언트 수를 담는 변수 등을 수정한다.
	    pthread_mutex_lock(&mutx); //mutex lock
	    clnt_socks[clnt_cnt] = ns; //클라이언트 배열에 연결된 클라이언트 배정
	    pthread_create(&t_id, NULL, handle_clnt,(void *)&ns); //클라이언트에게 쓰레드 할당
	    t_ids[clnt_cnt++] = t_id; //쓰레드 id도 배열에 지정 (혹 쓰일까봐..)
	    pthread_mutex_unlock(&mutx); //mutex unlock
	}
    
	//문제 고르기
	//srand로 랜덤하게
       srand(time(0));
       qnum = (rand() % QNUM) +1;
       //랜덤 문제 번호가 지정되면 해당 문제파일의 경로와 답지 파일의 경로 변수에 설정!
       sprintf(qpath,"./judge/questions/%d.txt",qnum); 
       sprintf(apath,"./judge/answers/%d.sol",qnum);
       

	//게임끝날 때 까지 기다린다. (클라이언트를 담담하는 쓰레드가 게임을 진행중임)
	while(winner == 0);
	//winner가 0이 아닌 값으로 설정되어 busy wait을 탈출했다면
	//승자가 정해졌다.

	//클라이언트배열에서 누가 승자인지 체크하고 승패를 각 클라이언트에게 알린다
	if (clnt_socks[0] == winner) {
	    msgsend(winner, "You win!!");
	    msgsend(clnt_socks[1], "You lose..");
	}
	else {
	    msgsend(clnt_socks[0], "You lose..");
	    msgsend(clnt_socks[1], "You win!!");
	}


	//battle done 쓰레드 소멸 유도하고 sock닫고 다시 2명의 클라이언트를 기다리는 부분으로 루프한다.
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
    int ns = *((int *)arg); //인자로 넘어온 디스크립터 int형태로 다시 캐스팅
    pid_t pid;

    //쓰레드가 실행되었다는 것은 클라이언트가 연결되어 쓰레드가 할당되었다는 뜻
    msgsend(ns, "서버와 연결되었습니다.\n상대방의 입장을 기다리는 중입니다..");
    while (clnt_cnt < 2); //서버에 두명이 연결될 때 까지 기다린다. busywait
    msgsend(ns,"상대방이 입장했습니다");
    sleep(1);

    msgsend(ns, "서버가 문제를 설정 중입니다...");
    sleep(1);
    msgsend(ns, "문제가 설정 되었습니다.");
    sleep(2);
    showrule(ns); // 클라이언트에게 겨루기의 규칙과 파일 제출법등을 알린다.
    msgsend(ns,"10초 후에 알고리즘 배틀을 시작합니다.");
    sleep(10);

    //문제 전송
    //서버가 랜덤으로 고른 문제의 경로는 qpath에 들어 있음
    if ((fp = fopen(qpath, "r")) == NULL) {
	perror("question fopen");
	exit(1);
    }
    //qpath열고 클라이언트에게 한줄씩 보낸다.
    while(fgets(buf, BUFSIZE, fp) != NULL) {
	
	usleep(SLEEPTIME);
	msgsend(ns,buf);
	
    }
    msgsend(ns,"s를 입력하여 파일을 제출하세요!"); 
    fclose(fp);

    //승자가 나올 때 까지 유저의 제출을 기다리며 제출했다면 채점 시작
    while(winner==0) {

	FILE *ufp; //유저가 제출한 파일을 가리킬 파일 포인터
	//유저의 제출은 usr.c로 저장되며 실행파일은 usr가 된다. 

	//유저의 제출을 기다린다. recv한게 있다면 다음 행으로 넘어가게 된다!
	recv(ns,buf,sizeof(buf), 0);

	//여기로 넘어왔다는 것은 유저가 제출을 시작했다는 뜻
	if (winner != 0 ) {
	    continue;
	}
	//심판 프로세스가 사용중인지 확인
	//사용 가능하다면 심판프로세스를 사용하겠다고 알리고 사용한다. 
	while(jg_inuse == 1); 
	if (jg_inuse = 0) {
	    pthread_mutex_lock(&mutx); //lock 걸고
	    jg_inuse = 1; //심판 사용한다라고 알리고
	    pthread_mutex_unlock(&mutx); //lock 푼다
	}

	//유저가 전송한 파일을 옮겨닮을 파일 "usr.c"
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
	ret = system("gcc -o usr usr.c"); //usr제출물을 컴파일 한다.

	//컴파일 실패
	if (ret != 0) {
	    strcpy(buf, "컴파일 에러 발생\ns를 입력하여 파일을 제출하세요!");
	    msgsend(ns, buf);
	    pthread_mutex_lock(&mutx);
	    jg_inuse--;
	    pthread_mutex_unlock(&mutx);
	    remove("usr.c");
	    remove("usr");
	}

	//컴파일 성공
	//컴파일이 성공했으니 이제 유저의 프로그램이 정답 프로그램인지 채점을 해야 한다.
	//judgeproc이라는 프로그램을 실행시킬 것임 인자로 유저의 프로그램과 답지 파일을 전달해 
	//유저 프로그램이 정답인지 judgeproc에게 물어본다.
	else {

	    char* argv[4];
	    //judgeproc을 실행시키기 위해 fork()
	    /* 여기서 잠깐!! 찾아본 결과 멀티 쓰레드 환경에서 fork는 조심해야 한다고 알리는 여러 문헌을 확인할 수 있었는데
	       우선 해당 프로그램은 fork된 프로그램의 쓰레드 구조를 필요로 하지 않으며
	       judgeproc에게 채점만을 부탁하기 위해 fork한 것이기에 문제가 없음을 알린다 */ 
	    switch(pid = fork()) {
		case -1:
		    perror("fork");
		    exit(1);
		    break;
		//judgeproc을 exec하며 유저의 실행파일인 usr와 답지인 apath를 인자로 전달
		case 0: 
		    argv[0] = "./judge/judgeproc";
		    argv[1] = "usr";
		    argv[2] = apath;
		    argv[3] = (char *)NULL;
		    execv("./judge/judgeproc",argv);
		    break;

		default:
		    wait(&status); //judgeproc은 정답으로 판정되면 1을, 오답으로 판정되면 -1을 반환
		    //status를 통해 유저 제출물이 오답인지 아닌지 판명가능하다.
		    status = status >> 8;
		    if (status == 1) {
			msgsend(ns,"정답입니다!");
			pthread_mutex_lock(&mutx);
			winner = ns;
			jg_inuse = 0;
			pthread_mutex_unlock(&mutx);
			remove("usr.c");
			remove("usr");

		    }
		    else {
			msgsend(ns,"오답입니다...\ns를 입력하여 파일을 제출하세요!");
			remove("usr.c");
			remove("usr");
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
		usleep(SLEEPTIME);
	}
}
