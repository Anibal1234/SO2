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
#define consolePipe "/tmp/CONSOLE_PIPE"
#define bufferLength 255
#define QUEUE_KEY 123

char write_info[bufferLength];
int msqid;

void sendInfo(){
  int fd;
  if((fd = open(consolePipe,O_WRONLY))<0){
      perror("ERROR OPENING CONSOLE PIPE FOR WRITING!!!\n");
      exit(0);
  }

  sprintf(write_info, "INFORMATION WRITE ON CONSOLE PIPE");
  if(write(fd, write_info, sizeof(write_info)) == -1){
    perror("ERROR WRITING IN CONSOLE NAMED PIPE!!!\n");
  }
  printf("INFO WROTE IN CONSOLE NAMED PIPE!\n");

}

void create_messageQueue(){
  if((msqid = msgget(QUEUE_KEY, IPC_CREAT | 0700)) == -1){
    perror("ERROR ON CREATION OF MESSAGE QUEUE!!!\n");
    exit(0);
  }
}

void console_Menu()
{
  printf("\t MENU \t\n");
  printf("Write the option u want!!!\n");
  printf(" 1: EXIT\n 2: STATS\n 3: RESET\n 4: SENSORS\n 5: ADD ALERT\n 6: REMOVE ALERT\n 7: LIST ALERTS\n");
  char choice[15];
  fgets(choice, sizeof(choice), stdin);
  if (strcmp(choice, "exit\n") == 0)
  {
    exit(0);
  }
  else if (strcmp(choice, "stats\n") == 0)
  {
    sendInfo();
    printf("You're in stats!!!\n");
    message_queue msg;
    printf("KKKKKK\n");
    msgrcv(msqid,&msg,sizeof(msg)-sizeof(long),0,0);
    printf("ok\n");
    printf("I GOT THIS MESSAGE : %d \n", msg.temp);

    console_Menu();
  }
  else if (strcmp(choice, "reset\n") == 0)
  {
    printf("You're in reset!!!\n");
    console_Menu();
  }
  else if (strcmp(choice, "sensors\n") == 0)
  {
    printf("You're in sensors!!!\n");
    console_Menu();
  }
  else if (strcmp(choice, "add alert\n") == 0)
  {
    printf("You're in add alert!!!\n");
    console_Menu();
  }
  else if (strcmp(choice, "remove alert\n") == 0)
  {
    printf("You're in remove alert!!!\n");
    console_Menu();
  }
  else if (strcmp(choice, "list alerts\n") == 0)
  {
    printf("You're in list alerts!!!\n");
    console_Menu();
  }
  else
  {
    printf("Invalid option, try again\n");
    console_Menu();
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
      create_messageQueue();
      console_Menu();

    }
  }else
  {
    printf("Invalid Option\n");
    exit(0);
  }

}
