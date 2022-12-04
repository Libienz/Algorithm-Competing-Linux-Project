#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#define NUMBER_OF_QUESTIONS 5 

int main()
{
  int randomNum;
  char question[10];
  char c;

  srand(time(0)); //난수 초기화. 이작업을 거치지않으면 프로그램 실행마다 항상 같은지점의 난수표에서 시작함
  randomNum = (rand() % NUMBER_OF_QUESTIONS) +1; //1~5사이 숫자
  sprintf(question,"%d",randomNum); //나온 숫자 문자열로 변환해주고
  char temptxt[10] = ".txt";
  strcat(question,temptxt);   //1.txt 처럼 합쳐준다
//  printf("%s\n",question);


  char buffer[BUFSIZ];
  FILE *fd;
  char wherefile[25] ="questions/";
  strcat(wherefile,question);
  printf("%s\n",wherefile);


  fd =fopen(wherefile, "r");
  while((c= fgetc(fd)) != EOF){
	printf("%c",c);
  }
  fclose(fd);

}
