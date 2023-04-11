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
  fflush(logfile);
  logfile = fopen("log.txt", "a");
  time_t clock;
  time(&clock);
  timeinfo = localtime(&clock);
  sprintf(logger, "%d:%d:%d %s", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, string);
  fprintf(logfile, "%s\n", logger);
  printf("%s\n", logger);
  fflush(logfile);
}
void confAttribution(char StringName[])
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
    if (conf_counter == 1)
    {
      sem_wait(file_mutex);
      shm->queue_size = atoi(buffer);
      sem_post(file_mutex);
    }
    else if (conf_counter == 2)
    {
      sem_wait(file_mutex);
      shm->n_workers = atoi(buffer);
      sem_post(file_mutex);
    }
    else if (conf_counter == 3)
    {
      sem_wait(file_mutex);
      shm->max_keys = atoi(buffer);
      sem_post(file_mutex);
    }
    else if (conf_counter == 4)
    {
      sem_wait(file_mutex);
      shm->max_sensors = atoi(buffer);
      sem_post(file_mutex);
    }
    else if (conf_counter == 5)
    {
      sem_wait(file_mutex);
      shm->max_alerts = atoi(buffer);
      sem_post(file_mutex);
    }
    else
    {
      printf("Config file is wrong!! \n");
      exit(0);
    }
  }
  fclose(conf);
  // printf("Conf info: %d, %d, %d, %d, %d \n",queue_size,n_workers,max_keys,max_sensors,max_alerts);
}

void *thread_test()
{

  pthread_t tid = pthread_self();
  printf("Thread %ld \n", tid);
  sleep(1);
  pthread_exit(NULL);
}

void createSem()
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

void accessResources(){
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

void createsharedmem()
{
  shmid = shmget(1234, sizeof(shm_t), IPC_CREAT | 0777);
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

void createThreads()
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

void consoleMenu()
{
  printf("\t MENU \t\n");
  printf("Write the option u want!!!\n");
  printf(" 1: EXIT\n 2: STATS\n 3: RESET\n 4: SENSORS\n 5: ADD ALERT\n 6: REMOVE ALERT\n 7: LIST ALERTS \n");
  char choice[15];
  fgets(choice, sizeof(choice), stdin);
  if (strcmp(choice, "exit\n") == 0)
  {
    exit(0);
  }
  else if (strcmp(choice, "stats\n") == 0)
  {
    printf("You're in stats!!!\n");
    consoleMenu();
  }
  else if (strcmp(choice, "reset\n") == 0)
  {
    printf("You're in reset!!!\n");
    consoleMenu();
  }
  else if (strcmp(choice, "sensors\n") == 0)
  {
    printf("You're in sensors!!!\n");
    consoleMenu();
  }
  else if (strcmp(choice, "add alert\n") == 0)
  {
    printf("You're in add alert!!!\n");
    consoleMenu();
  }
  else if (strcmp(choice, "remove alert\n") == 0)
  {
    printf("You're in remove alert!!!\n");
    consoleMenu();
  }
  else if (strcmp(choice, "list alerts\n") == 0)
  {
    printf("You're in list alerts!!!\n");
    consoleMenu();
  }
  else
  {
    printf("Invalid option, try again\n");
    consoleMenu();
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

void end_it_all()
{
  shmdt(shm);
  shmctl(shmid, IPC_RMID, NULL);
  sem_close(file_mutex);
  sem_unlink("file_write");
  fclose(logfile);
}

void waitforchilds()
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
    if (strcmp(argv[1], "user_console") == 0)
    {
      //printf("MY PID IS: %d\n", getpid());
      //printf("This is the user console!!! \n");
      strcpy(ConsoleID, argv[2]);
      printf("Console ID: %s\n", ConsoleID);
      accessResources();
      consoleMenu();

    }
    else if (strcmp(argv[1], "home_iot") == 0)
    {
      //printf("MY PID IS: %d\n", getpid());
      logfile = fopen("log.txt", "w");
      logging("SIMULATOR STARTING");
      char *ConfName;
      ConfName = (char *)malloc(100 * sizeof(char));
      ConfName = argv[2];
      //printf("This is the system Manager!!! \n");
      createSem();
      createsharedmem();
      shm->start = true;
      confAttribution(ConfName);
      createThreads();
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
      waitforchilds();
      if(pid > 0 ){
      end_it_all();
      logging("HOME_IOT SIMULATOR CLOSING");
      }

    }
    else
    {
      printf("Invalid Option\n");
      exit(0);
    }
  }
  else if (argc == 7)
  {
    if (strcmp(argv[1], "sensor") == 0)
    {
      //printf("MY PID IS: %d\n", getpid());
      printf("This is the sensor!!! \n");
      accessResources();
      sensor sens;
      if (argv[2][2] == '\0' || argv[4][2] == '\0' || alfanum_check(argv[2],1) == 0 || alfanum_check(argv[4],2) == 0 )
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
    }
    else
    {
      printf("Invalid Option\n");
      exit(0);
    }
  }
  else
  {
    printf("Invalid Option\n");
    exit(0);
  }

  exit(0);
}
