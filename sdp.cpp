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

#define MAX_STR 500

using namespace std;

typedef struct{
    float x;
    float y;
    char name[MAX_STR +1];
    int numberOfPoints;
    int coverage;
}point;

// Questi ci serviranno per riscalare l'immagine. 

float MAX_WIDTH = 0;
float MAX_HEIGHT = 0;
// writeImage() and setRGB() da http://www.labbookpages.co.uk/software/imgProc/libPNG.html

inline void setRGB(png_byte *ptr, float val)
{
	// from https://stackoverflow.com/questions/6394304/algorithm-how-do-i-fade-from-red-to-green-via-yellow-using-rgb-values
	ptr[0] = 2.0 * (1-val);
	ptr[1] = 2.0 * val;
	ptr[2] = 0;
}


int writeImage(char* filename, int width, int height, float *buffer, char* title)
{
	int code = 0;
	FILE *fp = NULL;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_bytep row = NULL;
	
	// Open file for writing (binary mode)
	fp = fopen(filename, "wb");
	if (fp == NULL) {
		fprintf(stderr, "Could not open file %s for writing\n", filename);
		code = 1;
		goto finalise;
	}

	// Initialize write structure
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fprintf(stderr, "Could not allocate write struct\n");
		code = 1;
		goto finalise;
	}

	// Initialize info structure
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		fprintf(stderr, "Could not allocate info struct\n");
		code = 1;
		goto finalise;
	}

	// Setup Exception handling
	if (setjmp(png_jmpbuf(png_ptr))) {
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
	if (title != NULL) {
		png_text title_text;
		title_text.compression = PNG_TEXT_COMPRESSION_NONE;
		title_text.key = "Title";
		title_text.text = title;
		png_set_text(png_ptr, info_ptr, &title_text, 1);
	}

	png_write_info(png_ptr, info_ptr);

	// Allocate memory for one row (3 bytes per pixel - RGB)
	row = (png_bytep) malloc(3 * width * sizeof(png_byte));

	// Write image data
	int x, y;
	for (y=0 ; y<height ; y++) {
		for (x=0 ; x<width ; x++) {
			setRGB(&(row[x*3]), buffer[y*width + x]);
		}
		png_write_row(png_ptr, row);
	}

	// End write
	png_write_end(png_ptr, NULL);

	finalise:
	if (fp != NULL) fclose(fp);
	if (info_ptr != NULL) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
	if (png_ptr != NULL) png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
	if (row != NULL) free(row);

	return code;
}

int main(int argc, char ** argv){
    char *cell_file, *coverage_file, *output_file;
    
    char tmp_signal_name[MAX_STR + 1];
    float x, y, coverage;

    std::unordered_map<std::string, Point*> map_points;
    
    if(argc != (1+2+2+2+2)){
        fprintf(stderr, "USAGE ERROR: %s -d CELL_FILE -c COVERAGE_FILE -o OUTPUT_FILE -r HEIGHTxWIDTH", argv[0]);
        return -1;
    }
    /*
    if(argv[1] == "-d")
        strcpy(cell_file, argv[2]);
    else if(argv[3] == "-d")
        strcpy(cell_file, argv[4]);
    else if(argv[5] == "-d")
        strcpy(cell_file, argv[6]);

    if(argv[1] == "-c")
        strcpy(coverage_file, argv[2]);
    else if(argv[3] == "-c")
        strcpy(coverage_file, argv[4]);
    else if(argv[5] == "-c")
        strcpy(coverage_file, argv[6]);

    if(argv[1] == "-o")
        strcpy(output_file, argv[2]);
    else if(argv[3] == "-o")
        strcpy(output_file, argv[4]);
    else if(argv[5] == "-o")
        strcpy(output_file, argv[6]);
    */
   /*
    int width = atoi(strtok(argv[8], "x"));
	int height = atoi(strtok(NULL, "x"));
*/
    int width = 50;
    int height = 25;

    std::ifstream myfile;
    std::string signal_name;
    std::unordered_map<std::string, Point*>::iterator it;
    it = map_points.begin();

    // Leggo file dei segnali
    // TODO: eliminare argv[2]
    myfile.open(argv[2]);
    if(myfile.is_open()){
        while(myfile){
            myfile >> signal_name >> x >> y;
            if(x > MAX_WIDTH)
                MAX_WIDTH = x;
            if(y > MAX_HEIGHT)
                MAX_HEIGHT = y;
            //Point *p = new Point;
            //p->setPosition(x,y);

            //printf("%p\t", &p);
            //Point(x,y).toString();
            //cout << signal_name << "\t" << x << "\t"<< y << "\n";
            
            //std::pair<std::string,Point *> my_point (signal_name,new Point(x,y));
            //map_points.insert(my_point);
            
            map_points.insert(it, std::pair<std::string, Point *>(signal_name, new Point(x,y)));
        }
    }

    myfile.close();

    // Leggo file delle coperture
    // TODO: eliminare argv[4]
    myfile.open(argv[4]);
    if(myfile.is_open()){
        while(myfile){
            myfile >> signal_name >> coverage;
            //cout << coverage;
            it = map_points.find(signal_name);
            if(it != map_points.end()){
                it->second->setCoverage(coverage);
            }
        }
    }
    myfile.close();

    // Controllo che la lettura sia andata a buon fine, usato solo per i primi test.
    for (auto & x : map_points)
        //cout<< x.first << "\n";
        x.second->toString();

    // Inizializzo il buffer
    float *buffer = (float *) malloc(width * height * sizeof(float));
	if (buffer == NULL) {
		fprintf(stderr, "Could not create image buffer\n");
		return NULL;
	}

    for(int i = 0; i < width * height; i++)
        buffer[i] = 0.0;
    

    for(auto &x : map_points){
        buffer[x.second->getPosition(width)] = x.second->getCoverage();
    }

    for(int i = 0; i < width * height; i++){
         printf("%.2f\t", buffer[i]);
    }

    int result = writeImage(argv[1], width, height, buffer, argv[1]);

	// Free up the memorty used to store the image
	free(buffer);

    

}

