
#include <Arduino.h>
#include "eyebot.h"

// Red crosshair colour
#define COLOUR_RED 0xFF0000

// RGB image: 160 x 120 x 3 bytes
BYTE img[QQVGA_SIZE];

// HSI images: 160 x 120 bytes each
BYTE hue[QQVGA];
BYTE saturation[QQVGA];
BYTE intensity[QQVGA];

void setup()
{
    // Initialise EyeBot hardware
    eyebotStart();

    // Initialise camera at 160 x 120 resolution
    CAMInit(QQVGA);

    // Clear LCD
    LCDClear();

    // Display startup message
    LCDSetPrintf(130, 0, "start");
}

void loop()
{
    // Capture an RGB camera image
    CAMGet(img);

    // Convert the complete RGB image to HSI
    IPCol2HSI(img, hue, saturation, intensity);

    // Display camera image
    LCDImage(img);

    // Draw horizontal red crosshair line
    LCDLine(70, 60, 90, 60, COLOUR_RED);

    // Draw vertical red crosshair line
    LCDLine(80, 50, 80, 70, COLOUR_RED);

    // Check whether a button was pressed
    if (KEYReadEdge())
    {
        // Sum of the nine centre pixel hues
        int sum_h = 0;

        // Process rows 59, 60 and 61
        for (int y = 59; y <= 61; y++)
        {
            // Process columns 79, 80 and 81
            for (int x = 79; x <= 81; x++)
            {
                // Convert (x, y) to a 1D array index
                // No multiplication by 3 is needed
                // because hue has one byte per pixel
                int p = y * 160 + x;

                // Add the pixel's hue to the sum
                sum_h += hue[p];
            }
        }

        // Calculate average hue of nine pixels
        int avg_hue = sum_h / 9;

        // Display the result
        LCDClear();
        LCDSetPrintf(130, 0, "Hue: %3d   ", avg_hue);
    }

    // Delay the current task for approximately 20 ms -  Free Real-Time Operating System ticks
    vTaskDelay(pdMS_TO_TICKS(20));
}
