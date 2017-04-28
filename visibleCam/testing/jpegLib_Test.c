#include <stdio.h>
#include <jpeglib.h>

// Function descriptions
void writeJpeg(char * filename, int quality, int Height, int Width, JSAMPARRAY image_buffer);
readJpeg(char * filename);


int main(){

}

readJpeg(char * filename){
    // create compression struct
    struct jpeg_decompress_struct cinfo;
    // create error handling struct
    struct jpeg_error_mgr jerr;

    // set compression struct error handler to be the default error handler
    cinfo.err = jpeg_std_error(&jerr);
    // ?
    jpeg_create_decompress(&cinfo);

    // try to open infile
    FILE * infile;
    if ((infile = fopen(filename, "rb")) == NULL ) {
        fprintf(strerr, "can't open %s\n",filename);
        exit(1);
    }
    // set decompression input to be input file
    jpeg_stdio_src(&cinfo,infile);

    // read header
    jpeg_read_header(&cinfo,TRUE);

    // start decompression
    jpeg_start_decompression(&cinfo);

    // extract relevant parameters from image
    int Width = cinfo.output_width;
    int Height = cinfo.output_height;
    int ColorComp = cinfo.out_color_components;
    int ColorMap = cinfo.colormap;

    // read scanlines until EOF
    

    // finish decompression
    jpeg_finish_decompression(&cinfo);

    // release memory for decompression struct
    jpeg_destroy_decompress(&cinfo);

}

void writeJpeg(char * filename, int quality, int Height, int Width, JSAMPARRAY image_buffer){
    // create compression struct
    struct jpeg_compress_struct cinfo;
    // create error handling struct
    struct jpeg_error_mgr jerr;

    // set compression struct error handler to be the default error handler
    cinfo.err = jpeg_std_error(&jerr);
    // ?
    jpeg_create_compress(&cinfo);
    
    // try to open outfile
    FILE * outfile;
    if ((outfile = fopen(filename, "wb")) == NULL){
        fprintf(stderr, "cant open %s\n",filename);
        exit(1);
    }
    // set output for compression to be output file
    jpeg_stdio_dest(&cinfo,outfile);

    // set image width
    cinfo.image_width = Width;
    // set image height
    cinfo.image_height = Height;
    // set image color gamut
    cinfo.image_components = 3;
    // set image colorspace
    cinfo.in_color_space = JCS_RGB;
    // commit
    jpeg_set_defaults(&cinfo);
    // set jpeg quality
    jpeg_set_quality(&cinfo,quality,TRUE);

    // start compression
    jpeg_start_compress(&cinfo, TRUE);

    // write scanlines
    // pointer to a single line
    JSAMPROW row_pointer[1];
    // physical row width in buffer
    int row_stride;
    row_stride = cinfo.image_width * 3;
    // iterate over scanlines until EOF
    while(cinfo.next_scanline < cinfo.image_height){
        // update pointer to point at next scanline
        row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
        // write that single scanline to file
        jpeg_write_scanlines(&cinfo, row_pointer,1);
    }
    
    // finish compression
    jpeg_finish_compress(&cinfo);

    // release compression struct memory
    jpeg_destroy_compress(&cinfo);
}
