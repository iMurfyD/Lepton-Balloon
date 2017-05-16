#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <signal.h>
 
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

static int fd;
static int wd;

void intHandler(int dummy){
  printf("Exiting...\n");
  // remove watcher and inotify instance
  ( void ) inotify_rm_watch( fd, wd );
  ( void ) close( fd );
  // exit
  exit( 0 );
}
 
int main( int argc, char **argv ) 
{
  signal(SIGINT,intHandler);
  int length, i = 0;
  int j;
  char buffer[BUF_LEN];
  // allocate an inotify instance
  fd = inotify_init();
  // ensure inotify could be allocated
  if ( fd < 0 ) {
    perror( "inotify_init" );
  }
  // initialize event watcher
  wd = inotify_add_watch( fd, ".",IN_CREATE);
  while(1){
    // blocks until event occurs
    length = read( fd, buffer, BUF_LEN );  
    // ensure read did not fail
    if ( length < 0 ) {
      perror( "read" );
    }  
    // iterate through all events
    i=0;
    while ( i < length ) {
      struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
      if ( event->len ) {
        // if file was created
        if ( event->mask & IN_CREATE ) {
          // check if file was directory
          if ( event->mask & IN_ISDIR ) {
            printf( "The directory %s was created.\n", event->name );       
          }
          else {
            printf( "The file %s was created.\n", event->name );
            j=0;
            char temp = event->name[j];
            // get length of string
            while(temp != '\0'){
              j++;
              temp = event->name[j];
            }
            if(event->name[j-1]=='p' && event->name[j-2] == 'm' && event->name[j-3] == 't' && event->name[j-4]=='.'){
              printf("Valid file.\n");
            }
            else{
              printf("Invalid.\n");
            }
          }
        }
      }
      // increment byte counter by event size
      i += EVENT_SIZE + event->len;
    }
  }
}
