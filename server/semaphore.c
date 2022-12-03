#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

//세마포어 공용체 정의
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

//세마포어 생성과 초기화
//인자로 받은 키를 지정해 세마포어 식별자를 생성 
int initsem(key_t semkey) {
    union semun semunarg;
    int status = 0, semid;
    
    semid = semget(semkey, 1, IPC_CREAT | IPC_EXCL | 0600);

    //이미 존재하는 세마포어 식별자이면 기존 식별자 가져오기
    if (semid == -1) {
	if (errno == EEXIST)
	    semid = semget(semkey, 1,0);
    }
    //그렇지 않다면 세마포어 값을 1로 초기화
    else {
	semunarg.val = 1;
	status = semctl(semid, 0, SETVAL, semunarg);
    }

    if (semid == -1 || status == -1) {
	perror("initsem");
	return (-1);
    }

    return semid;
}

//세마포어 잠금
//semlock(): sem_op 값을 -1로 -> 공유 자원을 얻고 잠금 기능을 수행할 수 있도록
int semlock(int semid) {
    struct sembuf buf;

    buf.sem_num = 0;
    buf.sem_op = -1;
    buf.set_flg = SEM_UNDO;
    if (semop(semid, &buf, 1) == -1) {
	perror("semlock failed");
	exit(1);
    }
    return 0;
}

int semunlock(int semid) {
    struct sembuf buf;

    buf.sem_num = 0;
    buf.sem_op = 1;
    buf.semflg = SEM_UNDO;
    if (semop(semid, &buf, 1) == -1) {
	perror("semunlock failed");
	exit(1);
    }
    return 0;
}

