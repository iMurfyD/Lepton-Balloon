#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

void intHandler(int dummy){
  printf("Exiting...\n");
  exit(0);
}

int main(){
  char command[32];
  int i = 0;
  FILE *fp;
  // register intHandler as sigint handler
  signal(SIGINT,intHandler);
  // remove all .tmp files
  // create command
  snprintf(command,32,"rm -f *.tmp");
  // execute rm
  fp = popen(command,"r");
  // wait
  pclose(fp);
  // continuously monitor for files
  while(1){
    // create command
    snprintf(command,32,"./getfile -o %d.tmp",i);
    // execute getFile
    fp = popen(command,"r");
    // wait for file
    pclose(fp);
    // increment file counter
    i++;
    // wait a bit
    usleep(1000);
  }
}
