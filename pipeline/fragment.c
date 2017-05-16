#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>

#define MAXFILESIZE 1000
#define REDUNDANCY 0.2
#define MINFILES 2

int main(int argc, char **argv){
  int fileSize,nFiles,rFiles;
  char *inFilename = NULL;
  char command[64];
  int inFile;
  int index;
  int c;
  FILE *fp;

  // parse args

  opterr = 0;
  // parse arguments
  while ((c = getopt (argc, argv, "i:")) != -1)
    switch (c)
      {
      case 'i':
        inFilename = optarg;
        break;
      case '?':
        if (optopt == 'i')
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
        return 1;
      default:
        abort ();
      }
  // check if input file was specified
  if(inFilename == NULL){
    printf("Input file required\n");
    return 0;
  }

  // open input file
  inFile = open(inFilename,O_RDONLY);
  if(inFile<0){
    perror("could not open input file");
  }
  // check file size
  fileSize = lseek(inFile,0,SEEK_END);
  // close file
  close(inFile);
  // decide how many files to make
  nFiles = fileSize/MAXFILESIZE;
  if(nFiles < MINFILES){
    nFiles = MINFILES;
  }
  // decide how many files we need
  rFiles = nFiles*(1.0-REDUNDANCY);
  if(rFiles < 1){
    rFiles = 1;
  }
  // create zfec command
  snprintf(command,64,"zfec -m %d -k %d %s",nFiles,rFiles,inFilename);
  // call zfec
  fp = popen(command,"r");
  //if(fp<0){
  //  perror("zfec failed");
  //}
  // close fp (waits for zfec to finish)
  pclose(fp);
}
