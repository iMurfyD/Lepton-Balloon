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
#define BYTES_LINE 23

// interrupt handler
void intHandler(int dummy){
  printf("Exiting...\n");
  exit(0);
}

int main(){
  signal(SIGINT,intHandler);
  char inFilename[32];
  char outFilename[64];
  char dataBuf[NLINES*BYTES_LINE];
  int inFile,outFile;
  int nFiles = 0;
  int i = 0;
  int k = 0;
  int emptyLog = 0;
  while(1){
    // create filenames
    snprintf(inFilename,32,"/balloonLogs/MS5637.dat");
    snprintf(outFilename,64,"/downlinkStaging/MS5637_%d.tmp",k);
    // open input file
    inFile = open(inFilename, O_RDONLY);
    if (inFile<0){
      printf("Failed to open input file\n");
      return -1;
    }
    // seek to end of file - ammount to read
    lseek(inFile,-NLINES*BYTES_LINE,SEEK_END);
    // try to read NLINES lines of gps data
    if(read(inFile,&dataBuf,NLINES*BYTES_LINE) == NLINES*BYTES_LINE){
      // open output file
      outFile = open(outFilename,O_WRONLY|O_CREAT|O_TRUNC);
      if (outFile<0){
        printf("Failed to open output file\n");
        close(inFile);
        return -1;
      }
      // write data to output file
      write(outFile,&dataBuf,NLINES*BYTES_LINE);
      // close output file
      close(outFile);
    }
    else{
      printf("Empty File\n");
    }
    // close infile
    close(inFile);
    // wait a bit
    sleep(16);
    k++;
  }
}
