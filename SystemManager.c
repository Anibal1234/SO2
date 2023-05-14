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
int shmid, msqid;
FILE *conf;
FILE *logfile;
char buffer[bufferLength];
char logger[stringLength];
sem_t *file_mutex;
sem_t *shm_sem;
sem_t *worker_sem;
int conf_counter = 0;
pid_t pid;
pthread_t dispacher;
pthread_t sensorReader;
pthread_t consoleReader;
struct tm *timeinfo;
int **channel;
char write_info[bufferLength];
char read_info[bufferLength];
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_full = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex_full = PTHREAD_MUTEX_INITIALIZER;

void logging(char string[])
{
  sem_wait(file_mutex);
  fflush(logfile);
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
    conf_counter++;
    if (conf_counter == 1 )
    {
      if(conf_check(buffer,1) == 1){
        confInfo->queue_size = atoi(buffer);
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
  }

bool queue_full(){
  int cont = 0;
  for(int k = 0;k<confInfo->queue_size;k++){
    if(int_queue[k].message[0] != '\0'){
      cont++;
    }
  }
  if(cont == confInfo->queue_size){
    return true;
  }else{
    return false;
  }
}

void sensor_on(char id[]){
  for(int i = 0; i<confInfo->max_sensors;i++){
    if(strcmp(id,shm->sens[i].id) == 0){
      shm->sens[i].sent = 1;
      break;
    }
  }
}

void *sensor_reader_f(){
  int fd;
  internal_queue aux;
  pthread_t tid = pthread_self();
  printf("Thread %ld: console reader \n", tid);

  if((fd = open(sensorPipe, O_RDONLY))<0){
    perror("ERROR OPEN SENSOR NAMED PIPE FOR READING!!!\n");
    exit(0);
  }

  bool test = true;
  while(test == true){

  if(read(fd, write_info, sizeof(write_info)) == -1){
    perror("ERROR READING IN SENSOR NAMED PIPE!!!\n");
  }

  char aux2[bufferLength];
  strcpy(aux2,write_info);
  char *token = strtok(aux2, "#");
  int cont;
    while (token != NULL) {
        cont ++;
        token = strtok(NULL, "#");
    }
    if(cont != 3){
      char b[bufferLength];
      strcpy(b,"WRONG COMMAND -> ");
      strcat(b,write_info);
      logging(b);
      cont = 0;
    }else{
  cont = 0;
  strcpy(aux.message,write_info);
  aux.type = 2;
  for(int i = 0; i<confInfo->queue_size;i++){
    if(int_queue[i].message[0] == '\0'){
      pthread_mutex_lock(&mutex);
      int_queue[i] = aux;
      //printf("SENSOR SENT THIS INFO THROUGH INTERNAL MESSAGE QUEUE: %s\n",int_queue[i].message);
      pthread_cond_signal(&cond);
      pthread_mutex_unlock(&mutex);
      break;
    }else if(i == confInfo->queue_size -1 && int_queue[i].message[0] != '\0'){
      logging("IGNOROU UM SENSOR VALUE!!!");
    }
  }

  }
}
  return NULL;

}

void *console_reader_f(){
  int fd;
  internal_queue aux;
  pthread_t tid = pthread_self();
  printf("Thread %ld: console reader \n", tid);
  if((fd = open(consolePipe, O_RDONLY))<0){
    perror("ERROR OPEN CONSOLE NAMED PIPE FOR READING!!!\n");
    exit(0);
  }

  bool test = true;
  while(test == true){
  if(read(fd, write_info, sizeof(write_info)) == -1){
    perror("ERROR READING IN CONSOLE NAMED PIPE!!!\n");
  }
  //printf("INFO READ FROM CONSOLE NAMED PIPE : %s!\n", write_info);
  strcpy(aux.message,write_info);
  aux.type = 1;//prioridade de console

  for(int i = 0; i<confInfo->queue_size;i++){
    if(int_queue[i].message[0] == '\0'){
      pthread_mutex_lock(&mutex);
      int_queue[i] = aux;
      //printf("CONSOLE SENT THIS INFO THROUGH INTERNAL MESSAGE QUEUE: %s\n",int_queue[i].message);
      pthread_cond_signal(&cond);
      pthread_mutex_unlock(&mutex);
      break;
    }else if(i == confInfo->queue_size -1 && int_queue[i].message[0] != '\0'){
      pthread_mutex_lock(&mutex_full);
      while(queue_full() == true){
        logging("ESPERA POR ESPAÇO NA INTERNAL QUEUE!!!");
        pthread_cond_wait(&cond_full, &mutex_full);
      }
      pthread_mutex_unlock(&mutex_full);
      int_queue[i] = aux;

    }
  }

}

  return NULL;

}


bool check_queue(){
  if(int_queue[0].message[0] != '\0'){
    return true;
  }else{
    return false;
  }
}

void *dispacher_f(){
  internal_queue aux;
  int flag = 0;
  pthread_t tid = pthread_self();
  int work = -1;
  int ind;
  printf("Thread %ld: dispacher \n", tid);
  bool test = true;
  while(test == true){
  pthread_mutex_lock(&mutex);
  while(check_queue() != true){
    pthread_cond_wait(&cond, &mutex);
  }
  pthread_mutex_unlock(&mutex);
  pthread_mutex_lock(&mutex_full);
  if(check_queue() == true){
  for(int i = 0;i<confInfo->queue_size;i++){
    if(int_queue[i].message[0] != '\0' && int_queue[i].type == 1){
      aux= int_queue[i];
      ind = i;
      break;
    }else if(int_queue[i].message[0] != '\0' && int_queue[i].type == 2){
      if(flag == 0){
        aux = int_queue[i];
        ind = i;
        flag = 1;
      }
    }
  }
  flag = 0;
  for(int l = ind+1;l<confInfo->queue_size;l++){
    int_queue[l-1]=int_queue[l];
  }
  int_queue[confInfo->queue_size-1].message[0] = '\0';
  int_queue[confInfo->queue_size-1].type = 0;

  pthread_cond_signal(&cond_full);
  pthread_mutex_unlock(&mutex_full);

  //printf("INFO READ FROM INTERNAL MESSAGE QUEUE: %s\n",aux.message);

  strcpy(write_info, aux.message);
  sem_wait(worker_sem);
  for(int k = 0;k<confInfo->n_workers;k++){
      if(shm->workers[k] == 0){
        work = k;
        break;
      }
  }
  //printf(" WORKER AVAILABLE : %d\n", work);
  shm->workers[work] =1;
  if(write(channel[work][1], &aux, sizeof(internal_queue)) == -1){
    perror("ERROR WRITING IN UNNAMED PIPE!!!\n");
  }
  //printf("INFO SENT THROUGH UNNAMED PIPE : %s!\n", aux.message);
    }
  }
    return NULL;
  }


void create_Message_Queue(){
  if((msqid = msgget(QUEUE_KEY, IPC_CREAT | 0700)) == -1){
    perror("ERROR ON CREATION OF MESSAGE QUEUE!!!\n");
    exit(0);
  }
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
      perror("Semaphore already exists! \n");
    }
    exit(0);
  }

  sem_close(shm_sem);
  sem_unlink("shm_mutex");
  shm_sem = sem_open("shm_mutex",O_CREAT | O_EXCL, 0700, 1);
  if (shm_sem== SEM_FAILED)
  {
    fprintf(stderr, "sem_open() failed. errno:%d\n", errno);
    if (errno == EEXIST)
    {
      perror("Semaphore already exists!! \n");
    }
    exit(0);
  }

  sem_close(worker_sem);
  sem_unlink("worker_mutex");
  worker_sem = sem_open("worker_mutex",O_CREAT | O_EXCL, 0700, confInfo->n_workers);
  if (worker_sem == SEM_FAILED)
  {
    fprintf(stderr, "sem_open() failed. errno:%d\n", errno);
    if (errno == EEXIST)
    {
      perror("Semaphore already exists!!! \n");
    }
    exit(0);
  }

}




void create_unnamed_pipes(int i){
  if(pipe(channel[i]) == -1){
    perror("ERROR CREATING UNNAMED PIPE !!!!\n");
    exit(0);
  }
}

void create_pipes(){
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
  shmid = shmget(1234, sizeof(shm_t) + confInfo->n_workers * sizeof(int) + confInfo->max_sensors * sizeof(sensor_t) + confInfo->max_keys *sizeof(key_t) + confInfo->max_alerts *sizeof(alert_t), IPC_CREAT | 0777);
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
  shm->workers = (int*)( ((void*)shm) + sizeof(shm_t));
  shm->sens = (sensor_t*)( ((void*)shm->workers) + sizeof(int) * confInfo->n_workers);
  shm->keys = (keys_t*)( ((void*)shm->sens) + sizeof(sensor_t) * confInfo->max_sensors);
  shm->alerts = (alert_t*)( ((void*)shm->keys) + sizeof(keys_t) * confInfo->max_keys);
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

}

void addSensorInfo(char info[]){
  char *id = strtok(info, "#");
  char *key = strtok(NULL, "#");
  char *value = strtok(NULL,"#");
  int val = atoi(value);

  for(int i = 0; i<confInfo->max_sensors;i++){
    if( i == (confInfo->max_sensors - 1) && shm->sens[i].id[0] != '\0'){
      logging("NUMERO MAXIMO DE SENSORS ATINGIDO!!!!");
      break;
    }else if(strcmp(id,shm->sens[i].id)==0){
      break;

    }else if(shm->sens[i].id[0] == '\0'){
        strcpy(shm->sens[i].id, id );
        sensor_on(id);
        //printf("SENSOR INFO :%s",shm->sens[i].id);
        break;
    }
  }
    for(int l = 0; l< confInfo->max_keys;l++){
      if(l == (confInfo->max_keys-1) && strcmp(shm->keys[l].key,key) != 0){
        logging("NUMERO MAXIMO DE KEYS ATINGIDO!!!!\n");
        break;
      }
      if(strcmp(shm->keys[l].key,key) == 0){
        shm->keys[l].lastValue = val;
        shm->keys[l].updates += 1;
        shm->keys[l].sum += val;
        shm->keys[l].mean = (shm->keys[l].sum / shm->keys[l].updates);
        if(val > shm->keys[l].maxValue){
          shm->keys[l].maxValue = val;
        }
        if(val< shm->keys[l].minValue){
          shm->keys[l].minValue = val;
        }
        break;
      }else if(shm->keys[l].key && shm->keys[l].key[0] == '\0'){
        strcpy(shm->keys[l].key, key );
        shm->keys[l].lastValue = val;
        shm->keys[l].updates = 1;
        shm->keys[l].sum = val;
        shm->keys[l].mean = (shm->keys[l].sum / shm->keys[l].updates);
        shm->keys[l].maxValue = val;
        shm->keys[l].minValue = val;
        break;
      }

    }
}

bool check_alerts(char id[], char key[]){
  for(int i = 0; i<confInfo->max_alerts;i++){
    if(strcmp(shm->alerts[i].id,id) == 0){
      return false;
    }
  }
  for(int l = 0; l<confInfo->max_keys;l++){
    if(strcmp(shm->keys[l].key,key) == 0){
      return true;
    }
  }
  return false;
}

void addAlertInfo(char id[], char key[], int min, int max){
  for(int k = 0 ; k < confInfo->max_alerts;k++){
    if(k == (confInfo->max_alerts-1) && shm->alerts[k].id[0] != '\0'){
      logging("NUMERO MAXIMO DE ALERTS ATINGIDO!!!!");
      break;
    }
    if(shm->alerts[k].id[0] == '\0'){
      strcpy(shm->alerts[k].id, id);
      strcpy(shm->alerts[k].key,key);
      shm->alerts[k].min = min;
      shm->alerts[k].max = max;
      //printf("ALERT : %s\n", shm->alerts[k].id);
      break;
    }
  }

}

void console_print(char inf[]){
  char *fl = strtok(inf, " ");
  char str[1024] = "";
  char aux[34];
  message_queue mess_q;
  if(strcmp(fl,"stats") == 0){
    for(int i = 0; i<confInfo->max_keys;i++){
      if(shm->keys[i].key[0] !='\0'){
        strcat(str,shm->keys[i].key);
        strcat(str," ");
        sprintf(aux,"%d",shm->keys[i].lastValue);
        strcat(str,aux);
        strcat(str," ");
        sprintf(aux,"%d",shm->keys[i].minValue);
        strcat(str,aux);
        strcat(str," ");
        sprintf(aux,"%d",shm->keys[i].maxValue);
        strcat(str,aux);
        strcat(str," ");
        sprintf(aux,"%d",shm->keys[i].mean);
        strcat(str,aux);
        strcat(str," ");
        sprintf(aux,"%d",shm->keys[i].updates);
        strcat(str,aux);
        strcat(str,"\n");

      }else{
        break;
      }
    }
    strcpy(mess_q.temp,str);
    mess_q.msgtype = 1;
    msgsnd(msqid,&mess_q,sizeof(mess_q)-sizeof(long),0);
  }else if(strcmp(fl,"reset") == 0){
    for(int i = 0; i<confInfo->max_keys;i++){
      if(shm->keys[i].key[0] !='\0'){
        sem_wait(shm_sem);
        shm->keys[i].key[0] = '\0';
        shm->keys[i].lastValue = 0;
        shm->keys[i].minValue = 0;
        shm->keys[i].maxValue = 0;
        shm->keys[i].mean = 0;
        shm->keys[i].updates = 0;
        shm->keys[i].sum = 0;
        sem_post(shm_sem);
      }else{
        break;
      }
    }
    strcpy(mess_q.temp,"(OK)");
    mess_q.msgtype = 1;
    msgsnd(msqid,&mess_q,sizeof(mess_q)-sizeof(long),0);

  }else if(strcmp(fl,"sensors") == 0){
    for(int i = 0;i<confInfo->max_sensors;i++){
      if(shm->sens[i].sent == 1){
        strcat(str,shm->sens[i].id);
        strcat(str," \n");
      }
    }
    strcpy(mess_q.temp,str);
    mess_q.msgtype = 1;
    msgsnd(msqid,&mess_q,sizeof(mess_q)-sizeof(long),0);
  }else if(strcmp(fl,"add_alert") == 0){
    char *alert_id = strtok(NULL," ");
    char *key = strtok(NULL," ");
    char *min = strtok(NULL," ");
    int new_min = atoi(min);
    char *max = strtok(NULL," ");
    int new_max = atoi(max);
    if(check_alerts(alert_id,key) == true ){
      sem_wait(shm_sem);
      addAlertInfo(alert_id,key,new_min,new_max);
      sem_post(shm_sem);
      strcpy(mess_q.temp,"(OK)");
      mess_q.msgtype = 1;
      msgsnd(msqid,&mess_q,sizeof(mess_q)-sizeof(long),0);

    }else{
      strcpy(mess_q.temp,"(ERROR)");
      mess_q.msgtype = 1;
      msgsnd(msqid,&mess_q,sizeof(mess_q)-sizeof(long),0);
    }

  }else if(strcmp(fl,"remove_alert") == 0){
    char * id = strtok(NULL, " ");
    int newline_index = strcspn(id, "\n");
    if (id[newline_index] == '\n') {
        id[newline_index] = '\0';
    }
    int flag = 0;
    for(int i = 0; i<confInfo->max_alerts;i++){
      if(strcmp(shm->alerts[i].id,id) == 0){
        sem_wait(shm_sem);
        shm->alerts[i].id[0] = '\0';
        shm->alerts[i].key[0] = '\0';
        shm->alerts[i].min = 0;
        shm->alerts[i].max = 0;
        sem_post(shm_sem);
        flag =1;
        break;
      }
  }
  if(flag == 1){
    strcpy(mess_q.temp,"(OK)");
    mess_q.msgtype = 1;
    msgsnd(msqid,&mess_q,sizeof(mess_q)-sizeof(long),0);
  }else{
    strcpy(mess_q.temp,"(ERROR)");
    mess_q.msgtype = 1;
    msgsnd(msqid,&mess_q,sizeof(mess_q)-sizeof(long),0);
  }

}else if(strcmp(fl,"list_alerts") == 0){

  for(int i = 0;i<confInfo->max_alerts;i++){
    if(shm->alerts[i].id[0] != '\0'){
      strcat(str,shm->alerts[i].id);
      //printf(" IDS DE ALERTS: %s \n", shm->alerts[i].id);
      strcat(str," ");
      strcat(str,shm->alerts[i].key);
      strcat(str," ");
      sprintf(aux,"%d",shm->alerts[i].min);
      strcat(str,aux);
      strcat(str," ");
      sprintf(aux,"%d",shm->alerts[i].max);
      strcat(str,aux);
      strcat(str," \n");
    }
  }

  strcpy(mess_q.temp,str);
  mess_q.msgtype = 1;
  msgsnd(msqid,&mess_q,sizeof(mess_q)-sizeof(long),0);

}else if(strcmp(fl,"INVALID") == 0){
  strcpy(mess_q.temp,"(ERROR)");
  mess_q.msgtype = 1;
  msgsnd(msqid,&mess_q,sizeof(mess_q)-sizeof(long),0);
}
}

void wait_for_childs()
{
  if(pid > 0){

  }
  for (int i = 0; i < confInfo->n_workers + 1; i++)
  {
    wait(NULL);
  }
}

void clean(int signum){
  printf("CTRL-C Pressed...\n");
  if(pid>0){
  logging("HOME_IOT SIMULATOR WAITING FOR LAST TASKS TO FINISH");
}
  wait_for_childs();
  shmdt(shm);
  shmctl(shmid, IPC_RMID, NULL);
  unlink(consolePipe);
  unlink(sensorPipe);
  for (int i = 0; i < confInfo->n_workers; i++) {
    free(channel[i]);
  }
  free(channel);
  free(confInfo);
  msgctl(msqid,IPC_RMID,NULL);
  sem_close(file_mutex);
  sem_unlink("file_write");
  fclose(logfile);
  pthread_join(dispacher, NULL);
  pthread_join(sensorReader, NULL);
  pthread_join(consoleReader, NULL);
  exit(0);
}

void end_it_all()
{
  shmdt(shm);
  shmctl(shmid, IPC_RMID, NULL);
  unlink(consolePipe);
  unlink(sensorPipe);
  for (int i = 0; i < confInfo->n_workers; i++) {
    free(channel[i]);
  }
  free(channel);
  free(confInfo);
  msgctl(msqid,IPC_RMID,NULL);
  sem_close(file_mutex);
  sem_unlink("file_write");
  sem_close(shm_sem);
  sem_unlink("shm_mutex");
  sem_close(worker_sem);
  sem_unlink("worker_mutex");
  fclose(logfile);
  pthread_join(dispacher, NULL);
  pthread_join(sensorReader, NULL);
  pthread_join(consoleReader, NULL);
}



void init_alerts(){
  for(int h= 0; h<confInfo->max_alerts;h++){
    shm->alerts[h].id[0] = '\0';
    shm->alerts[h].key[0] = '\0';
    shm->alerts[h].min = 0;
    shm->alerts[h].max = 0;
  }
}

int main(int argc, char **argv)
{

  if (argc == 3)
  {
    if (strcmp(argv[1], "home_iot") == 0)
    {
      //printf("MY PID IS: %d\n", getpid());
      char *ConfName;
      ConfName = (char *)malloc(100 * sizeof(char));
      ConfName = argv[2];
      conf_Attribution(ConfName);
      channel = malloc(confInfo->n_workers * sizeof(int*));
      for(int i = 0; i<confInfo->n_workers; i++){
        channel[i] = malloc(2 * sizeof(int));
      }
      int_queue = malloc(confInfo->queue_size * sizeof(internal_queue));
      logfile = fopen("log.txt", "w");
      logfile = fopen("log.txt", "a");

      create_Sem();
      create_shared_mem();
      create_pipes();
      create_Message_Queue();
      logging("SIMULATOR STARTING");

      //printf("This is the system Manager!!! \n");

      //signal(SIGINT,clean);

      shm->start = true;

      create_Threads();
      for (int i = 0; i < confInfo->n_workers; i++)
      {
        create_unnamed_pipes(i);
        pid = fork();
        if (pid == 0)
        {
          char str[14];
          char num[2];
          internal_queue aux;
          strcpy(str, "WORKER ");
          sprintf(num,"%d",i+1);
          strcat(str,num);
          strcat(str," READY");

          shm->workers[i] = 0;

          logging(str);
          close(channel[i][1]);
          while(true){


          int n = read(channel[i][0], &aux, sizeof(internal_queue));
          if(n == -1){
            perror("ERRO A LER  UNNAMED PIPE!!!!\n");
            exit(0);
          }

            //printf("[WORKER %s] Received (%s) from master to add.\n",num,aux.message);
            if(aux.type == 2){
              sem_wait(shm_sem);
              char temporary[bufferLength];
              strcpy(temporary,aux.message);
              char *s = strtok(temporary,"#");
              char *k = strtok(NULL,"#");
              char l[bufferLength];
              strcpy(l,"DISPATCHER: ");
              strcat(l,k);
              strcat(l," DATA (FROM ");
              strcat(l,s);
              strcat(l," SENSOR) SENT FOR PROCESSING ON WORKER ");
              strcat(l,num);
              logging(l);
              addSensorInfo(aux.message);
              char l2[bufferLength];
              strcpy(l2,"WORKER");
              strcat(l2,num);
              strcat(l2,": ");
              strcat(l2,k);
              strcat(l2," DATA PROCESSING COMPLETED");
              logging(l2);
              sem_post(shm_sem);
            }else if(aux.type == 1){

              char l[bufferLength];
              strcpy(l,"DISPATCHER: ");
              strcat(l,aux.message);
              strcat(l," SENT FOR PROCESSING ON WORKER ");
              strcat(l,num);
              logging(l);
              console_print(aux.message);
              char l2[bufferLength];
              strcpy(l2,"WORKER");
              strcat(l2,num);
              strcat(l2,": ");
              strcat(l2,aux.message);
              strcat(l2," DATA PROCESSING COMPLETED");
              logging(l2);
            }

            /*for(int i =0; i< confInfo->max_keys;i++){
              if(shm->keys[i].key){
                printf("ITERAÇAO %d\n",i);
                printf("INFORMATION IN KEYS :%s ; %d; %d; %d; %d; %d; %d; %d; %d;", shm->keys[i].key,shm->keys[i].min,shm->keys[i].max,shm->keys[i].lastValue,shm->keys[i].minValue,shm->keys[i].maxValue,shm->keys[i].mean,shm->keys[i].updates,shm->keys[i].sum);
              }else{
                printf("DEU BREAK\n");
                break;
              }

            }*/

          shm->workers[i] = 0;
          char w[bufferLength];
          strcpy(w,"WORKER ");
          strcat(w,num);
          strcat(w," AVAILABLE");
          sem_post(worker_sem);

        }

          //printf("%d : I'm a child/worker process with a pid of %d and my dad is %d\n", i + 1, getpid(), getppid());
          close(channel[i][0]);
          exit(0);
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
          //init_alerts();
          /*for(int j = 0; j<confInfo->max_alerts;j++){
            if(shm->alerts[j].id[0] != '\0'){
              printf("ID ALERTS : %s no indice %d\n", shm->alerts[j].id,j);
            }
          }*/
          while(true){
            message_queue mess;
            char str[bufferLength];
            sem_wait(shm_sem);
            for(int i = 0; i<confInfo->max_alerts;i++){
              if(i != 0 || i!= 2){
              for(int k = 0; k< confInfo->max_keys;k++){
                if(shm->keys[k].key[0] != '\0' && shm->alerts[i].id[0] != '\0' && strcmp(shm->alerts[i].key,shm->keys[k].key) == 0){
                  if(shm->keys[k].lastValue > shm->alerts[i].max){
                    strcpy(str,"ALERTA: ");
                    strcat(str,shm->keys[k].key);
                    strcat(str," obteve um valor superior a ");
                    char m[5];
                    sprintf(m,"%d",shm->alerts[i].max);
                    strcat(str,m);
                    strcat(str," : ");
                    sprintf(m,"%d",shm->keys[k].lastValue);
                    strcat(str,m);
                    strcat(str," \n");
                    strcpy(mess.temp,str);
                    mess.msgtype = 2;
                    msgsnd(msqid,&mess,sizeof(mess)-sizeof(long),0);
                    break;
                  }
                  else if(shm->keys[k].lastValue < shm->alerts[i].min){
                    strcpy(str,"ALERTA: ");
                    strcat(str,shm->keys[k].key);
                    strcat(str," obteve um valor inferior a ");
                    char m[5];
                    sprintf(m,"%d",shm->alerts[i].max);
                    strcat(str,m);
                    strcat(str," : ");
                    sprintf(m,"%d",shm->keys[k].lastValue);
                    strcat(str,m);
                    strcat(str," \n");
                    strcpy(mess.temp,str);
                    mess.msgtype = 2;
                    msgsnd(msqid,&mess,sizeof(mess)-sizeof(long),0);
                    break;
                  }
                }
              }
            }
            }
            sem_post(shm_sem);
          }

        /*  message_queue mesqueue;
          mesqueue.msgtype = 1;
          strcpy(mesqueue.temp, "20");
          msgsnd(msqid,&mesqueue,sizeof(mesqueue)-sizeof(long),0);
          printf("SENT THIS INFO THROUGH MESSAGE QUEUE: %s\n",mesqueue.temp);*/
          //printf("I'm the alerts child process with a father with the id of %d\n", getppid());
          exit(0);
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

      if(pid > 0 ){
      wait_for_childs();
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
