#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(){
    int nBytes = 4097;
    char buff[nBytes];
    int error = 0;
    int i = 0;
    for(i=0;i<nBytes;i++){
        buff[i]=0;
    }
    int fd = open("/dev/spidev0.0",O_RDWR);
    read(fd,buff,nBytes);
    printf("buffer contents:\n");
    for(i=0;i<nBytes;i++){
        printf("%x\n",buff[i]);
    }
    error = close(fd);
    if(error != 0){
        printf("Could not close file! error: %d",error);
    }
    return 0;
}
