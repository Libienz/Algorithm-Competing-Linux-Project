#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define PORTNUM 9000
#define MAX_CLNT 10

/********* Critical Section *********/
int clnt_socks[MAX_CLNT]; //CLNT마다 할당될 sock파일 fd 배열
int clnt_count = 0; //현재 접속한 클라이언트 수
/********* Critical Section *********/



//서버는 statemachine 으로 ..? 
//fork는 어디에 ..?
int main() {
    char buf[256];
    struct sockaddr_in sin, cli;
    int sd, ns, clientlen = sizeof(cli);
    pid_t pid;

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
    sin.sin_addr.s_addr = inet_addr("192.168.147.129"); //str IP addr -> bin

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
	
	//연결 요청 수락 
	if ((ns = accept(sd, (struct sockaddr *)&cli, &clinetlen)) == -1) {
	    perror("accept");
	    exit(1);
	}
	
	if ((pid = fork()) < 0) { //fork failed
	    perror("fork");
	    exit(1);
	}

	if(pid == 0) { //child process





	//전역변수 사용 위해 mutual exclusive 
    }

    sprintf(buf, "Welcome Algorithm competing");
    
    //메세지 전송 
    if (send(ns, buf, strlen(buf) + 1, 0) == -1) {
	perror("send");
	exit(1);
    }
    close(ns);
    close(sd);

}


     
