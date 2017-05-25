#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <glob.h>

#define MAXFILESIZE 1000
#define REDUNDANCY 0.2
#define MINFILES 2

int main(int argc, char **argv){
  int fileSize,nFiles,rFiles;
  char *inFilename = NULL;
  char command[128];
  int inFile;
  int index;
  int c;
  FILE *fp;
  int useCp = 0;
  int ret = 0;
  char pattern[32];
  glob_t globbuf;

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
  if(rFiles < 2){
    useCp = 1;
  }
  // get base file name
  char baseName[32];
  int i = 0;
  //printf("%s\n",inFilename);
  while(inFilename[i] != '\0'){
    baseName[i] = inFilename[i];
    //printf("%c",inFilename[i]);
    i++;
  }
  baseName[i-4] = '\0';
  //printf("Base Name: %s\n",baseName);
  // check if there are already fec files
  snprintf(command,128,"%s.*.fec",baseName);
  globbuf.gl_offs = 1;
  ret = glob(command,GLOB_DOOFFS,NULL,&globbuf);
  // if there are files remove them
  if(ret != GLOB_NOMATCH){
    // create rm command
    snprintf(command,128,"rm %s.*.fec",baseName);
    // call rm
    fp = popen(command,"r");
    pclose(fp);
  }
  // create zfec command
  if(useCp == 1){
    snprintf(command,128,"cp %s %s.0_1.fec",inFilename,baseName);
  }
  else{
    snprintf(command,128,"zfec -m %d -k %d -p %s %s",nFiles,rFiles,baseName,inFilename);
  }
  // call zfec
  fp = popen(command,"r");
  //if(fp<0){
  //  perror("zfec failed");
  //}
  // close fp (waits for zfec to finish)
  pclose(fp);
  // create rm command
  snprintf(command,128,"rm %s",inFilename);
  // call rm
  fp = popen(command,"r");
  pclose(fp);
}
