// Transfers a binary file to arduino over I2C for downlink

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "gpio.h"

#define ADDR 0x0F
#define BUSYPIN 27
#define RSTPIN 22
#define CHUNK_SIZE 16

int main(int argc, char **argv){
  uint8_t buf[CHUNK_SIZE];
  char *inFilename = NULL;
  char i2cFilename[20];
  int I2C,inFile;
  int index;
  int c;
  int nBytes;
  int end = 0;

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
  // print file name for debugging
  printf ("inFilename = %s\n",inFilename);

  // open input file
  inFile = open(inFilename, O_RDONLY);
  if (inFile<0){
    printf("Failed to open input file\n");
    return 0;
  }
  // open i2c interface
  snprintf(i2cFilename,19,"/dev/i2c-%d",1);
  I2C = open(i2cFilename, O_RDWR);
  if (I2C<0){
    printf("Failed to initialize I2C Bus\n");
    // close input file
    close(inFile);
    return 0;
  }

  // get gpio
  gpioExport(BUSYPIN);
  gpioExport(RSTPIN);
  // set direction
  gpioDirection(BUSYPIN,GPIO_IN);
  gpioDirection(RSTPIN,GPIO_OUT);
  gpioSet(RSTPIN,1);
  printf("Got GPIO\n");

  // set device address
  if (ioctl(I2C, I2C_SLAVE, ADDR) < 0){
    // could not set device as slave
    printf("Could not find device\n");
    return 0;
  }

  // iterate through input file
  while(end<1){
    //printf(".");
    nBytes = read(inFile, buf, CHUNK_SIZE);
    if(nBytes < CHUNK_SIZE){
      end = 1;
    }
    // wait until arduino is no longer busy
    while(gpioGet(BUSYPIN)){
      usleep(10);
    }
    write(I2C,buf,nBytes);
  }
  //printf("\n");

  // unregister gpios
  gpioUnexport(BUSYPIN);
  gpioUnexport(RSTPIN);
  // close i2c interface
  close(I2C);
  // close input file
  close(inFile);
}
