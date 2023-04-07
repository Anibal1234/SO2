#ifndef UNTITLED1_STRUCTS_H
#define UNTITLED1_STRUCTS_H

#include <stdbool.h>


typedef struct Shared {
    int treta1;
    int treta2;
} shm_t;

shm_t* shm;

typedef struct Sensor{
  char id[32];
  int interval;
  char key[32];
  int min;
  int max;
} sensor;

#endif //UNTITLED1_STRUCTS_H
