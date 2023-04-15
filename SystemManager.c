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

#define bufferLength 255
#define stringLength 200


// global variables
int shmid;
FILE *conf;
FILE *logfile;
char buffer[bufferLength];
char logger[stringLength];
sem_t *file_mutex;
int conf_counter = 0;
pid_t pid;
pthread_t dispacher;
pthread_t sensorReader;
pthread_t consoleReader;
struct tm *timeinfo;

void logging(char string[])
{
  sem_wait(file_mutex);
  fflush(logfile);
  logfile = fopen("log.txt", "a");// abrir e fechar no main e verificacao dos ficheiros,se abriram bem ou nao.
  time_t clock;
  time(&clock);
  timeinfo = localtime(&clock);
  sprintf(logger, "%d:%d:%d %s", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, string);
  fprintf(logfile, "%s\n", logger);
  printf("%s\n", logger);
  fflush(logfile);
  sem_post(file_mutex);
}


int conf_check(char inf[],int flag){
  int aux = atoi(inf);
  if(flag==1){
    if(aux >= 1){
      return 1;//WRONG
    }else{
      return 0;//CORRECT
    }
  }else if(flag == 2){
    if(aux>=0){
      return 1;//CORRECT
    }else{
      return 0;//WRONG
    }
  }else{
    return 0;//WRONG
  }
}


void conf_Attribution(char StringName[])
{
  conf = fopen(StringName, "r");
  if (conf == NULL)
  {
    printf("Config file can't be opened\n");
    exit(0);
  }

  while (fgets(buffer, bufferLength, conf))
  {
    //printf("%s\n", buffer);
    conf_counter++;
    if (conf_counter == 1 )
    {
      if(conf_check(buffer,1) == 1){
      shm->queue_size = atoi(buffer);
    }else{
      perror("CONFIG FILE IS INCORRECT!!!\n");
      exit(0);
    }
    }
    else if (conf_counter == 2)
    {
      if(conf_check(buffer,1) == 1){
      shm->n_workers = atoi(buffer);
    }else{
      perror("CONFIG FILE IS INCORRECT!!!\n");
      exit(0);
    }
    }
    else if (conf_counter == 3)
    {
      if(conf_check(buffer,1) == 1){
      shm->max_keys = atoi(buffer);
    }else{
      perror("CONFIG FILE IS INCORRECT!!!\n");
      exit(0);
    }
    }
    else if (conf_counter == 4)
    {
      if(conf_check(buffer,1) == 1){
      shm->max_sensors = atoi(buffer);
    }else{
      perror("CONFIG FILE IS INCORRECT!!!\n");
      exit(0);
    }
    }
    else if (conf_counter == 5)
    {
      if(conf_check(buffer,2) == 1){
        shm->max_alerts = atoi(buffer);
    }else{
      perror("CONFIG FILE IS INCORRECT!!!\n");
      exit(0);
      }
    }
    else
    {
      printf("Config file is wrong!! \n");
      exit(0);
    }
  }
  fclose(conf);
  printf("Conf info: %d, %d, %d, %d, %d \n",shm->queue_size,shm->n_workers,shm->max_keys,shm->max_sensors,shm->max_alerts);
}

void *thread_test()
{

  pthread_t tid = pthread_self();
  printf("Thread %ld \n", tid);
  sleep(3);
  pthread_exit(NULL);
}

void create_Sem()
{
  sem_close(file_mutex);
  sem_unlink("file_write");
  file_mutex = sem_open("file_write", O_CREAT | O_EXCL, 0700, 1);
  if (file_mutex == SEM_FAILED)
  {
    fprintf(stderr, "sem_open() failed. errno:%d\n", errno);
    if (errno == EEXIST)
    {
      printf("Semaphore already exists \n");
    }
    exit(0);
  }
}

void access_Resources(){
  shmid = shmget(1234, sizeof(shm_t), 0777);
  if (shmid == -1)
  {
    perror("FAILED TO ACCESS SHARED MEMORY!");
    exit(0);
  }
  shm = (shm_t *)shmat(shmid, NULL, 0);
  if (shm == NULL)
  {
    perror("FAILED ATTACHING SHARED MEMORY!");
    exit(0);
  }

  file_mutex = sem_open("file_write",0);
  if (file_mutex == SEM_FAILED)
  {
    fprintf(stderr, "sem_open() failed. errno:%d\n", errno);
    if (errno == EEXIST)
    {
      printf("Semaphore already exists \n");
    }
    exit(0);
  }

}

void create_shared_mem()
{
  shmid = shmget(1234, sizeof(shm_t), IPC_CREAT | 0777);// * max alerts
  if (shmid == -1)
  {
    perror("FAILED TO CREATE SHARED MEMORY!");
    exit(0);
  }

  shm = (shm_t *)shmat(shmid, NULL, 0);
  if (shm == NULL)
  {
    perror("FAILED ATTACHING SHARED MEMORY!");
    exit(0);
  }

  logging("SHARED MEMORY CREATED AND ATTACHED!");
}

void create_Threads()
{
  pthread_create(&dispacher, NULL, thread_test, NULL);

  pthread_create(&sensorReader, NULL, thread_test, NULL);

  pthread_create(&consoleReader, NULL, thread_test, NULL);

  pthread_join(dispacher, NULL);
  pthread_join(sensorReader, NULL);
  pthread_join(consoleReader, NULL);
  logging("THREAD DISPACHER CREATED");
  logging("THREAD SENSOR_READER CREATED");
  logging("THREAD CONSOLE_READER CREATED");
}





void end_it_all()
{
  shmdt(shm);
  shmctl(shmid, IPC_RMID, NULL);
  sem_close(file_mutex);
  sem_unlink("file_write");
  fclose(logfile);
}

void wait_for_childs()
{
  if(pid > 0){
  logging("HOME_IOT SIMULATOR WAITING FOR LAST TASKS TO FINISH");
  }
  for (int i = 0; i < shm->n_workers + 1; i++)
  {
    wait(NULL);
  }
}

int main(int argc, char **argv)
{

  if (argc == 3)
  {
    if (strcmp(argv[1], "home_iot") == 0)
    {
      //printf("MY PID IS: %d\n", getpid());

      logfile = fopen("log.txt", "w");
      logging("SIMULATOR STARTING");
      conf_Attribution(ConfName);
      char *ConfName;
      ConfName = (char *)malloc(100 * sizeof(char));
      ConfName = argv[2];
      //printf("This is the system Manager!!! \n");
      create_Sem();
      create_shared_mem();
      shm->start = true;

      create_Threads();
      for (int i = 0; i < shm->n_workers; i++)
      {
        pid = fork();
        if (pid == 0)
        {
          char str[14];
          char num[2];
          strcpy(str, "WORKER ");
          sprintf(num,"%d",i+1);
          strcat(str,num);
          strcat(str," READY");
          logging(str);
          sleep(5);
          //printf("%d : I'm a child/worker process with a pid of %d and my dad is %d\n", i + 1, getpid(), getppid());
          break;
        }
        else if (pid < 0)
        {
          perror("Error creating child/worker processes");
          exit(0);
        }
        else if (pid > 0)
        {
          //printf("I'M THE BIG PAPA!!!\n");
        }
      }
      if (pid > 0)
      {
        pid = fork();
        if (pid == 0)
        {
          logging("PROCESS ALERTS_WATCHER CREATED");
          //printf("I'm the alerts child process with a father with the id of %d\n", getppid());
        }
        else if (pid < 0)
        {
          perror("Error creating alerts child process!!\n");
          exit(0);
        }
        else
        {
          //printf("I'M the DADDY !!!\n");
        }
      }
      wait_for_childs();
      if(pid > 0 ){
      logging("HOME_IOT SIMULATOR CLOSING");
      end_it_all();
      }

    }
    else
    {
      printf("Invalid Option\n");
      exit(0);
    }
  }
  else
  {
    perror("Wrong Number of arguments!!!\n");
    exit(0);
  }

  exit(0);
}
