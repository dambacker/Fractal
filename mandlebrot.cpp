#include <iostream>
#include <fstream>

using namespace std;

string colors[15] = {
	"\033[22;30m", //black[]    
	"\033[22;31m", //red[]      
	"\033[01;31m", //l_red[]    
	"\033[22;32m", //green[]    
	"\033[01;32m", //l_green[]  
	"\033[22;33m", //orange[]   
	"\033[01;33m", //yellow[]   
	"\033[22;34m", //blue[]     
	"\033[01;34m", //l_blue[]   
	"\033[22;35m", //magenta[]  
	"\033[01;35m", //l_magenta[]
	"\033[22;36m", //cyan[]     
	"\033[01;36m", //l_cyan[]   
	"\033[22;37m", //gray[]     
	"\033[01;37m", //white[]    
};

char black[]     = "\033[22;30m";
char red[]       = "\033[22;31m";
char l_red[]     = "\033[01;31m";
char green[]     = "\033[22;32m";
char l_green[]   = "\033[01;32m";
char orange[]    = "\033[22;33m";
char yellow[]    = "\033[01;33m";
char blue[]      = "\033[22;34m";
char l_blue[]    = "\033[01;34m";
char magenta[]   = "\033[22;35m";
char l_magenta[] = "\033[01;35m";
char cyan[]      = "\033[22;36m";
char l_cyan[]    = "\033[01;36m";
char gray[]      = "\033[22;37m";
char white[]     = "\033[01;37m";

double InterpolationPosition[6] = 
{   
    0.0,     
    0.16,    
    0.42,    
    0.6425,  
    0.8575,
    1.0
};

uint8_t InterpolationColor[6][3] = 
{
    {  0,   7, 100},
    { 32, 107, 203},
    {237, 255, 255},
    {255, 170,   0},
    {  0,   2,   0},
    {  0,   7, 100},
};

#define NUMCOLORS   256

uint8_t Color[NUMCOLORS][3];

void InitColors()
{
    for (int i=0; i<NUMCOLORS; i++)
    {
        int p = 0;
        double pos = ((double)i/(double)(NUMCOLORS-1));
        while (pos > InterpolationPosition[p+1])
        {
            p++;
            pos = ((double)i/(double)(NUMCOLORS-1));
        }
        double k = (pos - InterpolationPosition[p]) / (InterpolationPosition[p+1] - InterpolationPosition[p]);
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

    while (imageWidth != 0)
    {
        char* rgb = (char*)malloc(imageHeight*imageWidth*3);
        uint8_t* p = (uint8_t*)rgb;
        int iterations = 0;

        printf("\033[2J"); //clear screen, move to (0,0)
        printf("\033[0;0H");
        printf("fractal%04d\n", image);

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

                iterations = 0;
                while ((iterations++ < maxIterations) && (zrzr+zizi < 4.0))
                {
                    zrzr = zr*zr;
                    zizi = zi*zi;
                    zi = 2.0*zr*zi + ci;
                    zr = zrzr - zizi + cr;
                }

                if (iterations < maxIterations)
                {
                    *p++ = Color[iterations%NUMCOLORS][0];
                    *p++ = Color[iterations%NUMCOLORS][1];
                    *p++ = Color[iterations%NUMCOLORS][2];
                }
                else
                {
                    //black
                    *p++ = 0;
                    *p++ = 0;
                    *p++ = 0;
                }
            }

            int W = 32;
            if (y % (imageHeight/(W/2)) == 0)
            {
                for (int x=0; x<W; x++)
                {
                    int R = 0;
                    int G = 0;
                    int B = 0;
                    for (int wx=0; wx<W; wx++)
                    {
                        uint8_t r = rgb[(x*imageWidth/W+wx+y*imageWidth)*3 + 0];
                        uint8_t g = rgb[(x*imageWidth/W+wx+y*imageWidth)*3 + 1];
                        uint8_t b = rgb[(x*imageWidth/W+wx+y*imageWidth)*3 + 2];
                        R += r;
                        G += g;
                        B += b;
                    }
                    uint8_t r = (uint8_t)(R/W);
                    uint8_t g = (uint8_t)(G/W);
                    uint8_t b = (uint8_t)(B/W);
                    printf("\033[48;2;%d;%d;%dm ", r, g, b);
                }
                printf("\n");
            }
        }

        printf("\033[39m\033[49m");

        //save image to binary ppm file
        char filename[256];
        sprintf(filename, "fractal%04d.ppm", image++);
        ofstream ppmFile(filename, ios::out | ios::binary);
        ppmFile << "P6" << endl;
        ppmFile << imageWidth << " " << imageHeight << endl;
        ppmFile << "255" << endl;
        ppmFile.write(rgb, imageHeight*imageWidth*3);

        printf("Finished %s in %.3f\n", filename, (clock()-startImage)/CLOCKS_PER_SEC);

        inputFile >> imageWidth >> imageHeight >> maxIterations >> R >> I >> size;
    }

    printf("Total time for %d images: %.3f\n", image, (clock()-start)/CLOCKS_PER_SEC);

    return 0;
}
