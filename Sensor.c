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
sensor_t sens;
keys_t key;
int fd;
int count= 0;

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

int generateValue(int min, int max){
  srand(time(NULL));
  int number = (rand() % (max - min +1)) + min;
  return number;
}

void cleanup(int signum){
  printf("CTRL-C PRESSED....\n");
  close(fd);
  exit(0);
}

void show_count(int signum){
  printf("SENSOR HAS SENT %d MESSAGES!!!\n",count);
}

void openpipe(){
  if((fd = open(sensorPipe,O_WRONLY))<0){
      perror("ERROR OPENING SENSOR PIPE FOR WRITING!!!\n");
      exit(0);
  }
}

void sendInfo(){

  int value;
  char info[bufferLength];
  char num[4];
  value = generateValue(key.min,key.max);
  sprintf(num, "%d", value);
  strcpy(info,sens.id);
  strcat(info,"#");
  strcat(info,key.key);
  strcat(info,"#");
  strcat(info,num);
  strcpy(write_info, info);
  if(write(fd, write_info, sizeof(write_info)) == -1){
    perror("ERROR WRITING IN CONSOLE SENSOR PIPE!!!\n");
    exit(0);
  }
  //printf("INFO WROTE IN SENSOR NAMED PIPE ; %s \n",write_info);
  count +=1;

}


int main(int argc, char **argv)

{
    if (argc == 7)
    {
      if (strcmp(argv[1], "sensor") == 0)
      {
        //printf("MY PID IS: %d\n", getpid());
        //printf("This is the sensor!!! \n");
        signal(SIGINT, cleanup);
        signal(SIGTSTP, show_count);
        int max = atoi(argv[6]);
        int min = atoi(argv[5]);
        if (argv[2][2] == '\0' || argv[4][2] == '\0' || alfanum_check(argv[2],1) == 0 || alfanum_check(argv[4],2) == 0 || interval_check(min,max) == 0)
        {
          perror("COMMAND ARGUMENTS WRONG!!!\n");
          exit(0);
        }
        strcpy(sens.id, argv[2]);
        sens.interval = atoi(argv[3]);
        strcpy(key.key, argv[4]);
        key.min = atoi(argv[5]);
        key.max = atoi(argv[6]);
        //printf("HERE'S THE INFO: %s, %d, %s, %d, %d\n", sens.id, sens.interval, key.key, key.min, key.max);
        openpipe();
        while(true){
        sendInfo();
        sleep(sens.interval);
      }
        close(fd);
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
