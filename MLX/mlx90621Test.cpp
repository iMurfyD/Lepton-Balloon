#include "mlx90621.cpp"

// instantiate sensor
MLX90621 mlx;

// databuffer for IR data
int16_t dataBuf[64] ={};

int main() {
  int i,j;
  mlx.init();
  uint16_t T_amb_r = mlx.readTamb();
  printf("raw T_amb = %d\n",T_amb_r);
  double T_amb = mlx.calcTa(T_amb_r);
  printf("calcd T_amb = %g\n",T_amb);
  mlx.readFrame(dataBuf);

  printf("Raw Sensor Data\n");

  for(j=0;j<16;j++){
    for(i=0;i<4;i++){
      printf("%d,\t",dataBuf[j*4+1]);
    }
    printf("\n");
  }
  
//  printf("%g\n",mlx.calcTo(dataBuf[0],0));

  printf("Calculated Temperature Data\n");

  for(j=0;j<16;j++){
    for(i=0;i<4;i++){
      printf("%.2f,\t",mlx.calcTo(dataBuf[j*4+1],j*4+i));
    }
    printf("\n");
  }

  return 0;
}
