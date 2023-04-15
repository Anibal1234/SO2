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
    printf("You're in stats!!!\n");
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
      console_Menu();

    }
  }else
  {
    printf("Invalid Option\n");
    exit(0);
  }

}
