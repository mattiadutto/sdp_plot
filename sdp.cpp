#define PRINT 0 // Per debug, stampa alcuni dati extra
#define TIME 0  // Per calcolo tempo esecuzione
#define THREAD 0
#define BOOST 1 // Libreria per la gestione degli argomenti.
#define RESIZE 1

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
#include <cstdlib>
#include <map>

using namespace std;
#if TIME
#include <chrono>
#endif
#if THREAD
#define N_THREADS 2
#include <thread>
#endif
#if BOOST
#include <boost/program_options.hpp>
namespace po = boost::program_options;
#endif
#include "Point.h"

#define MAX_STR 50 // Suppongo che i file non abbiano un nome superiore ai 50 caratteri.
#define TITLE "Title"

// Questi ci serviranno per riscalare l'immagine.
float MAX_WIDTH = 0;
float MAX_HEIGHT = 0;

// writeImage() and setRGB() da http://www.labbookpages.co.uk/software/imgProc/libPNG.html
inline void setRGB(png_byte *ptr, float val, unsigned char count);
int writeImage(char *filename, int width, int height, float *buffer, float *buffer_count, char *title);
void finalise(FILE *fp, png_infop info_ptr, png_structp png_ptr);

int main(int argc, char **argv)
{
#if TIME
    auto start = chrono::steady_clock::now();
#endif
#if BOOST
    std::string cell_file, coverage_file, output_file;
#else
    char cell_file[MAX_STR + 1], coverage_file[MAX_STR + 1], output_file[MAX_STR + 1];
#endif
    float width, height;

    std::ifstream file_pointer;
    std::unordered_map<std::string, PointClass *> map_points;
    std::string signal_name;

    float x, y;
    float coverage;

    std::unordered_map<std::string, PointClass *>::iterator it;
#if BOOST
    //Command line argument handling
    po::options_description desc("Allowed options");
    desc.add_options()("CELL_FILE,d", po::value<string>(), "path to cell file")("COVERAGE_FILE,c", po::value<string>(), "path to coverage file")("OUTPUT_FILE,o", po::value<string>(), "path to output file")("HEIGHTxWIDTH,r", po::value<string>(), "size of the image");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("CELL_FILE"))
    {
        cell_file = vm["CELL_FILE"].as<string>();
    }
    else
    {
        fprintf(stderr, "USAGE ERROR: %s -d CELL_FILE -c COVERAGE_FILE -o OUTPUT_FILE -r HEIGHTxWIDTH", argv[0]);
        return -1;
    }

    if (vm.count("COVERAGE_FILE"))
    {
        coverage_file = vm["COVERAGE_FILE"].as<string>();
    }
    else
    {
        fprintf(stderr, "USAGE ERROR: %s -d CELL_FILE -c COVERAGE_FILE -o OUTPUT_FILE -r HEIGHTxWIDTH", argv[0]);
        return -1;
    }

    if (vm.count("OUTPUT_FILE"))
    {
        output_file = vm["OUTPUT_FILE"].as<string>();
    }
    else
    {
        fprintf(stderr, "USAGE ERROR: %s -d CELL_FILE -c COVERAGE_FILE -o OUTPUT_FILE -r HEIGHTxWIDTH", argv[0]);
        return -1;
    }

    if (vm.count("HEIGHTxWIDTH"))
    {
        std::string dim = vm["HEIGHTxWIDTH"].as<string>();
        width = stoi(dim.substr(0, dim.find("x")));
        dim.erase(0, dim.find("x") + 1);
        height = stoi(dim.substr(0, dim.find("x")));
    }
    else
    {
        fprintf(stderr, "USAGE ERROR: %s -d CELL_FILE -c COVERAGE_FILE -o OUTPUT_FILE -r HEIGHTxWIDTH", argv[0]);
        return -1;
    }
#else
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
#endif
    //printf("SIZE OF PAIR: %lu", sizeof(std::pair<std::string,   PointClass>));

    // Leggo il file dei segnali
    cout << "Lettura file segnali"
         << "\n";
    file_pointer.open(cell_file);
    if (file_pointer.is_open())
    {
        while (!file_pointer.eof())
        {
            file_pointer >> signal_name >> y >> x;
            // servira' per quando avremo a che fare con il ridimensionamento
            signal_name = "testbench/top/" + signal_name;
            if (x > MAX_WIDTH)
                MAX_WIDTH = x;
            if (y > MAX_HEIGHT)
                MAX_HEIGHT = y;
            // std::pair<std::string,  PointClass *> my_point (signal_name,new  PointClass(x,y)); // alternativa alla riga di sotto
            // map_points.insert(my_point);
            it = map_points.find(signal_name);

            if (it == map_points.end() && !file_pointer.eof())
                map_points.insert(it, std::pair<std::string, PointClass *>(signal_name, new PointClass(x, y)));
        }
    }

#if TIME
    auto end_structure_file = chrono::steady_clock::now();
    cout << "Elapsed time [READING STRUCTURE FILE] in seconds: "
         << chrono::duration_cast<chrono::seconds>(end_structure_file - start).count()
         << " sec";
#endif

    file_pointer.close();
    cout << "Fine lettura file segnali"
         << "\n";
    // Leggo il file delle coperture
    cout << "Lettura file coperture"
         << "\n";
    file_pointer.open(coverage_file);
    if (file_pointer.is_open())
    {
        while (!file_pointer.eof())
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
    cout << "Fine lettura file coperture"
         << "\n";
#if TIME
    auto end_coverage_file = chrono::steady_clock::now();
    cout << "Elapsed time [READING STRUCTURE FILE] in seconds: "
         << chrono::duration_cast<chrono::seconds>(end_coverage_file - end_structure_file).count()
         << " sec";
#endif
    //printf("# OF POINTS: %lld", (long long int)map_points.size());

    // Controllo che la lettura sia andata a buon fine, usato solo per i primi test.
#if PRINT
    for (auto &x : map_points)
    {
        cout << x.first << "\t";
        x.second->toString();
    }
#endif

#if PRINT
    printf("WIDTH x HEIGHT: %d x %d\n", width, height);
    //printf("SIZE: %f x %f\n", MAX_WIDTH, MAX_HEIGHT);
    printf("BUFFER SIZE: %d\n", width * height);
#endif
    // Alloco e inizializzo il buffer
#if !RESIZE
    float *buffer = (float *)malloc(width * height * sizeof(float));
    unsigned char *buffer_count = (unsigned char *)malloc(width * height * sizeof(unsigned char));
#else
    float *buffer = (float *)malloc((MAX_WIDTH + 1) * (MAX_HEIGHT + 1) * sizeof(float));
    unsigned char *buffer_count = (unsigned char *)malloc((MAX_WIDTH + 1) * (MAX_HEIGHT + 1) * sizeof(unsigned char));
#endif
    int pos = -1;

    if (buffer == NULL || buffer_count == NULL)
    {
        fprintf(stderr, "Could not create image buffer\n");
        return -2;
    }
#if !RESIZE
    for (int i = 0; i < width * height; i++)
#else
    for (int i = 0; i < (MAX_WIDTH + 1) * (MAX_HEIGHT + 1); i++)
#endif
    {
        buffer[i] = -1.0;
        buffer_count[i] = 0;
    }

    for (auto &x : map_points)
    {
#if !RESIZE
        pos = x.second->getPosition(width);
#else
        pos = x.second->getPosition(MAX_WIDTH + 1);
#endif
        if (buffer[pos] == -1)
            buffer[pos] = x.second->getCoverage();
        else
            buffer[pos] += x.second->getCoverage();
        buffer_count[pos]++;

        //cout << buffer[pos] << "\t" << buffer_count[pos] << endl;
        //cout << x.second->getCoverage() << endl;
    }

#if RESIZE
    float resize_factor_width = (float)width / (MAX_WIDTH + 1);
    float resize_factor_height = (float)height / (MAX_HEIGHT + 1);

    // Per limitare le modifiche se il ridimensionamento è sotto il 20% non viene effettuato
    //if (resize_factor_width - 1 > 0 && resize_factor_width - 1 < 0.2)
    if (abs(1 - resize_factor_width) < 0.2)
    {
        resize_factor_width = 1;
        width = MAX_WIDTH + 1;
    }
    //if (resize_factor_height - 1 > 0 && resize_factor_height - 1 < 0.2)
    if (abs(1 - resize_factor_height) < 0.2)
    {
        resize_factor_height = 1;
        height = MAX_HEIGHT + 1;
    }

    cout << width << "\t" << height << endl;
    cout << (MAX_WIDTH + 1) << "\t" << (MAX_HEIGHT + 1) << endl;
    cout << "Resize factor: "
         << "\t" << resize_factor_width << "\t" << resize_factor_height << endl;
    cout << "Rounded: "
         << "\t" << ceil(resize_factor_width) << "\t" << ceil(resize_factor_height) << endl;

    cout << width << "\t" << height << endl;

    // Dichiaro vettori di supporto per il resize
    float *buffer_resize = (float *)malloc(width * height * sizeof(float));
    float *buffer_count_resize = (float *)malloc(width * height * sizeof(float));

    for (int i = 0; i < width * height; i++)
    {
        buffer_resize[i] = 0.0;
        buffer_count_resize[i] = 0;
    }

    float rw, rh, x1, y1;
    int new_pos, new_height, new_height_cmp, prev, new_width;
    float diff, diff_width, diff_height;

    for (auto &x : map_points)
    {
        pos = x.second->getPosition(MAX_WIDTH + 1);

        rh = resize_factor_height;
        x1 = x.second->getX();
        y1 = x.second->getY();

        for (int h = 0; h < ceil(resize_factor_height); h++, rh--)
        {
            rw = resize_factor_width;
            prev = -1;
            for (int w = 0; w < ceil(resize_factor_width); w++, rw--)
            {
                //((reY x Y + h) *reX LARGHEZZA ) + (reX * X +w)
                new_pos = floor(floor(resize_factor_width * x1) + w + (floor(resize_factor_height * y1) + h) * width);
                //diff = (float)(resize_factor_width * x1 + w) + (float)((resize_factor_height * y1 + h) * width) - new_pos;
                new_width = new_pos % (int)width;

                diff_width = (resize_factor_width * x1 + w) - new_width;
                diff_height = (resize_factor_height * y1) - floor(resize_factor_height * y1);

                new_height = floor(resize_factor_height * y1 + h);
                //cout << new_pos << "\t" << (resize_factor_width * x1) << "\t" << (float)(resize_factor_height * y1 * width) << "\t" << diff << "\t" << rw << "\t\t" << (ceil(resize_factor_width) - resize_factor_width) << "\t\t";
                //cout << diff << "\t\t" << pos << "\t" << x1 << "\t" << y1 << "\t" << new_pos << "\t" << new_width << "\t" << round(new_pos / width) << "\t";
                if (rw < 1 && rh < 1)
                {
                    if (rw > 1 - diff_width && rh > 1 - diff_height)
                    {
                        buffer_resize[new_pos] += (1 - diff_width) * (1 - diff_height) * buffer[pos];
                        buffer_count_resize[new_pos] += buffer_count[pos];
                        if (new_width + 1 <= width)
                        {
                            buffer_resize[new_pos + 1] += (rw - 1 + diff_width) * (1 - diff_height) * buffer[pos];
                            buffer_count_resize[new_pos + 1] += buffer_count[pos];
                        }
                        if (new_pos + width <= width * height)
                        {
                            buffer_resize[new_pos + (int)width] += (1 - diff_width) * (rh - 1 + diff_height) * buffer[pos];
                            buffer_count_resize[new_pos + (int)width] += buffer_count[pos];
                        }
                        if (new_width + 1 <= width)
                        {
                            buffer_resize[new_pos + (int)width + 1] += (rw - 1 + diff_width) * (rh - 1 + diff_height) * buffer[pos];
                            buffer_count_resize[new_pos + (int)width + 1] += buffer_count[pos];
                        }
                    }
                    else if (rw <= 1 - diff_width && rh <= 1 - diff_height)
                    {
                        buffer_resize[new_pos] += rw * rh * buffer[pos];
                        buffer_count_resize[new_pos] += buffer_count[pos];
                    }
                    else if (rw > 1 - diff_width && rh <= 1 - diff_height)
                    {
                        buffer_resize[new_pos] += (1 - diff_width) * rh * buffer[pos];
                        buffer_count_resize[new_pos] += buffer_count[pos];
                        if (new_width + 1 <= width)
                        {
                            buffer_resize[new_pos + 1] += (rw - 1 + diff_width) * rh * buffer[pos];
                            buffer_count_resize[new_pos + 1] += buffer_count[pos];
                        }
                    }
                    else if (rw <= 1 - diff_width && rh > 1 - diff_height)
                    {
                        buffer_resize[new_pos] += (1 - diff_height) * rw * buffer[pos];
                        buffer_count_resize[new_pos] += buffer_count[pos];
                        if (new_pos + width <= width * height)
                        {
                            buffer_resize[new_pos + (int)width] += (rh - 1 + diff_height) * rw * buffer[pos];
                            buffer_count_resize[new_pos + (int)width] += buffer_count[pos];
                        }
                    }
                }
                else if (rw >= 1 && rh >= 1)
                {
                    buffer_resize[new_pos] += (1 - diff_width) * (1 - diff_height) * buffer[pos];
                    buffer_count_resize[new_pos] += buffer_count[pos];
                    if (new_width + 1 <= width && diff_width != 0)
                    {
                        buffer_resize[new_pos + 1] += diff_width * (1 - diff_height) * buffer[pos];
                        buffer_count_resize[new_pos + 1] += buffer_count[pos];
                    }
                    if (new_pos + width <= width * height && diff_height != 0)
                    {
                        buffer_resize[new_pos + (int)width] += (1 - diff_width) * diff_height * buffer[pos];
                        buffer_count_resize[new_pos + (int)width] += buffer_count[pos];
                    }
                    if (new_width + 1 <= width && diff_height != 0 && diff_width != 0)
                    {
                        buffer_resize[new_pos + (int)width + 1] += diff_width * diff_height * buffer[pos];
                        buffer_count_resize[new_pos + (int)width + 1] += buffer_count[pos];
                    }
                }
                else if (rw >= 1 && rh < 1)
                {
                    if (rh > 1 - diff_height)
                    {
                        buffer_resize[new_pos] += (1 - diff_width) * (1 - diff_height) * buffer[pos];
                        buffer_count_resize[new_pos] += buffer_count[pos];
                        if (new_width + 1 <= width && diff_width != 0)
                        {
                            buffer_resize[new_pos + 1] += diff_width * (1 - diff_height) * buffer[pos];
                            buffer_count_resize[new_pos + 1] += buffer_count[pos];
                        }
                        if (new_pos + width <= width * height )
                        {
                            buffer_resize[new_pos + (int)width] += (1 - diff_width) * (rh - 1 + diff_height) * buffer[pos];
                            buffer_count_resize[new_pos + (int)width] += buffer_count[pos];
                        }
                        if (new_width + 1 <= width && diff_width != 0)
                        {
                            buffer_resize[new_pos + (int)width + 1] += diff_width * (rh - 1 + diff_height) * buffer[pos];
                            buffer_count_resize[new_pos + (int)width + 1] += buffer_count[pos];
                        }
                    }
                    else
                    {
                        buffer_resize[new_pos] += (1 - diff_width) * rh * buffer[pos];
                        buffer_count_resize[new_pos] += buffer_count[pos];
                        if (new_pos + 1 <= width * height && diff_width != 0)
                        {
                            buffer_resize[new_pos + 1] += diff_width * rh * buffer[pos];
                            buffer_count_resize[new_pos + 1] += buffer_count[pos];
                        }
                    }
                }
                else if (rh >= 1 && rw < 1)
                {
                    if (rw > 1 - diff_width)
                    {
                        buffer_resize[new_pos] += (1 - diff_width) * (1 - diff_height) * buffer[pos];
                        buffer_count_resize[new_pos] += buffer_count[pos];
                        if (new_width + 1 <= width)
                        {
                            buffer_resize[new_pos + 1] += (rw - 1 + diff_width) * (1 - diff_height) * buffer[pos];
                            buffer_count_resize[new_pos + 1] += buffer_count[pos];
                        }
                        if (new_pos + width <= width * height && diff_height != 0)
                        {
                            buffer_resize[new_pos + (int)width] += (1 - diff_width) * diff_height * buffer[pos];
                            buffer_count_resize[new_pos + (int)width] += buffer_count[pos];
                        }
                        if (new_width + 1 <= width && diff_height != 0)
                        {
                            buffer_resize[new_pos + (int)width + 1] += (rw - 1 + diff_width) * diff_height * buffer[pos];
                            buffer_count_resize[new_pos + (int)width + 1] += buffer_count[pos];
                        }
                    }
                    else
                    {
                        buffer_resize[new_pos] += rw * (1 - diff_height) * buffer[pos];
                        buffer_count_resize[new_pos] += buffer_count[pos];
                        if (new_pos + width <= width * height && diff_height != 0)
                        {
                            buffer_resize[new_pos + (int)width] += rw * diff_height * buffer[pos];
                            buffer_count_resize[new_pos + (int)width] += buffer_count[pos];
                        }
                    }
                }
                /*
                if (new_pos <= width * height && (prev < new_width || prev == -1)) // per evitare che vada a sporcare la prima fila sia verticale sia orizzontale.
                {
                    if (rw >= 1 && rh >= 1)
                    {
                        float tmp_pos = buffer[pos];
                        float tmp_count = buffer_count[pos];
                        if (new_width != ((resize_factor_width * x1) + w))
                        {
                            buffer_resize[new_pos] += buffer[pos] * (1 - diff_width);
                            buffer_count_resize[new_pos] += buffer_count[pos]; // * (1-diff_width);
                            if (new_width + 1 <= width)
                            {
                                buffer_resize[new_pos + 1] += buffer[pos] * diff_width;
                                buffer_count_resize[new_pos + 1] += buffer_count[pos]; // * diff_width;
                            }

                            if (new_height != (resize_factor_height * y1) + h)
                            {
                                // Siamo nel caso in cui il pixel va diviso su 4 pixel
                                buffer_resize[new_pos] += buffer[pos] * (1 - diff_height);
                                buffer_count_resize[new_pos] += buffer_count[pos];
                                if (new_width + 1 <= width)
                                {
                                    buffer_resize[new_pos + 1] += buffer[pos] * (1 - diff_height);
                                    buffer_count_resize[new_pos + 1] += buffer_count[pos];
                                }
                                if (new_pos + (int)width <= width * height)
                                {
                                    buffer_count_resize[new_pos + (int)width] += buffer_count[pos];
                                    buffer_resize[new_pos + (int)width] += buffer[pos] * (1 - diff_width) * diff_height;
                                    if (new_pos + (int)width + 1 <= width * height)
                                    {
                                        buffer_resize[new_pos + (int)width + 1] += buffer[pos] * diff_width * diff_height;
                                        buffer_count_resize[new_pos + (int)width + 1] += buffer_count[pos];
                                    }
                                }
                            }
                        }
                        else if (new_height != (resize_factor_height * y1) + h)
                        {
                            // Siamo divisi in verticale
                            buffer_resize[new_pos] += buffer[pos] * (1 - diff_height);
                            buffer_count_resize[new_pos] += buffer_count[pos];

                            if (new_pos + (int)width <= width * height)
                            {
                                buffer_resize[new_pos + (int)width] += buffer[pos] * diff_height;
                                buffer_count_resize[new_pos + (int)width] += buffer_count[pos];
                            }
                        }
                        else
                        {
                            buffer_resize[new_pos] += buffer[pos];
                            buffer_count_resize[new_pos] += buffer_count[pos];
                        }
                    }
                    else if (rw < 1 && rh >= 1)
                    {
                        if (resize_factor_width < 1 || resize_factor_height < 1)
                        {
                            buffer_resize[new_pos] += buffer[pos];
                            buffer_count_resize[new_pos] += buffer_count[pos];
                        }
                        else if (new_width != ((resize_factor_width * x1) + w))
                        {
                            if (diff_width <= rw)
                            {
                                // aggiunto caso altezza
                                if (new_height != (resize_factor_height * y1) + h)
                                {
                                    buffer_resize[new_pos] += buffer[pos] * rw * (1 - diff_height);
                                    buffer_count_resize[new_pos] += buffer_count[pos] * rw;
                                    if (new_pos + width <= width * height)
                                    {
                                        buffer_resize[new_pos + (int)width] += buffer[pos] * rw * diff_height;
                                        buffer_count_resize[new_pos + (int)width] += buffer_count[pos] * rw;
                                    }
                                }
                                else
                                {
                                    buffer_resize[new_pos] += buffer[pos] * rw;
                                    buffer_count_resize[new_pos] += buffer_count[pos] * rw;
                                }
                            }
                            else
                            {
                                buffer_resize[new_pos] += buffer[pos] * (1 - diff_width) * rw;
                                if (new_width + 1 <= width)
                                    buffer_resize[new_pos + 1] += buffer[pos] * diff_width * rw;

                                buffer_count_resize[new_pos] += buffer_count[pos];     // * 0.5;
                                buffer_count_resize[new_pos + 1] += buffer_count[pos]; // * (0.5 - rw);
                                // aggiunto caso altezza
                                if (new_height != (resize_factor_height * y1) + h)
                                {
                                    buffer_resize[new_pos] *= (1 - diff_height);
                                    if (new_width + 1 <= width)
                                        buffer_resize[new_pos + 1] *= (1 - diff_height);

                                    buffer_count_resize[new_pos + (int)width] += buffer_count[pos];
                                    buffer_resize[new_pos + (int)width] += buffer[pos] * (1 - diff_width) * diff_height * rw;
                                    if (new_pos + (int)width + 1 <= width * height)
                                    {
                                        buffer_resize[new_pos + (int)width + 1] += buffer[pos] * diff_width * diff_height * rw;
                                        buffer_count_resize[new_pos + (int)width + 1] += buffer_count[pos];
                                    }
                                }
                                
                            }
                        }
                        else
                        {
                            if (new_pos <= width * height)
                            {
                                buffer_resize[new_pos] += buffer[pos] * rw;
                                buffer_count_resize[new_pos] += buffer_count[pos]; // *rw;
                            }
                        }
                    }
                    else if (rh < 1 && rw >= 1)
                    {
                        if (resize_factor_width < 1 || resize_factor_height < 1)
                        {
                            buffer_resize[new_pos] += buffer[pos];
                            buffer_count_resize[new_pos] += buffer_count[pos];
                        }
                        else if (new_height != (resize_factor_height * y1) + h)
                        {
                            if (diff_height <= rh)
                            {
                                buffer_resize[new_pos] += buffer[pos] * rh;
                                buffer_count_resize[new_pos] += buffer_count[pos] * rh;
                                // Aggiunto caso larghezza
                                if (diff_width != 0)
                                {
                                    buffer_resize[new_pos] *= (1 - diff_width);
                                    if (new_width + 1 <= width)
                                    {
                                        buffer_resize[new_pos + 1] += buffer[pos] * diff_width * rh;
                                        buffer_count_resize[new_pos + 1] += buffer_count[pos]; // * diff_width;
                                    }
                                }
                            }
                            else
                            {
                                buffer_resize[new_pos] += buffer[pos] * (1 - diff_height) * rh;
                                buffer_count_resize[new_pos] += buffer_count[pos]; // * 0.5;
                                if (new_pos + (int)width <= width * height)
                                {
                                    buffer_resize[new_pos + (int)width] += buffer[pos] * diff_height * rh;
                                    buffer_count_resize[new_pos + (int)width] += buffer_count[pos]; // * (0.5 - rh);
                                }
                                // Aggiunto caso larghezza
                                if (diff_width != 0)
                                {
                                    buffer_resize[new_pos] *= (1 - diff_width);
                                    if (new_width + 1 <= width)
                                    {
                                        buffer_resize[new_pos + 1] += buffer[pos] * (1 - diff_height) * diff_width * rh;
                                        buffer_count_resize[new_pos] += buffer_count[pos]; // * 0.5;
                                    }

                                    if (new_pos + (int)width <= width * height)
                                        buffer_resize[new_pos + (int)width] *= diff_width;

                                    if (new_pos + width + 1 <= width * height)
                                    {
                                        buffer_resize[new_pos + (int)width + 1] += buffer[pos] * diff_width * (diff_height)*rh;
                                        buffer_count_resize[new_pos + (int)width + 1] += buffer_count[pos]; // * (0.5 - rh);
                                    }
                                }
                            }
                        }
                        else
                        {
                            if (new_pos <= width * height) // per evitare che vada oltre
                            {
                                buffer_resize[new_pos] += buffer[pos] * rh;
                                buffer_count_resize[new_pos] += buffer_count[pos];
                            }
                        }
                    }
                    else
                    {
                        if (resize_factor_width < 1 || resize_factor_height < 1)
                            buffer_resize[new_pos] += buffer[pos];
                        else
                        {
                            // Aggiunti casi altezza e larghezza
                            if (diff_width > rw)
                            {
                                buffer_resize[new_pos] += buffer[pos] * (1 - diff_width) * rw * rh;
                                buffer_count_resize[new_pos] += buffer_count[pos]; // * (1-diff_width);
                                if (new_width + 1 <= width)
                                {
                                    buffer_resize[new_pos + 1] += buffer[pos] * (diff_width) * rw * rh;
                                    buffer_count_resize[new_pos + 1] += buffer_count[pos]; // * diff_width;
                                }
                            }
                            if (diff_height > rh)
                            {
                                buffer_resize[new_pos] += buffer[pos] * (1 - diff_height) * rw * rh;
                                buffer_count_resize[new_pos] += buffer_count[pos];

                                if (new_pos + (int)width <= width * height)
                                {
                                    buffer_resize[new_pos + (int)width] += buffer[pos] * (rh-(1-diff_height)) * rw;
                                    buffer_count_resize[new_pos + (int)width] += buffer_count[pos];
                                }
                                if (diff_width > rw)
                                {
                                    if (new_width + 1 <= width)
                                        buffer_resize[new_pos + 1] *= (1 - diff_height);
                                    if (new_pos + (int)width <= width * height)
                                        buffer_resize[new_pos + (int)width] *= (1 - diff_width);
                                    if (new_pos + (int)width + 1 <= width * height)
                                        buffer_resize[new_pos + (int)width + 1] *= diff_width * (rh - (1-diff_height)) * rw;
                                }
                            }
                            if (diff_width <= rw && diff_height <= rh)
                            {
                                buffer_resize[new_pos] += buffer[pos] * rw * rh;
                                buffer_count_resize[new_pos] += buffer_count[pos];
                            }
                        }
                    }
                    prev = new_width;
                }
                */
                // cout << buffer_resize[new_pos] << "\t" << buffer_count_resize[new_pos] << "P" << buffer[pos] << "\t" << buffer_count[pos] << endl;
            }
        }
#if 0
        cout.precision(2);
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                pos = i * width + j;
                cout << buffer_resize[pos] << "\t";
            }
            cout << endl;
        }
        cout << endl
             << endl
             << endl;
#endif
    }

#if 1
    cout.precision(2);
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            pos = i * width + j;
            cout << buffer_resize[pos] << "\t";
        }
        cout << endl;
    }
    cout << endl
         << endl
         << endl;
#endif

#if 1
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            pos = i * width + j;
            cout << buffer_count_resize[pos] << "\t";
        }
        cout << endl;
    }
#endif

    free(buffer);
    free(buffer_count);

    cout << "Creazione immagine"
         << "\n";
    char tmp[MAX_STR + 1];

    output_file = "resize_" + output_file;

    strcpy(tmp, output_file.c_str());

    int result = writeImage(tmp, width, height, buffer_resize, buffer_count_resize, tmp);

    free(buffer_resize);
    free(buffer_count_resize);

    cout << "Fine"
         << "\n";
#else
    // Creo l'immagine
    cout << "Creazione immagine"
         << "\n";
    char tmp[MAX_STR + 1];
    strcpy(tmp, output_file.c_str());
    //int result = writeImage(tmp, MAX_WIDTH + 1, MAX_HEIGHT + 1 ,  buffer, buffer_count, tmp);
    int result = writeImage(tmp, width, height, buffer, buffer_count, tmp);

    // Free up the memorty used to store the image
    free(buffer);
    free(buffer_count);
    cout << "Fine"
         << "\n";
#endif

#if TIME
    auto end = chrono::steady_clock::now();
    cout << "Elapsed time in seconds: "
         << chrono::duration_cast<chrono::seconds>(end - start).count()
         << " sec";
#endif
    return 0;
}

inline void setRGB(png_byte *ptr, float val, float count)
{
    // da https://stackoverflow.com/questions/6394304/algorithm-how-do-i-fade-from-red-to-green-via-yellow-using-rgb-values
    float value;
    if (count != 0.0)
    {
        // if (count != 1)
        //     value = val / count;
        // else
        value = val;
        //cout << value << "\t";
        ptr[0] = (1 - value) * 255;
        ptr[1] = value * 255;
        ptr[2] = 0;

        //printf("%f\t%f\t%d %d %d\n", val, count, ptr[0], ptr[1], ptr[2]); // stampa valori di val + rgb
    }
    else
    {
        ptr[0] = ptr[1] = ptr[2] = 255;
    }

    //printf("%f\t%d %d %d\n", val, ptr[0], ptr[1], ptr[2]); // stampa valori di val + rgb
}

int writeImage(char *filename, int width, int height, float *buffer, float *buffer_count, char *title)
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
        finalise(fp, info_ptr, png_ptr);
    }

    // Initialize write structure
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL)
    {
        fprintf(stderr, "Could not allocate write struct\n");
        code = 1;
        finalise(fp, info_ptr, png_ptr);
    }

    // Initialize info structure
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL)
    {
        fprintf(stderr, "Could not allocate info struct\n");
        code = 1;
        finalise(fp, info_ptr, png_ptr);
    }

    // Setup Exception handling
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        fprintf(stderr, "Error during png creation\n");
        code = 1;
        finalise(fp, info_ptr, png_ptr);
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
        title_text.key = TITLE;
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
        for (x = 0; x < width; x++)
        {
#if PRINT
            if (buffer_count[y * width + x] != 0)
            {
                printf("%dx%d\t%f\t%d\n", x, y, buffer[y * width + x], buffer_count[y * width + x]);
            }
#endif

            setRGB(&(row[x * 3]), buffer[y * width + x], buffer_count[y * width + x]);
        }
        png_write_row(png_ptr, row);
    }

    // End write
    png_write_end(png_ptr, NULL);

    if (row != NULL)
        free(row);

    return code;
}

void finalise(FILE *fp, png_infop info_ptr, png_structp png_ptr)
{
    if (fp != NULL)
        fclose(fp);
    if (info_ptr != NULL)
        png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    if (png_ptr != NULL)
        png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    exit(-3);
}