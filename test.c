#include <stdio.h>
#include <time.h>
#include <string.h>
FILE *log;
int main()
{
    time_t clock;
    struct tm *timeinfo;
    log = fopen("test.txt", "a+");
    for (int i = 0; i < 9; i++)
    {
        time(&clock);
        timeinfo = localtime(&clock);
        char logger[]= ("%d:%d:%d\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
        //strcpy(logger, ("%d:%d:%d\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec));
        
        fprintf(log, logger);
        puts(logger);
        sleep(1);
    }
    return 0;
}