// Simple Caleidoscope
// Modified by Jason Coon for Smart Matrix
// 
// Funky Clouds 2014
// Toys For Matrix Effects
// www.stefan-petrick.de/wordpress_beta
// http://pastebin.com/5fmCZs3B
// https://www.youtube.com/watch?v=09CfrdcuOvE

#include "SmartMatrix.h"
#include "FastLED.h"

#define HAS_IR_REMOTE 1

#if (HAS_IR_REMOTE == 1)

#include "IRremote.h"

#define IR_RECV_CS     18

// IR Raw Key Codes for SparkFun remote
#define IRCODE_HOME  0x10EFD827   

IRrecv irReceiver(IR_RECV_CS);

bool isOff = false;

#endif

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

rgb24 sin8Color = CHSV(160, 255, 255);
rgb24 quadwave8Color = CHSV(0, 255, 255); // red
rgb24 cubicwave8Color = CHSV(40, 255, 255); // yellow
rgb24 triwave8Color = CHSV(80, 255, 255); // green

void setup()
{
    // Initialize 32x32 LED Matrix
    matrix.begin();
    matrix.setBrightness(DEFAULT_BRIGHTNESS);
    matrix.setColorCorrection(cc24);

    // Clear screen
    matrix.fillScreen(COLOR_BLACK);
    matrix.swapBuffers();

#if (HAS_IR_REMOTE == 1)

    // Initialize IR receiver
    irReceiver.enableIRIn();

#endif
}

#if (HAS_IR_REMOTE == 1)

unsigned long handleInput() {
    unsigned long input = 0;

    decode_results results;

    results.value = 0;

    // Attempt to read an IR code ?
    if (irReceiver.decode(&results)) {
        input = results.value;

        // Prepare to receive the next IR code
        irReceiver.resume();
    }

    if (input == IRCODE_HOME) {
        isOff = !isOff;

        if (isOff){
            matrix.fillScreen(COLOR_BLACK);
            matrix.swapBuffers();
        }

        return input;
    }

    return input;
}

#endif

void loop()
{
#if (HAS_IR_REMOTE == 1)
    handleInput();

    if (isOff) {
        delay(100);
        return;
    }
#endif

    leds = matrix.backBuffer();

    count = count + 5;

    // first plant the seed into the buffer
    buffer[XY(map(sin8(count), 0, 255, 0, 31), map(cos8(count), 0, 255, 0, 31))] = sin8Color; // the circle  (blue)
    // lines following different wave fonctions
    buffer[XY(map(quadwave8(count), 0, 255, 0, 31), 10)] = quadwave8Color; // red
    buffer[XY(map(quadwave8(count), 0, 255, 0, 31), 11)] = quadwave8Color; // red
    buffer[XY(map(cubicwave8(count), 0, 255, 0, 31), 12)] = cubicwave8Color; // yellow
    buffer[XY(map(cubicwave8(count), 0, 255, 0, 31), 13)] = cubicwave8Color; // yellow
    buffer[XY(map(triwave8(count), 0, 255, 0, 31), 14)] = triwave8Color; // green
    buffer[XY(map(triwave8(count), 0, 255, 0, 31), 15)] = triwave8Color; // green

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

// translates from x, y into an index into the LED array
int XY(int x, int y) {
    if (y >= HEIGHT) { y = HEIGHT - 1; }
    if (y < 0) { y = 0; }
    if (x >= WIDTH) { x = WIDTH - 1; }
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
        buffer[i] = CRGB(0, 0, 0);
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