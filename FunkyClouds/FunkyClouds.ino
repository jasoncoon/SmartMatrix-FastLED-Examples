// Mini Funky Clouds
// Modified by Jason Coon for Smart Matrix:
// 
// Funky Clouds 2014
// Toys For Matrix Effects
// www.stefan-petrick.de/wordpress_beta
// https://gist.github.com/anonymous/68298debac462330719b
// https://www.youtube.com/watch?v=EC1MuRhXd1U

#include "SmartMatrix_32x32.h"
#include "FastLED.h"

#define HAS_IR_REMOTE 0

#if (HAS_IR_REMOTE == 1)

#include "IRremote.h"

#define IR_RECV_CS     18

// IR Raw Key Codes for SparkFun remote
#define IRCODE_HOME  0x10EFD827   

IRrecv irReceiver(IR_RECV_CS);

#endif

SmartMatrix matrix;

const int DEFAULT_BRIGHTNESS = 255;

const CRGB COLOR_BLACK = { 0, 0, 0 };

bool isOff = false;

#define FRAMES_PER_SECOND 60

// Matrix Size

const uint8_t WIDTH = 32;
const uint8_t HEIGHT = 32;

// LED Setup

#define NUM_LEDS (WIDTH * HEIGHT)

CRGB *leds;

// Oscillator Setup

struct timer {
    unsigned long tact;
    unsigned long lastMillis;
    unsigned long count;
    int delta;
    byte up;
    byte down;
};
timer multiTimer[7];
int timers = sizeof(multiTimer) / sizeof(multiTimer[0]);

void setup() {
    // Setup serial interface
    Serial.begin(9600);

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

    randomSeed(analogRead(5));

    // Triangular Oscillator Setup
    // range (up/down), speed (tact=ms between steps) 

    multiTimer[0].tact = 70;     //x1
    multiTimer[0].up = WIDTH - 1;
    multiTimer[0].down = 0;

    multiTimer[1].tact = 50;     //y1
    multiTimer[1].up = HEIGHT - 1;
    multiTimer[1].down = 0;

    multiTimer[2].tact = 3;      //color
    multiTimer[2].up = 255;
    multiTimer[2].down = 0;

    multiTimer[3].tact = 76;     //x2  
    multiTimer[3].up = WIDTH - 1;
    multiTimer[3].down = 0;

    multiTimer[4].tact = 99;     //y2
    multiTimer[4].up = HEIGHT - 1;
    multiTimer[4].down = 0;

    multiTimer[5].tact = 73;    //center spiral x
    multiTimer[5].up = HEIGHT - 4;
    multiTimer[5].down = 0;

    multiTimer[6].tact = 145;    //center spiral y
    multiTimer[6].up = HEIGHT - 4;
    multiTimer[6].down = 0;

    // set counting directions positive for the beginning
    // and start with random values to keep the start interesting

    for (int i = 0; i < timers; i++) {
        multiTimer[i].delta = 1;
        multiTimer[i].count = random(multiTimer[i].up);
    }
}

// counts everything with different speeds linear up and down
// = oscillators following a triangular function
void UpdateTimers()
{
    unsigned long now = millis();
    for (int i = 0; i < timers; i++)
    {
        while (now - multiTimer[i].lastMillis >= multiTimer[i].tact)
        {
            multiTimer[i].lastMillis += multiTimer[i].tact;
            multiTimer[i].count = multiTimer[i].count + multiTimer[i].delta;
            if ((multiTimer[i].count == multiTimer[i].up) || (multiTimer[i].count == multiTimer[i].down))
            {
                multiTimer[i].delta = -multiTimer[i].delta;
            }
        }
    }
}

// finds the right index for our matrix
int XY(int x, int y) {
    if (y > HEIGHT) { y = HEIGHT; }
    if (y < 0) { y = 0; }
    if (x > WIDTH) { x = WIDTH; }
    if (x < 0) { x = 0; }

    return (y * WIDTH) + x;
}

// fade the image buffer arround
// x, y: center   r: radius   dimm: fade down
void SpiralStream(int x, int y, int r, byte dimm) {
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

// give it a linear tail
void StreamVertical(byte scale)
{
    for (int x = 0; x < WIDTH; x++) {
        for (int y = 1; y < HEIGHT; y++) {
            leds[XY(x, y)] += leds[XY(x, y - 1)];
            leds[XY(x, y)].nscale8(scale);
        }
    }
    for (int x = 0; x < WIDTH; x++)
        leds[XY(x, 0)].nscale8(scale);
}

// give it a linear tail
void StreamHorizontal(byte scale)
{
    for (int x = 1; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            leds[XY(x, y)] += leds[XY(x - 1, y)];
            leds[XY(x, y)].nscale8(scale);
        }
    }
    for (int y = 0; y < HEIGHT; y++)
        leds[XY(0, y)].nscale8(scale);
}

// scale the brightness of the screenbuffer down
void DimmAll(byte value)
{
    for (int i = 0; i < NUM_LEDS; i++)
    {
        leds[i].nscale8(value);
    }
}

void loop()
{
#if (HAS_IR_REMOTE == 1)
    handleInput();

    if (isOff) {
        delay(100);
        return;
    }
#endif

    leds = (CRGB*) matrix.backBuffer();

    // let the oscillators swing
    UpdateTimers();

    // the "seed": 3 moving dots

    leds[XY(multiTimer[0].count, multiTimer[1].count)] =
        CHSV(multiTimer[2].count, 255, 255);

    leds[XY(multiTimer[3].count, multiTimer[4].count)] =
        CHSV(255 - multiTimer[2].count, 255, 255);

    // coordinates are the average of 2 oscillators
    leds[XY((
        multiTimer[0].count + multiTimer[1].count) / 2,
        (multiTimer[3].count + multiTimer[4].count) / 2
        )] = CHSV(multiTimer[2].count / 2, 255, 255);


    // the balance of the (last) values of the following functions affects the
    // appearence of the effect a lot

    // a moving spiral
    SpiralStream(multiTimer[5].count, multiTimer[6].count, 3, 210); // play here

    // y wind
    StreamVertical(120);    // and here

    // x wind
    StreamHorizontal(110);  // and here

    // main spiral
    SpiralStream(15, 15, 15, 150); // and here

    // increase the contrast
    DimmAll(250);

    // done.
    //FastLED.show();
    matrix.swapBuffers();

    // delay(1000 / FRAMES_PER_SECOND);
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