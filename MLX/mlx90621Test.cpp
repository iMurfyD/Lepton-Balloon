#include "mlx90621.cpp"

// instantiate sensor
MLX90621 mlx;

// databuffer for IR data
uint16_t dataBuf[64] ={};

int main() {
  mlx.init();
  uint16_t T_amb = mlx.readTamb();
  printf("T_amb = %d\n",T_amb);
  mlx.closeI2C();
  return 0;
}
