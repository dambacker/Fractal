// 2023 - Dam Backer
//
// Basic implementation of Mandelbrot fractal
// Initial input.txt zooms into "Seahorse valley" using Ultra Fractal (-like) color scheme 
//
// references:
//  Seahorse Valley: https://www.mrob.com/pub/muency/seahorsevalley.html
//  Ultra Fractal Colors: https://stackoverflow.com/questions/16500656/which-color-gradient-is-used-to-color-mandelbrot-in-wikipedia 

#include <iostream>
#include <fstream>

using namespace std;

//color scheme from Ultra Fractal, based on bicubic interpolation of 5 color points (6 to make it connect)
double InterpolationPosition[6] = { 0.0, 0.16, 0.42, 0.642, 0.8, 1.0 };
uint8_t InterpolationColor[6][3] = { {  0,   7, 100}, { 32, 107, 203}, {237, 255, 255}, {255, 170,   0}, {  0,   2,   0}, {  0,   7, 100} };
//color palette to be used with fractal calculation iterations
#define NUMCOLORS   256
uint8_t Color[NUMCOLORS][3];

void InitColors()
{
    for (int i=0; i<NUMCOLORS; i++)
    {
        int p = 0;
        while (((double)i/(double)(NUMCOLORS-1)) > InterpolationPosition[p+1])
            p++;
        double k = (((double)i/(double)(NUMCOLORS-1)) - InterpolationPosition[p]) / (InterpolationPosition[p+1] - InterpolationPosition[p]);
        Color[i][0] = (1.0 - k)*InterpolationColor[p][0] + k*InterpolationColor[p+1][0];
        Color[i][1] = (1.0 - k)*InterpolationColor[p][1] + k*InterpolationColor[p+1][1];
        Color[i][2] = (1.0 - k)*InterpolationColor[p][2] + k*InterpolationColor[p+1][2];
    }
}

int main()
{
    InitColors();

    ifstream inputFile("input.txt");
    if (!inputFile)
    {
        printf("Could not open file!\n");
        return 1;
    }

    int imageWidth, imageHeight, maxIterations;
    double R, I, size;

    inputFile >> imageWidth >> imageHeight >> maxIterations >> R >> I >> size;

    int image = 0;

    double start = clock();

    unsigned int* iterations = (unsigned int*)malloc(imageHeight*imageWidth*sizeof(unsigned int));
    uint8_t* rgb = (uint8_t*)malloc(imageHeight*imageWidth*3);

    while (imageWidth != 0)
    {
        //reset color, clear sceen, position (0,0) + fractal image info
        printf("\033[39m\033[49m\033[2J\033[0;0H(%.1f) fractal%04d\n", (clock()-start)/CLOCKS_PER_SEC, image);

        double startImage = clock();

        for (int y = 0; y < imageHeight; y++)
        {
            for (int x = 0; x < imageWidth; x++)
            {
                double cr = R - size/2 + size * x / imageWidth;
                double ci = I - size/2 + size * y / imageHeight;

                double zr = 0;
                double zi = 0;

                double zrzr = 0;
                double zizi = 0;

                int iteration = 0;
                while ((iteration++ < maxIterations) && (zrzr+zizi < 4.0))
                {
                    zrzr = zr*zr;
                    zizi = zi*zi;
                    zi = 2.0*zr*zi + ci;
                    zr = zrzr - zizi + cr;
                }

                iterations[x+y*imageWidth] = iteration;
            }

            //colored text progress bar showing a scaled down version of the fractal
            unsigned int width  = 32;
            unsigned int height = 16;
            if (y % (imageHeight/height) == 0)
            {
                for (int x=0; x<width; x++)
                {
                    unsigned int r = 0;
                    unsigned int g = 0;
                    unsigned int b = 0;
                    for (int wx=0; wx<imageWidth/width; wx++)
                    {
                        r += (iterations[x*imageWidth/width+wx+y*imageWidth] < maxIterations) ? Color[iterations[x*imageWidth/width+wx+y*imageWidth]%NUMCOLORS][0] : 0;
                        g += (iterations[x*imageWidth/width+wx+y*imageWidth] < maxIterations) ? Color[iterations[x*imageWidth/width+wx+y*imageWidth]%NUMCOLORS][1] : 0;
                        b += (iterations[x*imageWidth/width+wx+y*imageWidth] < maxIterations) ? Color[iterations[x*imageWidth/width+wx+y*imageWidth]%NUMCOLORS][2] : 0;
                    }
                    printf("\033[48;2;%d;%d;%dm ", r/width, g/width, b/width);  //space with background color (r,g,b)
                }
                printf("\n");
            }
        }

        //turn iterations into rgb color image
        for (int y = 0; y < imageHeight; y++)
        {
            for (int x = 0; x < imageWidth; x++)
            {
                rgb[(x+y*imageWidth)*3+0] = (iterations[x+y*imageWidth] < maxIterations) ? Color[iterations[x+y*imageWidth]%NUMCOLORS][0] : 0;
                rgb[(x+y*imageWidth)*3+1] = (iterations[x+y*imageWidth] < maxIterations) ? Color[iterations[x+y*imageWidth]%NUMCOLORS][1] : 0;
                rgb[(x+y*imageWidth)*3+2] = (iterations[x+y*imageWidth] < maxIterations) ? Color[iterations[x+y*imageWidth]%NUMCOLORS][2] : 0;
            }
        }

        //save image to binary ppm file
        char filename[256];
        sprintf(filename, "fractal%04d.ppm", image++);
        ofstream ppmFile(filename, ios::out | ios::binary);
        ppmFile << "P6" << endl << imageWidth << " " << imageHeight << endl << "255" << endl;
        ppmFile.write((char*)rgb, imageHeight*imageWidth*3);
        ppmFile.close();

        //read next zoom detail
        inputFile >> imageWidth >> imageHeight >> maxIterations >> R >> I >> size;
    }

    printf("\033[39m\033[49mTotal time for %d images: %.3f\n", image, (clock()-start)/CLOCKS_PER_SEC); //reset color + total time

    free(rgb);
    free(iterations);

    return 0;
}
