#include "gpio.h"

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
