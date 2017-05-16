#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

// number of GPS data lines to conglomerate
#define NLINES 16
#define BYTES_LINE 38

int main(){
  char inFilename[20];
  char outFilename[20];
  int inFile,outFile;
  int nFiles = 0;
  int i = 0;
  // create filenames
  snprintf(inFilename,20,"Gps.dat");
  snprintf(outFilename,20,"Gps_%d.tmp",nFiles);
  // open input file
  inFile = open(inFilename, O_RDONLY);
  if (inFile<0){
    printf("Failed to open input file\n");
    return 0;
  }
  // open input file
  outFile = open(outFilename,O_WRONLY|O_CREAT|O_TRUNC);
  if (outFile<0){
    printf("Failed to open output file\n");
    close(inFile);
    return 0;
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
}
