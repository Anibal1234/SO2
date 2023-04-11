#ifndef UNTITLED1_STRUCTS_H
#define UNTITLED1_STRUCTS_H

// Anibal Rodrigues 2019224911
//Guilherme Junqueira 2019221958

#include <stdbool.h>


typedef struct Shared {
    bool start;
    int queue_size;
    int n_workers;
    int max_keys;
    int max_sensors;
    int max_alerts;
} shm_t;

shm_t* shm;

typedef struct Sensor{
  char id[32];
  int interval;
  char key[32];
  int min;
  int max;
} sensor;

char ConsoleID[32];


#endif //UNTITLED1_STRUCTS_H
