#ifndef _gpio
  #define _gpio

  #include <stdlib.h>
  #include <stdio.h>
  #include <unistd.h>
  #include <stdint.h>
  #include <fcntl.h>
  #include <string.h>
  #include "gpio.c"

  #define GPIO_OUT 1
  #define GPIO_IN 0

  void gpioExport(int gpio);
  void gpioUnexport(int gpio);
  void gpioDirection(int gpio, int direction);
  void gpioSet(int gpio, int value);
  int gpioGet(int gpio);

#endif

