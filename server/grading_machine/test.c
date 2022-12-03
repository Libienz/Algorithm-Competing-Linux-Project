#include <stdio.h>
#include <stdlib.h>
int main(int argc, char *argv[]){

  int n;
  n= atoi(argv[1]);
  for(int i=2; i<=n+1; i++){
    printf("%s \n",argv[i]);
  }

}


  
