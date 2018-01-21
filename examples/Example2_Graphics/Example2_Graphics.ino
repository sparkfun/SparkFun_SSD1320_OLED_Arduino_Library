
/*
  Control a SSD1320 based flexible OLED display
  By: Nathan Seidle
  SparkFun Electronics
  Date: November 21st, 2017
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  The flexible OLED is grayscale meaning we can display some neat looking images. To do this
  we need grayscale information. This example takes the raw bytes from a header file and sends
  them to the display. Uncomment either macaque.h or che.h and upload the program to see
  the image on the OLED.

  The header files were generated using a python script. See for details:
  https://github.com/sparkfun/BMPtoArray

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
#include "macaque.h" //Raw bytes of the Macaque monkey selfie
//#include "che.h" //Raw bytes of the comic che

//Initialize the display with the follow pin connections
SSD1320 flexibleOLED(10, 9); //10 = CS, 9 = RES

void setup()
{
  Serial.begin(115200);

  flexibleOLED.begin(160, 32); //Display is 160 wide, 32 high

  flexibleOLED.clearDisplay(); //Clear display and buffer

  flexibleOLED.setRowAddress(0);
  flexibleOLED.setColumnAddress(0);

  //Send the bytes from program memory to OLED display
  for (int i = 0 ; i < sizeof(myGraphic) ; i++)
  {
    byte theByte = pgm_read_byte(myGraphic + i);
    flexibleOLED.data(theByte); //Write byte directly to display
  }
}

void loop()
{

}
