/*
  Control a SSD1320 based flexible OLED display
  By: Nathan Seidle
  SparkFun Electronics
  Date: November 21st, 2017
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  To connect the display to an Arduino:
  (Arduino pin) = (Display pin)
  Pin 13 = SCLK
  11 = SDIN
  10 = !CS
  8 = !RES

  The display is 160 pixels long and 32 pixels wide
  Each 4-bit nibble is the 4-bit grayscale for that pixel
  Therefore each byte of data written to the display paints two sequential pixels
  Loops that write to the display should be 80 iterations wide and 32 iterations tall
*/

#include "SSD1320_OLED.h"
#include "image.h"

SSD1320 oled(10, 9, 13, 11); //CS, RES, SCLK, MOSI(SDIN on OLED board)

void setup()
{
  Serial.begin(115200);

  oled.begin();

  oled.clearDisplay(); //Clear display and buffer

  oled.setRowAddress(0);
  oled.setColumnAddress(0);

  

  //We have 160 pixels. Each pixel is 4-bit so we have 80 bytes wide.
  //Display is 32 rows tall so 32 * 80 = 2,560 bytes to write.
  for (int i = 0 ; i < 2560 ; i++)
  {
    byte theByte = pgm_read_byte(LCD_graphic + 2559 - i);
    theByte ^= 0xFF; //We must invert the byte because the software converted it backwards
    oled.data(theByte); //Write byte directly to display
  }
}

void loop()
{

}





