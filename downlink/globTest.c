#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glob.h>

int main(){
  int ret = 0;
  // create pattern to match
  char pattern[32];
  snprintf(pattern,32,"*.fec");
  // glob struct
  glob_t globbuf;
  // number of files to search for at maximum
  globbuf.gl_offs = 1;
  // actually call glob
  ret = glob(pattern,GLOB_DOOFFS,NULL,&globbuf);
  // check return
  if(ret == GLOB_NOMATCH){
    printf("No Match\n");
  }
  // check if there are any matching files
  if(globbuf.gl_pathc > 0){
    printf("Found matching file\n");
  }
  else{
    printf("No matching file found.\n");
  }
}
