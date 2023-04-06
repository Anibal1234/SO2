//gcc -Wall -pthread SystemManager.c -o run



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

#include "structs.h"

#define bufferLength 255

//global variables
int shmid;
FILE* conf;
char buffer[bufferLength];
int queue_size;
int n_workers;
int max_keys;
int max_sensors;
int max_alerts;
int conf_counter = 0;
pid_t pid;

void confAttribution(){
  conf = fopen("configFile.txt","r");
  if(conf == NULL){
    printf("Config file can't be opened\n");
    exit(0);
  }
  while(fgets(buffer,bufferLength, conf)){
    printf("%s\n", buffer);
    conf_counter ++;
    if(conf_counter == 1){
        queue_size = atoi(buffer);
    }else if(conf_counter == 2){
        n_workers = atoi(buffer);
    }else if(conf_counter == 3){
        max_keys = atoi(buffer);
    }else if(conf_counter == 4){
        max_sensors = atoi(buffer);
    }else if(conf_counter == 5){
        max_alerts = atoi(buffer);
    }else{
      printf("Config file is wrong!! \n");
      exit(0);
    }
  }
  fclose(conf);
  //printf("SAO ESTAS : %d, %d, %d, %d, %d \n",queue_size,n_workers,max_keys,max_sensors,max_alerts);
}

void sharedmem(){
  shmid = shmget(IPC_PRIVATE,sizeof(shm_t),IPC_CREAT|0777);
  if(shmid == -1){
		perror("FAILED TO CREATE SHARED MEMORY!");
    exit(0);
	}

  shm = (shm_t*) shmat(shmid,NULL,0);

  if(shm == NULL){
    perror("FAILED ATTACHING SHARED MEMORY!");
    exit(0);
  }

  printf("SHARED MEMORY CREATED AND ATTACHED!\n");

}

void end_it_all(){
  shmdt(shm);
  shmctl(shmid,IPC_RMID, NULL);

}

void waitforworkers() {
    for (int i = 0; i < n_workers; i++) {
      wait(NULL);
    }
}


int main(int argc, char** argv) {

  if(argc == 3){
    if(strcmp(argv[1],"user_console") == 0){
      printf("This is the user console!!! \n");
    }else if(strcmp(argv[1],"home_iot") == 0){
      printf("This is the system Manager!!! \n");
      confAttribution();
      sharedmem();

      for(int i = 0; i<n_workers;i++){
        pid = fork();
        if(pid == 0){
          printf("%d : I'm a child/worker process with a pid of %d and my dad is %d\n", i+1,getpid(), getppid());
          break;
        }else if(pid<0){
          perror("Error creating child/worker processes");
          exit(0);
        }else{
          printf("I'M THE BIG PAPA!!!\n");
        }
      }


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

waitforworkers();
end_it_all();
}
