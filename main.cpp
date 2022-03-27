#include <iostream>
#include <omp.h>
#include <malloc.h>
#include <functional>
#include <cmath>

#pragma ide diagnostic ignored "DanglingPointer"
#pragma clang diagnostic pop

#define NUM_THREADS 1
#define IS_TESTED false
#define WRITE_TO_DISK false
#define IMAGE_SIZE 54

using namespace std;

int radius;

// The size of the struct of is 53
// We need the pack here, because we want to match the structure of bmp files.
// https://stackoverflow.com/questions/3318410/pragma-pack-effect
# pragma pack(push, 2)
typedef struct {
    char sign;
    int size;
    int notused;
    int data;
    int headwidth;
    int width;
    int height;
    short numofplanes;
    short bitpix;
    int method;
    int arraywidth;
    int horizresol;
    int vertresol;
    int colnum;
    int basecolnum;
} img;
# pragma pop

unsigned char *openImg(const char *fileName, img *img) {
    FILE *file;
    // You should use "rb" if you're opening non-text files, because in this case, the translations are not appropriate.
    if (!(file = fopen(fileName, "rb"))) {
        cout << "The file " << fileName << " was not found!";
        // Free the memory of the image
        delete img;
        exit(1);
    }

    // Read everything in our struct
    fread(img, IMAGE_SIZE, 1, file);

    if (img->bitpix != 24) {
        cout << "The file " << fileName << " does not have 24 bit!";
        // Free the memory of the image
        delete img;
        exit(1);
    }

    // Read only the image-data in an array
    auto data = new unsigned char[img->arraywidth];
    fseek(file, img->data, SEEK_SET);
    fread(data, img->arraywidth, 1, file);
    fclose(file);

    return data;
}

void writeImage(unsigned char *imageData, img *outputImage, char *fileName) {
    FILE *file;

    string s = "result-";
    s += fileName;

    const char *newFileName = s.c_str();

    file = fopen(newFileName, "wb");
    fwrite(outputImage, IMAGE_SIZE, 1, file);
    fseek(file, outputImage->data, SEEK_SET);
    fwrite(imageData, outputImage->arraywidth, 1, file);
    fclose(file);
}

int clampIndex(int index, int min, int max) {
    return index < min ? min : index > max ? max : index;
}

int main(int argc, char *argv[]) {
    int threadNum = NUM_THREADS;

    char *fileName;
    unsigned char *imgData;
    img *bmp = new img[IMAGE_SIZE];

    if (argc > 3) {
        radius = atoi(argv[1]);
        fileName = argv[2];
        threadNum = atoi(argv[3]);
    }
    omp_set_num_threads(threadNum);

    imgData = openImg(fileName, bmp);

    int width = bmp->width;
    int height = bmp->height;
    int size = width * height;

    int rgbWidth = width * 3;

    // Create the red, green and blue layer
    auto *red = new unsigned char[size];
    auto *blue = new unsigned char[size];
    auto *green = new unsigned char[size];

    int pixel = 0;
    // loop every row
    // Set the position of every pixel color
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width * 3; j += 3, pixel++) {
            red[pixel] = imgData[i * rgbWidth + j];
            green[pixel] = imgData[i * rgbWidth + j + 1];
            blue[pixel] = imgData[i * rgbWidth + j + 2];
        }
    }

    double wtime = omp_get_wtime();

#pragma omp parallel for default(none) shared(height, width, radius, red, green, blue) collapse(2) schedule(guided)
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int row, col;
            double redSum = 0, greenSum = 0, blueSum = 0, weightSum = 0;

            for (row = i - radius; row <= i + radius; row++) {
                for (col = j - radius; col <= j + radius; col++) {
                    int x = clampIndex(col, 0, width - 1);
                    int y = clampIndex(row, 0, height - 1);

                    int tempPos = y * width + x;

                    double square = (col - j) * (col - j) + (row - i) * (row - i);
                    double sigma = radius * radius;
                    double weight = exp(-square / (2 * sigma)) / (M_PI * 2 * sigma);

                    redSum += red[tempPos] * weight;
                    greenSum += green[tempPos] * weight;
                    blueSum += blue[tempPos] * weight;
                    weightSum += weight;
                }
            }
            red[i * width + j] = round(redSum / weightSum);
            green[i * width + j] = round(greenSum / weightSum);
            blue[i * width + j] = round(blueSum / weightSum);
        }
    }

    wtime = omp_get_wtime() - wtime;

    if (IS_TESTED) {
        cout << wtime << endl;
    } else {
        cout << "Elapsed time: " << wtime << " seconds" << endl;
    }

    if (WRITE_TO_DISK) {
        pixel = 0;
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width * 3; j += 3, pixel++) {
                imgData[i * rgbWidth + j] = red[pixel];
                imgData[i * rgbWidth + j + 1] = green[pixel];
                imgData[i * rgbWidth + j + 2] = blue[pixel];
            }
        }

        // Write our new image to the disk
        writeImage(imgData, bmp, fileName);
    }

    delete[] red;
    delete[] green;
    delete[] blue;
    delete[] bmp;

    return 0;
}