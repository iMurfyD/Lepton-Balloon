#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(){
int file;
// probably dynamically determined
int adapter_nr = 1;
char filename[20];
// output buffer
unsigned char outbuf[1];
// input buffer
unsigned char inbuf[2];
// message structs
struct i2c_rdwr_ioctl_data packets;
struct i2c_msg messages[2];

// construct filename for i2c-dev interface
snprintf(filename,19,"/dev/i2c-%d",adapter_nr);
// open file
file = open(filename, O_RDWR);
if (file<0){
  // check errno to see what went wrong
  exit(1);
}

int addr = 0x50; // i2c address to talk to
outbuf[0] = 0x00;

// output struct
messages[0].addr = addr;
messages[0].flags = 0;
messages[0].len = sizeof(outbuf);
messages[0].buf = outbuf;

// input struct
messages[1].addr = addr;
messages[1].flags = I2C_M_RD/* | I2C_M_NOSTART*/;
messages[1].len = sizeof(inbuf);
messages[1].buf = inbuf;

// send request to kernel
packets.msgs = messages;
packets.nmsgs = 2;
if(ioctl(file, I2C_RDWR, &packets) < 0){
  // unable to send data
  printf("Unable to send data\n");
  return 1;
}

printf("Recieved: %x, %x\n",inbuf[0],inbuf[1]);

// close file and exit
close(file);
exit(0);
}
