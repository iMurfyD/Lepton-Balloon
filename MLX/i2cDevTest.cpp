#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(){
int file;
// probably dynamically determined
int adapter_nr = 1;
char filename[20];

// construct filename for i2c-dev interface
snprintf(filename,19,"/dev/i2c-%d",adapter_nr);
// open file
file = open(filename, O_RDWR);
if (file<0){
  // check errno to see what went wrong
  exit(1);
}

int addr = 0x50; // i2c address to talk to
char buf[10]; // buffer for i2c traffic

// set device as slave
if (ioctl(file, I2C_SLAVE, addr) < 0){
  // check errno to see what went wrong
}

// read a byte
if (read(file, buf, 1) != 1){
  // i2c transaction failed
}
else{
  printf("Recieved %X",buf[0]);
}

// close file and exit
close(file);
exit(0);
}
