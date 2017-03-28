#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

static void pabort(const char *s){
    perror(s);
    abort();
}


int main(){
    int nBytes = 4097;
    char buff[nBytes];
    int ret = -1;
    int i = 0;
    // SPI settings
    int bits = 8;
    int mode = SPI_CPHA|SPI_CPOL;
    int speed = 20000000;

    // initialize array to zero
    for(i=0;i<nBytes;i++){
        buff[i]=0;
    }
    // open spi port
    int fd = open("/dev/spidev0.0",O_RDWR);
    // check if spi port is actually open
    if(fd<0)
        pabort("can't open port");
    // set spi mode
    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1)
        pabort("can't set spi mode");
    // read spi mode
    ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
    if (ret == -1)
        pabort("can't get spi mode");
    // set bits per word
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1)
        pabort("can't set bits per word");
    // read bits per word
    ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret == -1)
        pabort("can't get bits per word");
    // set max speed hz
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1)
        pabort("can't set max speed hz");
    // read max speed hz
    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret == -1)
        pabort("can't get max speed hz");
    // print spi settings
    printf("spi mode: %d\n", mode);
    printf("bits per word: %d\n", bits);
    printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
    printf("buffer contents:\n");
    // read bytes from spi bus
    read(fd,buff,nBytes);
    // print bytes to console
    for(i=0;i<nBytes;i++){
        printf("%x\n",buff[i]);
    }
    // close spi bus
    ret = close(fd);
    if(ret != 0)
        pabort("Could not close file");
    return 0;
}

