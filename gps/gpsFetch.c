#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>

// number of GPS data lines to conglomerate
#define NLINES 16
#define BYTES_LINE 38

// interrupt handler
void intHandler(int dummy){
  printf("Exiting...\n");
  exit(0);
}

int main(){
  signal(SIGINT,intHandler);
  char inFilename[32];
  char outFilename[32];
  int inFile,outFile;
  int nFiles = 0;
  int i = 0;
  while(1){
    // create filenames
    snprintf(inFilename,32,"/balloonLogs/Gps.dat");
    snprintf(outFilename,32,"/downlinkStaging/Gps_%d.tmp",nFiles);
    // open input file
    inFile = open(inFilename, O_RDONLY);
    if (inFile<0){
      printf("Failed to open input file\n");
      return -1;
    }
    // open input file
    outFile = open(outFilename,O_WRONLY|O_CREAT|O_TRUNC);
    if (outFile<0){
      printf("Failed to open output file\n");
      close(inFile);
      return -1;
    }
    // seek to end of file
    lseek(inFile,-NLINES*BYTES_LINE,SEEK_END);
    // read last NLINES lines of gps data file and log
    char temp = NULL;
    for(i=0;i<NLINES;i++){
      printf("%d\n",i);
      while((temp != '\n')&&(read(inFile,&temp,1)>0)){
        write(outFile,&temp,1);
      }
      temp = ' ';
    }
    // close files
    close(inFile);
    close(outFile);
    // increment file number
    nFiles++;
    // wait a bit
    sleep(16);
  }
}
