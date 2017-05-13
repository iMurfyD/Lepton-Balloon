#include "mlx90261.h"

// instantiate sensor
MLX90621 mlx;

// databuffer for IR data
uint16_t dataBuf[64] ={};

int main() {
  mlx.init();
  return 0;
}
