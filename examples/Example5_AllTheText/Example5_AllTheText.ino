/*
  Control a SSD1320 based flexible OLED display
  By: Nathan Seidle
  SparkFun Electronics
  Date: November 21st, 2017
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

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
}

void loop()
{
  smallTextExample();
  largeTextExample();
}

void smallTextExample()
{
  printTitle("Small text", 0);

  flexibleOLED.setFontType(0); //Small text

  byte thisFontHeight = flexibleOLED.getFontHeight();

  flexibleOLED.clearDisplay(); //Clear display RAM and local display buffer
  flexibleOLED.setCursor(0, thisFontHeight * 3);
  flexibleOLED.print("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  flexibleOLED.setCursor(0, thisFontHeight * 2);
  flexibleOLED.print("abcdefghijklmnopqrstuvwxyz");
  flexibleOLED.setCursor(0, thisFontHeight * 1);
  flexibleOLED.print("1234567890!@#$%^&*(),.<>/?");
  flexibleOLED.setCursor(0, thisFontHeight * 0);
  flexibleOLED.print(";:'\"[]{}-=_+|\\~`");

  flexibleOLED.display();

  delay(2000);
}

void largeTextExample()
{
  printTitle("Large text", 0);

  flexibleOLED.setFontType(1); //Larger text
  byte theDisplayHeight = flexibleOLED.getDisplayHeight();
  byte thisFontHeight = flexibleOLED.getFontHeight();

  flexibleOLED.clearDisplay(); //Clear display RAM and local display buffer

  flexibleOLED.setCursor(0, theDisplayHeight - (thisFontHeight * 1));
  flexibleOLED.print("ABCDEFGHIJKLMNOPQ");
  flexibleOLED.setCursor(0, theDisplayHeight - (thisFontHeight * 2));
  flexibleOLED.print("abcdefghij1234567");

  flexibleOLED.display();

  delay(2000);
}

// Center and print a small title
// This function is quick and dirty. Only works for titles one line long.
void printTitle(String title, int font)
{
  int middleX = flexibleOLED.getDisplayWidth() / 2;
  int middleY = flexibleOLED.getDisplayHeight() / 2;

  flexibleOLED.clearDisplay();
  flexibleOLED.setFontType(font);

  // Set the cursor in the middle of the screen
  flexibleOLED.setCursor(middleX - (flexibleOLED.getFontWidth() * (title.length() / 2)),
                 middleY - (flexibleOLED.getFontHeight() / 2));

  // Print the title:
  flexibleOLED.print(title);
  flexibleOLED.display();

  delay(1500);

  flexibleOLED.clearDisplay(); //Clear everything
}

