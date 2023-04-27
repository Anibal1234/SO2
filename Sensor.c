// Anibal Rodrigues 2019224911
//Guilherme Junqueira 2019221958
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
#include <ctype.h>
#include <errno.h>

#include "structs.h"
#define sensorPipe "/tmp/SENSOR_PIPE"
#define bufferLength 255

char write_info[bufferLength];

int interval_check(int min, int max){
  if(max-min>0){
    return 1;//positive interval
  }else{
    return 0;//not positive interval
  }

}

int alfanum_check(char str[],int flag){
  if(flag == 1){//check for alphanumeric only
  for(int i = 0; i<strlen(str);i++){
    if(!isalnum(str[i])){
      return 0;//not alphanumeric
    }
  }
  return 1;// alphanumeric
}else if(flag == 2){//check for alphanumeric and _
  for(int i = 0; i<strlen(str);i++){
    if(!isalnum(str[i]) && str[i] != '_'){
      return 0;//not alphanumeric or _
    }
  }
  return 1;
}else{
  return 0;
}
}

void sendInfo(){
  int fd;
  printf("ENTREI!!!\n");
  if((fd = open(sensorPipe,O_WRONLY))<0){
      perror("ERROR OPENING SENSOR PIPE FOR WRITING!!!\n");
      exit(0);
  }
  printf("ENTREI123!!!\n");
  sprintf(write_info, "INFORMATION WROTE ON SENSOR PIPE");
  if(write(fd, write_info, sizeof(write_info)) == -1){
    perror("ERROR WRITING IN CONSOLE SENSOR PIPE!!!\n");
  }
  printf("INFO WROTE IN SENSOR NAMED PIPE!\n");

}



int main(int argc, char **argv)
{
    if (argc == 7)
    {
      if (strcmp(argv[1], "sensor") == 0)
      {
        //printf("MY PID IS: %d\n", getpid());
        printf("This is the sensor!!! \n");
        sensor_t sens;
        int max = atoi(argv[6]);
        int min = atoi(argv[5]);
        if (argv[2][2] == '\0' || argv[4][2] == '\0' || alfanum_check(argv[2],1) == 0 || alfanum_check(argv[4],2) == 0 || interval_check(min,max) == 0)
        {
          perror("COMMAND ARGUMENTS WRONG!!!\n");
          exit(0);
        }
        strcpy(sens.id, argv[2]);
        sens.interval = atoi(argv[3]);
        strcpy(sens.key, argv[4]);
        sens.min = atoi(argv[5]);
        sens.max = atoi(argv[6]);
        printf("HERE'S THE INFO: %s, %d, %s, %d, %d\n", sens.id, sens.interval, sens.key, sens.min, sens.max);
        sendInfo();
        exit(0);
      }
      else
      {
        printf("Invalid Option\n");
        exit(0);
      }
    }else{
      printf("Invalid Option\n");
      exit(0);
    }
}
