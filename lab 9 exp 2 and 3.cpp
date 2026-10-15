#include <Arduino.h>
#include "eyebot.h"

#define COLOR_GREEN 0x00FF00

BYTE img[QQVGA_SIZE];
BYTE display_img[QQVGA_SIZE];

// Added for IPCol2HSI()
BYTE hue[QQVGA];

BYTE intensity[QQVGA];

int target_hue = 5;
int hue_tolerance = 10;
int min_intensity = 30;
int blob_threshold = 15;

void setup()
{
    eyebotStart();
    CAMInit(QQVGA);
    LCDClear();
    LCDSetPrintf(130, 0, "start");
}

void loop()
{
    CAMGet(img);

    // Convert entire RGB image to HSI
    IPCol2HSI(img, hue, display_img, intensity);

    int col_hist[WIDTH] = {};
    int row_hist[HEIGHT] = {};

    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            int p = 3 * (y * WIDTH + x); //r, g, b pixels

            // Read hue and intensity from HSI arrays
            int pixel = y * WIDTH + x;
            BYTE h = hue[pixel];
            BYTE i = intensity[pixel];

            int diff = abs((int)h - target_hue);

            if (diff > 128) //half the colour cirlce
            {
                diff = 256 - diff; // hue 0 to 255
            }

            if ((diff <= hue_tolerance) && (i >= min_intensity)) //check match
            {
                col_hist[x]++;
                row_hist[y]++;

                display_img[p] = 255;
                display_img[p + 1] = 0;
                display_img[p + 2] = 0; // (255, 0 ,0) bright red for matching pixels
            }
            else //doesnt match
            {
                display_img[p] = i;
                display_img[p + 1] = i;
                display_img[p + 2] = i; // (i, i ,i) is grey
            }
        }
    }

    LCDImage(display_img);

    int max_col_val = 0; // highest number of matching pixels in any column
    int max_x = -1; //x coord of column with max matching pixels

    for (int x = 0; x < WIDTH; x++) //check every column
    {
        if (col_hist[x] > max_col_val)
        {
            max_col_val = col_hist[x];
            max_x = x;
        }
    }

    int max_row_val = 0;
    int max_y = -1;

    for (int y = 0; y < HEIGHT; y++) //highest column y coord
    {
        if (row_hist[y] > max_row_val)
        {
            max_row_val = row_hist[y];
            max_y = y;
        }
    }

    if (max_col_val >= blob_threshold && max_row_val >= blob_threshold) // checks object big enough
    {
        LCDLine(max_x - 10, max_y, max_x + 10, max_y, COLOR_GREEN); //horizontal green line at pos
        LCDLine(max_x, max_y - 10, max_x, max_y + 10, COLOR_GREEN); //vert line at pos

        LCDSetPrintf(145, 0, "Blob at: (%i, %i) ", max_x, max_y);

     
        // EXP 3 ONLY - COMMENT OUT FOR EXP 2
      
        int error = max_x - WIDTH / 2;  // how far robot is from detecting image (from centre of camera)
        int turn_speed = error / 5; //error x Kp

        // Proportional controller
        VWSetSpeed(0, turn_speed);

        // END EXP 3 ONLY
    
    }
    else
    {
        LCDSetPrintf(145, 0, "No blob detected    ");

        // EXP 3 ONLY - COMMENT OUT FOR EXP 2
      

        VWSetSpeed(0, 5);


        // END EXP 3 ONLY

    }

    vTaskDelay(20 / portTICK_PERIOD_MS);
}
