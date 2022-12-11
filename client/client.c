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

/* 
   클라이언트는 두개의 쓰레드 가진다
   서버로 부터의 메세지를 반복적으로 수신하는 쓰레드 (recv_msg를 메인으로)
   서버에게 파일을 전송(채점받기 위해)하는 쓰레드 (send_file을 메인으로)
 */
void *send_file(void *arg);
void *recv_msg(void *arg);

int gameset = 0; //game 끝을 알려줄 변수

int main() {

    int sd, len;
    char buf[BUFSIZE];
    struct sockaddr_in sin;
    pthread_t snd_thread, rcv_thread;
    void *thread_return;
    
    //서버 IP 주소 지정과 포트 번호 설정
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

//파일 보내는 쓰레드 메인 함수. 
void *send_file(void *arg) {

    
    //인자로 받은 sd casting
    int sd = *((int *)arg);
    int fd;
    int n;
    char buf[BUFSIZE];
    char fn[BUFSIZE];
    
    //게임이 끝날 때 까지 제출 반복 가능하도록 (누군가 정답을 맞추면 게임은 종료된다.)
    while(gameset == 0) {

	//유저가 s를 눌러 제출하려 한다
	if (getchar() == 's' ){
	    printf("제출할 파일이름을 입력해주세요: ");
	    scanf("%s",fn); //유저에게 제출할 파일 이름을 입력받고 fn에 저장
	    if ((fd = open(fn, O_RDONLY)) == -1) {
		perror("open"); //유저가 얘기한 파일을 여는데에 실패했다면 에러메세지 출력하고 다시 입력대기
		fprintf(stderr,"s를 입력하여 파일을 제출하세요!\n");
		continue;
	    }
	}
	else continue;
	
	//제어가 이곳으로 넘어왔다는 것은 유저가 존재하는 파일을 제출하려 한다는 것 
	strcpy(buf,"send start");
	//서버에게 이제부터 유저의 제출물을 전송할 것을 알린다.
	send(sd, buf, BUFSIZE, 0);

	//유저가 제출하고자 하는 파일을 읽어 소켓에다가 씀으로써 서버에 제출
	while((n= read(fd,buf,BUFSIZE))>0) {
	    //printf("%s",buf);
	    if(write(sd,buf,n) !=n) perror("write");	
	}
	close(fd);
    }
    return NULL;
}

// 수신 쓰레드의 메인함수
void *recv_msg(void *arg) {
    int str_len;
    char msg[BUFSIZE];
    char yl[BUFSIZE];
    char yw[BUFSIZE];
    //인자로 받은 sd casting
    int sd = *((int *)arg);

    strcpy(yl,"You lose..");
    strcpy(yw,"You win!!");
    
    //무한 반복으로 서버에서 들어온 메세지를 수신한다.
    while(1) {
	//서버에서 들어온 메세지 수신
	str_len = read(sd, msg, BUFSIZE-1);
	msg[str_len] = '\0';

	//수신 받은 메세지가 you lose나 you win등 게임의 끝을 알리는 메세지 이면 무한 반복이 깨지고 
	//쓰레드가 소멸되게 된다. 
	if (strcmp(msg, yl) == 0 || strcmp(msg, yw) == 0) {
	    fprintf(stderr,"%s 게임끝! 엔터를 눌러 종료하십시오",msg);
	    //printf로 하면 stdout이 stdin으로 들어가버려서 stderr로 사용 
	    gameset = 1;
	    break;
	}

	//str_len이 -1이라는 건 소켓과 연결이 끊어졌다는 뜻
	if (str_len == -1) {
	    return (void *)-1; //pthread_join 실행
	}

	msg[str_len] = '\0';
	printf("%s\n",msg);
    }

   return NULL;
}

