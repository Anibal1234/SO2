#ifndef UNTITLED1_STRUCTS_H
#define UNTITLED1_STRUCTS_H

// Anibal Rodrigues 2019224911
//Guilherme Junqueira 2019221958

#include <stdbool.h>

#define bufferLength 255


typedef struct key{
  char key[32];
  int min;
  int max;
  int lastValue;
  int minValue;
  int maxValue;
  int mean;//provavelmente sera necessario uma forma de guardar os valores, maybe um atribute que os vai mantendo somados
  int updates;
  int sum;
}keys_t;


typedef struct Sensor{
  int interval;
  char id[32];
  keys_t* keys;

} sensor_t;


typedef struct Worker{
    int state;
} worket_t;

typedef struct Data{ //Struct usado para guardar as informaçoes de config.txt
    int queue_size;
    int n_workers;
    int max_keys;
    int max_sensors;
    int max_alerts;

} data_t;

typedef struct {
  long msgtype;
  char temp[bufferLength];
}message_queue;

extern data_t* confInfo;

typedef struct Shared {
    bool start;
    int queue_size;
    int n_workers;
    int max_keys;
    int max_sensors;
    int max_alerts;
    sensor_t* sens;
} shm_t;

shm_t* shm;

char ConsoleID[32];


#endif //UNTITLED1_STRUCTS_H
