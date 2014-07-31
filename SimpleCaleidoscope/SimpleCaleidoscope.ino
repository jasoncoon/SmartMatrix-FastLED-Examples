// Simple Caleidoscope
// Modified by Jason Coon for Smart Matrix
// 
// Funky Clouds 2014
// Toys For Matrix Effects
// www.stefan-petrick.de/wordpress_beta
// http://pastebin.com/5fmCZs3B
// https://www.youtube.com/watch?v=09CfrdcuOvE

#include "SmartMatrix.h"

SmartMatrix matrix;

const rgb24 COLOR_BLACK = { 0, 0, 0 };

// Matrix dimensions

const uint8_t WIDTH = 32;
const uint8_t HEIGHT = 32;

// LED stuff 
const int DEFAULT_BRIGHTNESS = 255;
#define NUM_LEDS (WIDTH * HEIGHT)
rgb24 *leds;
rgb24 buffer[NUM_LEDS];

byte count;

#define FRAMES_PER_SECOND 120

void setup()
{
    // Initialize 32x32 LED Matrix
    matrix.begin();
    matrix.setBrightness(DEFAULT_BRIGHTNESS);
    matrix.setColorCorrection(cc24);

    // Clear screen
    matrix.fillScreen(COLOR_BLACK);
    matrix.swapBuffers();
}

void loop()
{
    leds = matrix.backBuffer();

    count = count + 5;

    // first plant the seed into the buffer
    buffer[XY(map(sin8(count), 0, 255, 0, 31), map(cos8(count), 0, 255, 0, 31))] = CHSV(160, 255, 255); // the circle  
    buffer[XY(map(quadwave8(count), 0, 255, 0, 31), 8)] = CHSV(0, 255, 255); // lines following different wave fonctions
    buffer[XY(map(cubicwave8(count), 0, 255, 0, 31), 12)] = CHSV(40, 255, 255);
    buffer[XY(map(triwave8(count), 0, 255, 0, 31), 19)] = CHSV(80, 255, 255);

    // duplicate the seed in the buffer
    Caleidoscope4();

    // add buffer to leds
    ShowBuffer();

    // clear buffer
    ClearBuffer();

    // rotate leds
    Spiral(15, 15, 16, 110);

    matrix.swapBuffers();

    // do not delete the current leds, just fade them down for the tail effect
    //DimmAll(220);

    delay(1000 / FRAMES_PER_SECOND);
}

// finds the right index for our matrix
int XY(int x, int y) {
    if (y > HEIGHT) { y = HEIGHT; }
    if (y < 0) { y = 0; }
    if (x > WIDTH) { x = WIDTH; }
    if (x < 0) { x = 0; }

    return (y * WIDTH) + x;
}

// scale the brightness of the screenbuffer down
void DimmAll(byte value)
{
    for (int i = 0; i < NUM_LEDS; i++)
    {
        leds[i].nscale8(value);
    }
}

/*
Caleidoscope1 mirrors from source to A, B and C

y

|       |
|   B   |   C
|_______________
|       |
|source |   A
|_______________ x

*/
void Caleidoscope1() {
    for (int x = 0; x < WIDTH / 2; x++) {
        for (int y = 0; y < HEIGHT / 2; y++) {
            leds[XY(WIDTH - 1 - x, y)] = leds[XY(x, y)];              // copy to A
            leds[XY(x, HEIGHT - 1 - y)] = leds[XY(x, y)];             // copy to B
            leds[XY(WIDTH - 1 - x, HEIGHT - 1 - y)] = leds[XY(x, y)]; // copy to C

        }
    }
}

/*
Caleidoscope2 rotates from source to A, B and C

y

|       |
|   C   |   B
|_______________
|       |
|source |   A
|_______________ x

*/
void Caleidoscope2() {
    for (int x = 0; x < WIDTH / 2; x++) {
        for (int y = 0; y < HEIGHT / 2; y++) {
            leds[XY(WIDTH - 1 - x, y)] = leds[XY(y, x)];    // rotate to A
            leds[XY(WIDTH - 1 - x, HEIGHT - 1 - y)] = leds[XY(x, y)];    // rotate to B
            leds[XY(x, HEIGHT - 1 - y)] = leds[XY(y, x)];    // rotate to C
        }
    }
}

// adds the color of one quarter to the other 3
void Caleidoscope3() {
    for (int x = 0; x < WIDTH / 2; x++) {
        for (int y = 0; y < HEIGHT / 2; y++) {
            leds[XY(WIDTH - 1 - x, y)] += leds[XY(y, x)];    // rotate to A
            leds[XY(WIDTH - 1 - x, HEIGHT - 1 - y)] += leds[XY(x, y)];    // rotate to B
            leds[XY(x, HEIGHT - 1 - y)] += leds[XY(y, x)];    // rotate to C
        }
    }
}

// add the complete buffer 3 times while rotating
void Caleidoscope4() {
    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            buffer[XY(WIDTH - 1 - x, y)] += buffer[XY(y, x)];    // rotate to A
            buffer[XY(WIDTH - 1 - x, HEIGHT - 1 - y)] += buffer[XY(x, y)];    // rotate to B
            buffer[XY(x, HEIGHT - 1 - y)] += buffer[XY(y, x)];    // rotate to C
        }
    }
}

void ShowBuffer() {
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] += buffer[i];
    }
}

void ClearBuffer() {
    for (int i = 0; i < NUM_LEDS; i++) {
        buffer[i] = { 0, 0, 0 };
    }
}

void Spiral(int x, int y, int r, byte dimm) {
    for (int d = r; d >= 0; d--) {                // from the outside to the inside
        for (int i = x - d; i <= x + d; i++) {
            leds[XY(i, y - d)] += leds[XY(i + 1, y - d)];   // lowest row to the right
            leds[XY(i, y - d)].nscale8(dimm);
        }
        for (int i = y - d; i <= y + d; i++) {
            leds[XY(x + d, i)] += leds[XY(x + d, i + 1)];   // right colum up
            leds[XY(x + d, i)].nscale8(dimm);
        }
        for (int i = x + d; i >= x - d; i--) {
            leds[XY(i, y + d)] += leds[XY(i - 1, y + d)];   // upper row to the left
            leds[XY(i, y + d)].nscale8(dimm);
        }
        for (int i = y + d; i >= y - d; i--) {
            leds[XY(x - d, i)] += leds[XY(x - d, i - 1)];   // left colum down
            leds[XY(x - d, i)].nscale8(dimm);
        }
    }
}

// HSV to RGB color conversion
// Input arguments
// hue in degrees (0 - 360.0)
// saturation (0.0 - 1.0)
// value (0.0 - 1.0)
// Output arguments
// red, green blue (0.0 - 1.0)
void hsvToRGB(float hue, float saturation, float value, float * red, float * green, float * blue) {

    int i;
    float f, p, q, t;

    if (saturation == 0) {
        // achromatic (grey)
        *red = *green = *blue = value;
        return;
    }
    hue /= 60;                  // sector 0 to 5
    i = floor(hue);
    f = hue - i;                // factorial part of h
    p = value * (1 - saturation);
    q = value * (1 - saturation * f);
    t = value * (1 - saturation * (1 - f));
    switch (i) {
        case 0:
            *red = value;
            *green = t;
            *blue = p;
            break;
        case 1:
            *red = q;
            *green = value;
            *blue = p;
            break;
        case 2:
            *red = p;
            *green = value;
            *blue = t;
            break;
        case 3:
            *red = p;
            *green = q;
            *blue = value;
            break;
        case 4:
            *red = t;
            *green = p;
            *blue = value;
            break;
        default:
            *red = value;
            *green = p;
            *blue = q;
            break;
    }
}

#define MAX_COLOR_VALUE     255

// Create a HSV color
rgb24 createHSVColor(float hue, float saturation, float value) {

    float r, g, b;
    rgb24 color;

    hsvToRGB(hue, saturation, value, &r, &g, &b);

    color.red = r * MAX_COLOR_VALUE;
    color.green = g * MAX_COLOR_VALUE;
    color.blue = b * MAX_COLOR_VALUE;

    return color;
}

rgb24 CHSV(int _h, int _s, int _v) {
    int h = map(_h, 0, 255, 0, 360);
    float s = (float) map(_s, 0, 255, 0, 1000) / 1000.0;
    float v = (float) map(_v, 0, 255, 0, 1000) / 1000.0;

    return createHSVColor(h, s, v);
}