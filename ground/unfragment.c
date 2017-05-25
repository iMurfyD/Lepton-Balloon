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
  int fileSize,nFile,nFiles;
  char *inFilename = NULL;
  char command[128];
  char baseName[64];
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
  // extract base filename
  // remove .fec
  int i = 0;
  while(1){
    if(inFilename[i]=='.' && inFilename[i+1]=='f' && inFilename[i+2]=='e' &&  inFilename[i+3]=='c'){
      break;
    }
    baseName[i] = inFilename[i];
    i++;
  }
  // remove numbering
  if(baseName[i-2]=='_'){
    // single number
    baseName[i-4] = '\0';
    // get current file number
    nFile = baseName[i-3] - 48;
    // get toal number of files
    nFiles = baseName[i-1] - 48;
  }
  else if(baseName[i-3]=='_'){
    // double number
    baseName[i-6] = '\0';
    // get current file number
    nFile = baseName[i-5] + baseName[i-4]*10 - 48;
    // get toal number of files
    nFiles = baseName[i-2] + baseName[i-1]*10 - 48;
  }
  // print numbers for debugging
  printf("nFiles:%d nFile:%d \n",nFiles,nFile);
  // check if it is time to recombine
  if(nFiles - nFile < 2){
    // create zfec command
    if(nFiles < 2){
      snprintf(command,128,"mv %s %s",inFilename,baseName);
    }
    else{
      snprintf(command,128,"zunfec -o %s %s*",baseName,baseName);
    }
    printf("%s\n",command);
    // call zfec
    fp = popen(command,"r");
    //if(fp<0){
    //  perror("zfec failed");
    //}
    // close fp (waits for zfec to finish)
    pclose(fp);
    // create rm command
    snprintf(command,128,"rm %s.*",baseName);
    // call rm
    fp = popen(command,"r");
    pclose(fp);
  }
}
