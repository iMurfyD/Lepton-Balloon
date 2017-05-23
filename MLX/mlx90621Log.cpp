#include "mlx90621.cpp"
#include <signal.h>

void intHandler(int dummy){
  printf("Exiting...\n");
  exit(0);
}

int main() {
  signal(SIGINT,intHandler);
  int i,j,k;
  char fileName[20];
  // databuffer for IR data
  double calcData[64] = {};
  int16_t dataBuf[64] ={};
  // instantiate sensor
  MLX90621 mlx;
  // initialize sensor
  mlx.init();
  for(k=1;k<10;k++){
    // get ambient temp and perform calculation
    uint16_t T_amb_r = mlx.readTamb();
    double T_amb = mlx.calcTa(T_amb_r);
    // get full frame
    mlx.readFrame(dataBuf);
    // calculate pixel temps for full frame
    for(j=0;j<16;j++){
      for(i=0;i<4;i++){
        calcData[j*4+i]=mlx.calcTo(dataBuf[j*4+i],j*4+i);
      }
    }
    // print image number
    printf("image %d\n",k);
    for(i = 0;i<16;i++){
      for(j=0;j<4;j++){
        //printf("%0.2f,",calcData[4*i+j]);
      }
      //printf("\n");
    }
    // output text file
    //sprintf(fileName,"MLXImage_%d.txt",k);
    //mlx.exportText(calcData,fileName);
    // output png file
    sprintf(fileName,"MLXImage_%d.png",k);
    mlx.exportPng(calcData,fileName);
    // wait a bit
    usleep(1000000L);
  }
  return 0;
}

