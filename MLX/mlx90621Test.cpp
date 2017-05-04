#include "mlx90621.cpp"

// instantiate sensor
MLX90621 mlx;

// databuffer for IR data
uint16_t dataBuf[64] ={};

int main() {
  int i,j;
  mlx.init();
  uint16_t T_amb = mlx.readTamb();
  printf("T_amb = %d\n",T_amb);
  mlx.readFrame(dataBuf);

  for(j=0;j<16;j++){
    for(i=0;i<4;i++){
      printf("%x,",dataBuf[j*4+1]);
    }
    printf("\n");
  }

  printf("\n%g\n\n",mlx.calcTo(dataBuf[1],1));
/*
  for(j=0;j<16;j++){
    for(i=0;i<4;i++){
      printf("%g,",mlx.calcTo(dataBuf[j*4+1],j*4+i));
    }
    printf("\n");
  }
*/
  return 0;
}
