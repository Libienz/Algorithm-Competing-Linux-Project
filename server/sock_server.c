#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define PORTNUM 9000

//서버는 statemachine 으로 ..? 
int main() {
    char buf[256];
    struct sockaddr_in sin, cli;
    int sd, ns, clientlen = sizeof(cli);

    //Socket 생성
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	perror("socket");
	exit(1);
    }

    //서버 IP 주소 지정과 포트 번호 설정
    //서버로 돌릴 컴퓨터 지정하고 IP 수정해야 함
    memset((char *)&sin, '\0', sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORTNUM);
    sin.sin_addr.s_addr = inet_addr("192.168.147.129");

    //bind함수로 소켓 이름 지정
    //listen 함수를 호출해 접속 요청을 받을 준비를 마쳤음을 알림
    if (bind(sd, (struct sockaddr *) &sin, sizeof(sin))) {
	perror("bind");
	exit(1);
    }

    if (listen(sd, 5)) {
	perror("listen");
	exit(1);
    }

    if ((ns = accept(sd, (struct sockaddr *)&cli, &clinetlen)) == -1) {
	perror("accept");
	exit(1);
    }

    sprintf(buf, "Your IP address is %s", inet_nota(cli.sin_addr));
    
    //메세지 전송 
    if (send(ns, buf, strlen(buf) + 1, 0) == -1) {
	perror("send");
	exit(1);
    }
    close(ns);
    close(sd);

}


     
