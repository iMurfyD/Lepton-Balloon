// Transfers a binary file to arduino over I2C for downlink

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "../downlink/gpio.h"

#define ADDR 0x0F
#define BUSYPIN 27
#define RSTPIN 22
//#define CHUNK_SIZE 16
#define BUFSIZE 1024

// commands
#define READ 0
#define PEEK 1
#define CHECKEOF 2

int main(int argc, char **argv){
  //uint8_t buf[CHUNK_SIZE];
  char *outFilename = NULL;
  char i2cFilename[20];
  int I2C,outFile;
  int index;
  int c;
  int end = 0;

  opterr = 0;
  // parse arguments
  while ((c = getopt (argc, argv, "o:")) != -1)
    switch (c)
      {
      case 'o':
        outFilename = optarg;
        break;
      case '?':
        if (optopt == 'o')
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
  if(outFilename == NULL){
    printf("Output file required\n");
    return 0;
  }
  // print file name for debugging
  printf ("outFilename = %s\n",outFilename);

  // open input file
  outFile = open(outFilename,O_WRONLY|O_CREAT|O_TRUNC);
  if (outFile<0){
    printf("Failed to open output file\n");
    return 0;
  }
  // open i2c interface
  snprintf(i2cFilename,19,"/dev/i2c-%d",1);
  I2C = open(i2cFilename, O_RDWR);
  if (I2C<0){
    printf("Failed to initialize I2C Bus\n");
    // close input file
    close(outFile);
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

  uint8_t command[3];
  uint8_t data[2];
  uint8_t dataBuf[BUFSIZE];
  uint16_t nBytes = 0;
  uint8_t newFile = 0;
  // iterate until EOF
  while(!newFile){
   // check available data
   command[0]= PEEK;
   write(I2C,command,1);
   read(I2C,data,2);
   nBytes = (data[0] << 8) | data[1];
   // if there is data
   if(nBytes>0){
     printf("got %d bytes\n",nBytes);
     // write read data command
     command[0] = READ;
     command[1] = data[0];
     command[2] = data[1];
     write(I2C,command,3);
     // read data
     nBytes = read(I2C,dataBuf,nBytes);
     // write data to file 
     write(outFile,dataBuf,nBytes);
   }
   // check if EOF
   else{
     command[0] = CHECKEOF;
     write(I2C,command,1);
     read(I2C,data,1);
     if(data[0]==1){
       // raise EOF flag
       newFile = 1;
       printf("New File!\n");
     }
   }
   // prevent loop from becoming overly fast
   usleep(5000);
  }

  // unregister gpios
  gpioUnexport(BUSYPIN);
  gpioUnexport(RSTPIN);
  // close i2c interface
  close(I2C);
  // close input file
  close(outFile);
}
