#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <png.h>

void save_png(FILE* fp, int width, int height, uint16_t *buf)
{
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
}

int main()
{
  int i;
  int width = 80;
  int height = 60;
  uint16_t buf[width*height];
  for(i=0;i<width*height;i++){
    buf[i]=i*13;
  }
  
  FILE* f;
  if((f=fopen("test.png", "wb"))!=NULL)
  {
    save_png(f, width,height,buf);

    fclose(f);
  }
  return 0;
}
