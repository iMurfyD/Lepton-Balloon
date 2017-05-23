#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <time.h>
#include <math.h>
#include <malloc.h>
#include <png.h>
#include <signal.h>

#define SPI_SPD 20000000
#define PACKET_LEN 164
#define FRAME_PACKETS 60
#define LEP_WIDTH 80
#define LEP_HEIGHT 60

static void pabort(const char *s){
    perror(s);
    abort();
}

void intHandler(int dummy){
  printf("Exiting...\n");
  exit(0);
}

uint8_t* leptonSync(int fd);
uint8_t* getFrame(int fd, uint8_t *packet);
void exportText(uint8_t *frameBuffer, char *fileName);
uint8_t* pollPacket(int fd);
uint16_t* pack(uint8_t *frameBuffer);
int initSPI(int fd,int mode, int bits, int speed);
void save_png(char * fileName, int width, int height, uint16_t *buf);

int main(){
    // register intHandler to catch SIGINT
    signal(SIGINT,intHandler);
    int nBytes = 4097;
    int nFrames = 10;
    char buff[nBytes];
    int i = 0;
    int n = 0;
    int ret = -1;
    char fileName[32];
    // SPI settings
    int bits = 8;
    int mode = SPI_CPHA|SPI_CPOL;
    int speed = 20000000;
    // time
    struct tm *timeinfo;
    time_t rawtime;

    // initialize array to zero
    for(i=0;i<nBytes;i++){
        buff[i]=0;
    }
    while(1){
        // open spi port
        int fd = open("/dev/spidev0.0",O_RDWR);
        // check if spi port is actually open
        if(fd<0)
            pabort("can't open port");
        // initialize spi port
        ret = initSPI(fd,mode,bits,speed);
        if (ret == -1)
            pabort("error configuring SPI");
        // sync to lepton and get first packet
        //uint8_t packet[PACKET_LEN];
        uint8_t *packet;
        packet = leptonSync(fd);
        // get the rest of the first frame
        //uint8_t frameBuffer[PACKET_LEN*FRAME_PACKETS];
        uint8_t *frameBuffer;
        frameBuffer = getFrame(fd,packet);
        // export frame as text
        //sprintf(fileName,"LeptonCImage%d.txt",n);
        //exportText(frameBuffer,fileName);
        // pack frame
        uint16_t *pngBuf;
        pngBuf = pack(frameBuffer);
        printf("Packed Succesfully!\n");
        // get current time
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        // export frame as png
        snprintf(fileName,32,"%d.%d.%d_lep.png",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
        save_png(fileName,LEP_WIDTH,LEP_HEIGHT,pngBuf);
        // close spi bus
        ret = close(fd);
        if(ret != 0)
            pabort("Could not close spi port");
        // wait a bit
        usleep(1000000);
    }
}

uint8_t* leptonSync(int fd){
    static uint8_t packet[PACKET_LEN];
    uint8_t header = 0xff;
    printf("syncing...\n");
    // wait > 5 frame periods (185us)
    sleep(1);
    // read packets until a new frame is ready
    while(1){
        // get entire packet 
        read(fd,packet,PACKET_LEN);
        // get ID 
        header = packet[0];
        // print ID
        printf("%x",header);
        // check if header is non-throwaway
        if ((header & 0x0f) != 0x0f){
            return packet;
        }
    }
}

uint8_t* getFrame(int fd, uint8_t *packet){
    static uint8_t frameBuffer[PACKET_LEN*FRAME_PACKETS];
    int i = 0;
    // assign first packet to frameBuffer
    for(i = 0;i<PACKET_LEN;i++){
        frameBuffer[i]=packet[i];
    }
    // get entire rest of the frame all at once starting after previous frame
    read(fd,frameBuffer+PACKET_LEN,PACKET_LEN*(FRAME_PACKETS-1));
    return frameBuffer;
}

void exportText(uint8_t *frameBuffer, char *fileName){
    int i,j;
    int ret = -1;
    uint16_t tempVal = 0;
    char buf[6];
    // open file
    int outFile = open(fileName,O_WRONLY|O_CREAT,S_IRWXO);
    // check if file is actually open
    if(outFile<0)
        pabort("can't open output file");
    // write frame data to file
    for(j=0;j<FRAME_PACKETS;j++){
        for(i=0;i<PACKET_LEN-4;i=i+2){
            tempVal = ((frameBuffer[j*PACKET_LEN+i+4] << 8) | frameBuffer[j*PACKET_LEN+i+5])&0x3fff;
            sprintf(buf,"%u,",tempVal);
            write(outFile,buf,6);
        }
        write(outFile,"\n",1);
    }
    // close file
    ret = close(outFile);
    // check if file actually closed
    if (ret == -1)
        pabort("failed to close output file");
    return;
}

uint8_t* pollPacket(int fd){
    static uint8_t packet[PACKET_LEN];
    uint8_t header = 0xff;
    // read packets until a new frame is ready
    while(1){
        // get entire packet 
        read(fd,packet,PACKET_LEN);
        // get ID 
        header = packet[0];
        // check if header is non-throwaway
        if ((header & 0x0f) != 0x0f){
            return packet;
        }
    }
}

uint16_t* pack(uint8_t *frameBuffer){
    int i,j;
    static uint16_t outputBuffer[FRAME_PACKETS*(PACKET_LEN-4)/2];
    // concatenate two bytes into one uint16_t
    // note: strips CRC and ID
    //for(i=0;i<(FRAME_PACKETS*(PACKET_LEN-4)/2);i=i+2){
    //    outputBuffer[i] = ((frameBuffer[i+4] << 8) | frameBuffer[i+5]);
    //}
    // write frame data to file
    for(j=0;j<FRAME_PACKETS;j++){
        for(i=0;i<PACKET_LEN-4;i=i+2){
            outputBuffer[(j*(PACKET_LEN-4)+i)/2] = ((frameBuffer[j*PACKET_LEN+i+4] << 8) | frameBuffer[j*PACKET_LEN+i+5])&0x3fff;
        }
    }
    return outputBuffer;
}

int initSPI(int fd,int mode, int bits, int speed){
    int ret = 0;
    if (fd<0)
        pabort("file not open");
    // set spi mode
    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1)
        pabort("can't set spi mode");
    // read spi mode
    ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
    if (ret == -1)
        pabort("can't get spi mode");
    // set bits per word
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1)
        pabort("can't set bits per word");
    // read bits per word
    ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret == -1)
        pabort("can't get bits per word");
    // set max speed hz
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1)
        pabort("can't set max speed hz");
    // read max speed hz
    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret == -1)
        pabort("can't get max speed hz");
    // print spi settings
    printf("spi mode: %d\n", mode);
    printf("bits per word: %d\n", bits);
    printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
    return ret;
}

void save_png(char * fileName, int width, int height, uint16_t *buf)
{
    FILE* fp;
    fp = fopen(fileName,"wb");
    if (fp == NULL)
        pabort("Could not open PNG file");
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    size_t x, y;
    png_bytepp row_pointers;

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
        return ;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        png_destroy_write_struct(&png_ptr, NULL);
        return ;
    }

     if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return ;
    }

    png_set_IHDR(png_ptr, info_ptr,
                 width, height, // width and height
                 16, // bit depth
                 PNG_COLOR_TYPE_GRAY, // color type
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    /* Initialize rows of PNG. */
    row_pointers = (png_bytepp)png_malloc(png_ptr,
        width*png_sizeof(png_bytep));
    int i = 0;
    for (i=0; i<height; i++)
       row_pointers[i]=NULL;

    for (i=0; i<height; i++)
       row_pointers[i]=png_malloc(png_ptr, width*2);

    //set row data
    short temp;
    for (y = 0; y < height; ++y) {
        png_bytep row = row_pointers[y];
        for (x = 0; x < width; ++x) {
                temp = buf[y*width+x];
                *row++ = (png_byte)(temp >> 8);
                *row++ = (png_byte)(temp & 0xFF);
        }
    }

    /* Actually write the image data. */
    png_init_io(png_ptr, fp);
    png_set_rows(png_ptr, info_ptr, row_pointers);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
    //png_write_image(png_ptr, row_pointers);

    /* Cleanup. */
    for (y = 0; y < height; y++) {
        png_free(png_ptr, row_pointers[y]);
    }
    png_free(png_ptr, row_pointers);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
}
