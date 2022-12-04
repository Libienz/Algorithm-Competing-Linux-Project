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
#define MAX_CLNT 2
#define BUFSIZE 1024
void myfileprint(char* path);

int main() {
    char buf[BUFSIZE];
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
    sin.sin_addr.s_addr = inet_addr("192.168.219.107"); //str IP addr -> bin

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
	
	if ((ns = accept(sd, (struct sockaddr *)&cli, &clientlen)) == -1) {
	    perror("accept");
	    exit(1);
	 }
/*        
	sprintf(buf, "server connected");
        if (send(ns, buf, strlen(buf) + 1, 0) == -1) {
	    perror("send");
	    exit(1);
	 }
*/

	//연결 성공을 알리고 settingmsg 출력
	myfileprint("settingmsg.txt");	
    
	
	


    }

    
    //ready 들어올 때 까지 wait 
	//ready 눌렀는데 상대방이 ready가 되지 않았다면 상대방의 ready 기다리는 중 이런식으로 출력
    //둘다 ready 되었다면 game_start라는 제어로 넘어감 
    //서버는 while로 signal 받을 때 까지 대기중 
    //누군가 judge를 이용해서 정답을 맞추었다면 깨어남 
    //게임 결과 및 게임 종료한다고 출력


    
    close(ns);
    close(sd);

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
