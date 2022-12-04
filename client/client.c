#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define PORTNUM 9000
#define BUFSIZE 256

void myfileprint(char* path);

int main() {
    int sd, len;
    char buf[BUFSIZE];
    struct sockaddr_in sin;

    //서버 IP 주소 지정과 포트 번호 설정
    //서버로 돌릴 컴퓨터 지정하고 IP 수정해야 함

    memset((char *)&sin, '\0', sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORTNUM);//HBO -> NBO
    sin.sin_addr.s_addr = inet_addr("192.168.219.107");//str IP addr -> bin

    //Socket 생성
    //AF_INET: IPv4 인터넷 프로토콜 사용
    //SOCK_STREAM: TCP 사용
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    //connect 함수로 클라이언트가 서버에 접속 요청
    //sd가 가리키는 소켓을 sin 이 가리키는 주소로 연결한다, 마지막인자는 sin의 크기
    printf("==> Create Socket\n");
    if (connect(sd, (struct sockaddr *)&sin, sizeof(sin))) {
        perror("connect");
        exit(1);
    }

    //recv 함수는 소켓 sd를 통해 전송받은 메시지를 sizeof(buf)크기인 버퍼buf에 저장한다.
    //마지막 인자인 0은 flag고 send()함수에서 사용하는 플래그와 같다.
    printf("==> Connect Server\n");
    myfileprint("settingmsg.txt");

    
    if ((len = recv(sd, buf, sizeof(buf), 0)) == -1) {
        perror("recv");
        exit(1);
    }
    if ((len = recv(sd, buf, sizeof(buf), 0)) == -1) {
        perror("recv");
        exit(1);
    }
    if ((len = recv(sd, buf, sizeof(buf), 0)) == -1) {
        perror("recv");
        exit(1);
    }

    buf[len] = '\0';

    printf("==> From Server : %s\n", buf);
    //소켓을 닫는다.
    close(sd);
}

void myfileprint(char * path) {

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

