// gcc -Wall -pthread SystemManager.c -o run

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

#include "structs.h"

#define bufferLength 255
#define stringLength 200
// #define ConfFile

// global variables
int shmid;

FILE *conf;
FILE *logfile;
char buffer[bufferLength];
char logger[stringLength];
int queue_size;
int n_workers;
int max_keys;
int max_sensors;
int max_alerts;
int conf_counter = 0;
pid_t pid;
pthread_t dispacher;
pthread_t sensorReader;
pthread_t consoleReader;

void logging(char string[])
{
  logfile = fopen("log.txt", "a+");
  time_t clock;
  struct tm *timeinfo;
  time(&clock);
  timeinfo = localtime(&clock);
  sprintf(logger, "%d:%d:%d %s\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, string);
  printf("%s", logger);
  fprintf(logfile, "%s", logger);
  sleep(1);
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
    printf("%s\n", buffer);
    conf_counter++;
    if (conf_counter == 1)
    {
      queue_size = atoi(buffer);
    }
    else if (conf_counter == 2)
    {
      n_workers = atoi(buffer);
    }
    else if (conf_counter == 3)
    {
      max_keys = atoi(buffer);
    }
    else if (conf_counter == 4)
    {
      max_sensors = atoi(buffer);
    }
    else if (conf_counter == 5)
    {
      max_alerts = atoi(buffer);
    }
    else
    {
      printf("Config file is wrong!! \n");
      exit(0);
    }
  }
  fclose(conf);
  // printf("SAO ESTAS : %d, %d, %d, %d, %d \n",queue_size,n_workers,max_keys,max_sensors,max_alerts);
}

void *thread_test()
{
  pthread_t tid = pthread_self();
  printf("Thread %ld \n", tid);
  pthread_exit(NULL);
}

void sharedmem()
{
  shmid = shmget(IPC_PRIVATE, sizeof(shm_t), IPC_CREAT | 0777);
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
  logging("THREAD DISPACHER CREATED");
  pthread_create(&sensorReader, NULL, thread_test, NULL);
  logging("THREAD SENSOR_READER CREATED");
  pthread_create(&consoleReader, NULL, thread_test, NULL);
  logging("THREAD CONSOLE_READER CREATED");
  pthread_join(dispacher, NULL);
  pthread_join(sensorReader, NULL);
  pthread_join(consoleReader, NULL);
}

void consoleMenu()
{
  printf("\t MENU \t\n");
  printf(" 1: EXIT\n 2: STATS\n 3: RESET\n 4: SENSORS\n 5: ADD ALERT\n 6: REMOVE ALERT\n 7: LIST ALERTS \n");
  int choice;
  scanf("%d", &choice);
  if (choice > 7)
  {
    printf("Incorrect option, please try again!!\n");
    consoleMenu();
  }
  else
  {
    switch (choice)
    {
    case 1:
      exit(0);
    case 2:
      printf("You're in stats!!!\n");
      break;
    case 3:
      printf("You're in reset!!!\n");
      break;
    case 4:
      printf("You're in sensors!!!\n");
      break;
    case 5:
      printf("You're in add alert!!!\n");
      break;
    case 6:
      printf("You're in remove alert!!!\n");
      break;
    case 7:
      printf("You're in list alerts!!!\n");
      break;
    default:
      printf("Nao existe essa opção!!!\n");
      consoleMenu();
    }
  }
}

void end_it_all()
{
  shmdt(shm);
  shmctl(shmid, IPC_RMID, NULL);
  fclose(logfile);
}

void waitforchilds()
{
  for (int i = 0; i < n_workers + 1; i++)
  {
    wait(NULL);
  }
}

int main(int argc, char **argv)
{
  logfile = fopen("log.txt", "w");

  if (argc == 3)
  {
    if (strcmp(argv[1], "user_console") == 0)
    {
      time_t clock;
      struct tm *timeinfo;
      time(&clock);
      timeinfo = localtime(&clock);
      sprintf(logger, "%d:%d:%d SIMULATOR STARTING\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
      printf("%s", logger);
      fprintf(logfile, "%s", logger);
      sleep(1);
      // printf("This is the user console!!! \n");
      strcpy(ConsoleID, argv[2]);
      printf("Console ID: %s\n", ConsoleID);
      consoleMenu();
    }
    else if (strcmp(argv[1], "home_iot") == 0)
    {
      /*logfile = fopen("log.txt", "w");
      time_t clock;
      struct tm *timeinfo;
      time(&clock);
      timeinfo = localtime(&clock);
      sprintf(logger, "%d:%d:%d SIMULATOR STARTING\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
      printf("%s", logger);
      fprintf(logfile, "%s", logger);
      sleep(1);
      */
      char *ConfName;
      ConfName = (char *)malloc(100 * sizeof(char));
      ConfName = argv[2];
      printf("This is the system Manager!!! \n");
      confAttribution(ConfName);
      sharedmem();
      createThreads();

      for (int i = 0; i < n_workers; i++)
      {
        pid = fork();
        if (pid == 0)
        {
          printf("%d : I'm a child/worker process with a pid of %d and my dad is %d\n", i + 1, getpid(), getppid());
          break;
        }
        else if (pid < 0)
        {
          perror("Error creating child/worker processes");
          exit(0);
        }
        else
        {
          printf("I'M THE BIG PAPA!!!\n");
        }
      }
      if (pid > 0)
      {
        pid = fork();
        if (pid == 0)
        {
          printf("I'm the alerts child process with a father with the id of %d\n", getppid());
        }
        else if (pid < 0)
        {
          perror("Error creating alerts child process!!\n");
          exit(0);
        }
        else
        {
          printf("I'M the DADDY !!!\n");
        }
      }
    }
    else
    {
      printf("Opções invalidas!!\n");
      exit(0);
    }
  }
  else if (argc == 7)
  {
    if (strcmp(argv[1], "sensor") == 0)
    {
      printf("This is the sensor!!! \n");
      sensor sens; // to do: confirmar que a infor esta correta!
      if (argv[2][2] == '\0' || argv[4][2] == '\0')
      {
        perror("COMMAND ARGUMENTS WRONG!!!\n");
        exit(0);
      }
      strcpy(sens.id, argv[2]);
      sens.interval = atoi(argv[3]);
      strcpy(sens.key, argv[4]);
      sens.min = atoi(argv[5]);
      sens.max = atoi(argv[6]);
      printf(" HERE'S THE INFO: %s, %d, %s, %d, %d ", sens.id, sens.interval, sens.key, sens.min, sens.max);
    }
    else
    {
      printf("Opções invalidas!!!\n");
      exit(0);
    }
  }
  else
  {
    printf("Opçoes invalidas!\n");
    exit(0);
  }

  waitforchilds();
  end_it_all();
  exit(0);
}
