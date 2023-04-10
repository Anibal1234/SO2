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
#include <errno.h>

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
sem_t *file_mutex;
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

struct tm *timeinfo;
//(&shm)->start = false;

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
  sleep(1);
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
    printf("%s\n", buffer);
    conf_counter++;
    if (conf_counter == 1)
    {
      sem_wait(file_mutex);
      shm->queue_size = atoi(buffer);
      sem_post(file_mutex);
      printf("funcionou?: %d \n", shm->queue_size);
    }
    else if (conf_counter == 2)
    {
      sem_wait(file_mutex);
      shm->n_workers = atoi(buffer);
      sem_post(file_mutex);
      printf("funcionou?: %d \n", shm->n_workers);
    }
    else if (conf_counter == 3)
    {
      sem_wait(file_mutex);
      shm->max_keys = atoi(buffer);
      sem_post(file_mutex);
      printf("funcionou?: %d \n", shm->max_keys);
    }
    else if (conf_counter == 4)
    {
      sem_wait(file_mutex);
      shm->max_sensors = atoi(buffer);
      sem_post(file_mutex);
      printf("funcionou?: %d \n", shm->max_sensors);
    }
    else if (conf_counter == 5)
    {
      sem_wait(file_mutex);
      shm->max_alerts = atoi(buffer);
      sem_post(file_mutex);
      printf("funcionou?: %d \n", shm->max_alerts);
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
  sleep(1);
  pthread_exit(NULL);
}

void createSem()
{
  printf("YEEEEEEEE\n");
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

void sharedmem()
{
  printf("YA\n");
  shmid = shmget(IPC_PRIVATE, sizeof(shm_t), IPC_CREAT | 0777);
  if (shmid == -1)
  {
    perror("FAILED TO CREATE SHARED MEMORY!");
    exit(0);
  }

  shm = (shm_t *)shmat(shmid, NULL, 0);
  printf("help\n");
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
  }
  else if (strcmp(choice, "reset\n") == 0)
  {
    printf("You're in reset!!!\n");
  }
  else if (strcmp(choice, "sensors\n") == 0)
  {
    printf("You're in sensors!!!\n");
  }
  else if (strcmp(choice, "add alert\n") == 0)
  {
    printf("You're in add alert!!!\n");
  }
  else if (strcmp(choice, "remove alert\n") == 0)
  {
    printf("You're in remove alert!!!\n");
  }
  else if (strcmp(choice, "list alerts\n") == 0)
  {
    printf("You're in list alerts!!!\n");
  }
  else
  {
    printf("Invalid option, try again\n");
    consoleMenu();
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
  for (int i = 0; i < n_workers + 1; i++)
  {
    wait(NULL);
  }
}

int main(int argc, char **argv)
{
  logfile = fopen("log.txt", "w");
  logging("SIMULATOR STARTING");
  createSem();
  sharedmem();
  if (shm->start != true)
  {
    // printf("YEET\n");

    shm->start = true;
  }

  if (argc == 3)
  {
    if (strcmp(argv[1], "user_console") == 0)
    {
      printf("This is the user console!!! \n");
      strcpy(ConsoleID, argv[2]);
      printf("Console ID: %s\n", ConsoleID);
      consoleMenu();
    }
    else if (strcmp(argv[1], "home_iot") == 0)
    {

      char *ConfName;
      ConfName = (char *)malloc(100 * sizeof(char));
      ConfName = argv[2];
      printf("This is the system Manager!!! \n");
      confAttribution(ConfName);

      createThreads();
      sleep(1);
      printf("OIOIOI\n");
      for (int i = 0; i < shm->n_workers; i++)
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
        else if (pid > 0)
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
      printf("Invalid Option\n");
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

  waitforchilds();
  end_it_all();
  exit(0);
}
