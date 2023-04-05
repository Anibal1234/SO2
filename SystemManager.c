
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <pthread.h>



int main(int argc, char** argv) {

  if(argc == 3){
    if(strcmp(argv[1],"user_console") == 0){
      printf("This is the user console!!! \n");
    }else if(strcmp(argv[1],"home_iot") == 0){
      printf("This is the system Manager!!! \n");
    }else{
      printf("Opções invalidas!!\n");
      exit(0);
    }

  }else if(argc== 7){
    if(strcmp(argv[1],"sensor") == 0){
      printf("This is the sensor!!! \n");
    }else{
      printf("Opções invalidas!!!\n");
      exit(0);
    }
  }else{
    printf("Opçoes invalidas!\n");
  }


}
