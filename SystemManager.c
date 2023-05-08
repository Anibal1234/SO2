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
#define consolePipe "/tmp/CONSOLE_PIPE"
#define sensorPipe "/tmp/SENSOR_PIPE"
#define QUEUE_KEY 123

data_t* confInfo;
// global variables
int shmid, msqid, int_msqid;
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
int channel[2];
char write_info[bufferLength];
char read_info[bufferLength];

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
  confInfo = malloc(sizeof(data_t));
  while (fgets(buffer, bufferLength, conf))
  {
    printf("%s\n", buffer);
    conf_counter++;
    if (conf_counter == 1 )
    {
      if(conf_check(buffer,1) == 1){
        printf("e aqui %d  \n",atoi(buffer));
        confInfo->queue_size = atoi(buffer);
        printf("pois e\n");
    }else{
      perror("CONFIG FILE IS INCORRECT!!!\n");
      exit(0);
    }
    }
    else if (conf_counter == 2)
    {
      if(conf_check(buffer,1) == 1){
      confInfo->n_workers = atoi(buffer);
    }else{
      perror("CONFIG FILE IS INCORRECT!!!\n");
      exit(0);
    }
    }
    else if (conf_counter == 3)
    {
      if(conf_check(buffer,1) == 1){
      confInfo->max_keys = atoi(buffer);
    }else{
      perror("CONFIG FILE IS INCORRECT!!!\n");
      exit(0);
    }
    }
    else if (conf_counter == 4)
    {
      if(conf_check(buffer,1) == 1){
      confInfo->max_sensors = atoi(buffer);
    }else{
      perror("CONFIG FILE IS INCORRECT!!!\n");
      exit(0);
    }
    }
    else if (conf_counter == 5)
    {
      if(conf_check(buffer,2) == 1){
        confInfo->max_alerts = atoi(buffer);
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
  printf("BISH\n");
  printf("Conf info: %d, %d, %d, %d, %d \n",confInfo->queue_size,confInfo->n_workers,confInfo->max_keys,confInfo->max_sensors,confInfo->max_alerts);
}

void *thread_test()
{

  pthread_t tid = pthread_self();
  printf("Thread %ld \n", tid);
  sleep(3);
  pthread_exit(NULL);
}

void *sensor_reader_f(){
  int fd;
  pthread_t tid = pthread_self();
  printf("Thread %ld: console reader \n", tid);

  if((fd = open(sensorPipe, O_RDONLY))<0){
    perror("ERROR OPEN SENSOR NAMED PIPE FOR READING!!!\n");
    exit(0);
  }
  if(read(fd, write_info, sizeof(write_info)) == -1){
    perror("ERROR READING IN SENSOR NAMED PIPE!!!\n");
  }
  printf("INFO READ FROM SENSOR NAMED PIPE : %s!\n", write_info);

  message_queue mesq;
  mesq.msgtype = 2;
  strcpy(mesq.temp , write_info) ;
  msgsnd(int_msqid,&mesq,sizeof(mesq)-sizeof(long),0);// mandar um estrutura com a mensagem e de quem veio, aqui e no console reader
  printf("SENSOR SENT THIS INFO THROUGH INTERNAL MESSAGE QUEUE: %s\n",mesq.temp);


  return NULL;

}

void *console_reader_f(){
  int fd;
  pthread_t tid = pthread_self();
  printf("Thread %ld: console reader \n", tid);
  if((fd = open(consolePipe, O_RDONLY))<0){
    perror("ERROR OPEN CONSOLE NAMED PIPE FOR READING!!!\n");
    exit(0);
  }
  if(read(fd, write_info, sizeof(write_info)) == -1){
    perror("ERROR READING IN CONSOLE NAMED PIPE!!!\n");
  }
  printf("INFO READ FROM CONSOLE NAMED PIPE : %s!\n", write_info);

  message_queue mesq;
  mesq.msgtype = 1;
  strcpy(mesq.temp, "10");
  msgsnd(int_msqid,&mesq,sizeof(mesq)-sizeof(long),0);
  printf("CONSOLE SENT THIS INFO THROUGH INTERNAL MESSAGE QUEUE: %s\n",mesq.temp);

  return NULL;

}


void *dispacher_f(){
  pthread_t tid = pthread_self();
  printf("Thread %ld: dispacher \n", tid);
  //close(channel[0]);

  message_queue mesq;
  msgrcv(int_msqid,&mesq,sizeof(mesq)-sizeof(long),0,2);//checar a prioridade
  printf("RECEIVED THIS INFO THROUGH INTERNAL MESSAGE QUEUE: %s\n",mesq.temp);
  printf("INFO READ FROM INTERNAL MESSAGE QUEUE: %s\n",mesq.temp);
  //msgrcv(int_msqid,&mesq,sizeof(mesq)-sizeof(long),0,0);//checar a prioridade
  //printf("RECEIVED THIS INFO THROUGH INTERNAL MESSAGE QUEUE: %s\n",mesq.temp);

  strcpy(write_info, mesq.temp);
  if(write(channel[1], write_info, sizeof(write_info)) == -1){
    perror("ERROR WRITING IN UNNAMED PIPE!!!\n");
  }
  printf("INFO SENT THROUGH UNNAMED PIPE : %s!\n", write_info);
  close(channel[1]);//por no cleanup no futuro
  return NULL;
}


void create_Message_Queue(){
  if((msqid = msgget(QUEUE_KEY, IPC_CREAT | 0700)) == -1){
    perror("ERROR ON CREATION OF MESSAGE QUEUE!!!\n");
    exit(0);
  }
  if((int_msqid = msgget(IPC_PRIVATE, IPC_CREAT | 0700)) == -1){
    perror("ERROR CREATING INTERNAL QUEUE!!!\n");
    exit(0);
  }
}

void create_Sem()//kill ipc sh na ficha de shared memory
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


void create_pipes(){
  if(pipe(channel) == -1){
    perror("ERROR CREATING UNNAMED PIPE!!!!\n");
    exit(0);
  }
  unlink(consolePipe);
  if(mkfifo(consolePipe,O_CREAT | O_EXCL | 0777)<0){
    perror("ERROR CREATING CONSOLE NAMED PIPE!!!!\n");
    exit(0);
  }
  unlink(sensorPipe);
  if(mkfifo(sensorPipe, O_CREAT | O_EXCL | 0777) <0){
    perror("ERROR CREATING SENSOR NAMED PIPE !!!\n");
    exit(0);
  }
}


void create_shared_mem()
{
  printf("ENTREI\n");
  shmid = shmget(1234, confInfo->max_alerts*sizeof(shm_t) + confInfo->max_sensors * sizeof(sensor_t) + confInfo->max_keys *sizeof(key_t), IPC_CREAT | 0777);//  ADICIONAR AS ESTRUTURAS QUE VAO ESTAR NO SHARED MEMORY
  printf("TAMBEM\n");
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
  pthread_create(&dispacher, NULL, dispacher_f, NULL);
  logging("THREAD DISPACHER CREATED");
  pthread_create(&sensorReader, NULL, sensor_reader_f, NULL);
  logging("THREAD SENSOR_READER CREATED");
  pthread_create(&consoleReader, NULL, console_reader_f, NULL);
  logging("THREAD CONSOLE_READER CREATED");

  pthread_join(dispacher, NULL);
  pthread_join(sensorReader, NULL);
  pthread_join(consoleReader, NULL);

}


void addSensorInfo(char info[]){
  printf(" ENTREI ADDSENSORINFO!!\n");
  char *id = strtok(info, "#");
  char *key = strtok(NULL, "#");
  char *value = strtok(NULL,"#");
  int val = atoi(value);
  for(int i = 0; i<confInfo->max_sensors;i++){
    printf("NO MAX SENSORS %d \n", i);
    printf("shm->sens[%d].id = '%s'\n", i, shm->sens[i].id);
    if(shm->sens[i].id && shm->sens[i].id[0] != '\0'){
      printf("yeet\n ");
      printf("UNO \n");
      if(strcmp(shm->sens[i].id,id) == 0){
        printf("DOS \n");
        for(int l = 0; l< confInfo->max_keys;l++){
          printf("NO MAX KEYS %d \n", l);
          if(strcmp(shm->sens->keys[l].key,key) == 0){
            printf("TRES \n");
            shm->sens->keys[l].lastValue = val;
            shm->sens->keys[l].updates += 1;
            shm->sens->keys[l].sum += val;
            shm->sens->keys[l].mean = (shm->sens->keys[l].sum / shm->sens->keys[l].updates);
            if(val > shm->sens->keys[l].maxValue){
              printf("QUATRO\n");
              shm->sens->keys[l].maxValue = val;
            }
            if(val< shm->sens->keys[l].minValue){
              printf("cinco\n");
              shm->sens->keys[l].minValue = val;
            }
            break;
          }else if(shm->sens->keys[l].key == NULL){
            printf("SEXTO\n");
            shm->sens->keys[l].lastValue = val;
            shm->sens->keys[l].updates = 1;
            shm->sens->keys[l].sum = val;
            shm->sens->keys[l].mean = (shm->sens->keys[l].sum / shm->sens->keys[l].updates);
            shm->sens->keys[l].maxValue = val;
            shm->sens->keys[l].minValue = val;
            break;
          }
          if(l == confInfo->max_keys){
            printf("NUMERO MAXIMO DE KEYS ATINGIDO!!!!\n");//falta aqui cenas, temos de ver como fazemos com que o processo acabe
          }
        }
      }
      printf("KJKJKJKJKJK\n");
    }else if(shm->sens[i].id ){
      printf("SETE\n");
      strcpy(shm->sens->id, id );
      shm->sens->keys[0].lastValue = val;
      shm->sens->keys[0].updates = 1;
      shm->sens->keys[0].sum = val;
      shm->sens->keys[0].mean = (shm->sens->keys[0].sum / shm->sens->keys[0].updates);
      shm->sens->keys[0].maxValue = val;
      shm->sens->keys[0].minValue = val;
      break;
  } if( i == confInfo->max_sensors){
    printf("NUMERO MAXIMO DE SENSORS ATINGIDO!!!!\n");//mesma merda que em cima
  }
  printf("NICE \n");
}
printf("WELELELELELELEL\n");
}

void end_it_all()
{
  shmdt(shm);
  shmctl(shmid, IPC_RMID, NULL);
  unlink(consolePipe);
  unlink(sensorPipe);
  free(confInfo);
  msgctl(msqid,IPC_RMID,NULL);
  msgctl(int_msqid,IPC_RMID,NULL);
  sem_close(file_mutex);
  sem_unlink("file_write");
  fclose(logfile);
}

void wait_for_childs()
{
  if(pid > 0){
  logging("HOME_IOT SIMULATOR WAITING FOR LAST TASKS TO FINISH");
  }
  for (int i = 0; i < confInfo->n_workers + 1; i++)
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
      printf("MY PID IS: %d\n", getpid());
      char *ConfName;
      ConfName = (char *)malloc(100 * sizeof(char));
      ConfName = argv[2];
      conf_Attribution(ConfName);
      logfile = fopen("log.txt", "w");
      printf("YUIIIII\n");
      create_Sem();
      printf("poiss\n");
      create_shared_mem();
      printf("shift\n");
      create_pipes();
      create_Message_Queue();
      logging("SIMULATOR STARTING");

      printf("This is the system Manager!!! \n");


      shm->start = true;

      create_Threads();
      for (int i = 0; i < confInfo->n_workers; i++)
      {
        pid = fork();
        if (pid == 0)
        {
          int state = 0;
          //ler pipe e criar uma flag para apenas um worker ler
          char str[14];
          char num[2];
          strcpy(str, "WORKER ");
          sprintf(num,"%d",i+1);
          strcat(str,num);
          strcat(str," READY");
          logging(str);
          if(state == 0){
            state = 1;
            close(channel[1]);
            read(channel[0], read_info, sizeof(read_info));
            printf("[WORKER %s] Received (%s) from master to add.\n",num,read_info);//condicoes para em caso de ser leitura de console ou de sensor
            printf(" POIS YHA MAN !!!\n");
            addSensorInfo(read_info);
            printf(" GUILHERME JUNQUEIRA !!!!\n");
            for(int i =0; i< confInfo->max_keys;i++){
              if(shm->sens[0].keys[i].key){
                printf("ITERAÇAO %d\n",i);
                printf("INFORMATION IN KEYS :%s ; %d; %d; %d; %d; %d; %d; %d; %d;", shm->sens[0].keys[i].key,shm->sens[0].keys[i].min,shm->sens[0].keys[i].max,shm->sens[0].keys[i].lastValue,shm->sens[0].keys[i].minValue,shm->sens[0].keys[i].maxValue,shm->sens[0].keys[i].mean,shm->sens[0].keys[i].updates,shm->sens[0].keys[i].sum);
              }else{
                printf("DEU BREAK\n");
                break;
              }

            }
          }
          state = 0;
          close(channel[0]);
          //sleep(5);
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
          message_queue mesqueue;
          mesqueue.msgtype = 1;
          strcpy(mesqueue.temp, "20");
          msgsnd(msqid,&mesqueue,sizeof(mesqueue)-sizeof(long),0);
          printf("SENT THIS INFO THROUGH MESSAGE QUEUE: %s\n",mesqueue.temp);
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
