// LibPNG Test

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <malloc.h>
#include <png.h>

// write png image to file
int writeImage(char* filename, int width, int height, uint16_t *buffer);

int main()
{
  int width = 500;
  int height = 300; 
  int maxVal = 16384;
  uint16_t shade[width*height];
  int x,y;
  for(y=0;y<height;y++){
      for(x=0;x<width;x++){
          shade[y*width+x] = (y*width+x)%maxVal;
      }
  }
  int result = writeImage("TestImage.png",width,height,shade);

}

int writeImage(char* filename, int width, int height, uint16_t *buffer){
    FILE *fp = NULL;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    png_bytep row = NULL;

    // open file for reading (binary mode)
    fp = fopen(filename,"wb");
    if (fp == NULL){
        fprintf(stderr, "Could not open file %s for writing\n",filename);
        return 1;
    }

    // initialize write structure
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);

    // initialize info structure
    info_ptr = png_create_info_struct(png_ptr);

    // initialize io to png file
    png_init_io(png_ptr, fp);

    // write header
    png_set_IHDR(png_ptr, info_ptr, width, height, 16, PNG_COLOR_TYPE_GRAY,
            PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_BASE,PNG_FILTER_TYPE_BASE);

    // write info to png
    png_write_info(png_ptr,info_ptr);

    // Allocate memory for one row (2 bytes per pixel - RGB)
    row = (png_bytep) malloc(width * sizeof(png_byte));
    //row = (uint16_t) malloc(2* width * sizeof(png_byte));


    // write image data
    int x,y;
    for(y=0;y<height;y++){
        for(x=0;x<width;x++){
            //setRGB(&(row[x*3]),buffer[y*width+x]);
            row[x] = buffer[y*width+x];
        }
        png_write_row(png_ptr,row);
    }
    // End write
    png_write_end(png_ptr, NULL);
    fclose(fp);
    png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    free(row);
    
    return 0;

}
