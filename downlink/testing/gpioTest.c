#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>

#define GPIO_OUT 1
#define GPIO_IN 0


void gpioExport(int gpio);
void gpioUnexport(int gpio);
void gpioDirection(int gpio, int direction);
void gpioSet(int gpio, int value);
int gpioGet(int gpio);


int main(){
  int gpio = 27;
  int i;
  int input;
  char buf[32];
  // allow for gpio control
  gpioExport(gpio);
  // set as output
  gpioDirection(gpio,GPIO_OUT);
  // flip pin for a while
  for(i=0;i<1024;i++){
    //gpioSet(gpio,0);
    usleep(10);
    //gpioSet(gpio,1);
    usleep(10);
  }
  // read pin for a while
  gpioDirection(gpio,GPIO_IN);
  for(i=0;i<1024;i++){
    input = gpioGet(gpio);
    snprintf(buf,32,"Input is: %d\n",input);
    fprintf(stdout,"%s",buf);
    usleep(100);
  }
  // relinquish gpio control
  gpioUnexport(gpio);
}

int gpioGet(int gpio){
  char buf[64];
  int fd;
  uint8_t val[1];
  snprintf(buf,64,"/sys/class/gpio/gpio%d/value", gpio);
  fd = open(buf, O_RDONLY);
  if(-1==fd) printf("Failed to open gpio value\r\n");
  read(fd,val,1);
  close(fd);
  if(val[0] == 49)
    return 1;
  else
    return 0;
}

void gpioExport(int gpio)
{
  int fd;
  char buf[5];
  fd = open("/sys/class/gpio/export", O_WRONLY);
  snprintf(buf,5,"%d", gpio); 
  write(fd, buf, strlen(buf));
  close(fd);
}

void gpioUnexport(int gpio)
{
  int fd;
  char buf[5];
  fd = open("/sys/class/gpio/unexport", O_WRONLY);
  snprintf(buf,5,"%d", gpio); 
  write(fd, buf, strlen(buf));
  close(fd);
}

void gpioDirection(int gpio, int direction) // 1 for output, 0 for input
{
  char buf[64];
  int fd;
  snprintf(buf,64,"/sys/class/gpio/gpio%d/direction", gpio);
  fd = open(buf, O_WRONLY);
  if (direction){
    write(fd, "out", 3);
  }
  else{
    write(fd, "in", 2);
  }
  close(fd);
}

void gpioSet(int gpio, int value)
{
  char buf[64];
  int fd;
  snprintf(buf,64,"/sys/class/gpio/gpio%d/value", gpio);
  fd = open(buf, O_WRONLY);
  sprintf(buf, "%d", value);
  write(fd, buf, strlen(buf));
  close(fd);
}

