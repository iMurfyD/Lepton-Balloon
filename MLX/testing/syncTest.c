#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

void intHandler(int dummy){
  printf("Exiting...\n");
  exit(0);
}

int main(){
  signal(SIGINT,intHandler);
  time_t rawtime;
  struct tm *timeinfo;
  while(1){
    // get time
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    // check if time is on an even 10s
    if(timeinfo->tm_sec % 10 == 0){
      // print time
      printf("Current Time: %d:%d:%d\n",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
      // wait 2 seconds
      sleep(2);
    }
    usleep(500000);
  }
}

