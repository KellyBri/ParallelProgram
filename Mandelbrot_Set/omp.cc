#include <iostream>
#include <cstring>
#include <omp.h>
#include <png.h>
#define PNG_NO_SETJMP
#define MAX_ITER 10000

typedef struct {
    double x;
    double y;
}COMPLEX;


void write_png(const char* filename, const int width, const int height, const int* buffer) {
    FILE* fp = fopen(filename, "wb");
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info_ptr = png_create_info_struct(png_ptr);
    png_init_io(png_ptr, fp);
    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png_ptr, info_ptr);
    size_t row_size = 3 * width * sizeof(png_byte);
    png_bytep row = (png_bytep)malloc(row_size);
    for (int y = 0; y < height; ++y) {
        memset(row, 0, row_size);
        for (int x = 0; x < width; ++x) {
            int p = buffer[(height - 1 - y) * width + x];
            png_bytep color = row + x * 3;
            if (p != MAX_ITER) {
                if (p & 16) {
                    color[0] = 240;
                    color[1] = color[2] = p % 16 * 16;
                } else {
                    color[0] = p % 16 * 16;
                }
            }
        }
        png_write_row(png_ptr, row);
    }
    free(row);
    png_write_end(png_ptr, NULL);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
}



int main(int argc, char** argv){
    //parse parameter
    int threadNum = atoi(argv[1]);
    //[x_leftBound, x_rightBound)
    int x_point = atoi(argv[6]);
    double x_leftBound = atof(argv[2]);
    double x_rightBound = atof(argv[3]);
    //[y_lowerBound, y_upperBound)
    int y_point = atoi(argv[7]);
    double y_lowerBound = atof(argv[4]);
    double y_upperBound = atof(argv[5]);
    char *fileName = argv[8];

    int problemSize = x_point * y_point;
    double x_increment = (x_rightBound - x_leftBound) / x_point;
    double y_increment = (y_upperBound - y_lowerBound) / y_point;
    int *iteration = new int[problemSize];

    #pragma omp parallel for num_threads(threadNum) schedule(dynamic,1) collapse(2)
    for(int y=0; y<y_point; ++y){
        for(int x=0; x<x_point; ++x){
            int i = 0;
            double length = 0;
            COMPLEX Z, C;
            C.x = x_leftBound + x_increment * x;
            C.y = y_lowerBound + y_increment * y;
            Z.x = 0;
            Z.y = 0;
            
            while(i < MAX_ITER && length <= 4){
                double temp = Z.x * Z.x - Z.y * Z.y + C.x;
                Z.y = 2 * Z.x * Z.y + C.y;
                Z.x = temp;
                length = Z.x * Z.x + Z.y * Z.y;
                ++i;
            }
            iteration[y * x_point + x] = i;
        }
    }
    write_png(argv[8], x_point, y_point, iteration);
    delete[] iteration;
    return 0;
}
