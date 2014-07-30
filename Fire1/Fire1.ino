// Modified by Jason Coon for Smart Matrix https://github.com/pixelmatix/SmartMatrix
// from http://pastebin.com/xYEpxqgq
// 
// Fire2012 by Mark Kriegsman, July 2012
// as part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY
//
// This basic one-dimensional 'fire' simulation works roughly as follows:
// There's a underlying array of 'heat' cells, that model the temperature
// at each point along the line.  Every cycle through the simulation,
// four steps are performed:
//  1) All cells cool down a little bit, losing heat to the air
//  2) The heat from each cell drifts 'up' and diffuses a little
//  3) Sometimes randomly new 'sparks' of heat are added at the bottom
//  4) The heat from each cell is rendered as a color into the leds array
//     The heat-to-color mapping uses a black-body radiation approximation.
//
// Temperature is in arbitrary units from 0 (cold black) to 255 (white hot).
//
// This simulation scales it self a bit depending on NUM_LEDS; it should look
// "OK" on anywhere from 20 to 100 LEDs without too much tweaking.
//
// I recommend running this simulation at anywhere from 30-100 frames per second,
// meaning an interframe delay of about 10-35 milliseconds.
//
//
// There are two main parameters you can play with to control the look and
// feel of your fire: COOLING (used in step 1 above), and SPARKING (used
// in step 3 above).
// 
// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 55, suggested range 20-100
#define COOLING  55

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 120

#define FRAMES_PER_SECOND 60

#include "SmartMatrix.h"

#define HAS_IR_REMOTE 0

#if (HAS_IR_REMOTE == 1)

#include "IRremote.h"

#define IR_RECV_CS     18

// IR Raw Key Codes for SparkFun remote
#define IRCODE_HOME  0x10EFD827   

IRrecv irReceiver(IR_RECV_CS);

#endif

#define width 32
#define height 32

SmartMatrix matrix;

const int DEFAULT_BRIGHTNESS = 100;

const rgb24 COLOR_BLACK = { 0, 0, 0 };

bool isOff = false;

// Array of temperature readings at each simulation cell
byte heat[width][height];

rgb24 *leds;

enum Direction {
    Right = 0,
    Down = 1,
    Left = 2,
    Up = 3,
};

Direction direction = Up;

// rgb24 HeatColor( uint8_t temperature)
// [to be included in the forthcoming FastLED v2.1]
//
// Approximates a 'black body radiation' spectrum for
// a given 'heat' level.  This is useful for animations of 'fire'.
// Heat is specified as an arbitrary scale from 0 (cool) to 255 (hot).
// This is NOT a chromatically correct 'black body radiation'
// spectrum, but it's surprisingly close, and it's extremely fast and small.
//
// On AVR/Arduino, this typically takes around 70 bytes of program memory,
// versus 768 bytes for a full 256-entry RGB lookup table.

rgb24 HeatRgb24(uint8_t temperature)
{
    rgb24 heatcolor;

    // Scale 'heat' down from 0-255 to 0-191,
    // which can then be easily divided into three
    // equal 'thirds' of 64 units each.
    uint8_t t192 = scale8_video(temperature, 192);

    // calculate a value that ramps up from
    // zero to 255 in each 'third' of the scale.
    uint8_t heatramp = t192 & 0x3F; // 0..63
    heatramp <<= 2; // scale up to 0..252

    // now figure out which third of the spectrum we're in:
    if (t192 & 0x80) {
        // we're in the hottest third
        heatcolor.red = 255; // full red
        heatcolor.green = 255; // full green
        heatcolor.blue = heatramp; // ramp up blue

    }
    else if (t192 & 0x40) {
        // we're in the middle third
        heatcolor.red = 255; // full red
        heatcolor.green = heatramp; // ramp up green
        heatcolor.blue = 0; // no blue

    }
    else {
        // we're in the coolest third
        heatcolor.red = heatramp; // ramp up red
        heatcolor.green = 0; // no green
        heatcolor.blue = 0; // no blue
    }

    return heatcolor;
}

void setup()
{
    // Setup serial interface
    Serial.begin(9600);

    // Initialize 32x32 LED Matrix
    matrix.begin();
    matrix.setBrightness(DEFAULT_BRIGHTNESS);
    matrix.setColorCorrection(cc24);

#if (HAS_IR_REMOTE == 1)

    // Initialize IR receiver
    irReceiver.enableIRIn();

#endif

    // Clear screen
    matrix.fillScreen(COLOR_BLACK);
    matrix.swapBuffers();

    randomSeed(analogRead(5));
}

// finds the right index for our matrix
int XY(int x, int y) {
    if (y > height) { y = height; }
    if (y < 0) { y = 0; }
    if (x > width) { x = width; }
    if (x < 0) { x = 0; }

    return (y * width) + x;
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
    
    leds = matrix.backBuffer();

    // Loop for each column individually
    for (int x = 0; x < width; x++) {
        // Step 1.  Cool down every cell a little
        for (int i = 0; i < height; i++) {
            heat[x][i] = qsub8(heat[x][i], random(0, ((COOLING * 10) / height) + 2));
        }

        // Step 2.  Heat from each cell drifts 'up' and diffuses a little
        for (int k = height; k > 0; k--) {
            heat[x][k] = (heat[x][k - 1] + heat[x][k - 2] + heat[x][k - 2]) / 3;
        }

        // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
        if (random(255) < SPARKING) {
            int y = random(7);
            heat[x][y] = qadd8(heat[x][y], random(160, 255));
        }

        // Step 4.  Map from heat cells to LED colors
        for (int y = 0; y < height; y++) {
            rgb24 color;

            switch (direction) {
                case Right:
                    color = HeatRgb24(heat[y][x]);
                    break;

                case Down:
                    color = HeatRgb24(heat[x][y]);
                    break;

                case Left:
                    color = HeatRgb24(heat[y][(width - 1) - x]);
                    break;

                case Up:
                    color = HeatRgb24(heat[x][(height - 1) - y]);
                    break;
            }

            leds[XY(x, y)] = color;
        }
    }

    matrix.swapBuffers();

    delay(1000 / FRAMES_PER_SECOND);
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