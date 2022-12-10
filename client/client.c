#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>

#define PORTNUM 9000
#define BUFSIZE 4096
#define SERV_IP "192.168.219.107"

void *send_file(void *arg);
void *recv_msg(void *arg);
int gameset = 0;
int main() {
    int sd, len;
    char buf[BUFSIZE];
    struct sockaddr_in sin;
    pthread_t snd_thread, rcv_thread;
    void *thread_return;
    //서버 IP 주소 지정과 포트 번호 설정
    //서버로 돌릴 컴퓨터 지정하고 IP 수정해야 함

    memset((char *)&sin, '\0', sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORTNUM);//HBO -> NBO
    sin.sin_addr.s_addr = inet_addr(SERV_IP);//str IP addr -> bin

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

    //쓰레드 두개 생성 
    //파일 보낼 때 사용할 쓰레드 
    //서버에서 오는 메세지 수신할 쓰레드
    pthread_create(&snd_thread, NULL, send_file, (void *)&sd);
    pthread_create(&rcv_thread, NULL, recv_msg, (void *)&sd);

    //쓰레드 종료 대기 및 소멸 유도
    pthread_join(snd_thread, &thread_return);
    pthread_join(rcv_thread, &thread_return);

    close(sd);
    return 0;
}

void *send_file(void *arg) {

    //인자로 받은 sd casting
    int sd = *((int *)arg);
    int fd;
    int n;
    int sendmod = 0;
    char buf[BUFSIZE];
    char fn[BUFSIZE];
    
    //유저가 s를 입력할 때 까지 busywait
    
    while(sendmod == 0) {
	printf("enter 's' if you want to send\n");
	if (getchar() == 's'){
	    printf("filename: ");
	    scanf("%s",fn);
	    if ((fd = open(fn, O_RDONLY)) == -1) {
		perror("wrong file name");
	    }
	    else sendmod=1;
	}
    }
    strcpy(buf,"send start");
    send(sd, buf, BUFSIZE, 0);

    while((n= read(fd,buf,BUFSIZE))>0) {
	printf("%s",buf);
	if(write(sd,buf,n) !=n) perror("write");	
	
    }
    while(1);
}

void *recv_msg(void *arg) {
    int str_len;
    char msg[BUFSIZE];
    //인자로 받은 sd casting
    int sd = *((int *)arg);
    while(1) {
	//서버에서 들어온 메세지 수신
	str_len = read(sd, msg, BUFSIZE-1);

	//str_len이 -1이라는 건 소켓과 연결이 끊어졌다는 뜻
	if (str_len == -1) {
	    return (void *)-1; //pthread_join 실행
	}

	msg[str_len] = '\0';
	printf("%s\n",msg);
    }

   return NULL;
}

