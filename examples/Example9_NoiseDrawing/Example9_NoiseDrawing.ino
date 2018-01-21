/*
  Control a SSD1320 based flexible OLED display
  By: Nathan Seidle
  SparkFun Electronics
  Date: November 21st, 2017
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  This example takes random noise and prints it to the screen.

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

//Get access to the screen buffer
byte *displayMemory = flexibleOLED.getScreenBuffer();

void setup()
{
  Serial.begin(115200);

  flexibleOLED.begin(160, 32); //Display is 160 wide, 32 high

  randomSeed(analogRead(A0) * analogRead(A1));
}

void loop()
{
  writeToBuffer();
  delay(2000);

  writeDirect();
  delay(2000);
}

//Write directly to the display
//Because we are writing directly we can do grayscale
//And we have 80 columns (not 20)
void writeDirect()
{
  flexibleOLED.clearDisplay(); //Clear display RAM and local display buffer

  for (int rows = 0 ; rows < 32 ; rows++)
  {
    for (int columns = 0 ; columns < 80 ; columns++)
    {
      byte noise = random(0xFF);
      flexibleOLED.data(noise);
    }
  }
}

//Write to the local buffer then push to display
//The display buffer is limited to on or off. It can't do gray scale.
void writeToBuffer()
{
  flexibleOLED.clearDisplay(); //Clear display RAM and local display buffer

  for (int rows = 0 ; rows < 32 ; rows++)
    for (int columns = 0 ; columns < 20 ; columns++)
    {
      byte noise = random(0xFF);
      displayMemory[rows * 20 + columns] = noise;
    }

  flexibleOLED.display(); //Push the buffer out to the display
}




