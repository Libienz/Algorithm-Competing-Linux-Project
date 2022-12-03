#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUMBER_OF_QUESTIONS 5 
int main()
{
  int randomNum;
  srand(time(0)); //난수 초기화. 이작업을 거치지않으면 프로그램 실행마다 항상 같은지점의 난수표에서 시작함
  randomNum = (rand() % NUMBER_OF_QUESTIONS) +1; //1~5사이 숫자
  printf("%d\n",randomNum);
}
