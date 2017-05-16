#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/inotify.h>
//#include <inotify-syscalls.h>

int main(){
  int wd,fd;
  fd = inotify_init();
  if(fd<0){
    printf("Could not instantiate inotify");
    return 0;
  }
  wd = inotify_add_watch(fd,".",IN_CREATE);
  if(wd<0){
    printf("Could not add watch");
    return 0;
  }
}
