#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
FILE *logfile;
int main()
{
    time_t clock;
    struct tm *timeinfo;
    logfile = fopen("test.txt", "a+");
    for (int i = 0; i < 2; i++)
    {
        time(&clock);
        timeinfo = localtime(&clock);
        char logger[100];
        sprintf(logger, "%d:%d:%d\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
        sleep(1);
        fprintf(logfile, "%s",logger);
        printf("%s", logger);
    }
    return 0;
}