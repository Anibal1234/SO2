#ifndef UNTITLED1_STRUCTS_H
#define UNTITLED1_STRUCTS_H

// Anibal Rodrigues 2019224911
//Guilherme Junqueira 2019221958

#include <stdbool.h>

#define bufferLength 255
#define messageLength 1024

typedef struct intData{
  char message[bufferLength];
  int type;
}internal_queue;


typedef struct key{
  char key[33];
  int min;// para o sensor
  int max;// para o sensor
  int lastValue;
  int minValue;
  int maxValue;
  int mean;//provavelmente sera necessario uma forma de guardar os valores, maybe um atribute que os vai mantendo somados
  int updates;
  int sum;
}keys_t;


typedef struct Sensor{
  int sent;
  int interval;
  char id[33];

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
  char temp[messageLength];
}message_queue;

extern data_t* confInfo;

typedef struct Shared {
    bool start;
    int queue_size;
    int n_workers;
    int max_keys;
    int max_sensors;
    int max_alerts;
    int *workers;
    sensor_t* sens;
    keys_t *keys;
} shm_t;

shm_t* shm;

internal_queue* int_queue;

char ConsoleID[33];


#endif //UNTITLED1_STRUCTS_H
