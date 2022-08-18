#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int main(int argc,char *argv[]){

    if(argc != 2){
        printf("usage : stat <pid>\n");
        exit(1);
    }
    int pid = -1;
    char path[128];
    sprintf(path,"/proc/%s/stat",argv[1]);
    printf("%s\n",path);
    char str[100];
    char ch;
    unsigned dummy;
    unsigned long dummy1;
    unsigned long utime_before;
    unsigned long stime_before;

    unsigned long utime_after;
    unsigned long stime_after;
    FILE *stat_proc = fopen(path,"r");
    if(stat_proc == NULL){
        printf("%s can not be opened\n",path);
        exit(EXIT_FAILURE);
    }
    FILE *stat = fopen("/proc/stat","r");
    fscanf(stat_proc,
    "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu"\
    ,&pid,str,&ch,&pid,&pid,&pid,&pid,&pid,&dummy,&dummy1,
    &dummy1,&dummy1,&dummy1,&utime_before,&stime_before);

    // printf("%lu\n%lu\n================\n",utime,stime);

    fscanf(stat,"%s",str);
    unsigned long ttime_before;
    unsigned long totaltime_before=0;
    int i=0;
    while(i<10){
        fscanf(stat,"%lu",&ttime_before);
        totaltime_before += ttime_before;
        i++;
    }
    printf("total :%lu\n==========================\n",totaltime_before);
    
    
    sleep(1);
    
    
    fseek(stat_proc,0,SEEK_SET);
    fseek(stat,0,SEEK_SET);
    fscanf(stat_proc,
    "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu"\
    ,&pid,str,&ch,&pid,&pid,&pid,&pid,&pid,&dummy,&dummy1,
    &dummy1,&dummy1,&dummy1,&utime_after,&stime_after);

    fscanf(stat,"%s",str);
    unsigned long ttime_after=0;
    unsigned long totaltime_after=0;
    i=0;
    while(i<10){
        fscanf(stat,"%lu",&ttime_after);
        totaltime_after += ttime_after;
        i++;
    }
    printf("total :%lu\n==========================\n",totaltime_after);
    double upercent = 100.0 * (utime_after-utime_before) / (double)(totaltime_after-totaltime_before);
    printf("User CPU Percentage: %lf\n",upercent);


    return 0;
}