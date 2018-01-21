/*
  Control a SSD1320 based flexible OLED display
  By: Nathan Seidle
  SparkFun Electronics
  Date: November 21st, 2017
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  Display lines, pixels, circles, and rectangles.

  To connect the display to an Arduino:
  (Arduino pin) = (Display pin)
  Pin 13 = SCLK on display carrier
  11 = SDIN
  10 = !CS
  9 = !RES

  The display is 160 pixels long and 32 pixels wide
  Each 4-bit nibble is the 4-bit grayscale for that pixel
  Therefore each byte of data written to the display paints two sequential pixels
  Loops that write to the display should be 80 iterations wide and 32 iterations tall
*/

#include <SSD1320_OLED.h>

//Initialize the display with the follow pin connections
SSD1320 flexibleOLED(10, 9); //10 = CS, 9 = RES

void setup()
{
  Serial.begin(115200);

  flexibleOLED.begin(160, 32); //Display is 160 wide, 32 high

  flexibleOLED.clearDisplay(); //Clear display RAM and local display buffer

  byte displayWidth = flexibleOLED.getDisplayWidth();
  byte displayHeight = flexibleOLED.getDisplayHeight();

  //Put pixels in the four corners
  flexibleOLED.setPixel(0, 0);
  flexibleOLED.setPixel(0, displayHeight - 1);
  flexibleOLED.setPixel(displayWidth - 1, 0);
  flexibleOLED.setPixel(displayWidth - 1, displayHeight - 1);

  //Shapes
  flexibleOLED.rect(2, 2, 5, 10); //X, Y, Width, Height
  flexibleOLED.circleFill(15, 10, 5); //X, Y, Radius

  flexibleOLED.rectFill(25, 2, 5, 10);
  flexibleOLED.circle(40, 10, 5);
  
  flexibleOLED.display();
}

void loop()
{

}



