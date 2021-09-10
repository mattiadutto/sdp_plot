#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <png.h>

#include <string>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <utility>

#include "Point.h"

#define MAX_STR 50

using namespace std;

// Questi ci serviranno per riscalare l'immagine.
float MAX_WIDTH = 0;
float MAX_HEIGHT = 0;
// writeImage() and setRGB() da http://www.labbookpages.co.uk/software/imgProc/libPNG.html

inline void setRGB(png_byte *ptr, float val)
{
    // da https://stackoverflow.com/questions/6394304/algorithm-how-do-i-fade-from-red-to-green-via-yellow-using-rgb-values
    if (val != -1.0)
    {
        ptr[0] = (1 - val) * 255;
        ptr[1] = val * 255;
        ptr[2] = 0;
    }
    else
    {
        ptr[0] = ptr[1] = ptr[2] = 255;
    }

    // printf("%f\t%d %d %d\n", val, ptr[0], ptr[1], ptr[2]); // stampa valori di val + rgb
}

int writeImage(char *filename, int width, int height, float *buffer, char *title)
{
    int code = 0;
    FILE *fp = NULL;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    png_bytep row = NULL;

    // Open file for writing (binary mode)
    fp = fopen(filename, "wb");
    if (fp == NULL)
    {
        fprintf(stderr, "Could not open file %s for writing\n", filename);
        code = 1;
        goto finalise;
    }

    // Initialize write structure
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL)
    {
        fprintf(stderr, "Could not allocate write struct\n");
        code = 1;
        goto finalise;
    }

    // Initialize info structure
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL)
    {
        fprintf(stderr, "Could not allocate info struct\n");
        code = 1;
        goto finalise;
    }

    // Setup Exception handling
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        fprintf(stderr, "Error during png creation\n");
        code = 1;
        goto finalise;
    }

    png_init_io(png_ptr, fp);

    // Write header (8 bit colour depth)
    png_set_IHDR(png_ptr, info_ptr, width, height,
                 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    // Set title
    if (title != NULL)
    {
        png_text title_text;
        title_text.compression = PNG_TEXT_COMPRESSION_NONE;
        title_text.key = "Title";
        title_text.text = title;
        png_set_text(png_ptr, info_ptr, &title_text, 1);
    }

    png_write_info(png_ptr, info_ptr);

    // Allocate memory for one row (3 bytes per pixel - RGB)
    row = (png_bytep)malloc(3 * width * sizeof(png_byte));

    // Write image data
    int x, y;
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++){
            setRGB(&(row[x * 3]), buffer[y * width + x]);
    }
        png_write_row(png_ptr, row);
    }

    // End write
    png_write_end(png_ptr, NULL);

finalise:
    if (fp != NULL)
        fclose(fp);
    if (info_ptr != NULL)
        png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    if (png_ptr != NULL)
        png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    if (row != NULL)
        free(row);

    return code;
}

int main(int argc, char **argv)
{
    char cell_file[MAX_STR + 1], coverage_file[MAX_STR + 1], output_file[MAX_STR + 1];
    int width, height;

    std::ifstream file_pointer;
    std::unordered_map<std::string, Point *> map_points;
    std::string signal_name;

    float x, y;
    float coverage;

    std::unordered_map<std::string, Point *>::iterator it;

    if (argc != (1 + 2 + 2 + 2 + 2))
    {
        fprintf(stderr, "USAGE ERROR: %s -d CELL_FILE -c COVERAGE_FILE -o OUTPUT_FILE -r HEIGHTxWIDTH", argv[0]);
        return -1;
    }

    // Lettura dei parametri
    if (strcmp(argv[1], "-d") == 0)
        strcpy(cell_file, argv[2]);
    else if (strcmp(argv[3], "-d") == 0)
        strcpy(cell_file, argv[4]);
    else if (strcmp(argv[5], "-d") == 0)
        strcpy(cell_file, argv[6]);
    else if (strcmp(argv[7], "-d") == 0)
        strcpy(cell_file, argv[8]);

    if (strcmp(argv[1], "-c") == 0)
        strcpy(coverage_file, argv[2]);
    else if (strcmp(argv[3], "-c") == 0)
        strcpy(coverage_file, argv[4]);
    else if (strcmp(argv[5], "-c") == 0)
        strcpy(coverage_file, argv[6]);
    else if (strcmp(argv[7], "-c") == 0)
        strcpy(coverage_file, argv[8]);

    if (strcmp(argv[1], "-o") == 0)
        strcpy(output_file, argv[2]);
    else if (strcmp(argv[3], "-o") == 0)
        strcpy(output_file, argv[4]);
    else if (strcmp(argv[5], "-o") == 0)
        strcpy(output_file, argv[6]);
    else if (strcmp(argv[7], "-o") == 0)
        strcpy(output_file, argv[8]);

    if (strcmp(argv[1], "-r") == 0)
    {
        height = atoi(strtok(argv[2], "x"));
        width = atoi(strtok(NULL, "x"));
    }
    else if (strcmp(argv[3], "-r") == 0)
    {
        height = atoi(strtok(argv[4], "x"));
        width = atoi(strtok(NULL, "x"));
    }
    else if (strcmp(argv[5], "-r") == 0)
    {
        height = atoi(strtok(argv[6], "x"));
        width = atoi(strtok(NULL, "x"));
    }
    else if (strcmp(argv[7], "-r") == 0)
    {
        height = atoi(strtok(argv[8], "x"));
        width = atoi(strtok(NULL, "x"));
    }

    //printf("SIZE OF PAIR: %lu", sizeof(std::pair<std::string, Point>));

    // Leggo il file dei segnali
    file_pointer.open(cell_file);
    if (file_pointer.is_open())
    {
        while (file_pointer)
        {
            file_pointer >> signal_name >> y >> x;
            // servira' per quando avremo a che fare con il ridimensionamento
            signal_name = "testbench/top/" + signal_name;
            if (x > MAX_WIDTH)
                MAX_WIDTH = x;
            if (y > MAX_HEIGHT)
                MAX_HEIGHT = y;
            // std::pair<std::string,Point *> my_point (signal_name,new Point(x,y)); // alternativa alla riga di sotto
            // map_points.insert(my_point);

            map_points.insert(it, std::pair<std::string, Point *>(signal_name, new Point(x, y)));
        }
    }

    file_pointer.close();

    // Leggo il file delle coperture
    file_pointer.open(coverage_file);
    if (file_pointer.is_open())
    {
        while (file_pointer)
        {
            file_pointer >> signal_name >> coverage;
            it = map_points.find(signal_name);
            if (it != map_points.end())
            {
                it->second->setCoverage(coverage);
            }
        }
    }
    file_pointer.close();

    //printf("# OF POINTS: %lld", (long long int)map_points.size());

    // Controllo che la lettura sia andata a buon fine, usato solo per i primi test.
    //  for (auto &x : map_points){
    //      cout << x.first << "\t";
    //      x.second->toString();
    //  }

    // Se l'immagine è più piccola del MAX_WIDTH, ridimensiono l'immagine
    // if (MAX_WIDTH > width)
    //     width = MAX_WIDTH;
    // if (MAX_HEIGHT > height)
    //     height = MAX_HEIGHT;
    
    printf("WIDTH x HEIGHT: %d x %d\n", width, height);
    //printf("SIZE: %f x %f\n", MAX_WIDTH, MAX_HEIGHT);
    printf("BUFFER SIZE: %d\n", width * height);
    // Alloco e inizializzo il buffer
    float *buffer = (float *)malloc(width * height * sizeof(float));
    if (buffer == NULL)
    {
        fprintf(stderr, "Could not create image buffer\n");
        return NULL;
    }

    for (int i = 0; i < width * height; i++)
        buffer[i] = -1.0;

    for (auto &x : map_points)
    {
        buffer[x.second->getPosition(width)] = x.second->getCoverage();
    }
    
    // Creo l'immagine
    int result = writeImage(output_file, width, height, buffer, output_file);

    // Free up the memorty used to store the image
    free(buffer);
}